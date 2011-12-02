#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringList>
#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>

#include "appupdater.h"


class UpdateDialog : public QWidget
{
	Q_OBJECT
public:
	UpdateDialog(const QString &appName, const QString &newVersion) :
		QWidget()
	{
		QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
		layout->addWidget(new QLabel(tr("New version of %1 available: %2")
									 .arg(appName, newVersion)));

		QPushButton *pbDownload = new QPushButton("Install now");
		QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Vertical);
		buttonBox->addButton(pbDownload, QDialogButtonBox::ActionRole);
		buttonBox->addButton(QDialogButtonBox::Ignore);
		layout->addWidget(buttonBox);

		setAttribute(Qt::WA_DeleteOnClose);

		connect(pbDownload, SIGNAL(clicked()), SLOT(download()));
	}
};


AppUpdater::AppUpdater(const QString &appName, const QUrl &url,
					   const QString &version, QObject *parent) :
	QObject(parent),
	_externQnam(false),
	_qnam(NULL),
	_appName(appName),
	_url(url),
	_version(version)
{

}

void AppUpdater::setNetworkManager(QNetworkAccessManager *qnam)
{
	if (_qnam) {
		_qnam->disconnect(this);
		if (!_externQnam) {
			_qnam->deleteLater();
		}
	}
	_qnam = qnam;
	_externQnam = true;
	if (_qnam) {
		initNetworkManager();
	}
}

QNetworkAccessManager* AppUpdater::networkManager()
{
	if (!_qnam) {
		_qnam = new QNetworkAccessManager(this);
		_externQnam = false;
		initNetworkManager();
	}
	return _qnam;
}

void AppUpdater::initNetworkManager()
{
	//connect(_qnam, SIGNAL(finished(QNetworkReply*)),
	//		SLOT(replyFinished(QNetworkReply*)));
}

void AppUpdater::check()
{
	QNetworkReply *reply = networkManager()->get(QNetworkRequest(_url));
	connect(reply, SIGNAL(finished()), SLOT(versionCheckFinished()));
	connect(reply, SIGNAL(sslErrors(QList<QSslError>)), reply, SLOT(ignoreSslErrors()));
}

void AppUpdater::download()
{
	QNetworkReply *reply = networkManager()->get(QNetworkRequest(_url));
	connect(reply, SIGNAL(finished()), SLOT(downloadFinished()));
	connect(reply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(downloadProgress()));
	connect(reply, SIGNAL(sslErrors(QList<QSslError>)), reply, SLOT(ignoreSslErrors()));
}

void AppUpdater::versionCheckFinished()
{
	QNetworkReply *reply = reinterpret_cast<QNetworkReply *>(sender());
	if (reply->error() == QNetworkReply::NoError) {
		QString version = reply->readAll();
		if (version.contains(QRegExp("^\\d+(\\.\\d+){,3}$"))) {
			QStringList orig = _version.split('.');
			QStringList fresh = version.split('.');
			bool updated = false;
			for (int i = 0; i < fresh.size(); i++) {
				if (i == orig.size()) { // fresh has more version components
					updated = true;
					break;
				}
				int origN = orig.value(i).toInt();
				int freshN = fresh.value(i).toInt();
				if (origN == freshN) {
					continue;
				}
				updated = (freshN > origN);
				break;
			}
			if (updated) {
				UpdateDialog *dlg = new UpdateDialog(_appName, version);
				dlg->show();
			}
		}
	}
	reply->deleteLater();
}

#include "appupdater.moc"
