message( Run Qt < lrelease > before qmake. )

QT       += core network

QT       -= gui

DESTDIR = ../bin

TARGET = hsd
CONFIG   += console
CONFIG   -= app_bundle

 TRANSLATIONS = hsd_de.ts \

INCLUDEPATH += ../3rd_party/QsLog

TEMPLATE = app

include( ../3rd_party/QsLog/QsLog.pri )

SOURCES += main.cpp \
    tcpclient.cpp \
    tcpserver.cpp \
    model.cpp \
    koxml.cpp \
    hsd.cpp \
    eibdmsg.cpp \
    groupaddress.cpp \

HEADERS += \
    tcpclient.h \
    tcpserver.h \
    model.h \
    koxml.h \
    hsd.h \
    eibdmsg.h \
    groupaddress.h

RESOURCES += \
    ressources.qrc
