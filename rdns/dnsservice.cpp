#include <cstdio>
#include <iomanip>
#include <iostream>
#include "dnsservice.h"
#include "udptransaction.h"
#include "rr.h"

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

DNSService::DNSService(SocketPtr socket)
	: Service(socket)
{
	socket->setReadyReadHandler(CallbackPtr(
										new DNSServiceReadyReadCallback(this)));
}

void DNSService::onReadyRead()
{
	unsigned char buf[1024];
	SocketPtr transaction = socket->accept();
	ssize_t cnt = transaction->read(buf, 1024);
	if (cnt == -1) {
		cout << "hm\n";
		perror("failed to read data from udp");
	}
	else {

		cout << "received " << cnt << " bytes: " << std::hex << std::setfill('0');
		for (int i = 0; i < cnt; i++) {
			cout << std::setw(2) << (int)buf[i] << " ";
		}
		cout << std::endl;
	}
}

