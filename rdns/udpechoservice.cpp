#include <iomanip>
#include <iostream>
#include "udpechoservice.h"

using namespace rdns;
using std::cout;

class UDPEchoServiceReadyReadCallback : public Callback
{
public:
	UDPEchoServiceReadyReadCallback(UDPEchoService *service)
		: service(service)
	{ }

	void call() {
		service->onReadyRead();
	}

private:
	UDPEchoService *service;
};

UDPEchoService::UDPEchoService(SocketPtr socket)
	: Service(socket)
{
	socket->setReadyReadHandler(CallbackPtr(
								new UDPEchoServiceReadyReadCallback(this)));
}

void UDPEchoService::onReadyRead()
{
	unsigned char buf[1024];
	SocketPtr client = socket->accept();
	ssize_t cnt = socket->read(buf, 1024);
	if (cnt == -1) {
		cout << "hm\n";
	}
	else {
		cout << "received " << cnt << " bytes: " << std::hex << std::setfill('0');
		for (int i = 0; i < cnt; i++) {
			cout << std::setw(2) << (int)buf[i] << " ";
		}
		cout << std::endl;
		client->write("hello\n", 6);
	}
}
