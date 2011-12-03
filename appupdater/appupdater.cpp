#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringList>
#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QProgressBar>
#include <QDesktopServices>
#include <QDialog>

#include "appupdater.h"


class UpdateDialog : public QDialog
{
	Q_OBJECT


	QPushButton *pbDownload;
	QPushButton *pbHide;
	QProgressBar *pbarDownload;
	QLabel *lblDescr;
	AppUpdater *updater;

public:
	UpdateDialog(AppUpdater *updater, QWidget *parent = NULL) :
		QDialog(parent),
		updater(updater)
	{
		setWindowTitle(tr("%1 update").arg(updater->appName()));
		setAttribute(Qt::WA_DeleteOnClose);

		QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
		lblDescr = new QLabel(tr("New version of <a href=\"%1\">%2</a> is available: %3")
							  .arg(updater->downloadUrl().toEncoded(),
								   updater->appName(), updater->newVersion()));
		layout->addWidget(lblDescr);

		pbarDownload = new QProgressBar();
		pbarDownload->setMaximum(100);
		layout->addWidget(pbarDownload);

		pbDownload = new QPushButton(tr("&Install now"));
		pbHide = new QPushButton(tr("&Hide"));
		QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
		buttonBox->addButton(pbDownload, QDialogButtonBox::ActionRole);
		buttonBox->addButton(pbHide, QDialogButtonBox::ActionRole);
		buttonBox->addButton(QDialogButtonBox::Cancel);
		layout->addWidget(buttonBox);

		pbHide->setVisible(false);
		pbarDownload->setVisible(false);

		setLayout(layout);

		connect(updater, SIGNAL(fileNameChanged()), SLOT(updateDownloadFilename()));
		connect(updater, SIGNAL(downloadFailed()), SLOT(showDownloadError()));
		connect(updater, SIGNAL(downloadStarted(QNetworkReply*)), SLOT(downloadStarted(QNetworkReply*)));
		connect(pbDownload, SIGNAL(clicked()), SLOT(download()));
		connect(pbHide, SIGNAL(clicked()), SLOT(close()));
		connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(cancel()));
	}

public slots:
	void updateDownloadFilename()
	{
		lblDescr->setText(tr("Downloading.. %1").arg(updater->proposedFilename()));
	}

signals:
	void downloadAccepted();
	void cancelDownload();

private slots:
	void download()
	{
		pbDownload->hide();
		pbHide->show();
		pbarDownload->show();
		updateDownloadFilename();
		emit downloadAccepted();
	}

	void downloadStarted(QNetworkReply *reply)
	{
		connect(reply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(downloadProgress(qint64,qint64)));
	}

	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
	{
		int percent = float(bytesReceived) / float(bytesTotal) * 100.0;
		pbarDownload->setValue(percent);
		if (percent == 100) {
			hide();
		}
	}

	void cancel()
	{
		emit cancelDownload();
		close();
	}

	void showDownloadError()
	{
		lblDescr->setText("<b style=\"color:red\">" + updater->error() + "</b>");
	}

public:
	QUrl url;
};



//----------------------------------------------------------------
// AppUpdater
//----------------------------------------------------------------
AppUpdater::AppUpdater(const QString &appName, const QUrl &url,
					   const QString &version, QObject *parent) :
	QObject(parent),
	_externQnam(false),
	_qnam(NULL),
	_dlReply(NULL),
	_dlFile(NULL),
	_appName(appName),
	_url(url),
	_version(version),
	_newVersion(version)
{

}

void AppUpdater::setNetworkManager(QNetworkAccessManager *qnam)
{
	if (_qnam) {
		_qnam->disconnect(this);
		cancelDownload();
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

}

void AppUpdater::check()
{
	QNetworkReply *reply = networkManager()->get(QNetworkRequest(_url));
	connect(reply, SIGNAL(finished()), SLOT(reply_versionCheckFinished()));
	connect(reply, SIGNAL(sslErrors(QList<QSslError>)), reply, SLOT(ignoreSslErrors()));
}

void AppUpdater::cancelDownload()
{
	if (_dlReply && _dlReply->isRunning()) {
		_dlReply->disconnect(this);
		_dlReply->abort();
		_dlReply = NULL;
		delete _dlFile;
		_dlFile = NULL;
	}
}

void AppUpdater::download()
{
	_dlFile = new QTemporaryFile(this);
	if (!_dlFile->open()) {
		_error = _dlFile->errorString();
		emit downloadFailed();
		return;
	}

	_dlReply = networkManager()->get(QNetworkRequest(_downloadUrl));
	connect(_dlReply, SIGNAL(metaDataChanged()), SLOT(reply_metaDataCahnged()));
	connect(_dlReply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(reply_downloadProgress()));
	connect(_dlReply, SIGNAL(finished()), SLOT(reply_downloadFinished()));
	connect(_dlReply, SIGNAL(sslErrors(QList<QSslError>)), _dlReply, SLOT(ignoreSslErrors()));
	emit downloadStarted(_dlReply);
}

void AppUpdater::reply_metaDataCahnged()
{
	QString cd = _dlReply->rawHeader("Content-Disposition").trimmed();
	if (!cd.isEmpty() && cd.startsWith("attachment;")) {
		QString filename = cd.section("filename=", 1);
		if (!filename.isEmpty()) {
			_proposedFilename = filename;
			emit fileNameChanged();
		}
	}
}

void AppUpdater::reply_downloadProgress()
{
	_dlFile->write(_dlReply->readAll());
}

void AppUpdater::reply_downloadFinished()
{
	if (_dlReply->error() == QNetworkReply::NoError) {
		emit downloadFinished();
	} else if (_dlReply->error() != QNetworkReply::OperationCanceledError) {
		_error = _dlReply->errorString();
		emit downloadFailed();
	}
	_dlReply->deleteLater();
	_dlReply = NULL;
	_dlFile->close();
}


void AppUpdater::reply_versionCheckFinished()
{
	_error.clear();
	_newVersion = _version;
	QNetworkReply *reply = reinterpret_cast<QNetworkReply *>(sender());
	if (reply->error() == QNetworkReply::NoError) {
		_newVersion = reply->readLine().trimmed();
		if (_newVersion.contains(QRegExp("^\\d+(\\.\\d+){,3}$"))) {
			QStringList orig = _version.split('.');
			QStringList fresh = _newVersion.split('.');
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
				QUrl url = QUrl::fromEncoded(reply->readLine().trimmed(), QUrl::StrictMode);
				if (url.isValid()) {
					_downloadUrl = url;
				}
				if (_downloadUrl.isValid()) {
					_proposedFilename = _downloadUrl.path().section('/', -1);
					QWidget *parentWidget = dynamic_cast<QWidget *>(parent());
					UpdateDialog *dlg = new UpdateDialog(this, parentWidget);
					connect(dlg, SIGNAL(downloadAccepted()), SLOT(download()));
					connect(dlg, SIGNAL(cancelDownload()), SLOT(cancelDownload()));
					dlg->show();
				} else {
					_error = tr("Invalid download url");
				}
			}
		}
	}
	reply->deleteLater();
	emit checkFinished();
}

#include "appupdater.moc"
