#-------------------------------------------------
#
# Project created by QtCreator 2013-05-01T11:53:12
#
#-------------------------------------------------

QT       += core network

QT       -= gui

DESTDIR = ../bin

TARGET = hsd
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    tcpclient.cpp \
    tcpserver.cpp \
    model.cpp \
    koxml.cpp \
    hsd.cpp \
    eibdmsg.cpp

HEADERS += \
    tcpclient.h \
    tcpserver.h \
    model.h \
    koxml.h \
    hsd.h \
    eibdmsg.h
