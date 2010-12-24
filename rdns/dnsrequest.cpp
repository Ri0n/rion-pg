#include <time.h>
#include "dnsrequest.h"

namespace rdns
{

DNSRequest::DNSRequest(uint16_t id, const sockaddr_in &client)
	: _id(id)
	, _clientAddr(client)
{
	clock_gettime(CLOCK_MONOTONIC, &_time);
}

bool DNSRequest::isExpired() const
{
	timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	time_t secDelta = now.tv_nsec - _time.tv_sec;
	return secDelta > DNSRequestTTL.tv_sec || (secDelta == DNSRequestTTL.tv_sec
						  && now.tv_nsec - _time.tv_sec > DNSRequestTTL.tv_sec);
}

} // namespace rdns
