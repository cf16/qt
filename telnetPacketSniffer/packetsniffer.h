#ifndef PACKETSNIFFER_H
#define PACKETSNIFFER_H

#include <QtGlobal>

class PacketSniffer
{
public:
    explicit PacketSniffer();

    int start_sniffing( quint16 port);
    int stop_sniffing();
};

#endif // PACKETSNIFFER_H
