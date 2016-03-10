#-------------------------------------------------
#
# Project created by QtCreator 2010-10-21T13:09:18
#
#-------------------------------------------------

QT       += core gui uitools

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
} 

TARGET = popup
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    popup.cpp

HEADERS  += mainwindow.h \
    popup.h

FORMS    += mainwindow.ui \
    defaulttheme.ui \
    popup.ui

RESOURCES += \
    popup.qrc
