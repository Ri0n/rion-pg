#include <cstdio>
#include "epoll.h"

using namespace rdns;

Epoll::Epoll()
	: Reactor()
{
	_fd = epoll_create1(0);
	if (_fd == -1) {
		perror("Failed to create epoll reactor");
	}
}

Epoll::~Epoll()
{
	if (_fd != -1) {
		close(_fd);
	}
}

bool Epoll::addWatch(SocketPtr sock)
{
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = sock->fd();
	if (epoll_ctl(_fd, EPOLL_CTL_ADD, sock->fd(), &ev) == -1) {
		perror("epoll_ctl: listen_sock");
		return false;
	}
	return true;
}

int Epoll::wait()
{
	return epoll_wait(_fd, events, MaxEvents, -1);
}

epoll_event* Epoll::event(int n)
{
	return &events[n];
}
