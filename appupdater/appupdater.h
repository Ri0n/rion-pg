#ifndef APPUPDATER_H
#define APPUPDATER_H

#include <QObject>
#include <QUrl>
#include <QTemporaryFile>

class QNetworkAccessManager;
class QNetworkReply;

class AppUpdater : public QObject
{
    Q_OBJECT
public:
	enum CheckType
	{
		TypeVersionFile, // version (and probably download link) stored in file
		TypeVersionHeader // the url is the same for check and dl. New-Version
						  // must be present in HEAD response
	};

	explicit AppUpdater(const QString &appName, const QUrl &url,
						const QString &version, QObject *parent);
	void setNetworkManager(QNetworkAccessManager *qnam);
	inline void setDownloadUrl(const QUrl &u) { _downloadUrl = u; }
	inline QString filename() const { return _dlFile?_dlFile->fileName():""; }
	inline const QString &proposedFilename() const { return _proposedFilename; }
	inline const QUrl &downloadUrl() const { return _downloadUrl; }
	inline const QString &appName() const { return _appName; }
	inline const QString &version() const { return _version; }
	inline const QString &newVersion() const { return _newVersion; }
	inline const QString &error() const { return _error; }
	inline void setType(CheckType type) { _type = type; }

private:
	QNetworkAccessManager* networkManager();
	void initNetworkManager();

signals:
	void fileNameChanged();
	void downloadStarted(QNetworkReply*);
	void checkFinished();
	void finished();

public slots:
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
	QNetworkAccessManager *_qnam;
	QNetworkReply *_dlReply;
	QTemporaryFile *_dlFile;
	QString _appName;
	QUrl _url;
	QUrl _downloadUrl;
	QString _version;
	QString _newVersion;
	QString _proposedFilename;
	QString _error;
};

#endif // APPUPDATER_H
