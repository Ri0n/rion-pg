TEMPLATE = lib
CONFIG += staticlib create_prl
VERSION = 1.4.4

include(libjingle.pri)

HEADERS = $$SRTP_DIR/include/ekt.h \
	$$SRTP_DIR/include/getopt_s.h \
	$$SRTP_DIR/include/rtp.h \
	$$SRTP_DIR/include/rtp_priv.h \
	$$SRTP_DIR/include/srtp.h \
	$$SRTP_DIR/include/srtp_priv.h \
	$$SRTP_DIR/include/ut_sim.h
	
SOURCES = $$SRTP_DIR/crypto/cipher/aes.c \
   $$SRTP_DIR/crypto/cipher/aes_cbc.c \
   $$SRTP_DIR/crypto/cipher/aes_icm.c \
   $$SRTP_DIR/crypto/cipher/cipher.c \
   $$SRTP_DIR/crypto/cipher/null_cipher.c \
   $$SRTP_DIR/crypto/hash/auth.c \
   $$SRTP_DIR/crypto/hash/hmac.c \
   $$SRTP_DIR/crypto/hash/null_auth.c \
   $$SRTP_DIR/crypto/hash/sha1.c \
   $$SRTP_DIR/crypto/replay/rdb.c \
   $$SRTP_DIR/crypto/replay/rdbx.c \
   $$SRTP_DIR/crypto/replay/ut_sim.c \
   $$SRTP_DIR/crypto/math/datatypes.c \
   $$SRTP_DIR/crypto/math/stat.c \
   $$SRTP_DIR/crypto/kernel/alloc.c \
   $$SRTP_DIR/crypto/kernel/crypto_kernel.c \
   $$SRTP_DIR/crypto/kernel/err.c \
   $$SRTP_DIR/crypto/kernel/key.c \
   $$SRTP_DIR/crypto/rng/ctr_prng.c \
   $$SRTP_DIR/crypto/rng/rand_source.c \
   $$SRTP_DIR/srtp/ekt.c \
   $$SRTP_DIR/srtp/srtp.c
 
INCLUDEPATH += $$SRTP_DIR/include \
   $$SRTP_DIR/crypto/include \

win32-msvc*:QMAKE_CFLAGS += /wd4701 /wd4702

!exists($$SRTP_DIR/crypto/include/config.h) {
	win32 {
		SDW=$$replace(SRTP_DIR, /, \\)
		TPW=$$replace(THIRD_PARTY_DIR, /, \\)
		system(copy $$TPW\\srtp_config.hmingw $$SDW\\crypto\\include\\config.h)
	} else {
		message(Configuring SRTP)
		system(cd $$SRTP_DIR; ./configure)|error(SRTP Configure failed. Can't continue)
		message(SRTP configured)
	}
}

