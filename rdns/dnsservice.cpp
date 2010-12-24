#include <cstdio>
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "dnsservice.h"
#include "dnsmessage.h"
#include "udpsocket.h"
#include "reactor.h"

using namespace rdns;
using std::cout;
using std::cerr;

class DNSServiceReadyReadCallback : public Callback
{
public:
	DNSServiceReadyReadCallback(DNSService *service)
		: service(service)
	{ }

	void call() {
		service->onReadyRead();
	}

private:
	DNSService *service;
};

class DNSServiceRemoteReadyReadCallback : public Callback
{
public:
	DNSServiceRemoteReadyReadCallback(DNSService *service)
		: service(service)
	{ }

	void call() {
		service->onRemoteReadyRead();
	}

private:
	DNSService *service;
};

//--------------------------
// DNSService
//--------------------------

DNSService::DNSService(SocketPtr socket)
	: Service(socket)
{
	socket->setReadyReadHandler(
				CallbackPtr(new DNSServiceReadyReadCallback(this)) );
}

bool DNSService::setRemoteDns(const char *host, uint16_t port)
{
	_remoteDns = SocketPtr(new UDPSocket());
	UDPSocket *remote = (UDPSocket*)_remoteDns.get();
	if (remote->setEndPoint(host, port) && remote->connectToEndPoint()) {
		remote->setReadyReadHandler(
			CallbackPtr( new DNSServiceRemoteReadyReadCallback(this) )
		);
		Reactor::instance()->addWatch(_remoteDns);
		cout << "Successfully set remote dns server\n";
		return true;
	}
	cerr << "Failed to set remote dns server\n";
	return false;
}

void DNSService::onReadyRead()
{
	unsigned char buf[DNSMaxMessageSize];
	if (socket->accept(socket)) {
		ssize_t cnt = socket->read(buf, DNSMaxMessageSize);
		if (cnt == -1) {
			cout << "hm\n";
			perror("failed to read data from udp");
		}
		else {
			DNSMessagePtr message = DNSMessage::fromByteArray(buf, cnt);
			if (message.get()) {
				cout << "Received new dns request: " << message->toString() << "\n";
				std::string dname = message->domainName();
				// harcode words. no time to do smth more smart.
				if (dname.find("sex") != std::string::npos ||
					dname.find("xxx") != std::string::npos ||
					!_remoteDns.get())
				{
					message->setResponseBit(true);
					message->writeTo(socket);
				}
				else {
					DNSRequest *request = new DNSRequest(message->id(),
														 ((UDPSocket*)socket.get())->endPoint());
					_requests.insert(
						DNSRequestItem(message->id(), DNSRequestPtr(request))
					);
					message->writeTo(_remoteDns);
				}
			}
			else {
				socket->write("error", 5); // gotta read what to send in case of bad requests
			}
		}
	}
}


void DNSService::onRemoteReadyRead()
{
	cout << "remote ready read\n";
	unsigned char buf[DNSMaxMessageSize];
	int cnt;
	if ((cnt = _remoteDns->read(buf, DNSMaxMessageSize)) != -1) {
		DNSMessagePtr message = DNSMessage::fromByteArray(buf, cnt);
		DNSRequestIterator ri = _requests.find(message->id());
		if (ri != _requests.end()) {
			((UDPSocket*)socket.get())->setEndPoint((*ri).second->clientAddress());
			message->writeTo(socket);
		}
	}
}
