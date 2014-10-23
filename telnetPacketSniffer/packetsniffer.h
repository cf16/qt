#ifndef PACKETSNIFFER_H
#define PACKETSNIFFER_H

#include <QtGlobal>
#include <QObject>
#include <QThread>

class QPlainTextEdit;
class PcapWorker;
typedef struct pcap pcap_t;

/* Ethernet2, DIX */
#define ETHERNET2_MAX_SIZE 1518

class PacketSniffer : public QObject
{
    Q_OBJECT
public:
    explicit PacketSniffer( QObject *parent = 0);

    int start_sniffing( QString filter);
    int stop_sniffing();

signals:
    void hexTextReady( QString text);
    void base64TextReady( QString text);
    void binaryTextReady( QString text);
    void asciiTextReady( QString text);
    void allTextReady( QString text);
    void discoveryTextReady( QString text);
    void pcapError( QString);

private:
    QPlainTextEdit *base64Output_;
    QPlainTextEdit *binaryOutput_;
    QPlainTextEdit *asciiOutput_;
    PcapWorker *worker_;
};


class PcapWorker : public QThread {
    Q_OBJECT
public:
    void run() Q_DECL_OVERRIDE;
    explicit PcapWorker( QObject *ptr = 0) : QThread( ptr) {}
    PcapWorker( QObject *ptr, QPlainTextEdit *base64Output,
            QPlainTextEdit *binaryOutput, QPlainTextEdit *asciiOutput, QString filter);
    static PcapWorker contact_;

signals:
    void hexTextReady( QString text);
    void base64TextReady( QString text);
    void binaryTextReady( QString text);
    void asciiTextReady( QString text);
    void allTextReady( QString text);
    void discoveryTextReady( QString text);
    void pcapError( QString);

private:
    void errAllWindows( QString text);
    pcap_t* open_pcap_socket( char* device, const char* bpfstr);
    static void parse_frame( u_char *user, const struct pcap_pkthdr *frame_header, const u_char *frame_ptr);
    static void printHex( const struct pcap_pkthdr *frame_header, const u_char *frame_ptr);
    static void printBase64( const struct pcap_pkthdr *frame_header, const u_char *frame_ptr);
    static void printBinary( const struct pcap_pkthdr *frame_header, const u_char *frame_ptr);
    static void printAscii( const struct pcap_pkthdr *frame_header, const u_char *frame_ptr);
    static void printFrameInfo( const struct pcap_pkthdr *frame_header, const u_char *frame_ptr);
    QPlainTextEdit *base64Output_;
    QPlainTextEdit *binaryOutput_;
    QPlainTextEdit *asciiOutput_;
    QString bp_filter_;
    int link_type_;
    static int link_header_len_;
    int packets_;
    static unsigned char frame_[ETHERNET2_MAX_SIZE];
    pcap_t *pd_; /* pcap descriptor */
};

#endif // PACKETSNIFFER_H
