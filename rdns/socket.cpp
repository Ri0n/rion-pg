#include <iostream>
#include <cstdio>
#include <sstream>
#include <arpa/inet.h>
#include "socket.h"
#include "tcpsocket.h"
#include "udpsocket.h"

using namespace rdns;
using std::cout;
using std::cerr;
using std::endl;
using std::string;

Socket::Socket(int fd, const sockaddr_in &addr)
	: IODevice(fd)
	, _addr(addr)
{

}

bool Socket::makeAddress(const char *ip, unsigned int port, sockaddr_in *addr)
{
	if (inet_pton(AF_INET, ip, &addr->sin_addr) != 1) {
		cerr << "Failed to convert IP address: " << ip << endl;
		return false;
	}
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	return true;
}

ssize_t Socket::write(const void *buf, size_t count) const
{
	cout << "writing to socket: " << toString() << endl;
	return IODevice::write(buf, count);
}

ssize_t Socket::read(void *buf, size_t count)
{
	cout << "reading from socket: " << toString() << endl;
	return IODevice::read(buf, count);
}

bool Socket::connect()
{
	return connect(_addr);
}

bool Socket::connect(const sockaddr_in &addr)
{
	if (::connect(_fd, (sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("Failed to connect");
		return false;
	}
	return true;
}

bool Socket::listen()
{
	return true;
}

bool Socket::isStreamed() const
{
	return false;
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

IODevicePtr Socket::factory(const char *protoName, const char *ip,
							unsigned int port)
{
	string name(protoName);
	if (name == "UDP") {
		return IODevicePtr(new UDPSocket(ip, port));
	} else if (name == "TCP") {
		return IODevicePtr(new TCPSocket(ip, port));
	}
	return IODevicePtr();
}
