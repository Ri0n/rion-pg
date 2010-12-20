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

SocketPtr Daemon::addListener(const char *protoName, const char *ip, unsigned int port)
{
	SocketPtr ls = Socket::factory(protoName, ip, port);
	if (ls.get() && ls->isValid()) {
		Reactor::instance()->addWatch(ls);
		_listeners.insert(SocketItem(ls->fd(), ls));
		return ls;
	}
	return SocketPtr();
}

bool Daemon::listen()
{
	int amountListen = 0;
	int nfds;
	Reactor* reactor = Reactor::instance();

	if (!reactor->isValid()) {
		return false;
	}

	for (SocketIterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		if ((*i).second->listen()) {
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
		nfds = reactor->wait();
	}

	return true;
}
