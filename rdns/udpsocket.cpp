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
	: Socket()
{
	if (makeAddress(ip, port, &_addr)) {
		_fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
		if (_fd == -1) {
			perror("Failed to create socket");
		}
	}
}

UDPSocket::UDPSocket(int fd, const sockaddr_in &addr)
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

ssize_t UDPSocket::write(const void *buf, size_t count) const
{
	dump("sending datagram: ", (unsigned char *)buf, count);
	return sendto(_fd, buf, count, 0, (sockaddr*) &_endPoint, sizeof(_endPoint));
}

// If param `client` is self then self socket will be connected to client.
// Otherwise new UDP socket will be created and "connected" to client,
// `client` param socket will be replaced with new one, so bear this in mind.
bool UDPSocket::accept(SocketPtr &client)
{
	socklen_t addrLen = sizeof(_addr);
	sockaddr_in addr;
	//cout << "recvfrom " << source->fd() << endl;
	if (recvfrom(_fd, 0, 0, MSG_PEEK, (sockaddr*)&addr, &addrLen) == -1) {
		perror("Failed to retrieve UDP request info");
	}
	else {
		if (client.get() == this) { // connect self to client
			_endPoint = addr;
			return true;
		}
		else { // make new socket and store in client
			int fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
			if (fd != -1) {
				client = SocketPtr(new UDPSocket(fd, addr));
				if (client->connect()) {
					cout << "Accepted new UDP connection from " << client->toString() << endl;
					return true;
				}
			}
			else {
				perror("Failed to create socket");
			}
		}
	}
	return false;
}

void UDPSocket::setEndPoint(const sockaddr_in &addr)
{
	_endPoint = addr;
}

bool UDPSocket::setEndPoint(const char *ip, unsigned int port)
{
	return makeAddress(ip, port, &_endPoint);
}

bool UDPSocket::connectToEndPoint()
{
	return connect(_endPoint);
}
