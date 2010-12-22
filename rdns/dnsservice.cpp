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
	DNSServiceRemoteReadyReadCallback(DNSService *service,
									  const sockaddr_in &clientAddr,
									  UDPSocket *remoteSocket)
		: service(service)
		, clientAddr(clientAddr)
		, remoteSocket(remoteSocket)
	{ }

	void call() {
		service->onRemoteReadyRead(remoteSocket, clientAddr);
	}

private:
	DNSService *service;
	sockaddr_in clientAddr;
	UDPSocket *remoteSocket;
};

DNSService::DNSService(SocketPtr socket)
	: Service(socket)
{
	socket->setReadyReadHandler(CallbackPtr(
										new DNSServiceReadyReadCallback(this)));
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
					dname.find("xxx") != std::string::npos)
				{
					message->setResponseBit(true);
					message->writeTo(socket);
				}
				else {
					UDPSocket *remote = new UDPSocket("0.0.0.0", 52000);
					if (remote->isValid() && remote->connectTo("192.168.0.1", 53)) {
						remote->setReadyReadHandler(CallbackPtr(
							new DNSServiceRemoteReadyReadCallback(
								this, ((UDPSocket*)socket.get())->clientAddress(),
										remote)));
						SocketPtr remotePtr(remote);
						Reactor::instance()->addWatch(remotePtr);
						message->writeTo(remotePtr);
					}
					//socket->write("success", 5);
				}
			}
			else {
				socket->write("error", 5); // gotta read what to send in case of bad requests
			}
		}
	}
}


void DNSService::onRemoteReadyRead(UDPSocket *remoteSocket,
								   const sockaddr_in &clientAddr)
{
	cout << "remote ready read\n";
	unsigned char buf[DNSMaxMessageSize];
	int cnt;
	if ((cnt = remoteSocket->read(buf, DNSMaxMessageSize)) != -1) {
		((UDPSocket*)socket.get())->connectTo(clientAddr);
		socket->write(buf, cnt);
		Reactor::instance()->removeWatch(remoteSocket->fd());
	}
}
