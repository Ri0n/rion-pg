#ifndef SQLITESTORAGE_H
#define SQLITESTORAGE_H

#include "sqlstorage.h"

class SQLiteStorage : public SQLStorage
{
	Q_OBJECT
public:
	SQLiteStorage(QObject *parent);
};

#endif // SQLITESTORAGE_H
