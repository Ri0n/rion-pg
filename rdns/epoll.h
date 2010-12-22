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

	void close();
	bool addWatch(SocketPtr);
	void removeWatch(int fd);
	int wait();


protected:
	Epoll();
	bool addWatch(int fd);

private:
	sigset_t sigmask;
	int sigFd;
	epoll_event events[MaxEvents];
};

} // namespace rdns
#endif // EPOLL_H
