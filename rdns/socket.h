#ifndef LSOCKET_H
#define LSOCKET_H

#include <string>
#include <netinet/in.h>
#include "shared_ptr.h"
#include "iodevice.h"

namespace rdns
{

class Socket;
typedef shared_ptr<Socket> SocketPtr;

class Socket : public IODevice
{
public:
	Socket() : IODevice() { };
	Socket(const char *ip, unsigned int port);
	virtual ~Socket();
	bool isValid() const;
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
