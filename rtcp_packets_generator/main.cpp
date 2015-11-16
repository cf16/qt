#include "mpe_packets_generator.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Mpe_packets_generator w;
    w.show();

    return a.exec();
}
