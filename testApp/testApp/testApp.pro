#-------------------------------------------------
#
# Project created by QtCreator 2014-04-10T14:26:03
#
#-------------------------------------------------

QT       += core network widgets xml


QT       -= gui

TARGET = testApp
CONFIG   += console C++11 TUFAO1
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    DSMTS.cpp

HEADERS += \
    DSMTS.h
QMAKE_LFLAGS += -L${HOME}/src/tufao/run/lib
QMAKE_CXXFLAGS += -I${HOME}/src/tufao/run/include/tufao-1
