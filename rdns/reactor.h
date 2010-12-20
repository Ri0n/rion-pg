#ifndef REACTOR_H
#define REACTOR_H

#include <memory>
#include <map>
#include "socket.h"

namespace rdns
{

class Reactor;
typedef std::auto_ptr<Reactor> ReactorPtr;

class Reactor : public IODevice
{
public:
	enum ReactorType
	{
		EpollType
		// TODO add more
	};

	static Reactor* instance();
	static void setDefaultType(ReactorType);

	virtual bool addWatch(SocketPtr) = 0;
	virtual int wait() = 0;

protected:
	Reactor();
	SocketMap _watches;

private:
	static ReactorType _defaultType;
	static ReactorPtr _instance;
};

} //namespace rdns

#endif // REACTOR_H
