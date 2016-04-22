#-------------------------------------------------
#
# Project created by QtCreator 2016-04-21T23:45:57
#
#-------------------------------------------------

QT       += core gui dbus

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = padbus
TEMPLATE = app

CONFIG += link_pkgconfig
PKGCONFIG += libpulse-mainloop-glib


SOURCES += main.cpp\
        mainwindow.cpp \
    pulsectl.cpp

HEADERS  += mainwindow.h \
    pulsectl.h

FORMS    += mainwindow.ui
