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
	void onReadyRead();
	void onRemoteReadyRead(UDPSocket *remoteSocket, const sockaddr_in &clientAddr);
};

} // namespace rdns

#endif // DNSSERVICE_H
