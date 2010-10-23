TEMPLATE = lib
CONFIG += staticlib create_prl
VERSION = 2.0.1

include(libjingle.pri)

SOURCES += $$EXPAT_DIR/lib/xmlparse.c \
	$$EXPAT_DIR/lib/xmlrole.c \
	$$EXPAT_DIR/lib/xmltok.c

HEADERS += $$EXPAT_DIR/lib/xmlrole.h \
	$$EXPAT_DIR/lib/xmltok.h

DEFINES += XML_STATIC

INCLUDEPATH += $$EXPAT_DIR

unix {
	DEFINES += HAVE_EXPAT_CONFIG_H
	!exists($$EXPAT_DIR/expat_config.h) {
		message(Configuring Expat)
		system(cd $$EXPAT_DIR; ./configure)|error(Expat configure failed. Can't continue)
		message(Expat configured)
	}
}

win32 {
	DEFINES += COMPILED_FROM_DSP
}

