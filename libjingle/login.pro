TEMPLATE = app

include(libjingle.pri)

SOURCES += $$EXAMPLES_DIR/login/xmppthread.cc \
	$$EXAMPLES_DIR/login/login_main.cc

HEADERS += $$EXAMPLES_DIR/login/xmppthread.h

LIBS += -L. -ljingle -lexpat -lxmpphelp

unix {
	mac {
		LIBS += -lssl -lcrypto
	} else {
		CONFIG += link_pkgconfig
		PKGCONFIG += openssl
	}
}
