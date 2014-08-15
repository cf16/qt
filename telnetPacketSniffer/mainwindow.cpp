#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextBlock>

MainWindow::MainWindow( QWidget *parent) :
    QMainWindow( parent),
    ui( new Ui::MainWindow),
    currMsgIdx_( 0)
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
    QObject::connect( this, &MainWindow::openTelnetConnection, &telnetClient_, &TelnetClient::connect);

    QObject::connect( ui->disconnectButton, &QAbstractButton::clicked, this, &MainWindow::disconnectBtnClicked);
    QObject::connect( this, &MainWindow::closeTelnetConnection, &telnetClient_, &TelnetClient::disconnect);

    QObject::connect( &telnetClient_, SIGNAL( socketError(QString)), this, SLOT( displaySocketError(QString)));
    QObject::connect( &telnetClient_, SIGNAL( socketData(QString)), this, SLOT( telnetData(QString)));

    QObject::connect( ui->sendMsgButton, SIGNAL( clicked()), this, SLOT( sendMsg()));
    QObject::connect( ui->sendStashedBtn, SIGNAL( clicked()), this, SLOT( sendStashedMsg()));
    QObject::connect( ui->stashRevertBtn, SIGNAL( clicked()), this, SLOT( stashPopRevert()));
    QObject::connect( ui->popRevertBtn, SIGNAL( clicked()), this, SLOT( stashPopRevert()));

    QObject::connect( &telnetClient_, SIGNAL( msgSent(QString)), this, SLOT( msgSent(QString)));

    QObject::connect( &telnetClient_, SIGNAL( connected()), this, SLOT( telnetClientConnected()));
    QObject::connect( &telnetClient_, SIGNAL( disconnected()), this, SLOT( telnetClientDisconnected()));

    QObject::connect( ui->sendAllMsgListButton, SIGNAL( clicked()), this, SLOT( sendMsgList()));
    QObject::connect( ui->sendNextMsgListButton, SIGNAL( clicked()), this, SLOT( sendMsgAndGoToTheNext()));
    QObject::connect( ui->resetNextBtn, SIGNAL( clicked()), this, SLOT( resetNext()));
    QObject::connect( ui->loadListMsgButton, SIGNAL( clicked()), this, SLOT( loadListMsg()));
    QObject::connect( ui->listPlainTextEdit, SIGNAL( cursorPositionChanged()), this, SLOT( highlightCurrentLine()));
    QObject::connect( ui->sendThisOneBtn, SIGNAL( clicked()), this, SLOT( sendThisOne()));
    QObject::connect( ui->pickThisOneBtn, SIGNAL( clicked()), this, SLOT( pickThisOne()));
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
    if ( !telnetClientConnected_) {
        displaySocketError( "Not connected.");
        return;
    }

    QString msg = ui->msgLineEdit->text();

    if( !msg.isEmpty())
        telnetClient_.send( msg);
}

void MainWindow::sendStashedMsg()
{
    if ( !telnetClientConnected_) {
        displaySocketError( "Not connected.");
        return;
    }

    QString msg = ui->stashedMsgLineEdit->text();

    if( !msg.isEmpty())
        telnetClient_.send( msg);
}

void MainWindow::stashPopRevert()
{
    QString tmp = ui->msgLineEdit->text();
    ui->msgLineEdit->setText( ui->stashedMsgLineEdit->text());
    ui->stashedMsgLineEdit->setText( tmp);
}

void MainWindow::sendMsgList()
{
    if ( !telnetClientConnected_) {
        displaySocketError( "Not connected.");
        return;
    }

    QString msgs = ui->listPlainTextEdit->toPlainText();
    if( msgs.isEmpty())
        return;

    QString tvals = ui->timeIntervalTextEdit->toPlainText();

    telnetClient_.sendMsgList( msgs, tvals);
}


void MainWindow::sendMsgAndGoToTheNext()
{
    if ( !telnetClientConnected_) {
        displaySocketError( "Not connected.");
        return;
    }

    QString msgs = ui->listPlainTextEdit->toPlainText();

    if( msgs.isEmpty())
        return;

    QStringList msgList = msgs.split( QRegExp( "\n|\r\n|\r"), QString::SkipEmptyParts);
    if( currMsgIdx_  < msgList.size()) {
        telnetClient_.send( msgList[ currMsgIdx_++]);

        if( currMsgIdx_  < msgList.size()) {
            /* move the cursor 1 line down */
            QTextCursor tmp = ui->listPlainTextEdit->textCursor();
            tmp.movePosition( QTextCursor::StartOfLine);
            tmp.movePosition( QTextCursor::Down);

            /* will trigger cursorPositionChanged() and in result highlightCurrentLine() */
            ui->listPlainTextEdit->setTextCursor( tmp);
        }
    } else {
        ui->msgsPlainTextEdit->appendPlainText( "\nWarning: End of message list. Next call to 'send next' will send "
                                                " the 1st message again.");
        resetNext();
    }

//    /* move the cursor 1 line down */
//    QTextCursor tmp = ui->listPlainTextEdit->textCursor();
//    tmp.movePosition( QTextCursor::StartOfLine);
//    tmp.movePosition( QTextCursor::Down);

//    /* will trigger cursorPositionChanged() and in result highlightCurrentLine() */
//    ui->listPlainTextEdit->setTextCursor( tmp);
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

    if( !( ui->listPlainTextEdit->toPlainText().isEmpty())) {
        QTextCursor tmp = ui->listPlainTextEdit->textCursor();
        tmp.movePosition( QTextCursor::Start);
        ui->listPlainTextEdit->setTextCursor( tmp);
        highlightCurrentLine();
    }
}

void MainWindow::resetNext()
{
    currMsgIdx_ = 0;

    /* move the cursor to begin */
    QTextCursor tmp = ui->listPlainTextEdit->textCursor();
    tmp.movePosition( QTextCursor::StartOfLine);
    while ( tmp.blockNumber() > 0)
        tmp.movePosition( QTextCursor::Up);

    /* will trigger cursorPositionChanged() and in result highlightCurrentLine() */
    ui->listPlainTextEdit->setTextCursor( tmp);
}

void MainWindow::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!ui->listPlainTextEdit->isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground( lineColor);
        selection.format.setProperty( QTextFormat::FullWidthSelection, true);
        selection.cursor = ui->listPlainTextEdit->textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    ui->listPlainTextEdit->setExtraSelections(extraSelections);

    /* update nextMsgIdx_ */
    QTextCursor tmp = ui->listPlainTextEdit->textCursor();
    int p1 = tmp.position();
    int b1 = tmp.positionInBlock();
    int l1 = tmp.blockNumber();
    tmp.movePosition(( QTextCursor::StartOfLine));
    int currentLine = 0;
    int p2 = tmp.position();
    int b2 = tmp.positionInBlock();
    int l2 = tmp.blockNumber();
//    while( tmp.positionInBlock()>0) {
//        tmp.movePosition( QTextCursor::Up);
//        ++currentLine;
//    }
    currMsgIdx_ = tmp.blockNumber();
}

void MainWindow::sendThisOne()
{
    if ( !telnetClientConnected_) {
        displaySocketError( "Not connected.");
        return;
    }

    QString msg = ui->listPlainTextEdit->textCursor().block().text().trimmed();

    if ( !msg.isEmpty())
        telnetClient_.send( msg);
}

void MainWindow::pickThisOne()
{
    QString msg = ui->listPlainTextEdit->textCursor().block().text().trimmed();

    if ( !msg.isEmpty())
        ui->msgLineEdit->setText( msg);
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
        if ( telnetClientConnected_) telnetClient_.disconnect( hostText, portText);
        event->accept();
    }
}
