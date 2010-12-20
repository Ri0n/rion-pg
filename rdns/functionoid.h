#ifndef FUNCTIONOID_H
#define FUNCTIONOID_H

#include "shared_ptr.h"

namespace rdns
{

class Callback;
typedef shared_ptr<Callback> CallbackPtr;

class Callback
{
public:
	virtual void call() = 0;
};

} // namespace rdns

#endif // FUNCTIONOID_H
