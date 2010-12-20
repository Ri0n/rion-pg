#include <cstdio>
#include <iostream>
#include "epoll.h"
#include "socket.h"

using namespace rdns;
using std::cout;

Epoll::Epoll()
	: Reactor()
{
	_fd = epoll_create1(0);
	if (_fd == -1) {
		perror("Failed to create epoll reactor");
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
	_watches.insert(SocketItem(sock->fd(), sock));
	return true;
}

int Epoll::wait()
{
	int nfds = epoll_wait(_fd, events, MaxEvents, -1);
	for (int i = 0; i < nfds; i++) {
		SocketIterator si = _watches.find(events[i].data.fd);
		if (si != _watches.end()) {
			cout << "received events: " << events[i].events << "\n";
			if (events[i].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP)) {
				(*si).second->setReadyOnly(true); // maybe something like aboutToClose ?
			}
			if (events[i].events & EPOLLIN) {
				(*si).second->setReadyRead();
			}
			if (events[i].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP)) {
				(*si).second->close();
				_watches.erase((*si).first);
			}
		}
		else {
			// TODO O_o
			cout << "Received data from unknown client. Ignoring..\n";
		}
	}
	return nfds;
}
