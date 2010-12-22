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
	IODevice(int fd);
	virtual ~IODevice();
	int fd() const;
	virtual bool isValid() const;
	virtual ssize_t write(const void *buf, size_t count) const;
	virtual ssize_t read(void *buf, size_t count);
	virtual void close();
	void setReadyRead();
	void setReadyReadHandler(CallbackPtr);
	void setReadyOnly(bool state = true);

	static void dump(const char *prefix, const unsigned char *buf, size_t count);

protected:
	int _fd;
	bool _readyRead;
	bool _readOnly;
	CallbackPtr _readyReadCallback;
};

} // namespace rdns

#endif // IODEVICE_H
