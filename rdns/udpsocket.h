#ifndef LUDPSOCKET_H
#define LUDPSOCKET_H

#include "socket.h"

namespace rdns
{

class UDPSocket : public Socket
{
public:
	UDPSocket(const char *ip, unsigned int port);
	UDPSocket(int fd, sockaddr_in addr);
	bool listen();
	SocketPtr accept();
};

} // namespace rdns
#endif // LUDPSOCKET_H
