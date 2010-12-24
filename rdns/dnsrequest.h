#ifndef DNSREQUEST_H
#define DNSREQUEST_H

#include <netinet/in.h>
//#include <stdint.h>

namespace rdns
{

class DNSRequest
{
public:
	DNSRequest(uint16_t id, const sockaddr_in &client);
	inline uint16_t id() const { return _id; }
	inline sockaddr_in clientAddress() const { return _clientAddr; }

private:
	uint16_t _id;
	sockaddr_in _clientAddr;
};

} // namespace rdns

#endif // DNSREQUEST_H
