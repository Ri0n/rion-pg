#include "xmlrpcdaemon.h"
#include "rdaemonapp.h"

XMLRPCDaemon::XMLRPCDaemon(QObject* parent)
		: QObject(parent)
{
	server_ = new xmlrpc::Server(this);

	//register sum and difference methods, with return type int and two int parameters
	server_->registerMethod( "sum", QVariant::Int, QVariant::Int, QVariant::Int );
	server_->registerMethod( "difference", QVariant::Int, QVariant::Int, QVariant::Int );

	connect( server_, SIGNAL(incomingRequest( int, QString, QList<xmlrpc::Variant>)),
			 this, SLOT(processRequest( int, QString, QList<xmlrpc::Variant>)));
}

XMLRPCDaemon* XMLRPCDaemon::instance()
{
	if (!XMLRPCDaemon::instance_) {
		XMLRPCDaemon::instance_ = new XMLRPCDaemon(RDaemonApp::instance());
	}
	return XMLRPCDaemon::instance_;
}

bool XMLRPCDaemon::start()
{
	QSettings settings;
	int port = settings.value("port", 8080).toInt();

	if ( server_->listen( port ) ) {
		qDebug() << "Listening for XML-RPC requests on port" << port;
	}
	return server_->isListening();
}

bool XMLRPCDaemon::isStarted()
{
	return server_->isListening();
}

void XMLRPCDaemon::registerMethod( QString methodName, QVariant::Type returnType )
{
	server_->registerMethod( methodName, returnType );
}

void XMLRPCDaemon::processRequest( int requestId, QString methodName, QList<xmlrpc::Variant> parameters )
{
	// we doun't have to check parameters count and types here
	// since we registered methods "sum" and "difference"
	// with server->registerMethod() call

	// int x = parameters[0].toInt();
	// int y = parameters[1].toInt();

	qDebug() << methodName;

	XMLRPCRequest *request = new XMLRPCRequest(requestId, methodName, parameters, this);
	emit incomingRequest(request);

//	if ( methodName == "get" ) {
//		emit getReport(requestId);
//		server_->sendReturnValue( requestId, x+y );
//	}
//
//	if ( methodName == "difference" ) {
//		server_->sendReturnValue( requestId, x-y );
//	}
}

void XMLRPCDaemon::dispatchRequest(XMLRPCRequest* request)
{
	switch (request->value().type()) {
		case QVariant::String:
			server_->sendReturnValue( request->id(), request->value().value<QString>() );
			break;
		case QVariant::Bool:
			server_->sendReturnValue( request->id(), request->value().value<bool>() );
			break;
		default:
			throw "type is not supported";
	}
	request->deleteLater();
}

XMLRPCDaemon* XMLRPCDaemon::instance_ = NULL;
