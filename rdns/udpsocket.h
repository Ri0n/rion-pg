#ifndef LUDPSOCKET_H
#define LUDPSOCKET_H

#include "socket.h"

namespace rdns
{

class UDPSocket : public Socket
{
public:
	UDPSocket(const char *ip = "0.0.0.0", unsigned int port = 0);
	UDPSocket(int fd, const sockaddr_in &addr);
	bool listen();
	ssize_t write(const void *buf, size_t count) const;
	bool accept(IODevicePtr &client);
	void setEndPoint(const sockaddr_in &addr);
	bool setEndPoint(const char *ip, unsigned int port);
	bool connectToEndPoint();
	sockaddr_in &endPoint() { return _endPoint; }

private:
	sockaddr_in _endPoint;
};

} // namespace rdns
#endif // LUDPSOCKET_H
