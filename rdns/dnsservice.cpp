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
#include "timer.h"

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

class DNSServiceTimeoutCallback : public Callback
{
public:
	DNSServiceTimeoutCallback(DNSService *service)
		: service(service)
	{ }

	void call() {
		service->onTimeout();
	}

private:
	DNSService *service;
};


//--------------------------
// DNSService
//--------------------------

DNSService::DNSService(IODevicePtr socket)
	: Service(socket)
{
	socket->setReadyReadHandler(
				CallbackPtr(new DNSServiceReadyReadCallback(this)) );

	Timer *t = new Timer();
	t->setTimeout(DNSRequestTTL.tv_sec, DNSRequestTTL.tv_nsec);
	t->setInterval(DNSRequestTTL.tv_sec, DNSRequestTTL.tv_nsec);
	t->setReadyReadHandler(
		CallbackPtr( new DNSServiceTimeoutCallback(this) )
	);
	_timer = IODevicePtr(t);
	cout << "Timer fd = " << t->fd() << "\n";
	Reactor::instance()->addWatch(_timer);
}

bool DNSService::setRemoteDns(const char *host, uint16_t port)
{
	_remoteDns = IODevicePtr(new UDPSocket());
	cout << "Remote DNS fd = " << _remoteDns->fd() << "\n";
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
	if (((Socket*)socket.get())->accept(socket)) {
		ssize_t cnt = socket->read(buf, DNSMaxMessageSize);
		if (cnt == -1) {
			cout << "hm\n";
			perror("failed to read data from udp");
		}
		else {
			DNSMessagePtr message = DNSMessage::fromByteArray(buf, cnt);
			if (message.get() && !message->isResponse()) {
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
					DNSRequest *request = new DNSRequest(message->id(), dname,
														 ((UDPSocket*)socket.get())->endPoint());
					if (_requests.empty()) { // timer not started yet
						((Timer*)_timer.get())->start();
					}
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
			if ((*ri).second->domainName() == message->domainName()) {
				((UDPSocket*)socket.get())->setEndPoint((*ri).second->clientAddress());
				message->writeTo(socket);
			}
			_requests.erase(message->id());
		}
		if (_requests.empty()) {
			((Timer*)_timer.get())->stop();
		}
	}
}

void DNSService::onTimeout()
{
	cout << "Check requests for timeouts\n";
	// it's not too effective to iterate over all pending requests
	// but there are should not be too many requests at each moment of time
	// excluding a case when remote server is really down. moreover this
	// function is called one time per DNSRequestTTL period which is quite long.
	for (DNSRequestIterator ri = _requests.begin(); ri != _requests.end();) {
		if ((*ri).second->isExpired()) {
			cout << "Remove timed out request\n";
			_requests.erase(ri++);
		}
		else {
			ri++;
		}
	}
	if (_requests.empty()) {
		((Timer*)_timer.get())->stop();
	}
}

