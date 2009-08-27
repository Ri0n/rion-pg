#ifndef RDAEMONAPP_H
#define RDAEMONAPP_H

#include <QCoreApplication>
#include <QNetworkReply>

class XMLRPCRequest;

class RDaemonApp : public QCoreApplication
{
	Q_OBJECT
public:
	RDaemonApp(int & argc, char ** argv);
	bool isDaemonStarted();

private:
	class Private;
	Private *d;

private slots:
	void doNewRequest();
	void doProcessDowload(QNetworkReply*);
	void doProcessXMLRPC(XMLRPCRequest*);
};

#endif // RDAEMONAPP_H
