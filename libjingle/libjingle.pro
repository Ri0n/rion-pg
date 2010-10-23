TEMPLATE = subdirs
CONFIG += ordered create_prl examples

SUBDIRS += srtp expat jingle

srtp.file = srtp.pro
expat.file = expat.pro
jingle.file = jingle.pro

examples {
	SUBDIRS += xmpphelp login call
	xmpphelp.file = xmpphelp.pro
	login.file = login.pro
	call.file = call.pro
}
