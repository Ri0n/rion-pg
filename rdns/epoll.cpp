#include <cstdio>
#include <iostream>
#include "epoll.h"
#include "socket.h"
#include <signal.h>
#include <sys/signalfd.h>

using namespace rdns;
using std::cout;
using std::endl;

Epoll::Epoll()
	: Reactor()
	, sigFd(-1)
{
	_fd = epoll_create1(0);
	if (_fd == -1) {
		perror("Failed to create epoll reactor");
	}
	else {
		sigemptyset (&sigmask);
		sigaddset(&sigmask, SIGINT);
		sigaddset(&sigmask, SIGTERM);
		if (sigprocmask(SIG_BLOCK, &sigmask, NULL) == -1) {
			perror ("sigprocmask");
		}
		else {
			sigFd = signalfd (-1, &sigmask, SFD_NONBLOCK);
			addWatch(sigFd);
		}
	}
}

void Epoll::close()
{
	Reactor::close();
	if (_fd == -1) { // closed
		::close(sigFd);
	}
}

bool Epoll::addWatch(IODevicePtr sock)
{
	if (addWatch(sock->fd())) {
		_watches.insert(IODeviceItem(sock->fd(), sock));
		return true;
	}
	return false;
}

bool Epoll::addWatch(int fd)
{
	epoll_event ev;
	cout << "add watch fd=" << fd << endl;
	ev.data.u64 = 0; // make valgrind happy =)
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = fd;
	if (epoll_ctl(_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		perror("epoll_ctl: EPOLL_CTL_ADD");
		return false;
	}
	return true;
}

void Epoll::removeWatch(int fd)
{
	cout << "remove watch fd=" << fd << endl;
	_watches.erase(fd);
}

int Epoll::wait()
{
	int nfds = epoll_wait(_fd, events, MaxEvents, -1);
	for (int i = 0; i < nfds; i++) {
		if (events[i].data.fd == sigFd) {
			return -2;
		}
		IODeviceIterator si = _watches.find(events[i].data.fd);
		if (si != _watches.end()) {
			cout << "received events: " << events[i].events << "\n";
			if (events[i].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP) &&
					!((*si).second)->isPersistent()) {
				(*si).second->setReadyOnly(true); // maybe something like aboutToClose ?
			}
			if (events[i].events & EPOLLIN) {
				(*si).second->setReadyRead();
			}
			if (events[i].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP) &&
					!((*si).second)->isPersistent()) {
				removeWatch((*si).first); // destructor will close internal socket
			}
		}
		else {
			// TODO O_o
			cout << "Received data from unknown client. events: "
				 << events[i].events << " fd: " << events[i].data.fd << " Ignoring..\n";
		}
	}
	return nfds;
}
