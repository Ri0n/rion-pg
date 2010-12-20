#ifndef DAEMON_H
#define DAEMON_H

#include <string>
#include <map>
#include "shared_ptr.h"
#include "socket.h"

namespace rdns
{

class Daemon
{
public:
	Daemon();
	SocketPtr addListener(const char *protoName, const char *ip, unsigned int port);
	bool listen();

private:
	std::map<int, SocketPtr> _listeners;
	std::map<int, SocketPtr> _clients;
	int _listenEpollFd;
};

} // namespace rdns
#endif // DAEMON_H
