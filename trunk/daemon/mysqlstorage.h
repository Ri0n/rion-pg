#ifndef MYSQLSTORAGE_H
#define MYSQLSTORAGE_H

#include "sqlstorage.h"

class MySQLStorage : public SQLStorage
{
public:
	MySQLStorage(QObject *parent);
};

#endif // MYSQLSTORAGE_H
