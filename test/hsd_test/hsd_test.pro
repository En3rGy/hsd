#-------------------------------------------------
#
# Project created by QtCreator 2018-03-17T22:46:18
#
#-------------------------------------------------

QT       += testlib network

QT       -= gui

TARGET = tst_hsd_testtest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../../3rd_party/QsLog
INCLUDEPATH += ../../src


include( ../../3rd_party/QsLog/QsLog.pri )

SOURCES += ../../src/eibdmsg.cpp \
../../src/model.cpp \
../../src/groupaddress.cpp \
../../src/koxml.cpp \
../../src/tcpclient.cpp

HEADERS += \
    ../../src/eibdmsg.h \
../../src/model.h \
../../src/groupaddress.h \
../../src/koxml.h \
../../src/tcpclient.h


SOURCES += \
        tst_hsd_testtest.cpp 

DEFINES += SRCDIR=\\\"$$PWD/\\\"
