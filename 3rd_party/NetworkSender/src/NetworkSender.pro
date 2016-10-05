#-------------------------------------------------
#
# Project created by QtCreator 2013-01-05T11:26:39
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4) {
  QT += widgets webkitwidgets
  DEFINES += USING_QT5
}

TEMPLATE = app

CONFIG(debug, debug|release) {
  CONFIG -= debug release
  CONFIG += debug

  TARGET  = NetworkSenderD
}

CONFIG(release) {
  CONFIG -= debug release
  CONFIG += release

  TARGET  = NetworkSender
}

win32:DEFINES += win32
win32:DESTDIR  = ../bin

SOURCES  += main.cpp\
            NetworkSender.cpp \
            udpmanager.cpp \
            tcpmanager.cpp


HEADERS  += NetworkSender.h \
            udpmanager.h \
            tcpmanager.h

FORMS    += NetworkSender.ui
