#ifndef TIMER_H
#define TIMER_H

#include <time.h>
#include "iodevice.h"

namespace rdns {

class Timer : public IODevice
{
public:
    Timer();
	void setTimeout(time_t sec, long nsec = 0);
	void setInterval(time_t sec, long nsec = 0);
	void start();
	void stop();

	void setReadyRead();

private:
	itimerspec _value;
	bool _active;
};

} // namespace rdns

#endif // TIMER_H
