#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include "reactor.h"
#include "socket.h"

namespace rdns
{

class Epoll : public Reactor
{
public:
	const static int MaxEvents = 100;

    Epoll();
	~Epoll();

	bool addWatch(SocketPtr);
	int wait();
	epoll_event* event(int);


private:
	epoll_event events[MaxEvents];
};

} // namespace rdns
#endif // EPOLL_H
