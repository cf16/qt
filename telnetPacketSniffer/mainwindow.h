#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "telnetclient.h"

class QCloseEvent;

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
    void sendMsgList();

    void sendNextMsg();

    void telnetClientConnected();
    void telnetClientDisconnected();

    void loadListMsg();

signals:
    void openTelnetConnection( QString host, QString port);
    void closeTelnetConnection( QString host, QString port);

private:
    void closeEvent ( QCloseEvent *event);
    Ui::MainWindow *ui;
    TelnetClient telnetClient;
    QString hostText, portText;
    bool telnetClientConnected_;
};

#endif // MAINWINDOW_H
