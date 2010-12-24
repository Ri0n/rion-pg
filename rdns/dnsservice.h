#ifndef DNSSERVICE_H
#define DNSSERVICE_H

#include <map>
#include "service.h"
#include "udpsocket.h"
#include "shared_ptr.h"
#include "dnsrequest.h"

namespace rdns
{

typedef shared_ptr<DNSRequest> DNSRequestPtr;
typedef std::map<uint16_t, DNSRequestPtr> DNSRequestMap;
typedef std::pair<uint16_t, DNSRequestPtr> DNSRequestItem;
typedef DNSRequestMap::iterator DNSRequestIterator;

class DNSService : public Service
{
public:
	DNSService(SocketPtr);
	bool setRemoteDns(const char *host, uint16_t port);
	void onReadyRead();
	void onRemoteReadyRead();

private:
	SocketPtr _remoteDns;
	DNSRequestMap _requests;
};

} // namespace rdns

#endif // DNSSERVICE_H
