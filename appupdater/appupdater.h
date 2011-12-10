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

#ifndef APPUPDATER_H
#define APPUPDATER_H

#include <QObject>
#include <QUrl>
#include <QTemporaryFile>
#include <QPointer>

class QNetworkAccessManager;
class QNetworkReply;
class UpdateDialog;

/**
  The class does not actually update an application but is able to check
  for update, show gui for decision and download some file/update.
  The app should create an updater with appropriate params like url to
  version file and current version. The created object has `check` slot, which
  can be called periodically during life of updater object.
 **/
class AppUpdater : public QObject
{
    Q_OBJECT
public:
	enum CheckType
	{
		/*
		  VersionFile is used to store version, download url and changelog.
		  The format of file on server:

		  x.x.x.x
		  dl:scheme://host/uri
		  multiple lines of
			changelog text or some details

		  x.x.x.x - new version with wich we compare current one
		  dl:url - optional line with url to file to download
		  changelog is also optional
		 */
		TypeVersionFile,

		/*
		  VersionHeader type assumes server sends New-Version header, like:
		  New-Version: x.x.x.x
		  Version header is required on in check phase.
		  Url for download is the same as for check.
		  Changelog may be set externally with setDetails method.
		 */
		TypeVersionHeader
	};

	explicit AppUpdater(const QString &appName, const QUrl &url,
						const QString &version, QObject *parent);
	inline QNetworkAccessManager* networkManager() { return _qnam; }
	void setNetworkManager(QNetworkAccessManager *qnam);
	inline void setDownloadUrl(const QUrl &u) { _downloadUrl = u; }
	inline QString filename() const { return _dlFile?_dlFile->fileName():""; }
	inline const QString &proposedFilename() const { return _proposedFilename; }
	inline const QUrl &downloadUrl() const { return _downloadUrl; }
	inline const QString &appName() const { return _appName; }
	inline const QString &version() const { return _version; }
	inline const QString &newVersion() const { return _newVersion; }
	inline const QString &error() const { return _error; }
	inline const QString &details() const { return _details; }
	inline void setDetails(const QString &details)
	{
		_details = details;
		emit detailsChanged();
	}
	inline void setType(CheckType type) { _type = type; }
	inline bool isChecksEnabled() const { return _checksEnabled; }
	QString handleDownload(); // delete temp file object but saving file

private:
	QNetworkAccessManager* networkManager();
	void initNetworkManager();

signals:
	friend class UpdateDialog;
	void fileNameChanged(); // emitted when Content-Disposition header is met
	void downloadStarted(QNetworkReply*);
	void checkFinished(); // check finished or failed
	void finished(); // download finished or failed
	void checkEnabled(bool); // where user want to enable/disable future checks
	void detailsChanged(); // used in dl dialog. emitted only in setter

public slots:
	void setChecksEnabled(bool ce);
	void check();
	void cancelDownload();

private slots:
	void reply_versionCheckFinished();
	void download();
	void reply_metaDataCahnged();
	void reply_downloadProgress();
	void reply_downloadFinished();

private:
	CheckType _type;
	bool _externQnam;
	bool _checksEnabled;
	QNetworkAccessManager *_qnam;
	QNetworkReply *_dlReply;
	QTemporaryFile *_dlFile;
	QPointer<UpdateDialog> _dlg;
	QString _appName;
	QUrl _url;
	QUrl _downloadUrl;
	QString _version;
	QString _newVersion;
	QString _proposedFilename;
	QString _error;
	QString _details;
};

#endif // APPUPDATER_H
