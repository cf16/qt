#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow( QWidget *parent) :
    QMainWindow( parent),
    ui( new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->statusStateLabel->setStyleSheet( QString( "background-color: rgb(255, 0, 0);color: rgb(0, 0, 0);"));
    ui->hostLineEdit->setText( "10.0.0.90");
    hostText = "10.0.0.90";
    ui->portLineEdit->setText( "6502");
    portText = "6502";
    ui->msgLineEdit->setText( "TO\\NT4");

    telnetClientConnected_ = 0;

    QObject::connect( ui->hostLineEdit, SIGNAL( textChanged(QString)), this, SLOT( hostTextChanged(QString)));
    QObject::connect( ui->portLineEdit, &QLineEdit::textChanged, this, &MainWindow::portTextChanged);

    QObject::connect( ui->connectButton, &QAbstractButton::clicked, this, &MainWindow::connectBtnClicked);
    QObject::connect( this, &MainWindow::openTelnetConnection, &telnetClient, &TelnetClient::connect);

    QObject::connect( ui->disconnectButton, &QAbstractButton::clicked, this, &MainWindow::disconnectBtnClicked);
    QObject::connect( this, &MainWindow::closeTelnetConnection, &telnetClient, &TelnetClient::disconnect);

    QObject::connect( &telnetClient, SIGNAL( socketError(QString)), this, SLOT( displaySocketError(QString)));
    QObject::connect( &telnetClient, SIGNAL( socketData(QString)), this, SLOT( telnetData(QString)));

    QObject::connect( ui->sendMsgButton, SIGNAL( clicked()), this, SLOT( sendMsg()));
    QObject::connect( &telnetClient, SIGNAL( msgSent(QString)), this, SLOT( msgSent(QString)));

    QObject::connect( &telnetClient, SIGNAL( connected()), this, SLOT( telnetClientConnected()));
    QObject::connect( &telnetClient, SIGNAL( disconnected()), this, SLOT( telnetClientDisconnected()));

    QObject::connect( ui->sendAllMsgListButton, SIGNAL( clicked()), this, SLOT( sendMsgList()));
    QObject::connect( ui->sendNextMsgListButton, SIGNAL( clicked()), this, SLOT( sendNextMsg()));

    QObject::connect( ui->loadListMsgButton, SIGNAL( clicked()), this, SLOT( loadListMsg()));
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
            emit closeTelnetConnection( hostText, portText);
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

void MainWindow::sendMsg()
{
    QString msg = ui->msgLineEdit->text();
    if( msg.isEmpty())
        return;

    telnetClient.send( msg);
}

void MainWindow::sendMsgList()
{
    QString msgs = ui->listPlainTextEdit->toPlainText();
    if( msgs.isEmpty())
        return;

    QString tvals = ui->timeIntervalTextEdit->toPlainText();

    telnetClient.sendMsgList( msgs, tvals);
}


void MainWindow::sendNextMsg()
{
    static int idx = 0;
    QString msgs = ui->listPlainTextEdit->toPlainText();

    if( msgs.isEmpty())
        return;

    QStringList msgList = msgs.split( QRegExp( "\n|\r\n|\r"), QString::SkipEmptyParts);
    if( idx < msgList.size()) {
        telnetClient.send( msgList[ idx++]);
    } else {
        ui->msgsPlainTextEdit->appendPlainText( "\nWarning: End of message list. Next call to 'send next' will send "
                                                " the 1st message again.");
        idx = 0;
    }
}

void MainWindow::telnetClientConnected()
{
    telnetClientConnected_ = true;
    ui->statusStateLabel->setText( "connected");
    ui->msgsPlainTextEdit->appendPlainText( "Connected.");
    ui->statusStateLabel->setStyleSheet( QString( "background-color: rgb(85, 255, 127);"));
}

void MainWindow::telnetClientDisconnected()
{
    telnetClientConnected_ = false;
    ui->statusStateLabel->setText( "disconnected");
    ui->msgsPlainTextEdit->appendPlainText( "Disconnected.");
    ui->statusStateLabel->setStyleSheet( QString( "background-color: rgb(255, 0, 0);color: rgb(0, 0, 0);"));
}

void MainWindow::loadListMsg()
{
    QString fileName = QFileDialog::getOpenFileName( this, tr( "Open file"));
    QFile file( fileName);
    if ( !file.open( QFile::ReadOnly))
        return;

    QTextStream ts( &file);
    ui->listPlainTextEdit->appendPlainText( ts.readAll());
    file.close();
}

void MainWindow::closeEvent ( QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Simple question",
                                                                tr( "Are you sure?\n"),
                                                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::No);
    if ( resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
        if ( telnetClientConnected_) telnetClient.disconnect( hostText, portText);
        event->accept();
    }
}
