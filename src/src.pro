message( Run Qt < lrelease > before qmake. )

QT       += core network
QT       -= gui

CONFIG += c++17
CONFIG += lrelease

# QMAKE_CXXFLAGS += -std=c++17

DESTDIR = ../bin

# unix:target.path = /opt/hsd/bin
# unix:INSTALLS += target

TARGET = hsd
CONFIG   += console
CONFIG   -= app_bundle

TRANSLATIONS = hsd_de.ts

TEMPLATE = app

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


# Default rules for deployment.
# qnx: target.path = /tmp/$${TARGET}/bin
# else: unix:!android: target.path = /opt/$${TARGET}/bin
# !isEmpty(target.path): INSTALLS += target
