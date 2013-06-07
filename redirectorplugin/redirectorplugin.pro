CONFIG += release
TARGET = stopspamplugin

exists($$sdkdir) {
    include($$sdkdir/psiplugin.pri)
} else {
    include(../../psiplugin.pri)
}
SOURCES += stopspamplugin.cpp \
    view.cpp \
    viewer.cpp \
    typeaheadfind.cpp \
    deferredstanzasender.cpp
HEADERS += view.h \
    viewer.h \
    typeaheadfind.h \
    deferredstanzasender.h
FORMS += options.ui
