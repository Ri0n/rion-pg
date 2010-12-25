#ifndef DOMAINNAME_H
#define DOMAINNAME_H

#include <string>

namespace rdns
{

class DomainName
{
public:
    DomainName();

	size_t fromByteArray(const unsigned char *buf, size_t count);
	const std::string &toString() const { return _name; }

private:
	std::string _name;
};

} // namespace rdns
#endif // DOMAINNAME_H
