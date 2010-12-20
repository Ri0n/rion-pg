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

typedef std::map<int, SocketPtr>::iterator LSocketIterator;


Daemon::Daemon()
	: _listenEpollFd(0)
{

}

SocketPtr Daemon::addListener(const char *protoName, const char *ip, unsigned int port)
{
	SocketPtr ls = Socket::factory(protoName, ip, port);
	if (ls.get() && ls->isValid()) {
		_listeners.insert(pair<int,SocketPtr>(ls->fd(), ls));
		return ls;
	}
	return SocketPtr();
}

bool Daemon::listen()
{
	int amountListen = 0;
	int nfds;
	Epoll reactor;

	if (!reactor.isValid()) {
		return false;
	}

	for (LSocketIterator i = _listeners.begin(); i != _listeners.end(); ++i) {
		if ((*i).second->listen() && reactor.addWatch((*i).second)) {
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
		nfds = reactor.wait();
		// TODO rewrite this to use more generic reactor (class Reactor)
		for (int i = 0; i < nfds; i++) {
			LSocketIterator si;
			int eventFd = reactor.event(i)->data.fd;
			if ((si = _listeners.find(eventFd)) != _listeners.end()) {
				// found listener, accepting new connection
				if (reactor.event(i)->events && EPOLLIN) {
					cout << "new connection\n";
					if ((*si).second->isStreamed()) {
						SocketPtr client = (*si).second->accept();
						if (client->isValid()) {
							reactor.addWatch(client);
							_clients.insert(pair<int, SocketPtr>(client->fd(), client));
						}
					}
					else { // udp?
						(*si).second->setReadyRead();
					}
				}
			}
			else {
				if ((si = _clients.find(eventFd)) != _clients.end()) {
					cout << "Received event from client: " << (*si).second->toString() << "\n";
				}
				else {
					cout << "Received data from unknown client. Ignoring..\n";
				}
			}
		}
	}

	close(_listenEpollFd);
	return true;
}
