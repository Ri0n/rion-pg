#include <cstdio>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "iodevice.h"

using namespace rdns;
using std::cout;
using std::cerr;
using std::endl;

IODevice::IODevice()
	: _fd(-1)
	, _readyRead(false)
	, _readOnly(false)
{

}

IODevice::IODevice(int fd)
	: _fd(fd)
	, _readyRead(false)
	, _readOnly(false)
{

}

IODevice::~IODevice()
{
	cout << "destroying io device\n";
	close();
}

int IODevice::fd() const
{
	return _fd;
}

bool IODevice::isValid() const
{
	return _fd != -1;
}

ssize_t IODevice::write(const void *buf, size_t count) const
{
	if (_readOnly) {
		cerr << "Failed to write to read only device\n";
		return -1;
	}
	ssize_t bw = ::write(_fd, buf, count);
	if (bw == -1) {
		perror("Failed to read");
	}
	else {
		dump("written: ", (unsigned char*)buf, bw);
	}
	return bw;
}

ssize_t IODevice::read(void *buf, size_t count)
{
	//cout << "reading from " << _fd << endl;
	ssize_t br = ::read(_fd, (unsigned char*)buf, count);
	if (br == -1) {
		if (errno == EAGAIN) {
			_readyRead = false;
		}
		else {
			perror("Failed to read");
		}
	}
	else {
		dump("read: ", (unsigned char*)buf, br);
	}
	return br;
}

void IODevice::close()
{
	if (_fd != -1) {
		if (::close(_fd) == -1) {
			perror("Failed to close");
			if (errno != EBADF) {
				return;
			}
		}
		_fd = -1;
	}
}

int IODevice::setBlocking(bool state)
{
	int flags;

	if ((flags = fcntl(_fd, F_GETFL, 0)) == -1) {
		flags = 0;
	}
	return fcntl(_fd, F_SETFL, state? flags & ~O_NONBLOCK : flags | O_NONBLOCK);
}


void IODevice::setReadyRead()
{
	_readyRead = true;
	if (_readyReadCallback.get()) {
		_readyReadCallback->call();
	}
}

void IODevice::setReadyReadHandler(CallbackPtr cb)
{
	_readyReadCallback = cb;
}

void IODevice::setReadyOnly(bool state)
{
	_readOnly = state;
}

void IODevice::setPersistent(bool state)
{
	_persistent = state;
}

std::string IODevice::toString() const
{
	std::ostringstream stream;
	stream << "IODevice(fd=" << _fd << ")";
	return stream.str();
}

void IODevice::dump(const char *prefix, const unsigned char *buf, size_t count)
{
	cout << prefix << count << " bytes: " << std::hex << std::setfill('0');
	for (size_t i = 0; i < count; i++) {
		cout << std::setw(2) << (int)buf[i] << " ";
	}
	cout << std::endl;
}
