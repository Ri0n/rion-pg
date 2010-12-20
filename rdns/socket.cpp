#include <iostream>
#include <sstream>
#include <arpa/inet.h>
#include <fcntl.h>
#include "socket.h"
#include "tcpsocket.h"
#include "udpsocket.h"

using namespace rdns;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

Socket::Socket(const char *ip, unsigned int port)
	: IODevice()
{
	if (inet_pton(AF_INET, ip, &_addr.sin_addr) != 1) {
		cerr << "Failed to convert IP address: " << ip << endl;
		_addr.sin_port = 0; // lets consider 0 is error here
	}
	else {
		_addr.sin_family = AF_INET;
		_addr.sin_port = htons(port);
	}
}

Socket::~Socket()
{
	cout << "socket " << toString() <<" destroyed\n";
	close(_fd);
}

bool Socket::isValid() const
{
	return IODevice::isValid() && _addr.sin_port != 0;
}

bool Socket::listen()
{
	return true;
}

bool Socket::isStreamed() const
{
	return false;
}

SocketPtr Socket::accept()
{
	return SocketPtr(); // stub
}

string Socket::toString() const
{
	char buf[16]; // 4 * 3 + 3 + 1 for ipv4
	std::ostringstream stream;
	if (inet_ntop(AF_INET, &_addr.sin_addr, buf, 16)) {
		stream << buf;
	}
	else {
		stream << "BADADDRESS";
	}
	stream << ':' << ntohs(_addr.sin_port);
	return stream.str();
}

int Socket::setBlocking(bool state)
{
	int flags;

	if ((flags = fcntl(_fd, F_GETFL, 0)) == -1) {
		flags = 0;
	}
	return fcntl(_fd, F_SETFL, state? flags & ~O_NONBLOCK : flags | O_NONBLOCK);
}


SocketPtr Socket::factory(const char *protoName, const char *ip,
							unsigned int port)
{
	string name(protoName);
	if (name == "UDP") {
		return SocketPtr(new UDPSocket(ip, port));
	} else if (name == "TCP") {
		return SocketPtr(new TCPSocket(ip, port));
	}
	return SocketPtr();
}
