#include "uri.h"
#include "string.h"
#include <sstream>

namespace rdns
{

Uri::Uri()
	: _port(0)
{

}

Uri::Uri(const char *uri)
	: _port(0)
{
	std::string str(uri);
	_scheme = str.substr(0, str.find(':'));
	if (_scheme.size()) {
		int ipPos = _scheme.size() + 3;
		size_t dp = str.find(':', ipPos);
		if (dp != std::string::npos) {
			std::istringstream portStr(str.substr(dp + 1));
			portStr >> _port;
			_host = str.substr(ipPos, dp - ipPos);
		}
		else {
			_host = str.substr(ipPos);
		}
	}
}

void Uri::setScheme(const char *scheme)
{
	_scheme = std::string(scheme);
}

void Uri::setHost(const char *host)
{
	_host = std::string(host);
}

void Uri::setPort(uint16_t port)
{
	_port = port;
}

std::string Uri::toString() const
{
	std::ostringstream stream;
	stream << _scheme << "://" << _host << ':' << _port;
	return stream.str();
}

size_t Uri::toString(char *buf, size_t size) const
{
	std::string str = toString();
	size = size > str.size() ? str.size() : size;
	strncpy(buf, str.c_str(), size);
	return size;
}


} // namespace rdns
