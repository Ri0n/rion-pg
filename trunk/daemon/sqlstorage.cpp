#include "sqlstorage.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

SQLStorage::SQLStorage(QObject* parent)
		: BaseStorage(parent)
{

}

bool SQLStorage::isReady()
{
	return db.isOpen();
}

void SQLStorage::add(const QString text)
{
	QSqlQuery query;
	QString copy = text;
	query.exec(QString("INSERT INTO report (`text`) VALUES(\"%1\")").arg(copy.replace("\"", "\\\""))); //it's better to use bind
}

QString SQLStorage::getLast() const
{
	QSqlQuery query;
	query.exec("SELECT `text` FROM report ORDER BY id DESC LIMIT 1");
	if (query.first()) {
		return query.value(0).toString();
	}
	return QString();
}
