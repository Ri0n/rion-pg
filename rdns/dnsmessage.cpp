#include <iostream>
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

DNSMessage* DNSMessage::fromByteArray(unsigned char *buf, size_t count)
{
	if (count < 12) { // bad header
		cerr << "bad dns header\n";
		return 0;
	}
	DNSMessage *msg = new DNSMessage;
	msg->_id = *((uint16_t *)buf);
	msg->_flags = *((uint16_t *)&buf[2]);
	msg->_qdcount = *((uint16_t *)&buf[4]);
	msg->_ancount = *((uint16_t *)&buf[6]);
	msg->_nscount = *((uint16_t *)&buf[8]);
	msg->_arcount = *((uint16_t *)&buf[10]);

	msg->_question.fromByteArray(&buf[12], count - 12);

	return msg;
}

void DNSMessage::setId(uint16_t id)
{
	_id = id;
}

void DNSMessage::setFlags(uint16_t flags)
{
	_flags = flags;
}
