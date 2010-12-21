#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <iostream>
#include <cstdio>
#include <string.h>
#include <errno.h>
#include "udpsocket.h"

using namespace rdns;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

UDPSocket::UDPSocket(const char *ip, unsigned int port)
	: Socket(ip, port)
{
	if (_addr.sin_port) { // 0 in case of error
		_fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
		if (_fd == -1) {
			perror("Failed to create socket");
		}
	}
}

UDPSocket::UDPSocket(int fd, sockaddr_in addr)
	: Socket(fd, addr)
{

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
	socklen_t addrLen = sizeof(_addr);
	sockaddr_in addr;
	int fd;
	//cout << "recvfrom " << source->fd() << endl;
	if (recvfrom(_fd, 0, 0, MSG_PEEK, (sockaddr*)&addr, &addrLen) == -1) {
		perror("Failed to retrieve UDP request info");
	}
	else {
		fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
		if (fd == -1) {
			perror("Failed to create socket");
		}
		else {
			SocketPtr sock(new UDPSocket(fd, addr));
			if (sock->connect()) {
				cout << "Accepted new UDP connection from " << sock->toString() << endl;
			}
			return sock;
		}
	}
	return SocketPtr();
}

