#ifndef DNSMESSAGE_H
#define DNSMESSAGE_H

#include <cstdio>
#include <string>
#include <stdint.h>
#include "domainname.h"
#include "socket.h"

namespace rdns
{

static const size_t DNSMaxMessageSize = 512; //rfc says udp dns packet must be not greater than 512 bytes in size.


//15  14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
//+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
//|RA|   Z    |   RCODE   |QR|   Opcode  |AA|TC|RD|

union DNSMessageData {
	struct {
		uint16_t id;
		uint16_t flags;
		uint16_t qdcount;
		uint16_t ancount;
		uint16_t nscount;
		uint16_t arcount;
	} header;
	unsigned char buffer[DNSMaxMessageSize];
};

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

class DNSMessage;
typedef shared_ptr<DNSMessage> DNSMessagePtr;

class DNSMessage
{
public:
	enum OpCode {
		OpCodeQuery = 0,
		OpCodeIQuery = 1,
		OpCodeStatus = 2
	};

	DNSMessage();
	static DNSMessagePtr fromByteArray(unsigned char *buf, size_t count);
	bool writeTo(const SocketPtr &);

	std::string toString() const; // just to dump smth in caller

	inline uint16_t id() const { return ntohs(data.header.id); }
	inline uint16_t flags() const;
	inline uint16_t qdCount() const;
	inline uint16_t anCount() const;
	inline uint16_t nsCount() const;
	inline uint16_t arCount() const;
	inline bool isResponse() const;
	void setResponseBit(bool);
	//inline void setResponse() const;
	inline uint8_t opCode() const;
	std::string domainName() const;

private:
	DNSMessageData data;
	size_t _size;
};

} // namespace rdns

#endif // DNSMESSAGE_H
