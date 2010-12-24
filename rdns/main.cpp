#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>
#include "daemon.h"
#include "reactor.h"
#include "dnsservice.h"
#include "tcpechoservice.h"
#include "udpechoservice.h"
#include "uri.h"

using namespace std;

int main(int argc, char *argv[])
{
	auto_ptr<rdns::Daemon> daemon(new rdns::Daemon());
	vector<rdns::ServicePtr> services;
	rdns::Reactor::setDefaultType(rdns::Reactor::EpollType);

	string proto;
	rdns::Uri remoteDns;
	rdns::DNSService *dnsService = 0;
	for (int i = 1; i < argc; i++) {
		proto = "";
		rdns::Uri uriParam(argv[i]);
		if (uriParam.scheme() == "remotedns") {
			remoteDns = uriParam;
		}
		else if (uriParam.scheme() == "udp" || uriParam.scheme() == "dns") {
			proto = "UDP";
		}
		else if (uriParam.scheme() == "tcp") {
			proto = "TCP";
		}
		if (proto.size()) {
			rdns::SocketPtr sock = daemon->addListener(proto.c_str(),
							uriParam.host().c_str(), uriParam.port());
			if (sock.get()) { // inited
				if (uriParam.scheme() == "dns") {
					dnsService = new rdns::DNSService(sock);
					services.push_back(rdns::ServicePtr(dnsService));
				}
				else if (uriParam.scheme() == "tcp") {
					services.push_back(rdns::ServicePtr(new rdns::TCPEchoService(sock)));
				}
				else if (uriParam.scheme() == "udp") {
					services.push_back(rdns::ServicePtr(new rdns::UDPEchoService(sock)));
				}
			}
		}
	}

	if (dnsService && remoteDns.host().size()) {
		dnsService->setRemoteDns(remoteDns.host().c_str(), remoteDns.port());
	}

	if (!daemon->listen()) {
		cout << "Exiting because of errors\n";
		return 1;
	}
	return 0;
}
