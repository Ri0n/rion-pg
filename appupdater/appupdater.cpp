/*
Copyright (c) 2011 Il'inykh Sergey

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


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
#include <QTextEdit>
#include <QTimer>

#include "appupdater.h"

static QList<quint32> splitVersion(const QString &version)
{
	QList<quint32> result;
	foreach (const QString &s, version.split('.')) {
		result.append(s.toUInt());
	}
	while (result.size() && result.last() == 0) {
		result.removeLast();
	}
	if (!result.size()) {
		result.append(0);
	}
	return result;
}


class UpdateDialog : public QDialog
{
	Q_OBJECT

	QPushButton *pbDetails;
	QTextEdit *teDetails;
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

		teDetails = new QTextEdit;
		teDetails->setVisible(false);
		teDetails->setMinimumHeight(100);
		teDetails->resize(0, 100);
		teDetails->setReadOnly(true);
		layout->addWidget(teDetails);

		ckDontCheckUpdates = new QCheckBox(tr("Don't check for updates."));
		ckDontCheckUpdates->setChecked(!updater->isChecksEnabled());
		layout->addWidget(ckDontCheckUpdates);

		pbDetails = new QPushButton(tr("&Details"));
		pbDetails->setCheckable(true);
		pbDetails->setAutoDefault(false);
		pbDownload = new QPushButton(tr("&Install now"));
		pbHide = new QPushButton(tr("&Hide"));
		QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
		buttonBox->addButton(pbDetails, QDialogButtonBox::ActionRole);
		buttonBox->addButton(pbDownload, QDialogButtonBox::ActionRole);
		buttonBox->addButton(pbHide, QDialogButtonBox::AcceptRole);
		buttonBox->addButton(QDialogButtonBox::Cancel);
		layout->addWidget(buttonBox);

		setLayout(layout);
		pbHide->setVisible(false);
		pbarDownload->setVisible(false);
		pbDownload->setFocus();

		connect(updater, SIGNAL(fileNameChanged()), SLOT(updateDownloadFilename()));
		connect(updater, SIGNAL(finished()), SLOT(showDownloadError())); // if any
		connect(updater, SIGNAL(downloadStarted(QNetworkReply*)), SLOT(downloadStarted(QNetworkReply*)));
		connect(updater, SIGNAL(detailsChanged()), SLOT(updateDetails()));
		connect(pbDownload, SIGNAL(clicked()), SLOT(download()));
		connect(pbHide, SIGNAL(clicked()), SLOT(close()));
		connect(ckDontCheckUpdates, SIGNAL(stateChanged(int)), SLOT(dontCheckChanged(int)));
		connect(pbDetails, SIGNAL(toggled(bool)), SLOT(toggleDetails(bool)));
		connect(buttonBox, SIGNAL(rejected()), SLOT(reject()));
		connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
		updateDetails();
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
	void toggleDetails(bool state)
	{
		teDetails->setVisible(state);
		QTimer::singleShot(0, this, SLOT(resizeToContents()));
	}

	void resizeToContents()
	{
		resize(minimumSizeHint());
	}

	void download()
	{
		pbDownload->hide();
		ckDontCheckUpdates->hide();
		pbHide->show();
		pbarDownload->show();
		pbHide->setFocus();
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

	void reject()
	{
		emit cancelDownload();
		QDialog::reject();
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

	void updateDetails()
	{
		pbDetails->setChecked(false);
		pbDetails->setVisible(!updater->details().isEmpty());
		teDetails->setPlainText(updater->details());
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

QNetworkAccessManager* AppUpdater::qnam()
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
		reply = qnam()->get(req);
	} else {
		reply = qnam()->head(req);
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

	_dlReply = qnam()->get(QNetworkRequest(_downloadUrl));
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
			QList<quint32> orig = splitVersion(_version);
			QList<quint32> fresh = splitVersion(newVersion);
			bool updated = false;
			for (int i = 0; i < fresh.size(); i++) {
				if (i == orig.size()) { // fresh has more version components
					updated = true;
					break;
				}
				quint32 origN = orig.value(i);
				quint32 freshN = fresh.value(i);
				if (origN == freshN) {
					continue;
				}
				updated = (freshN > origN);
				break;
			}
			if (updated) {
				_details.clear();
				if (_type == TypeVersionFile) {
					QByteArray data = reply->read(3);
					if (data == "dl:") {
						QUrl url = QUrl::fromEncoded(reply->readLine().trimmed(),
													 QUrl::StrictMode);
						if (url.isValid()) {
							_downloadUrl = url;
						}
						data.clear();
					}
					data += reply->readAll();
					_details = QString::fromUtf8(data.constData(),
												 data.size()).trimmed();
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
