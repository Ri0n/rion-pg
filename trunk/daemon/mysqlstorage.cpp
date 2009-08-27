#include "mysqlstorage.h"
#include <QSettings>
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>

MySQLStorage::MySQLStorage(QObject *parent)
		: SQLStorage(parent)
{
	QSettings settings;
	db = QSqlDatabase::addDatabase("QMYSQL");
	db.setHostName(settings.value("sql.host").toString());
	db.setDatabaseName(settings.value("sql.db").toString());
	db.setUserName(settings.value("sql.user").toString());
	db.setPassword(settings.value("sql.pass").toString());
	if (!db.open()) {
		qDebug()<<QString("MySQL database connect failed: %1").arg(db.lastError().text());
	} else {
		if (db.tables().indexOf("report") == -1) {
			QSqlQuery query;
			query.exec("CREATE TABLE `report` (id INT PRIMARY KEY AUTO_INCREMENT, `text` TEXT)");
		}
	}
}
