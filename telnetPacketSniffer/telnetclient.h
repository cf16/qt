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

class Worker : public QThread {
    Q_OBJECT

public:
    void run() Q_DECL_OVERRIDE {

        sockfd_ = new QTcpSocket();
        bool b = sockfd_->setSocketDescriptor( fd_);
        //QObject::connect( sockfd_, SIGNAL( readyRead()),
        //        this, SLOT( read()));
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
public slots:
    //int read();

private:
    QStringList msgList_, tvalsList_;
    QTcpSocket *sockfd_;
    int fd_;
    void send( QString);
};

#endif // TELNETCLIENT_H
