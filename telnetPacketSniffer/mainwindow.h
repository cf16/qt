/*
 * @brief Telnet client application
 *
 * with capability to send list of messages with predefined
 * time intervals between them. Uses asynchronous socket
 * and handles sending messages in a separate thread
 *
 * @author Piotr Gregor peterg@sytelco.com
 * @date   10 Aug 2014
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "telnetclient.h"

class QCloseEvent;
class QPlainTextEdit;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow( QWidget *parent = 0);
    ~MainWindow();

private slots:
    void connectBtnClicked();
    void disconnectBtnClicked();

    void hostTextChanged( QString newHostText);
    void portTextChanged( QString newPortText);

    void displaySocketError( QString text);
    void telnetData( QString text);
    void msgSent( QString text);

    void sendMsg();
    void sendStashedMsg();
    void stashPopRevert();
    void sendWebMsg();

    void sendMsgList();
    void sendMsgAndGoToTheNext();

    void telnetClientConnected();
    void telnetClientDisconnected();

    void loadListMsg();
    void saveListMsg();
    void resetNext();

    void highlightCurrentLine();
    int cursorFirstLineInBlockNumber( const QPlainTextEdit*);

    void sendThisOne();
    void pickThisOne();

signals:
    void openTelnetConnection( QString host, QString port);
    void closeTelnetConnection( QString host, QString port);

private:
    void closeEvent ( QCloseEvent *event);

    Ui::MainWindow *ui;
    TelnetClient telnetClient_;
    QString hostText, portText;
    bool telnetClientConnected_;
    int currMsgIdx_;
};

#endif // MAINWINDOW_H
