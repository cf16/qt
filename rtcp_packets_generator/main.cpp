///  @file      main.cpp
///  @brief     RTCP packet generator.
///  @author    peterg at sytel.com
///  @date      03 Nov 2015 3:21 PM
///  @copyright GPL2

#include "mpe_packets_generator.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Mpe_packets_generator w;
    w.show();

    return a.exec();
}
