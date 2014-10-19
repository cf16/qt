#-------------------------------------------------
#
# Project created by QtCreator 2014-08-09T03:47:40
#
#-------------------------------------------------

QT       += core gui network
LIBS    += -lpcap

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = telnetPacketSniffer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    telnetclient.cpp \
    packetsniffer.cpp

HEADERS  += mainwindow.h \
    telnetclient.h \
    packetsniffer.h

FORMS    += mainwindow.ui
