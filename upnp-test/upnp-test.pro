#-------------------------------------------------
#
# Project created by QtCreator 2013-10-17T07:05:16
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = upnp-test
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
	natportmapper.cpp

win32:SOURCES += natportmapper_win.cpp

HEADERS  += mainwindow.h \
    natportmapper.h \
    natportmapper_win.h

FORMS    += mainwindow.ui

LIBS += -lole32 -loleaut32
