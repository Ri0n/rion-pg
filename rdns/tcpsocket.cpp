#include <iostream>
#include <cstdio>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <iostream>
#include <cstdio>
#include <string.h>
#include <errno.h>

#include "tcpsocket.h"

using namespace rdns;
using std::cerr;
using std::cout;
using std::endl;
using std::string;

TCPSocket::TCPSocket(const char *ip, unsigned int port)
	: Socket(ip, port)
{
	if (_addr.sin_port) { // 0 in case of error
		_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK,
							getprotobyname("TCP")->p_proto);
		if (_fd == -1) {
			perror("Failed to create socket");
		}
	}
}

TCPSocket::TCPSocket(int fd, sockaddr *addr)
	: Socket()
{
	_fd = fd;
	_addr = *((sockaddr_in *)addr);
}

bool TCPSocket::listen()
{
	if (::bind(_fd, (struct sockaddr *) &_addr, sizeof(sockaddr_in))) {
		string err(strerror(errno));
		cerr << "Failed to bind adderess tcp://" << toString() << ": " << err << "\n";
		return false;
	}
	cout << "socket tcp://" << toString() <<" binded\n";

	if (::listen(_fd, 10) == -1) {
		perror("Can't listen");
		return false;
	}
	return true;
}

bool TCPSocket::isStreamed() const
{
	return true;
}

SocketPtr TCPSocket::accept()
{
	sockaddr addr;
	socklen_t addrlen = sizeof(addr);
	int clientSock = ::accept(_fd, &addr, &addrlen);
	if (clientSock == -1 || addrlen == 0) {
		perror("Failed to accept connection");
		return SocketPtr();
	}

	SocketPtr sock(new TCPSocket(clientSock, &addr));
	sock->setBlocking(false);

	cout << "Accepted connection from: " << sock->toString() << endl;
	return sock;
}
