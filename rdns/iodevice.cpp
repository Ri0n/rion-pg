#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include "iodevice.h"

using namespace rdns;
using std::cout;
using std::cerr;
using std::endl;

IODevice::IODevice()
	: _fd(-1)
	, _readyRead(false)
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
	return br;
}

void IODevice::close()
{
	if (_fd != -1) {
		::close(_fd);
	}
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
