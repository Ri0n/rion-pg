#ifndef XMLRPCREQUEST_H
#define XMLRPCREQUEST_H

#include <QObject>
#include <qxmlrpc/server.h>
class XMLRPCDaemon;

class XMLRPCRequest : public QObject
{
public:
	XMLRPCRequest(const int &requestId, const QString &methodName, const QList<xmlrpc::Variant> &parameters, XMLRPCDaemon *parent);
	~XMLRPCRequest();
	void dispatch();
	int id();
	QVariant value() const;
	void setValue(const QVariant &);
	QString method() const;
	
private:
	class Private;
	Private *d;
};

#endif // XMLRPCREQUEST_H
