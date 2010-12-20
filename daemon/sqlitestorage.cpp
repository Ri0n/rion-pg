#include "sqlitestorage.h"

#include <QSqlError>
#include <QDebug>
#include <QStringList>
#include <QSqlQuery>

SQLiteStorage::SQLiteStorage(QObject *parent)
		: SQLStorage(parent)
{
	QString sqliteFile = "/home/rion/rdaemon.sqlite";
	db = QSqlDatabase::addDatabase("QSQLITE");
	db.setDatabaseName( sqliteFile );

	if (!db.open()) {
		qDebug()<<QString("SQlite database(%1) connect failed: %2").arg(sqliteFile, db.lastError().text());
	} else {
		if (db.tables().indexOf("report") == -1) {
			QSqlQuery query;
			query.exec("CREATE TABLE `report` (id INTEGER PRIMARY KEY AUTOINCREMENT, `text` TEXT)");
		}
	}
}
