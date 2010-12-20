#ifndef IODEVICE_H
#define IODEVICE_H

#include <unistd.h>
#include "shared_ptr.h"
#include "functionoid.h"

namespace rdns
{

class IODevice
{
public:
    IODevice();
	virtual ~IODevice();
	int fd() const;
	virtual bool isValid() const;
	virtual ssize_t write(const void *buf, size_t count) const;
	virtual ssize_t read(void *buf, size_t count);
	void setReadyRead();
	void setReadyReadHandler(CallbackPtr);

protected:
	int _fd;
	bool _readyRead;
	CallbackPtr _readyReadCallback;
};

} // namespace rdns

#endif // IODEVICE_H
