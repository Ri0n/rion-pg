#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <vector>
#include "daemon.h"
#include "dnsservice.h"
#include "tcpechoservice.h"
#include "udpechoservice.h"

using namespace std;

int main(int argc, char *argv[])
{
	auto_ptr<rdns::Daemon> daemon(new rdns::Daemon());
	vector<rdns::ServicePtr> services;

	string arg, host, scheme, proto;
	for (int i = 1; i < argc; i++) {
		arg = argv[i];
		host = "";
		proto = "";
		scheme = arg.substr(0, arg.find(':'));
		if (scheme == "udp" || scheme == "dns") {
			proto = "UDP";
		} else if (scheme == "tcp") {
			proto = "TCP";
		}
		if (proto.size()) {
			int ipPos = scheme.size() + 3;
			size_t dp = arg.find(':', ipPos);
			if (dp != string::npos) {
				std::istringstream portStr(arg.substr(dp + 1));
				int port;
				portStr >> port;
				rdns::SocketPtr sock = daemon->addListener(proto.c_str(),
								arg.substr(ipPos, dp - ipPos).c_str(), port);
				if (sock.get()) { // inited
					if (scheme == "dns") {
						services.push_back(rdns::ServicePtr(new rdns::DNSService(sock)));
					}
					else if (scheme == "tcp") {
						services.push_back(rdns::ServicePtr(new rdns::TCPEchoService(sock)));
					}
					else if (scheme == "udp") {
						services.push_back(rdns::ServicePtr(new rdns::UDPEchoService(sock)));
					}
				}
			}
		}
	}

	//daemon->addListener("UDP", "0.0.0.0", 53);
	//daemon->addListener("TCP", "0.0.0.0", 5300);
	//daemon->addListener("UDP", "0.0.0.0", 1235);
	if (!daemon->listen()) {
		cout << "Exiting because of errors\n";
		return 1;
	}
	return 0;
}
