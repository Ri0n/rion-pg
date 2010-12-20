#ifndef TCPECHOSERVICE_H
#define TCPECHOSERVICE_H

#include "service.h"

namespace rdns
{

class TCPEchoService : public Service
{
public:
	TCPEchoService(SocketPtr);
};

} // namespace rdns

#endif // TCPECHOSERVICE_H
