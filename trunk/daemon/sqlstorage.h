#ifndef SQLSTORAGE_H
#define SQLSTORAGE_H

#include <basestorage.h>
#include <QSqlDatabase>

class SQLStorage : public BaseStorage
{
	Q_OBJECT
public:
	SQLStorage(QObject* parent);
	void add(const QString text);
	bool isReady();
	QString getLast() const;

protected:
	QSqlDatabase db;
};

#endif // SQLSTORAGE_H
