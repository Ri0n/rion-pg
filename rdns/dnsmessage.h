#ifndef DNSMESSAGE_H
#define DNSMESSAGE_H

#include <cstdio>
#include <stdint.h>
#include "domainname.h"

namespace rdns
{

class DNSQuestion
{
public:
	DNSQuestion();
	void fromByteArray(unsigned char *buf, size_t count);

private:
	DomainName _domainName;
	uint16_t _qtype;
	uint16_t _qclass;
};

class DNSMessage
{
public:
    DNSMessage();
	static DNSMessage* fromByteArray(unsigned char *buf, size_t count);

	void setId(uint16_t);
	void setFlags(uint16_t);

private:
	uint16_t _id;
	uint16_t _flags;
	uint16_t _qdcount;
	uint16_t _ancount;
	uint16_t _nscount;
	uint16_t _arcount;
	DNSQuestion _question;
};

} // namespace rdns

#endif // DNSMESSAGE_H
