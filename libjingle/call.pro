TEMPLATE = app

include(libjingle.pri)

SOURCES += $$EXAMPLES_DIR/call/call_main.cc \
	$$EXAMPLES_DIR/call/callclient.cc \
	$$EXAMPLES_DIR/call/console.cc \
	$$EXAMPLES_DIR/call/discoitemsquerytask.cc \
	$$EXAMPLES_DIR/call/friendinvitesendtask.cc \
	$$EXAMPLES_DIR/call/mucinviterecvtask.cc \
	$$EXAMPLES_DIR/call/mucinvitesendtask.cc \
	$$EXAMPLES_DIR/call/presenceouttask.cc \
	$$EXAMPLES_DIR/call/presencepushtask.cc \
	$$EXAMPLES_DIR/call/voicemailjidrequester.cc

HEADERS += $$EXAMPLES_DIR/login/xmppthread.h \
	$$EXAMPLES_DIR/call/callclient.h \
	$$EXAMPLES_DIR/call/console.h \
	$$EXAMPLES_DIR/call/discoitemsquerytask.h \
	$$EXAMPLES_DIR/call/friendinvitesendtask.h \
	$$EXAMPLES_DIR/call/muc.h \
	$$EXAMPLES_DIR/call/mucinviterecvtask.h \
	$$EXAMPLES_DIR/call/mucinvitesendtask.h \
	$$EXAMPLES_DIR/call/presenceouttask.h \
	$$EXAMPLES_DIR/call/presencepushtask.h \
	$$EXAMPLES_DIR/call/status.h \
	$$EXAMPLES_DIR/call/voicemailjidrequester.h


LIBS += -L. -ljingle -lexpat -lsrtp -lxmpphelp

DEFINES += FEATURE_ENABLE_VOICEMAIL

unix {
	mac {
		LIBS += -lssl -lcrypto \
			-framework AudioToolbox \
			-framework AudioUnit \
			-framework Cocoa \
			-framework CoreAudio \
			-framework CoreFoundation \
			-framework IOKit \
			-framework QTKit \
			-framework QuickTime
	} else {
		CONFIG += link_pkgconfig
		PKGCONFIG += openssl alsa
	}
}

win32:LIBS += -ld3d9 -lgdi32 -lpowrprof -lstrmiids -lwinmm
