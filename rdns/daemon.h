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
	IODevicePtr addListener(const char *protoName, const char *ip, unsigned int port);
	bool listen();

private:
	IODeviceMap _listeners;
};

} // namespace rdns
#endif // DAEMON_H
