#ifndef SERVICE_H
#define SERVICE_H

#include "socket.h"
#include "shared_ptr.h"

namespace rdns
{

class Service;
typedef shared_ptr<Service> ServicePtr;

class Service
{
public:
	Service(SocketPtr);
	virtual ~Service() { }

protected:
	SocketPtr socket;
};

} // namespace rdns

#endif // SERVICE_H
