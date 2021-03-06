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

UDPEchoService::UDPEchoService(IODevicePtr socket)
	: Service(socket)
{
	socket->setReadyReadHandler(CallbackPtr(
								new UDPEchoServiceReadyReadCallback(this)));
}

void UDPEchoService::onReadyRead()
{
	char buf[1024];
	if (((Socket*)socket.get())->accept(socket)) {
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
			std::string repl = std::string("You sent: ") + std::string(buf, cnt);
			socket->write(repl.c_str(), repl.size());
		}
	}
}
