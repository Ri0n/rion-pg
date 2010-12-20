#include <cstdio>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include "udptransaction.h"

namespace rdns {
using std::cout;
using std::endl;

UDPTransaction::UDPTransaction(UDPSocket *source)
	: Socket()
	, source(source)
{
	socklen_t addrLen = sizeof(_addr);
	//cout << "recvfrom " << source->fd() << endl;
	if (recvfrom(source->fd(), 0, 0, MSG_PEEK, (sockaddr*)&_addr, &addrLen) == -1) {
		perror("Failed to retrieve UDP request info");
	}
	else {
		_fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK,
							getprotobyname("UDP")->p_proto);
		if (_fd == -1) {
			perror("Failed to create socket");
		}
	}
}

ssize_t UDPTransaction::read(void *buf, size_t count)
{
	return source->read(buf, count);
}

} // namespace rdns
