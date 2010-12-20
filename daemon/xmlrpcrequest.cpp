#include "xmlrpcrequest.h"
#include "xmlrpcdaemon.h"

class XMLRPCRequest::Private
{
	public:
	Private()
			: daemon(0)
	{
	}

	int requestId;
	QString methodName;
	QList<xmlrpc::Variant> parameters;
	XMLRPCDaemon *daemon;
	QVariant returnValue;
};

XMLRPCRequest::XMLRPCRequest(const int &requestId, const QString &methodName, const QList<xmlrpc::Variant> &parameters, XMLRPCDaemon *parent)
		: QObject(parent)
{
	d = new Private;
	d->requestId = requestId;
	d->methodName = methodName;
	d->parameters = parameters;
	d->daemon = parent;
}

XMLRPCRequest::~XMLRPCRequest()
{
	delete d;
}

void XMLRPCRequest::dispatch()
{
	d->daemon->dispatchRequest(this);
}

int XMLRPCRequest::id()
{
	return d->requestId;
}

QVariant XMLRPCRequest::value() const
{
	return d->returnValue;
}

void XMLRPCRequest::setValue(const QVariant &value)
{
	d->returnValue = value;
}

QString XMLRPCRequest::method() const
{
	return d->methodName;
}
