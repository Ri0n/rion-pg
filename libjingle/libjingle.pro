TEMPLATE = subdirs
CONFIG += ordered create_prl

SUBDIRS += srtp expat jingle

srtp.file = srtp.pro
expat.file = expat.pro
jingle.file = jingle.pro
