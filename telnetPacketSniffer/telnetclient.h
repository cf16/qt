#ifndef TELNETCLIENT_H
#define TELNETCLIENT_H

#include <QObject>
#include <QTcpSocket>

/* handles tcp connection */
class TelnetClient : public QObject
{
    Q_OBJECT
public:
    explicit TelnetClient(QObject *parent = 0);

signals:

public slots:
    void connect( QString host, QString port);
    void disconnect( QString host, QString port);
    void socketError(QAbstractSocket::SocketError);
    void send( QString);
    int read();

private slots:
    void socketConnected();
    void socketDisconnected();

signals:
    void socketError( QString);
    void socketData( QString);
    void msgSent( QString);
    void connected();
    void disconnected();

private:
    QTcpSocket sockfd;

};

#endif // TELNETCLIENT_H
