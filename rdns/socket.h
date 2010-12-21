#ifndef LSOCKET_H
#define LSOCKET_H

#include <string>
#include <map>
#include <netinet/in.h>
#include "shared_ptr.h"
#include "iodevice.h"

namespace rdns
{

class Socket;
typedef shared_ptr<Socket> SocketPtr;
typedef std::map<int, SocketPtr> SocketMap;
typedef std::pair<int,SocketPtr> SocketItem;
typedef SocketMap::iterator SocketIterator;

class Socket : public IODevice
{
public:
	Socket() : IODevice() { };
	Socket(const char *ip, unsigned int port);
	Socket(int fd, sockaddr_in addr);
	bool isValid() const;
	ssize_t write(const void *buf, size_t count) const;
	ssize_t read(void *buf, size_t count);
	virtual bool connect();
	virtual bool listen();
	virtual bool isStreamed() const;
	virtual SocketPtr accept();
	std::string toString() const;
	int setBlocking(bool state = true);

	static SocketPtr factory(const char *protoName, const char *ip,
							  unsigned int port);

protected:
	sockaddr_in _addr;
};

} // namespace rdns
#endif // LSOCKET_H
