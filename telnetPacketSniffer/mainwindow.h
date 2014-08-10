#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "telnetclient.h"

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
    void sendMsgBtnClicked();
    void telnetClientConnected();
    void telnetClientDisconnected();

signals:
    void openTelnetConnection( QString host, QString port);
    void closeTelnetConnection( QString host, QString port);

private:
    Ui::MainWindow *ui;
    TelnetClient telnetClient;
    QString hostText, portText;
};

#endif // MAINWINDOW_H
