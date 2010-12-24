#include <iostream>
#include <cstdio>
#include <sys/epoll.h>

#include "daemon.h"
#include "socket.h"
#include "epoll.h"

using namespace rdns;
using std::cout;
using std::cerr;
using std::endl;
using std::pair;


Daemon::Daemon()
{

}

IODevicePtr Daemon::addListener(const char *protoName, const char *ip, unsigned int port)
{
	IODevicePtr ls = Socket::factory(protoName, ip, port);
	if (ls.get() && ls->isValid()) {
		Reactor::instance()->addWatch(ls);
		_listeners.insert(IODeviceItem(ls->fd(), ls));
		return ls;
	}
	return IODevicePtr();
}

bool Daemon::listen()
{
	int amountListen = 0;
	Reactor* reactor = Reactor::instance();

	if (!reactor->isValid()) {
		return false;
	}

	for (IODeviceIterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		if (((Socket*)(*i).second.get())->listen()) {
			amountListen++;
		}
	}
	if (!amountListen) {
		cerr << "nothing to listen\n";
		return false;
	}

	cout << "start listening\n";
	while (true) {
		cout << "waiting for events\n";
		int cnt = reactor->wait();
		if (cnt < 0) {
			if (cnt == -1) {
				perror("rector error");
				return false;
			}
			cout << "termination requested\n";
			return true; // -2 - SIGINT or SIGTERM
		}

	}

	return true;
}
