#ifndef UDPECHOSERVICE_H
#define UDPECHOSERVICE_H

#include "service.h"

namespace rdns
{

class UDPEchoService : public Service
{
public:
	UDPEchoService(IODevicePtr);
	void onReadyRead();
};

} // namespace rdns

#endif // UDPECHOSERVICE_H
