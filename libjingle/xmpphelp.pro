TEMPLATE = lib
CONFIG += staticlib create_prl
VERSION = 0.5

include(libjingle.pri)

SOURCES += $$EXAMPLES_DIR/login/xmppauth.cc \
	$$EXAMPLES_DIR/login/xmpppump.cc \
	$$EXAMPLES_DIR/login/xmppsocket.cc

HEADERS += $$EXAMPLES_DIR/login/xmppauth.h \
	$$EXAMPLES_DIR/login/xmpppump.h \
	$$EXAMPLES_DIR/login/xmppsocket.h

LIBS += -ljingle