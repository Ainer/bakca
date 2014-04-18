#-------------------------------------------------
#
# Project created by QtCreator 2014-04-10T14:27:10
#
#-------------------------------------------------

QT       += network

QT       -= gui

TARGET = DSMTS
TEMPLATE = lib

DEFINES += DSMTS_LIBRARY

SOURCES += dsmts.cpp

HEADERS += dsmts.h\
        dsmts_global.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
