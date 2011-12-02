#ifndef APPUPDATER_H
#define APPUPDATER_H

#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

class AppUpdater : public QObject
{
    Q_OBJECT
public:
	explicit AppUpdater(const QString &appName, const QUrl &url,
						const QString &version, QObject *parent);
	void setNetworkManager(QNetworkAccessManager *qnam);

private:
	QNetworkAccessManager* networkManager();
	void initNetworkManager();

signals:

public slots:
	void check();

private slots:
	void versionCheckFinished(QNetworkReply *reply);
	void download();

private:
	bool _externQnam;
	QNetworkAccessManager *_qnam;
	QString _appName;
	QUrl _url;
	QString _version;
};

#endif // APPUPDATER_H
