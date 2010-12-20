#ifndef DNSSERVICE_H
#define DNSSERVICE_H

#include "service.h"

namespace rdns
{

class DNSService : public Service
{
public:
	DNSService(SocketPtr);
	void onReadyRead();
};

} // namespace rdns

#endif // DNSSERVICE_H
