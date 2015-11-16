#-------------------------------------------------
#
# Project created by QtCreator 2015-11-03T12:18:17
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = mpe_packets_generator
TEMPLATE = app


SOURCES += main.cpp\
        mpe_packets_generator.cpp \
    mpe_uint_validator.cpp

HEADERS  += mpe_packets_generator.h \
    rtp.h \
    mpe_uint_validator.h

FORMS    += mpe_packets_generator.ui

INCLUDEPATH +=
