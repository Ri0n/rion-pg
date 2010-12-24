#include "dnsrequest.h"

namespace rdns
{

DNSRequest::DNSRequest(uint16_t id, const sockaddr_in &client)
	: _id(id)
	, _clientAddr(client)
{

}

} // namespace rdns
