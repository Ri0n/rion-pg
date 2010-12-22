#ifndef LUDPSOCKET_H
#define LUDPSOCKET_H

#include "socket.h"

namespace rdns
{

class UDPSocket : public Socket
{
public:
	UDPSocket(const char *ip, unsigned int port = 0);
	UDPSocket(int fd, const sockaddr_in &addr);
	bool listen();
	ssize_t write(const void *buf, size_t count) const;
	bool accept(SocketPtr &client);
	bool connectTo(const sockaddr_in &addr);
	bool connectTo(const char *ip, unsigned int port);
	sockaddr_in &clientAddress() { return clientAddr; }

private:
	sockaddr_in clientAddr;
};

} // namespace rdns
#endif // LUDPSOCKET_H
