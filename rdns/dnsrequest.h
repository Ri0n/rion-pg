#ifndef DNSREQUEST_H
#define DNSREQUEST_H

#include <netinet/in.h>

namespace rdns
{

const timespec DNSRequestTTL = {3, 0};

class DNSRequest
{
public:
	DNSRequest(uint16_t id, const sockaddr_in &client);
	inline uint16_t id() const { return _id; }
	inline sockaddr_in clientAddress() const { return _clientAddr; }
	bool isExpired() const;

private:
	uint16_t _id;
	sockaddr_in _clientAddr;
	timespec _time;
};

} // namespace rdns

#endif // DNSREQUEST_H
