#-------------------------------------------------
#
# Project created by QtCreator 2010-12-18T16:22:54
#
#-------------------------------------------------

QT       -= core

QT       -= gui

TARGET = rdns
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    rr.cpp \
    domainname.cpp \
    request.cpp \
    userrequest.cpp \
    daemon.cpp \
    reactor.cpp \
    epoll.cpp \
    iodevice.cpp \
    socket.cpp \
    tcpsocket.cpp \
    udpsocket.cpp \
    service.cpp \
	dnsservice.cpp \
	tcpechoservice.cpp \
	udpechoservice.cpp \
    dnsmessage.cpp

HEADERS += \
    rr.h \
    domainname.h \
    request.h \
    userrequest.h \
    daemon.h \
    shared_ptr.h \
    reactor.h \
    epoll.h \
    iodevice.h \
    socket.h \
    tcpsocket.h \
    udpsocket.h \
    service.h \
	dnsservice.h \
	tcpechoservice.h \
	udpechoservice.h \
    functionoid.h \
    dnsmessage.h
