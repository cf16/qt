/*
 * @brief Telnet client implementation
 *
 * with capability to send list of message with predefined
 * time intervals between them. Uses asynchronous socket
 * and handles sending messages in a separate thread
 *
 * @author Piotr Gregor peterg@sytel.com
 * @date   10 Aug 2014
 */

#ifndef TELNETCLIENT_H
#define TELNETCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QThread>

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
    void sendWebRequest( QString);
    int read();
    void sendMsgList( QString, QString);
    void listHasBeenSent();

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
    QString peerName;
    quint16 peerPort;
};

/*
 * Sends a message list in a separate thread.
 * Waits a specified ( or default) time interval
 * before sending next message.
*/
class Worker : public QThread {
    Q_OBJECT

public:
    void run() Q_DECL_OVERRIDE {

        sockfd_ = new QTcpSocket();
        sockfd_->setSocketDescriptor( fd_);

        for( int i = 0; i < msgList_.size(); i++) {
             send( msgList_[ i]);
             usleep( tvalsList_[ i].split( " ")[0].toInt());
        }

        emit listHasBeenSent();
    }

public:
    Worker( QObject *ptr, QStringList msgList, QStringList tvalsList, int fd) :
        QThread( ptr), msgList_( msgList), tvalsList_( tvalsList), fd_( fd)
    {}

signals:
    void listHasBeenSent();
    void msgSent( QString);

private:
    QStringList msgList_, tvalsList_;
    QTcpSocket *sockfd_;
    int fd_;
    void send( QString);
};

#endif // TELNETCLIENT_H
