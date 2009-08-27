# -------------------------------------------------
# Project created by QtCreator 2009-08-23T22:33:08
# -------------------------------------------------
QT += network \
    sql \
    xml
QT -= gui
TARGET = daemon
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    rdaemonapp.cpp \
    xmlrpcdaemon.cpp \
    sqlstorage.cpp \
    basestorage.cpp \
    xmlrpcrequest.cpp \
    mysqlstorage.cpp \
    sqlitestorage.cpp
HEADERS += rdaemonapp.h \
    xmlrpcdaemon.h \
    sqlstorage.h \
    basestorage.h \
    xmlrpcrequest.h \
    mysqlstorage.h \
    sqlitestorage.h
LIBS += -lqxmlrpc
INCLUDEPATH += /usr/include
