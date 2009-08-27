#ifndef BASESTORAGE_H
#define BASESTORAGE_H

#include <QObject>

//just for fun. no one knows which storages will be used in the reality :)
class BaseStorage : public QObject
{
public:
	BaseStorage(QObject *parent);

	virtual void add(const QString text) = 0;
	virtual bool isReady() = 0;
	virtual QString getLast() const = 0;
};

#endif // BASESTORAGE_H
