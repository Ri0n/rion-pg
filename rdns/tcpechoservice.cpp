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
	TCPEchoServiceResponder(SocketPtr socket)
		: socket(socket) { }

	void call() {
		std::string answer = "You send: ";
		char buf[10124];
		int cnt = socket->read(buf, 1024);
		answer += std::string(buf).substr(0, cnt); // lets consider that was a string =)
		socket->write(answer.c_str(), answer.size());
	}

private:
	SocketPtr socket;
};


TCPEchoService::TCPEchoService(SocketPtr socket)
	: Service(socket)
{
	socket->setReadyReadHandler(CallbackPtr(
								new TCPEchoServiceNewConnectionCallback(this)));
}

void TCPEchoService::acceptConnection()
{
	SocketPtr client = socket->accept();
	if (client->isValid()) {
		cout << "new client tcp connection\n";
		Reactor::instance()->addWatch(client);
		client->setReadyReadHandler(CallbackPtr(
									new TCPEchoServiceResponder(client)));
	}
}
