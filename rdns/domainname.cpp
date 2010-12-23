#include "domainname.h"

namespace rdns
{

DomainName::DomainName()
{
}

size_t DomainName::fromByteArray(const unsigned char *buf, size_t count)
{
	_name.reserve(count > 255 ? 255 : count); // 255 name limit by rfc
	unsigned int c = 0;
	while (count - c > 0 && buf[c] && c + buf[c] + 1 < count) {
		_name += std::string((const char*)&buf[c + 1], (size_t)buf[c]);
		c += buf[c] + 1;
		if (buf[c]) {
			_name += '.';
		}
	}
	return c;
}

} // namespace rdns
