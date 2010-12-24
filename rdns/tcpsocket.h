#ifndef LTCPSOCKET_H
#define LTCPSOCKET_H

#include "socket.h"

namespace rdns
{

class TCPSocket : public Socket
{
public:
	TCPSocket(const char *ip = "0.0.0.0", unsigned int port = 0);
	TCPSocket(int fd, const sockaddr_in &addr);
	bool listen();
	bool isStreamed() const;
	bool accept(SocketPtr &client);
};

} // namespace rdns
#endif // LTCPSOCKET_H
