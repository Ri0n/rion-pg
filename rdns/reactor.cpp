#include <iostream>
#include "reactor.h"
#include "epoll.h"

using namespace rdns;

Reactor::Reactor()
	: IODevice()
{

}

Reactor* Reactor::instance()
{
	if (!_instance.get()) {
		if (_defaultType == EpollType) {
			ReactorPtr r = ReactorPtr(new Epoll());
			if (r->isValid()) {
				_instance = r;
			}
		}
		else {
			std::cerr << "Unsupported reactor type\n";
		}
	}
	return _instance.get();
}

void Reactor::setDefaultType(ReactorType type)
{
	Reactor::_defaultType = type;
}

Reactor::ReactorType Reactor::_defaultType = Reactor::EpollType;
ReactorPtr Reactor::_instance;
