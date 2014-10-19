#include "packetsniffer.h"

#include <pcap.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>

#include <QPlainTextEdit>

int PcapWorker::link_header_len_;
PcapWorker PcapWorker::contact_;

PacketSniffer::PacketSniffer(QObject *parent) :
    QObject( parent)
{
    /* Connect signals to slots. */
    QObject::connect ( &PcapWorker::contact_, SIGNAL( base64TextReady(QString)), this, SIGNAL( base64TextReady(QString)), Qt::QueuedConnection);
    QObject::connect ( &PcapWorker::contact_, SIGNAL( binaryTextReady(QString)), this, SIGNAL( binaryTextReady(QString)), Qt::QueuedConnection);
    QObject::connect ( &PcapWorker::contact_, SIGNAL( asciiTextReady(QString)), this, SIGNAL( asciiTextReady(QString)), Qt::QueuedConnection);
    QObject::connect ( &PcapWorker::contact_, SIGNAL( pcapError(QString)), this, SIGNAL( pcapError(QString)), Qt::QueuedConnection);
    QObject::connect ( &PcapWorker::contact_, SIGNAL( allTextReady(QString)), this, SIGNAL( allTextReady(QString)), Qt::QueuedConnection);
}

int PacketSniffer::start_sniffing( quint16 port, QPlainTextEdit *base64Output,
                                   QPlainTextEdit *binaryOutput,
                                   QPlainTextEdit *asciiOutput, QString filter)
{
    worker_ = new PcapWorker( this, base64Output_, binaryOutput_, asciiOutput_, filter);

    /* Connect signals to slots. */
    QObject::connect ( worker_, SIGNAL( base64TextReady(QString)), this, SIGNAL( base64TextReady(QString)), Qt::QueuedConnection);
    QObject::connect ( worker_, SIGNAL( binaryTextReady(QString)), this, SIGNAL( binaryTextReady(QString)), Qt::QueuedConnection);
    QObject::connect ( worker_, SIGNAL( asciiTextReady(QString)), this, SIGNAL( asciiTextReady(QString)), Qt::QueuedConnection);
    QObject::connect ( worker_, SIGNAL( pcapError(QString)), this, SIGNAL( pcapError(QString)), Qt::QueuedConnection);
    QObject::connect ( worker_, SIGNAL( allTextReady(QString)), this, SIGNAL( allTextReady(QString)), Qt::QueuedConnection);

    /* start run() method */
    worker_->start();

    return 0;
}

int PacketSniffer::stop_sniffing()
{
    return 0;
}


PcapWorker::PcapWorker( QObject *ptr, QPlainTextEdit *base64Output,
                QPlainTextEdit *binaryOutput, QPlainTextEdit *asciiOutput, QString filter) :
    QThread( ptr),
    base64Output_( base64Output),
    binaryOutput_( binaryOutput),
    asciiOutput_( asciiOutput),
    bp_filter_( filter),            /* berkley pcap filter          */
    packets_( 0)                    /* infinite number of packets   */
{
}

void PcapWorker::errAllWindows( QString text)
{
    emit pcapError( text);
    emit base64TextReady( text);
    emit binaryTextReady( text);
    emit asciiTextReady( text);
}

void PcapWorker::run() Q_DECL_OVERRIDE
{
    char bp_filter[256] = "";

    std::string stdf = bp_filter_.toStdString();
    strncpy( bp_filter, stdf.c_str(), 256);
    bp_filter[255] = '\0';
    char device[256] = "";

    pcap_t *pd = open_pcap_socket( device, bp_filter);

    char err[PCAP_ERRBUF_SIZE+40];

    // Determine the datalink layer type.
    if ( ( link_type_ = pcap_datalink(pd)) < 0)
    {
        sprintf( err, "pcap_datalink(): %s\n", pcap_geterr(pd));
        emit errAllWindows( err);
        return;
    }

    /* Data link layer header size. */
    switch (link_type_)
    {
    case DLT_NULL:
        link_header_len_ = 4;
        break;

    case DLT_EN10MB:
        link_header_len_ = 14;
        break;

    case DLT_SLIP:
    case DLT_PPP:
        link_header_len_ = 24;
        break;

    default:
        sprintf( err, "Unsupported datalink (%d)\n", link_type_);
        return;
    }

    /* Start capturing packets. */
    if ( pcap_loop( pd, packets_, parse_frame, 0) < 0) {
        sprintf( err, "pcap_loop failed: %s\n", pcap_geterr(pd));
        emit errAllWindows( err);
    }

    emit errAllWindows( QString( "Started to sniff raw packets.\nFilter: %1\n").arg(bp_filter_));
}


pcap_t* PcapWorker::open_pcap_socket( char *device, const char *bpfstr)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    char err[PCAP_ERRBUF_SIZE+40];
    pcap_t* pd;
    quint32  srcip, netmask;
    struct bpf_program  bpf;

    // If no network interface (device) is specfied, get the first one.
    if ( !*device && !(device = pcap_lookupdev(errbuf)))
    {
        sprintf( err, "pcap_lookupdev(): %s\n", errbuf);
        emit errAllWindows( err);
        return 0;
    }

    // Open the device for live capture, as opposed to reading a packet
    // capture file.
    if ( ( pd = pcap_open_live( device, BUFSIZ, 1, 0, errbuf)) == NULL)
    {
        sprintf( err, "pcap_open_live(): %s\n", errbuf);
        emit errAllWindows( err);
        return 0;
    }

    // Get network device source IP address and netmask.
    if ( pcap_lookupnet(device, &srcip, &netmask, errbuf) < 0)
    {
        sprintf( err, "pcap_lookupnet: %s\n", errbuf);
        emit errAllWindows( err);
        return 0;
    }

    // Convert the packet filter epxression into a packet
    // filter binary.
    if ( pcap_compile(pd, &bpf, (char*) bpfstr, 0, netmask))
    {
        sprintf( err, "pcap_compile(): %s\n", pcap_geterr(pd));
        emit errAllWindows( err);
        return 0;
    }

    // Assign the packet filter to the given libpcap socket.
    if ( pcap_setfilter( pd, &bpf) < 0)
    {
        sprintf( err, "pcap_setfilter(): %s\n", pcap_geterr(pd));
        emit errAllWindows( err);
        return 0;
    }

    return pd;
}

void PcapWorker::parse_frame( u_char *user, const pcap_pkthdr *packethdr, const u_char *packetptr)
{
    struct ip* iphdr;
    struct icmphdr* icmphdr;
    struct tcphdr* tcphdr;
    struct udphdr* udphdr;
    char iphdrInfo[256], srcip[256], dstip[256];
    unsigned short id, seq;

    /* Skip the datalink layer header to IP header fields. */
    packetptr += link_header_len_;
    iphdr = (struct ip*)packetptr;
    strcpy(srcip, inet_ntoa(iphdr->ip_src));
    strcpy(dstip, inet_ntoa(iphdr->ip_dst));
    sprintf(iphdrInfo, "ID:%d TOS:0x%x, TTL:%d IpLen:%d DgLen:%d",
            ntohs(iphdr->ip_id), iphdr->ip_tos, iphdr->ip_ttl,
            4*iphdr->ip_hl, ntohs(iphdr->ip_len));

    /* Advance to the transport layer header */
    char err[PCAP_ERRBUF_SIZE+40];
    packetptr += 4*iphdr->ip_hl;
    switch (iphdr->ip_p)
    {
    case IPPROTO_TCP:
        tcphdr = (struct tcphdr*)packetptr;
        sprintf( err, "TCP  %s:%d -> %s:%d\n", srcip, ntohs(tcphdr->source),
               dstip, ntohs(tcphdr->dest));
        emit contact_.allTextReady( err);
        sprintf( err, "%s\n", iphdrInfo);
        emit contact_.allTextReady( err);
        sprintf( err,"%c%c%c%c%c%c Seq: 0x%x Ack: 0x%x Win: 0x%x TcpLen: %d\n",
               (tcphdr->urg ? 'U' : '*'),
               (tcphdr->ack ? 'A' : '*'),
               (tcphdr->psh ? 'P' : '*'),
               (tcphdr->rst ? 'R' : '*'),
               (tcphdr->syn ? 'S' : '*'),
               (tcphdr->fin ? 'F' : '*'),
               ntohl(tcphdr->seq), ntohl(tcphdr->ack_seq),
               ntohs(tcphdr->window), 4*tcphdr->doff);
        emit contact_.allTextReady( err);
        break;

    case IPPROTO_UDP:
        udphdr = (struct udphdr*)packetptr;
        sprintf( err, "UDP  %s:%d -> %s:%d\n", srcip, ntohs(udphdr->source),
               dstip, ntohs(udphdr->dest));
        emit contact_.allTextReady( err);
        sprintf( err, "%s\n", iphdrInfo);
        emit contact_.allTextReady( err);
        break;

    case IPPROTO_ICMP:
        icmphdr = (struct icmphdr*)packetptr;
        sprintf( err, "ICMP %s -> %s\n", srcip, dstip);
        emit contact_.allTextReady( err);
        sprintf( err, "%s\n", iphdrInfo);
        emit contact_.allTextReady( err);
        memcpy(&id, (u_char*)icmphdr+4, 2);
        memcpy(&seq, (u_char*)icmphdr+6, 2);
        sprintf( err, "Type:%d Code:%d ID:%d Seq:%d\n", icmphdr->type, icmphdr->code,
               ntohs(id), ntohs(seq));
        emit contact_.allTextReady( err);
        break;
    }
    sprintf( err, "%s", "+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n\n");
    emit contact_.allTextReady( err);
}
