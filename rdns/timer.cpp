#include <cstdio>
#include <iostream>
#include <sys/timerfd.h>
#include <stdint.h>
#include <string.h>
#include "timer.h"


namespace rdns {
using std::cout;

Timer::Timer()
	: IODevice()
	, _active(false)
{
	memset((char *)&_value, 0, sizeof(_value));
	_fd = timerfd_create(CLOCK_MONOTONIC, 0);
	if (_fd != -1) {
		setBlocking(false);
	}
}

void Timer::setTimeout(time_t sec, long nsec)
{
	_value.it_value.tv_sec = sec;
	_value.it_value.tv_nsec = nsec;
}

void Timer::setInterval(time_t sec, long nsec)
{
	_value.it_interval.tv_sec = sec;
	_value.it_interval.tv_nsec = nsec;
}

void Timer::start()
{
	_active = false;
	if (timerfd_settime(_fd, 0, &_value, NULL) == -1) {
		perror("timerfd_settime:start");
	}
	else if (_value.it_value.tv_sec || _value.it_value.tv_nsec) {
		_active = true;
		cout << "Timer started\n";
	}
}

void Timer::stop()
{
	itimerspec sv;
	sv.it_value.tv_sec = 0;
	sv.it_value.tv_nsec = 0;
	if (timerfd_settime(_fd, 0, &sv, NULL) == -1) {
		perror("timerfd_settime:stop");
	}
	else {
		cout << "Timer stopped\n";
	}
}

void Timer::setReadyRead()
{
	uint64_t exp;
	if (read(&exp, sizeof(uint64_t)) != sizeof(uint64_t)) {
		perror("Timer::read");
	}
	else {
		if (_readyReadCallback.get()) {
			_readyReadCallback->call();
		}
		else {
			cout << "unhandled timer event\n";
		}
	}
}


} // namespace rdns
