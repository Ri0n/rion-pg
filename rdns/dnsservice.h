#ifndef DNSSERVICE_H
#define DNSSERVICE_H

#include "service.h"
#include "udpsocket.h"

namespace rdns
{

class DNSService : public Service
{
public:
	DNSService(SocketPtr);
	void setRemoteDns(const char *host, uint16_t port);
	void onReadyRead();
	void onRemoteReadyRead(UDPSocket *remoteSocket, const sockaddr_in &clientAddr);

private:
	std::string _remoteDnsIp;
	uint16_t _remoteDnsPort;
};

} // namespace rdns

#endif // DNSSERVICE_H
