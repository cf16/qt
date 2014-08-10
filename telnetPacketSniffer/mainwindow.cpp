#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow( QWidget *parent) :
    QMainWindow( parent),
    ui( new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusStateLabel->setStyleSheet( QString( "background-color: rgb(255, 0, 0);"));
    ui->hostLineEdit->setText( "10.0.0.90");
    hostText = "10.0.0.90";
    ui->portLineEdit->setText( "6502");
    portText = "6502";
    ui->msgLineEdit->setText( "TO\\NT4");

    QObject::connect( ui->hostLineEdit, SIGNAL( textChanged(QString)), this, SLOT( hostTextChanged(QString)));
    QObject::connect( ui->portLineEdit, &QLineEdit::textChanged, this, &MainWindow::portTextChanged);

    QObject::connect( ui->connectButton, &QAbstractButton::clicked, this, &MainWindow::connectBtnClicked);
    QObject::connect( this, &MainWindow::openTelnetConnection, &telnetClient, &TelnetClient::connect);

    QObject::connect( ui->disconnectButton, &QAbstractButton::clicked, this, &MainWindow::disconnectBtnClicked);
    QObject::connect( this, &MainWindow::closeTelnetConnection, &telnetClient, &TelnetClient::disconnect);

    QObject::connect( &telnetClient, SIGNAL( socketError(QString)), this, SLOT( displaySocketError(QString)));
    QObject::connect( &telnetClient, SIGNAL( socketData(QString)), this, SLOT( telnetData(QString)));

    QObject::connect( ui->sendMsgButton, SIGNAL( clicked()), this, SLOT( sendMsgBtnClicked()));
    QObject::connect( &telnetClient, SIGNAL( msgSent(QString)), this, SLOT( msgSent(QString)));

    QObject::connect( &telnetClient, SIGNAL( connected()), this, SLOT( telnetClientConnected()));
    QObject::connect( &telnetClient, SIGNAL( disconnected()), this, SLOT( telnetClientDisconnected()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectBtnClicked()
{
    ui->msgsPlainTextEdit->appendPlainText( QString( "Connecting to host %1, port %2...").arg( hostText, portText));
            emit openTelnetConnection( hostText, portText);
}

void MainWindow::disconnectBtnClicked()
{
    ui->msgsPlainTextEdit->appendPlainText( QString( "Closing connection to host %1, port %2...").arg( hostText, portText));
            emit closeTelnetConnection( ui->hostLabel->text(), ui->portLabel->text());
}

void MainWindow::hostTextChanged( QString newHostText)
{
    hostText = newHostText;
}

void MainWindow::portTextChanged( QString newPortText)
{
    portText = newPortText;
}

void MainWindow::displaySocketError( QString text)
{
    ui->msgsPlainTextEdit->appendPlainText( text);
}

void MainWindow::telnetData( QString text)
{
    ui->msgsPlainTextEdit->appendPlainText( QString("<-").append( text));
}

void MainWindow::msgSent( QString text)
{
    ui->msgsPlainTextEdit->appendPlainText( QString("->").append( text));
}

void MainWindow::sendMsgBtnClicked()
{
    QString msg = ui->msgLineEdit->text();
    if( msg.isEmpty())
        return;
    telnetClient.send( msg);
}

void MainWindow::telnetClientConnected()
{
    ui->statusStateLabel->setText( "connected");
    ui->msgsPlainTextEdit->appendPlainText( "Connected.");
    ui->statusStateLabel->setStyleSheet( QString( "background-color: rgb(85, 255, 127);"));
}

void MainWindow::telnetClientDisconnected()
{
    ui->statusStateLabel->setText( "disconnected");
    ui->msgsPlainTextEdit->appendPlainText( "Disconnected.");
    ui->statusStateLabel->setStyleSheet( QString( "background-color: rgb(255, 0, 0);"));
}
