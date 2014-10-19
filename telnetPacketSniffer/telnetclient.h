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
#include <QStringList>

class MessageListSender;
class PacketSniffer;

/* handles tcp connection */
class TelnetClient : public QObject
{
    Q_OBJECT
public:
    explicit TelnetClient( QString defaultInterval = "400000",
                                        QObject *parent = 0);

signals:
    void stopMsgList();

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
    void setDefaultInterval( QString i)
    {
        defaultInterval_ = i;
    }

signals:
    void socketError( QString);
    void socketData( QString);
    void msgSent( QString);
    void connected();
    void disconnected();

private:
    QTcpSocket sockfd_;
    QString peerName_;
    quint16 peerPort_;
    QString defaultInterval_;

    MessageListSender *msgListSender_;
};

/*
 * Sends a message list in a separate thread.
 * Waits a specified ( or default) time interval
 * before sending next message.
*/
class MessageListSender : public QThread {
    Q_OBJECT

public:
    void run() Q_DECL_OVERRIDE {

        sockfd_ = new QTcpSocket();
        sockfd_->setSocketDescriptor( fd_);

        for( int i = 0; i < msgList_.size(); i++) {
             send( msgList_[ i]);
             usleep( tvalsList_[ i].split( " ")[0].toInt());
             if ( isShutdownRequested_)
             {
                 emit listHasBeenSentIncomplete();
                 return;
             }
        }

        emit listHasBeenSent();
    }

public:
    MessageListSender( QObject *ptr, QStringList msgList, QStringList tvalsList, int fd) :
        QThread( ptr), msgList_( msgList), tvalsList_( tvalsList), fd_( fd),
        isShutdownRequested_( 0)
    {}

signals:
    void listHasBeenSent();
    void listHasBeenSentIncomplete();
    void msgSent( QString);

public slots:
    void shutdown() { isShutdownRequested_ = true; }

private:
    QStringList msgList_, tvalsList_;
    QTcpSocket *sockfd_;
    int fd_;
    void send( QString);
    bool isShutdownRequested_;
};

#endif // TELNETCLIENT_H
