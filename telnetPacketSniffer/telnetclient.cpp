#include "telnetclient.h"

#ifndef Q_OS_WIN32
    #include <unistd.h>
#endif

TelnetClient::TelnetClient( QString defaultInterval, QObject *parent) :
    QObject(parent), sockfd_( this),
    defaultInterval_( defaultInterval)
{
    QObject::connect( &sockfd_, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT( socketError(QAbstractSocket::SocketError)));
    QObject::connect( &sockfd_, SIGNAL(readyRead()),
            this, SLOT( read()));
    QObject::connect( &sockfd_, SIGNAL( connected()), this, SLOT( socketConnected()));
    QObject::connect( &sockfd_, SIGNAL( disconnected()), this, SLOT( socketDisconnected()));

}

void TelnetClient::connect( QString host, QString port) {

    sockfd_.abort();

    /*
     * As a result of calling connectToHost(), one of two things can happen:
     * 1. The connection is established. In this case QTcpSocket will emit
     * readyRead() every time it receives a block of data.
     * 2. An error occurs. In this case, QTcpSocket will emit error(),
     * and TelnetClient::socketError() will be called.
     */
    sockfd_.connectToHost( host, port.toInt());
}

void TelnetClient::disconnect( QString host, QString port) {

    if( sockfd_.state() != QAbstractSocket::ConnectedState) {
        socketError( QString( "Nothing to close. Host %1, port %2, socket not connected.")
                     .arg( host, port));
        return;
    }

    if ( peerName_ != sockfd_.peerName() || peerPort_ != sockfd_.peerPort()) {
        socketError( QString( "Disconnect from host %1 port %2 requested, but socket is"
                              " conencted to host %3 port %4.")
                     .arg( host, port, peerName_, QString::number( peerPort_)));
        return;
    }

    /* graceful attempt through CLOSE_WAIT */
    socketError( QString( "Closing connection to host %1, port %2...").arg( host, port));
    sockfd_.disconnectFromHost();
}

void TelnetClient::socketError( QAbstractSocket::SocketError err)
{
    QString errorText;

        switch ( err) {
        case QAbstractSocket::RemoteHostClosedError:
            errorText = "The remote host closed connection. RST.";
            break;
        case QAbstractSocket::HostNotFoundError:
            errorText = "The host was not found. Please check the "
                                        "host name and port.";
            break;
        case QAbstractSocket::ConnectionRefusedError:
            errorText = "The connection was refused by the peer. "
                                        "Make sure the server is running, "
                                        "and check that the host name and port "
                                        "are correct.";
            break;
        default:
            errorText = QString( "The following error occurred: %1.")
                                     .arg( sockfd_.errorString());
        }

        emit socketError( errorText);
}

void TelnetClient::send( QString msg)
{
   /*
    * Internally, QTextStream uses a Unicode based buffer,
    * and QTextCodec is used by QTextStream to automatically
    * support different character sets
   */

    if ( msg.isEmpty())
        return;

    QByteArray bytes;
    bytes.append( msg);
    sockfd_.write( bytes);

    /* send greetings */
    char CR = 0x0d;
    char LF = 0x0a;
    sockfd_.write( &CR, 1);
    sockfd_.write( &LF, 1);

    emit msgSent( msg);
}

/* send msg as "GET msg HTTP/1.0\r\n\r\n"
 * e.g. send /select?path=Name+Spaces request as
 * "GET /select?path=Name+Spaces HTTP/1.0\r\n" */
void TelnetClient::sendWebRequest( QString msg)
{
   /*
    * Internally, QTextStream uses a Unicode based buffer,
    * and QTextCodec is used by QTextStream to automatically
    * support different character sets
   */

    if ( msg.isEmpty())
        return;
    msg = "GET " + msg + " HTTP/1.0";

    QByteArray bytes;
    bytes.append( msg);
    sockfd_.write( bytes);

    /* send greetings */
    char CR = 0x0d;
    char LF = 0x0a;
    sockfd_.write( &CR, 1);
    sockfd_.write( &LF, 1);
    sockfd_.write( &CR, 1);
    sockfd_.write( &LF, 1);

    emit msgSent( msg);
}

int TelnetClient::read()
{
    int b = sockfd_.bytesAvailable();
    if ( !sockfd_.bytesAvailable())
        return 0;

    QString msg;
    QByteArray bytes = sockfd_.readAll();
    msg = QString::fromUtf8( bytes.data(), b);

    emit socketData( msg);

    return msg.size();
}

void TelnetClient::sendMsgList( QString msgs, QString tvals)
{
    /*
     * split, empty lines ( even something like "\n\n"
     * or "\r\n\r\n" will not be skipped but we will
     * skip them because of QString::SkipEmptyParts)
     * We will add CR LF in send() method anyway
    */

    QStringList msgList = msgs.split( QRegExp( "\n|\r\n|\r"), QString::SkipEmptyParts);

    /* get time intervals */
    if ( tvals.isEmpty())
        emit socketData( "Warning: Time interval list is empty, assuming default...");

    QStringList tvalsList = tvals.split( QRegExp( "\n|\r\n|\r"), QString::SkipEmptyParts);

    if ( tvalsList.size() < msgList.size()) {
        emit socketData( "Warning: Time interval list is smaller than message list. "
                         "Assuming default for missing values...");
        while( tvalsList.size() < msgList.size()) tvalsList.push_back( defaultInterval_);
    }

    if ( tvalsList.size() > msgList.size()) {
        emit socketData( "Warning: Time interval list is greater than message list. "
                         "Additional values will be omitted.");
    }

    /* send message list in separate thread */
    msgListSender_ = new MessageListSender( this, msgList, tvalsList, sockfd_.socketDescriptor());
    QObject::connect( msgListSender_, &MessageListSender::listHasBeenSent, this, &TelnetClient::listHasBeenSent);
    QObject::connect( msgListSender_, &MessageListSender::msgSent, this, &TelnetClient::msgSent);
    QObject::connect ( this, SIGNAL( stopMsgList()), msgListSender_, SLOT( shutdown()));
    msgListSender_->start();
}

void TelnetClient::listHasBeenSent()
{
    return;
}

void TelnetClient::socketConnected()
{
    peerName_ = sockfd_.peerName();
    peerPort_ = sockfd_.peerPort();
    emit connected();
}

void TelnetClient::socketDisconnected()
{
    emit disconnected();
}


void MessageListSender::send( QString msg)
{
   /*
    * Internally, QTextStream uses a Unicode based buffer,
    * and QTextCodec is used by QTextStream to automatically
    * support different character sets
   */

    if ( msg.isEmpty())
        return;

    QByteArray bytes;
    bytes.append( msg);
    sockfd_->write( bytes);

    /* send greetings */
    char CR = 0x0d;
    char LF = 0x0a;
    sockfd_->write( &CR, 1);
    sockfd_->write( &LF, 1);
    sockfd_->flush();
    emit msgSent( msg);
}
