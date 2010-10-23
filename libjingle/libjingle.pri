THIRD_PARTY_DIR = talk/third_party

EXPAT_DIR = $$THIRD_PARTY_DIR/expat-2.0.1
SRTP_DIR = $$THIRD_PARTY_DIR/srtp

DEFINES += LOGGING=1 \
	FEATURE_ENABLE_SSL \
	FEATURE_ENABLE_VOICEMAIL \
	FEATURE_ENABLE_PSTN \
	HAVE_SRTP

unix {
	DEFINES += HASHNAMESPACE=__gnu_cxx \
		HASH_NAMESPACE=__gnu_cxx \
		POSIX \
		DISABLE_DYNAMIC_CAST \
		HAVE_OPENSSL_SSL_H=1

	QMAKE_CFLAGS += -Wall \
		-Wno-switch \
		-fno-exceptions

	QMAKE_CXXFLAGS += -Wno-non-virtual-dtor \
		-Wno-ctor-dtor-privacy \
		-fno-rtti

	mac {
		DEFINES += OSX MAC_OS_X_VERSION_MIN_REQUIRED=1040
		QMAKE_CFLAGS += -fasm-blocks
		QMAKE_LFLAGS += -Wl,-search_paths_first -ObjC
		release:QMAKE_CFLAGS += -Wno-unused-variable
		debug {
			DEFINES += FLAVOR_DBG ENABLE_DEBUG
			QMAKE_CFLAGS += -O0
		}
		
	} else {
		# linux ?
		DEFINES += LINUX HAVE_GLIB
		QMAKE_LFLAGS += -Wl,--start-group
	}
}

win32 {
	DEFINES  += _ATL_CSTRING_EXPLICIT_CONSTRUCTORS \
		_CRT_SECURE_NO_WARNINGS \
		_SCL_SECURE_NO_WARNINGS \
		_UNICODE \
		UNICODE \
		WIN32 \
		WINVER=0x0600 \
		_WIN32_WINNT=0x0600 \
		_WIN32_IE=0x0600 \
		NTDDI_VERSION=NTDDI_WINXP
#		_USE_32BIT_TIME_T \
	win32-msvc* {
		DEFINES += _HAS_EXCEPTIONS=0
	} else {
		DEFINES += _HAS_EXCEPTIONS=1
	}
}
