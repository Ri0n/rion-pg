#ifndef UDPTRANSACTION_H
#define UDPTRANSACTION_H

#include "udpsocket.h"

namespace rdns {

class UDPTransaction : public Socket
{
public:
	UDPTransaction(UDPSocket *source);
	ssize_t read(void *buf, size_t count);

private:
	UDPSocket *source;
};

} // namespace rdns

#endif // UDPTRANSACTION_H
