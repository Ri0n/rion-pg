#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStringList>
#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QProgressBar>
#include <QCheckBox>
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
	QCheckBox *ckDontCheckUpdates;
	AppUpdater *updater;
	bool _externalChecksChange;

public:
	UpdateDialog(AppUpdater *updater, QWidget *parent = NULL) :
		QDialog(parent),
		updater(updater),
		_externalChecksChange(false)
	{
		setWindowTitle(tr("%1 update").arg(updater->appName()));
		setAttribute(Qt::WA_DeleteOnClose);

		QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
		lblDescr = new QLabel(tr("New version of <a href=\"%1\">%2</a> is available: %3")
							  .arg(updater->downloadUrl().toEncoded(),
								   updater->appName(), updater->newVersion()));
		lblDescr->setOpenExternalLinks(true);
		layout->addWidget(lblDescr);

		pbarDownload = new QProgressBar();
		pbarDownload->setMaximum(100);
		layout->addWidget(pbarDownload);

		ckDontCheckUpdates = new QCheckBox(tr("Don't check for updates."));
		ckDontCheckUpdates->setChecked(!updater->isChecksEnabled());
		layout->addWidget(ckDontCheckUpdates);

		pbDownload = new QPushButton(tr("&Install now"));
		pbHide = new QPushButton(tr("&Hide"));
		QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
		buttonBox->addButton(pbDownload, QDialogButtonBox::ActionRole);
		buttonBox->addButton(pbHide, QDialogButtonBox::ActionRole);
		buttonBox->addButton(QDialogButtonBox::Cancel);
		layout->addWidget(buttonBox);

		setLayout(layout);
		pbHide->setVisible(false);
		pbarDownload->setVisible(false);

		connect(updater, SIGNAL(fileNameChanged()), SLOT(updateDownloadFilename()));
		connect(updater, SIGNAL(finished()), SLOT(showDownloadError())); // if any
		connect(updater, SIGNAL(downloadStarted(QNetworkReply*)), SLOT(downloadStarted(QNetworkReply*)));
		connect(pbDownload, SIGNAL(clicked()), SLOT(download()));
		connect(pbHide, SIGNAL(clicked()), SLOT(close()));
		connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(cancel()));
		connect(ckDontCheckUpdates, SIGNAL(stateChanged(int)), SLOT(dontCheckChanged(int)));
	}

	inline void setChecksEnabled(bool ce)
	{
		_externalChecksChange = true;
		ckDontCheckUpdates->setChecked(!ce);
		_externalChecksChange = false;
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
		ckDontCheckUpdates->hide();
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
		int percent = bytesTotal? float(bytesReceived) /
								  float(bytesTotal) * 100.0 : 100;
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
		if (updater->filename().isEmpty()) {
			lblDescr->setText("<b style=\"color:red\">" + updater->error() + "</b>");
		}
	}

	void dontCheckChanged(int state)
	{
		if (!_externalChecksChange) {
			updater->_checksEnabled = (state == Qt::Unchecked);
			emit updater->checkEnabled(updater->_checksEnabled);
		}
	}
};



//----------------------------------------------------------------
// AppUpdater
//----------------------------------------------------------------
AppUpdater::AppUpdater(const QString &appName, const QUrl &url,
					   const QString &version, QObject *parent) :
	QObject(parent),
	_type(TypeVersionFile),
	_externQnam(false),
	_checksEnabled(true),
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

void AppUpdater::setChecksEnabled(bool ce)
{
	if (!_dlg.isNull()) {
		_dlg->setChecksEnabled(ce);
	}
}

void AppUpdater::check()
{
	QNetworkReply *reply;
	QNetworkRequest req(_url);
	req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
					 QNetworkRequest::AlwaysNetwork);
	if (_type == TypeVersionFile) {
		reply = networkManager()->get(req);
	} else {
		reply = networkManager()->head(req);
	}
	connect(reply, SIGNAL(finished()), SLOT(reply_versionCheckFinished()));
	connect(reply, SIGNAL(sslErrors(QList<QSslError>)), reply, SLOT(ignoreSslErrors()));
}

QString AppUpdater::handleDownload()
{
	QString fn = filename();
	if (_dlFile) {
		_dlFile->setAutoRemove(false);
		delete _dlFile;
		_dlFile = NULL;
	}
	return fn;
}

void AppUpdater::cancelDownload()
{
	if (_dlReply) {
		_dlReply->abort();
	}
}

void AppUpdater::download()
{
	if (_dlFile) {
		delete _dlFile;
	}
	_dlFile = new QTemporaryFile(this);
	if (!_dlFile->open()) {
		_error = _dlFile->errorString();
		delete _dlFile;
		_dlFile = NULL;
		emit finished();
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
	QString filename = cd.section("filename=", 1);
	if (filename.size() > 2 && filename[0] == '"' &&
			filename[filename.size()-1] == '"')
	{
		filename = filename.mid(1, filename.size() - 2).section('/', -1);
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
		_dlFile->close();
	} else {
		_error = _dlReply->errorString();
		delete _dlFile;
		_dlFile = NULL;
	}
	_dlReply->deleteLater();
	_dlReply = NULL;
	emit finished();
}


void AppUpdater::reply_versionCheckFinished()
{
	_error.clear();
	QNetworkReply *reply = reinterpret_cast<QNetworkReply *>(sender());
	if (reply->error() == QNetworkReply::NoError) {
		QString newVersion;
		if (_type == TypeVersionFile) {
			newVersion = reply->readLine().trimmed();
		} else {
			newVersion = reply->rawHeader("New-Version");
		}
		if (newVersion.contains(QRegExp("^\\d+(\\.\\d+){,3}$"))) {
			QStringList orig = _version.split('.');
			QStringList fresh = newVersion.split('.');
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
				if (_type == TypeVersionFile) {
					QUrl url = QUrl::fromEncoded(reply->readLine().trimmed(),
												 QUrl::StrictMode);
					if (url.isValid()) {
						_downloadUrl = url;
					}
				} else {
					_downloadUrl = _url;
				}
				if (_downloadUrl.isValid()) {
					_newVersion = newVersion;
					_proposedFilename = _downloadUrl.path().section('/', -1);
					QWidget *parentWidget = dynamic_cast<QWidget *>(parent());
					if (!_dlg.isNull()) {
						delete _dlg;
					}
					_dlg = new UpdateDialog(this, parentWidget);
					connect(_dlg, SIGNAL(downloadAccepted()), SLOT(download()));
					connect(_dlg, SIGNAL(cancelDownload()), SLOT(cancelDownload()));
					_dlg->show();
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
