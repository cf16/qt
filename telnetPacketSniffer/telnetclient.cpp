#include "telnetclient.h"

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

    /* graceful attempt through CLOZE_WAIT */
    sockfd.disconnectFromHost();
}

void TelnetClient::socketError(QAbstractSocket::SocketError err)
{
    QString errorText;

        switch ( err) {
        case QAbstractSocket::RemoteHostClosedError:
            //break;
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
    //QTextStream in( &sockfd);
    //in.setVersion( QDataStream::Qt_5_3);
    int b = sockfd.bytesAvailable();
    if ( !sockfd.bytesAvailable())
        return 0;

    QString msg;
    QByteArray bytes = sockfd.readAll();
    msg = QString::fromUtf8( bytes.data(), b);

    emit socketData( msg);

    return msg.size();
}

void TelnetClient::socketConnected()
{
    emit connected();
}

void TelnetClient::socketDisconnected()
{
    emit disconnected();
}
