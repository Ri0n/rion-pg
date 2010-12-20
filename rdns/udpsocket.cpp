#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <iostream>
#include <cstdio>
#include <string.h>
#include <errno.h>
#include "udpsocket.h"
#include "udptransaction.h"

using namespace rdns;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

UDPSocket::UDPSocket(const char *ip, unsigned int port)
	: Socket(ip, port)
{
	if (_addr.sin_port) { // 0 in case of error
		_fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK,
							getprotobyname("UDP")->p_proto);
		if (_fd == -1) {
			perror("Failed to create socket");
		}
	}
}

bool UDPSocket::listen()
{
	if (::bind(_fd, (struct sockaddr *) &_addr, sizeof(sockaddr_in))) {
		string err(strerror(errno));
		cerr << "Failed to bind adderess udp://" << toString() << ": " << err << "\n";
		return false;
	}

	cout << "socket udp://" << toString() <<" binded\n";
	return true;
}

SocketPtr UDPSocket::accept()
{
	SocketPtr trans(new UDPTransaction(this));

	cout << "new udp transaction with: " << trans->toString() << endl;
	return trans;
}

