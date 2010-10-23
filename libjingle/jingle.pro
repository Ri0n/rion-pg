TEMPLATE = lib
CONFIG += staticlib create_prl
VERSION = 0.5

include(libjingle.pri)

SOURCES += talk/base/asyncfile.cc \
	talk/base/asynchttprequest.cc \
	talk/base/asyncpacketsocket.cc \
	talk/base/asyncsocket.cc \
	talk/base/asynctcpsocket.cc \
	talk/base/asyncudpsocket.cc \
	talk/base/autodetectproxy.cc \
	talk/base/base64.cc \
	talk/base/bytebuffer.cc \
	talk/base/checks.cc \
	talk/base/common.cc \
	talk/base/diskcache.cc \
	talk/base/event.cc \
	talk/base/fileutils.cc \
	talk/base/firewallsocketserver.cc \
	talk/base/flags.cc \
	talk/base/helpers.cc \
	talk/base/host.cc \
	talk/base/httpbase.cc \
	talk/base/httpclient.cc \
	talk/base/httpcommon.cc \
	talk/base/httprequest.cc \
	talk/base/logging.cc \
	talk/base/md5c.c \
	talk/base/messagehandler.cc \
	talk/base/messagequeue.cc \
	talk/base/nethelpers.cc \
	talk/base/network.cc \
	talk/base/openssladapter.cc \
	talk/base/pathutils.cc \
	talk/base/physicalsocketserver.cc \
	talk/base/proxydetect.cc \
	talk/base/proxyinfo.cc \
	talk/base/signalthread.cc \
	talk/base/socketadapters.cc \
	talk/base/socketaddress.cc \
	talk/base/socketaddresspair.cc \
	talk/base/socketpool.cc \
	talk/base/socketstream.cc \
	talk/base/ssladapter.cc \
	talk/base/sslsocketfactory.cc \
	talk/base/stream.cc \
	talk/base/stringdigest.cc \
	talk/base/stringencode.cc \
	talk/base/stringutils.cc \
	talk/base/task.cc \
	talk/base/taskparent.cc \
	talk/base/taskrunner.cc \
	talk/base/thread.cc \
	talk/base/time.cc \
	talk/base/urlencode.cc \
	talk/p2p/base/constants.cc \
	talk/p2p/base/p2ptransport.cc \
	talk/p2p/base/p2ptransportchannel.cc \
	talk/p2p/base/parsing.cc \
	talk/p2p/base/port.cc \
	talk/p2p/base/pseudotcp.cc \
	talk/p2p/base/relayport.cc \
	talk/p2p/base/relayserver.cc \
	talk/p2p/base/rawtransport.cc \
	talk/p2p/base/rawtransportchannel.cc \
	talk/p2p/base/session.cc \
	talk/p2p/base/sessiondescription.cc \
	talk/p2p/base/sessionmanager.cc \
	talk/p2p/base/sessionmessages.cc \
	talk/p2p/base/stun.cc \
	talk/p2p/base/stunport.cc \
	talk/p2p/base/stunrequest.cc \
	talk/p2p/base/stunserver.cc \
	talk/p2p/base/tcpport.cc \
	talk/p2p/base/transport.cc \
	talk/p2p/base/transportchannel.cc \
	talk/p2p/base/transportchannelproxy.cc \
	talk/p2p/base/udpport.cc \
	talk/p2p/client/basicportallocator.cc \
	talk/p2p/client/httpportallocator.cc \
	talk/p2p/client/socketmonitor.cc \
	talk/session/tunnel/pseudotcpchannel.cc \
	talk/session/tunnel/tunnelsessionclient.cc \
	talk/session/tunnel/securetunnelsessionclient.cc \
	talk/session/phone/audiomonitor.cc \
	talk/session/phone/call.cc \
	talk/session/phone/channel.cc \
	talk/session/phone/channelmanager.cc \
	talk/session/phone/codec.cc \
	talk/session/phone/devicemanager.cc \
	talk/session/phone/filemediaengine.cc \
	talk/session/phone/mediaengine.cc \
	talk/session/phone/mediamonitor.cc \
	talk/session/phone/mediasessionclient.cc \
	talk/session/phone/rtpdump.cc \
	talk/session/phone/soundclip.cc \
	talk/session/phone/srtpfilter.cc \
	talk/xmllite/qname.cc \
	talk/xmllite/xmlbuilder.cc \
	talk/xmllite/xmlconstants.cc \
	talk/xmllite/xmlelement.cc \
	talk/xmllite/xmlnsstack.cc \
	talk/xmllite/xmlparser.cc \
	talk/xmllite/xmlprinter.cc \
	talk/xmpp/xmppconstants.cc \
	talk/xmpp/jid.cc \
	talk/xmpp/ratelimitmanager.cc \
	talk/xmpp/saslmechanism.cc \
	talk/xmpp/xmppclient.cc \
	talk/xmpp/xmppengineimpl.cc \
	talk/xmpp/xmppengineimpl_iq.cc \
	talk/xmpp/xmpplogintask.cc \
	talk/xmpp/xmppstanzaparser.cc \
	talk/xmpp/xmpptask.cc

HEADERS += talk/xmpp/xmppclientsettings.h \
	talk/xmpp/xmppengine.h \
	talk/xmpp/asyncsocket.h \
	talk/xmpp/ratelimitmanager.h \
	talk/xmpp/saslmechanism.h \
	talk/xmpp/jid.h \
	talk/xmpp/xmpplogintask.h \
	talk/xmpp/saslhandler.h \
	talk/xmpp/plainsaslhandler.h \
	talk/xmpp/xmppclient.h \
	talk/xmpp/constants.h \
	talk/xmpp/xmppstanzaparser.h \
	talk/xmpp/saslcookiemechanism.h \
	talk/xmpp/saslplainmechanism.h \
	talk/xmpp/xmppengineimpl.h \
	talk/xmpp/prexmppauth.h \
	talk/xmpp/xmpptask.h \
	talk/base/time.h \
	talk/base/signalthread.h \
	talk/base/asynctcpsocket.h \
	talk/base/httpcommon.h \
	talk/base/checks.h \
	talk/base/thread.h \
	talk/base/task.h \
	talk/base/firewallsocketserver.h \
	talk/base/fileutils.h \
	talk/base/socketpool.h \
	talk/base/sigslotrepeater.h \
	talk/base/network.h \
	talk/base/stream.h \
	talk/base/stringencode.h \
	talk/base/md5.h \
	talk/base/basictypes.h \
	talk/base/pathutils.h \
	talk/base/socketfactory.h \
	talk/base/proxyinfo.h \
	talk/base/httpcommon-inl.h \
	talk/base/taskrunner.h \
	talk/base/httpclient.h \
	talk/base/asyncsocket.h \
	talk/base/socketaddresspair.h \
	talk/base/openssladapter.h \
	talk/base/sec_buffer.h \
	talk/base/base64.h \
	talk/base/common.h \
	talk/base/basicdefs.h \
	talk/base/constructormagic.h \
	talk/base/diskcache.h \
	talk/base/physicalsocketserver.h \
	talk/base/stringdigest.h \
	talk/base/socketstream.h \
	talk/base/messagehandler.h \
	talk/base/ssladapter.h \
	talk/base/byteorder.h \
	talk/base/taskparent.h \
	talk/base/messagequeue.h \
	talk/base/logging.h \
	talk/base/helpers.h \
	talk/base/asyncfile.h \
	talk/base/socket.h \
	talk/base/socketaddress.h \
	talk/base/asyncpacketsocket.h \
	talk/base/asynchttprequest.h \
	talk/base/event.h \
	talk/base/asyncudpsocket.h \
	talk/base/stringutils.h \
	talk/base/nethelpers.h \
	talk/base/bytebuffer.h \
	talk/base/scoped_ptr.h \
	talk/base/socketserver.h \
	talk/base/httprequest.h \
	talk/base/linked_ptr.h \
	talk/base/socketadapters.h \
	talk/base/urlencode.h \
	talk/base/proxydetect.h \
	talk/base/httpbase.h \
	talk/base/criticalsection.h \
	talk/base/autodetectproxy.h \
	talk/base/flags.h \
	talk/base/cryptstring.h \
	talk/base/sslsocketfactory.h \
	talk/base/host.h \
	talk/base/Equifax_Secure_Global_eBusiness_CA-1.h \
	talk/base/sigslot.h \
	talk/p2p/base/stunserver.h \
	talk/p2p/base/port.h \
	talk/p2p/base/stunrequest.h \
	talk/p2p/base/p2ptransport.h \
	talk/p2p/base/common.h \
	talk/p2p/base/session.h \
	talk/p2p/base/sessionmanager.h \
	talk/p2p/base/transportchannel.h \
	talk/p2p/base/rawtransport.h \
	talk/p2p/base/stunport.h \
	talk/p2p/base/relayport.h \
	talk/p2p/base/transportchannelproxy.h \
	talk/p2p/base/sessionid.h \
	talk/p2p/base/udpport.h \
	talk/p2p/base/pseudotcp.h \
	talk/p2p/base/sessionclient.h \
	talk/p2p/base/transportchannelimpl.h \
	talk/p2p/base/candidate.h \
	talk/p2p/base/tcpport.h \
	talk/p2p/base/constants.h \
	talk/p2p/base/stun.h \
	talk/p2p/base/portallocator.h \
	talk/p2p/base/sessiondescription.h \
	talk/p2p/base/relayserver.h \
	talk/p2p/base/transport.h \
	talk/p2p/base/p2ptransportchannel.h \
	talk/p2p/base/sessionmessages.h \
	talk/p2p/base/parsing.h \
	talk/p2p/base/rawtransportchannel.h \
	talk/p2p/client/basicportallocator.h \
	talk/p2p/client/sessionmanagertask.h \
	talk/p2p/client/sessionsendtask.h \
	talk/p2p/client/socketmonitor.h \
	talk/p2p/client/httpportallocator.h \
	talk/xmllite/xmlconstants.h \
	talk/xmllite/xmlbuilder.h \
	talk/xmllite/xmlnsstack.h \
	talk/xmllite/qname.h \
	talk/xmllite/xmlprinter.h \
	talk/xmllite/xmlelement.h \
	talk/xmllite/xmlparser.h \
	talk/session/tunnel/securetunnelsessionclient.h \
	talk/session/tunnel/tunnelsessionclient.h \
	talk/session/tunnel/pseudotcpchannel.h \
	talk/session/phone/channel.h \
	talk/session/phone/mediasessionclient.h \
	talk/session/phone/mediachannel.h \
	talk/session/phone/cryptoparams.h \
	talk/session/phone/mediaengine.h \
	talk/session/phone/soundclip.h \
	talk/session/phone/videocommon.h \
	talk/session/phone/call.h \
	talk/session/phone/channelmanager.h \
	talk/session/phone/mediamonitor.h \
	talk/session/phone/devicemanager.h \
	talk/session/phone/codec.h \
	talk/session/phone/filemediaengine.h \
	talk/session/phone/srtpfilter.h \
	talk/session/phone/rtpdump.h \
	talk/session/phone/audiomonitor.h \
	talk/session/phone/voicechannel.h

unix {
	SOURCES += talk/base/unixfilesystem.cc \
		talk/base/opensslidentity.cc \
		talk/base/opensslstreamadapter.cc \
		talk/base/sslidentity.cc \
		talk/base/sslstreamadapter.cc
	   
	HEADERS += talk/base/unixfilesystem.h \
		talk/base/opensslidentity.h \
		talk/base/opensslstreamadapter.h \
		talk/base/sslidentity.h \
		talk/base/sslstreamadapter.h
   
	mac {
		SOURCES += talk/base/macconversion.cc \
			talk/base/macutils.cc \
			talk/session/phone/devicemanager_mac.mm
			
		HEADERS += talk/base/macconversion.h \
			talk/base/macutils.h

	} else {
		# linux ?
		SOURCES += talk/base/linux.cc \
			talk/session/phone/v4llookup.cc
			
		HEADERS += talk/base/linux.h \
			talk/session/phone/v4llookup.h

	}
}

win32 {
	INCLUDEPATH += $$THIRD_PARTY_DIR/mingw-include
	DEFINES += __x86_64
	SOURCES += talk/base/schanneladapter.cc \
		talk/base/win32.cc \
		talk/base/win32filesystem.cc \
		talk/base/win32securityerrors.cc \
		talk/base/win32socketserver.cc \
		talk/base/win32socketinit.cc \
		talk/base/win32window.cc \
		talk/base/winfirewall.cc \
		talk/base/winping.cc
		
	HEADERS += talk/base/schanneladapter.h \
		talk/base/win32.h \
		talk/base/win32filesystem.h \
		talk/base/win32socketserver.h \
		talk/base/win32socketinit.h \
		talk/base/win32window.h \
		talk/base/winfirewall.h \
		talk/base/winping.h
}

DEFINES += FEATURE_ENABLE_VOICEMAIL \
	EXPAT_RELATIVE_PATH \
	SRTP_RELATIVE_PATH \
	XML_STATIC

INCLUDEPATH += $$EXPAT_DIR \
	$$SRTP_DIR/include \
	$$SRTP_DIR/crypto/include

