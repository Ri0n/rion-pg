#ifndef PULSECTL_H
#define PULSECTL_H

#include <QObject>

class PulseCtlPrivate;


class PATask : public QObject
{
	Q_OBJECT

public:
	enum Action {
		None,
		Exit,
		List
	};

	enum ListType {
		Sources
	};

signals:
	void ready();

private:
	Action action;
};

class PulseCtl : public QObject
{
	Q_OBJECT
public:

	explicit PulseCtl(QObject *parent = 0);

	PATask *sourcesList();

signals:

public slots:

private:
	friend class PulseCtlPrivate;
	class PulseCtlPrivate *d;
};

#endif // PULSECTL_H
