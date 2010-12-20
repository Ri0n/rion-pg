#include "rdaemonapp.h"

#include <QNetworkAccessManager>
#include <QTimer>
#include "xmlrpcdaemon.h"
#include "mysqlstorage.h"
#include "sqlitestorage.h"

class RDaemonApp::Private
{
	public:
	Private()
			: nm(0), dlTimer(0), storage(0)
	{
	}

	QNetworkAccessManager *nm;
	QTimer *dlTimer;
	BaseStorage *storage;
};

RDaemonApp::RDaemonApp(int & argc, char ** argv)
		: QCoreApplication(argc, argv)
{
	d = new Private;

	QCoreApplication::setOrganizationName("rsoft");
	QCoreApplication::setOrganizationDomain("rionhost");
	QCoreApplication::setApplicationName("RDaemon");

	//make default settings
	QSettings s;
	if (!s.value("port").toInt()) { s.setValue("port", 8080); }
	if (s.value("sql.host").isNull()) { s.setValue("sql.host", "localhost"); }
	if (s.value("sql.user").isNull()) { s.setValue("sql.user", "root"); }
	if (s.value("sql.pass").isNull()) { s.setValue("sql.pass", "123"); }
	if (s.value("sql.db").isNull()) { s.setValue("sql.db", "daemon"); }
	s.sync();

	d->nm = new QNetworkAccessManager(this);
	d->dlTimer = new QTimer(this);
	d->storage = new MySQLStorage(this);
	if (!d->storage->isReady()) {
		qDebug()<<"Trying sqlite..";
		delete d->storage;
		d->storage = new SQLiteStorage(this);
	}

	XMLRPCDaemon *daemon = XMLRPCDaemon::instance();
	daemon->registerMethod("update", QVariant::Bool);
	daemon->registerMethod("last", QVariant::String);
	if (d->storage->isReady() && daemon->start()) {
		connect(daemon, SIGNAL(incomingRequest(XMLRPCRequest*)), this, SLOT(doProcessXMLRPC(XMLRPCRequest*)));
		connect(d->nm, SIGNAL(finished(QNetworkReply*)), this, SLOT(doProcessDowload(QNetworkReply*)));
		connect(d->dlTimer, SIGNAL(timeout()), this, SLOT(doNewRequest()));

		doNewRequest();
		d->dlTimer->start(30000);
	} else {
		qDebug()<<"Something went wrong. server is not started";
	}
}

bool RDaemonApp::isDaemonStarted()
{
	return d->storage->isReady() && XMLRPCDaemon::instance()->isStarted();
}

void RDaemonApp::doNewRequest()
{
	d->nm->get(QNetworkRequest(QUrl("http://vesna.yandex.ru/mathematics.xml")));
}

void RDaemonApp::doProcessDowload(QNetworkReply* report)
{
	//TODO check if previous request is not complete yet
	QByteArray data = report->readAll();
	QTextCodec *codec = QTextCodec::codecForHtml(data);
	QTextCodec::setCodecForCStrings(codec);
	QString content(data);
	content = content.section("<td colspan=\"9\" class=\"text\">", 1).section("</td>", 0, 0);
	content = content.replace(QRegExp("<[^>]+>"), ""); //remove tags
	d->storage->add(content);
	qDebug()<<"new report received and saved";
	delete report;
}

void RDaemonApp::doProcessXMLRPC(XMLRPCRequest* request)
{
	QString method = request->method();
	if (method == "last") { // it's better to make these checks in the daemon and set some "type" for request or even make reqest of derived class
		request->setValue(d->storage->getLast());
	} else if (method == "update") {
		doNewRequest();
		request->setValue(true); //FIXME we return return success response only after http request is complete.
	} else {
		// here is something for next version
	}
	request->dispatch();
}
