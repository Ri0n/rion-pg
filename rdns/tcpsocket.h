#ifndef LTCPSOCKET_H
#define LTCPSOCKET_H

#include "socket.h"

namespace rdns
{

class TCPSocket : public Socket
{
public:
	TCPSocket(const char *ip, unsigned int port);
	TCPSocket(int fd, sockaddr_in addr);
	bool listen();
	bool isStreamed() const;
	SocketPtr accept();
};

} // namespace rdns
#endif // LTCPSOCKET_H
