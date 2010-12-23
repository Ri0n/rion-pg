#ifndef URI_H
#define URI_H

#include <string>
#include <stdint.h>

namespace rdns
{

class Uri
{
public:
    Uri();
	Uri(const char *uri);

	inline std::string scheme() const { return _scheme; }
	inline std::string host() const { return _host; }
	inline uint16_t port() const { return _port; }

	inline void setScheme(const char *);
	inline void setHost(const char *);
	inline void setPort(uint16_t);

	std::string toString() const;
	size_t toString(char *buf, size_t size) const;

private:
	std::string _scheme;
	std::string _host;
	uint16_t _port;
};

} // namespace rdns

#endif // URI_H
