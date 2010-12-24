#include <iostream>
#include "tcpechoservice.h"
#include "reactor.h"

using namespace rdns;
using std::cerr;
using std::cout;

class TCPEchoServiceNewConnectionCallback : public Callback
{
public:
	TCPEchoServiceNewConnectionCallback(TCPEchoService *service)
		: service(service) { }

	void call() { service->acceptConnection(); }

private:
	TCPEchoService *service;
};

class TCPEchoServiceResponder : public Callback
{
public:
	TCPEchoServiceResponder(IODevicePtr socket)
		: socket((Socket*)socket.get()) { } // don't allow recursive links to allow "gc" to its job =)

	void call() {
		char buf[10124];
		int cnt = socket->read(buf, 1024);
		if (cnt) {
			// lets consider that was a string =)
			std::string answer = std::string("You sent: ") + std::string(buf, cnt);
			socket->write(answer.c_str(), answer.size());
		}
		else {
			Reactor::instance()->removeWatch(socket->fd());
		}
	}

private:
	Socket* socket;
};


TCPEchoService::TCPEchoService(IODevicePtr socket)
	: Service(socket)
{
	socket->setReadyReadHandler(CallbackPtr(
								new TCPEchoServiceNewConnectionCallback(this)));
}

void TCPEchoService::acceptConnection()
{
	IODevicePtr client;
	if (((Socket*)socket.get())->accept(client) && client->isValid()) {
		cout << "new client tcp connection\n";
		Reactor::instance()->addWatch(client);
		client->setReadyReadHandler(CallbackPtr(
									new TCPEchoServiceResponder(client)));
	}
}
