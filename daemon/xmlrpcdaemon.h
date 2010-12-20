#ifndef XMLRPCDAEMON_H
#define XMLRPCDAEMON_H

#include <QObject>
#include <qxmlrpc/server.h>
#include <xmlrpcrequest.h>

class XMLRPCDaemon;

class XMLRPCDaemon : public QObject
{
	Q_OBJECT

public:
	XMLRPCDaemon(QObject *parent);

	static XMLRPCDaemon* instance();
	bool isStarted();
	bool start();
	void dispatchRequest(XMLRPCRequest*);
	void registerMethod( QString methodName, QVariant::Type returnType );

signals:
	void incomingRequest(XMLRPCRequest*);

private:
	static XMLRPCDaemon* instance_;

private slots:
	void processRequest( int requestId, QString methodName, QList<xmlrpc::Variant> parameters );

private:
	xmlrpc::Server *server_;
};

#endif // XMLRPCDAEMON_H
