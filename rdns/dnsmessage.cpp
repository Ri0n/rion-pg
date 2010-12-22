#include <iostream>
#include <sstream>
#include <cstring>
#include "dnsmessage.h"

using namespace rdns;
using std::cerr;


DNSQuestion::DNSQuestion()
{

}

void DNSQuestion::fromByteArray(unsigned char *buf, size_t count)
{
	size_t ns = _domainName.fromByteArray(buf, count);
	_qtype = *((uint16_t *)&buf[ns + 1]);
	_qclass = *((uint16_t *)&buf[ns + 3]);
}

//----------------------------------------
// DNSMessage
//----------------------------------------
DNSMessage::DNSMessage()
{
}

DNSMessagePtr DNSMessage::fromByteArray(unsigned char *buf, size_t count)
{
	if (count < 12) { // bad header
		cerr << "dns header too small\n";
		return DNSMessagePtr();
	}
	if (count > DNSMaxMessageSize) {
		cerr << "dns message too big\n";
		return DNSMessagePtr();
	}
	DNSMessagePtr msg(new DNSMessage);
	std::memcpy(msg->data.buffer, buf, count);
	msg->_size = count;
	return msg;
}

bool DNSMessage::writeTo(const SocketPtr &socket)
{
	return socket->write(data.buffer, _size);
}

std::string DNSMessage::toString() const // just to dump smth in caller
{
	std::ostringstream stream;
	stream << "id=" << id() << " ";
	stream << "QR=" << (isResponse()?"response":"query") << " ";
	uint8_t oc = opCode();
	stream << "OPCODE=" << (oc == OpCodeQuery?"QUERY":
						   (oc == OpCodeIQuery?"IQUERY":
						   (oc == OpCodeStatus?"STATUS":"UNRECOGNIZED"))) << " ";
	stream << "qdcount=" << qdCount() << " ";
	stream << "ancount=" << anCount() << " ";
	stream << "nscount=" << nsCount() << " ";
	stream << "arcount=" << arCount() << " ";
	stream << "domain=" << domainName() << " ";
	return stream.str();
}

uint16_t DNSMessage::id() const
{
	return ntohs(data.header.id);
}

uint16_t DNSMessage::flags() const
{
	return data.header.flags;
}

uint16_t DNSMessage::qdCount() const
{
	return ntohs(data.header.qdcount);
}

uint16_t DNSMessage::anCount() const
{
	return ntohs(data.header.ancount);
}

uint16_t DNSMessage::nsCount() const
{
	return ntohs(data.header.nscount);
}

uint16_t DNSMessage::arCount() const
{
	return ntohs(data.header.arcount);
}

bool DNSMessage::isResponse() const
{
	return data.header.flags & (1 << 7);
}

uint8_t DNSMessage::opCode() const
{
	return (data.header.flags >> 3) & 0xf;
}

void DNSMessage::setResponseBit(bool state)
{
	uint16_t b = (1 << 7);
	if (state) {
		data.header.flags |= b;
	}
	else {
		data.header.flags &= ~b;
	}
}

std::string DNSMessage::domainName() const
{
	DomainName dn;
	dn.fromByteArray(&data.buffer[sizeof(data.header)], DNSMaxMessageSize); // big size to be sure )
	return dn.toString();
}
