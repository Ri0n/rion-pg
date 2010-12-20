#include "domainname.h"

namespace rdns
{

DomainName::DomainName()
{
}

size_t DomainName::fromByteArray(unsigned char *buf, size_t count)
{
	_name = (char *)buf;
	int c = 0;
	while (count - c > 0 && buf[c]) {
		c += buf[c];
		_name[c] = '.';
	}
	return c;
}

} // namespace rdns
