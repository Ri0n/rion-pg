#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include "reactor.h"
#include "socket.h"

namespace rdns
{

class Epoll : public Reactor
{
	friend class Reactor;
public:
	const static int MaxEvents = 100;

	bool addWatch(SocketPtr);
	void removeWatch(int fd);
	int wait();


protected:
	Epoll();

private:

	epoll_event events[MaxEvents];
};

} // namespace rdns
#endif // EPOLL_H
