#include "telnetclient.h"
#include <unistd.h>

TelnetClient::TelnetClient(QObject *parent) :
    QObject(parent), sockfd( this)
{
    QObject::connect( &sockfd, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT( socketError(QAbstractSocket::SocketError)));
    QObject::connect( &sockfd, SIGNAL(readyRead()),
            this, SLOT( read()));
    QObject::connect( &sockfd, SIGNAL( connected()), this, SLOT( socketConnected()));
    QObject::connect( &sockfd, SIGNAL( disconnected()), this, SLOT( socketDisconnected()));
}

void TelnetClient::connect( QString host, QString port) {

    sockfd.abort();

    /*
     * As a result of calling connectToHost(), one of two things can happen:
     * 1. The connection is established. In this case QTcpSocket will emit
     * readyRead() every time it receives a block of data.
     * 2. An error occurs. In this case, QTcpSocket will emit error(),
     * and TelnetClient::socketError() will be called.
     */
    sockfd.connectToHost( host, port.toInt());
}

void TelnetClient::disconnect( QString host, QString port) {

    if( sockfd.state() != QAbstractSocket::ConnectedState) {
        socketError( QString( "Nothing to close. Host %1, port %2, socket not connected.")
                     .arg( host, port));
        return;
    }

    if ( peerName != sockfd.peerName() || peerPort != sockfd.peerPort()) {
        socketError( QString( "Disconnect from host %1 port %2 requested, but socket is"
                              " conencted to host %3 port %4.")
                     .arg( host, port, peerName, QString::number( peerPort)));
        return;
    }

    /* graceful attempt through CLOSE_WAIT */
    socketError( QString( "Closing connection to host %1, port %2...").arg( host, port));
    sockfd.disconnectFromHost();
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
                                     .arg( sockfd.errorString());
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
    sockfd.write( bytes);

    /* send greetings */
    char CR = 0x0d;
    char LF = 0x0a;
    sockfd.write( &CR, 1);
    sockfd.write( &LF, 1);

    emit msgSent( msg);
}


int TelnetClient::read()
{
    int b = sockfd.bytesAvailable();
    if ( !sockfd.bytesAvailable())
        return 0;

    QString msg;
    QByteArray bytes = sockfd.readAll();
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
        emit socketData( "Warning: Time interval list is empty, assuming 0.4 s...");

    QStringList tvalsList = tvals.split( QRegExp( "\n|\r\n|\r"), QString::SkipEmptyParts);

    if ( tvalsList.size() < msgList.size()) {
        emit socketData( "Warning: Time interval list is smaller than message list. "
                         "Assuming 0.4 s for missing values...");
        while( tvalsList.size() < msgList.size()) tvalsList.push_back( "400000");
    }

    if ( tvalsList.size() > msgList.size()) {
        emit socketData( "Warning: Time interval list is greater than message list. "
                         "Additional values will be omitted.");
    }

    /* send message list in separate thread */
    Worker *w = new Worker( this, msgList, tvalsList, sockfd.socketDescriptor());
    QObject::connect( w, &Worker::listHasBeenSent, this, &TelnetClient::listHasBeenSent);
    QObject::connect( w, &Worker::msgSent, this, &TelnetClient::msgSent);
    w->start();
}

void TelnetClient::listHasBeenSent()
{
    return;
}

void TelnetClient::socketConnected()
{
    peerName = sockfd.peerName();
    peerPort = sockfd.peerPort();
    emit connected();
}

void TelnetClient::socketDisconnected()
{
    emit disconnected();
}


void Worker::send( QString msg)
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
