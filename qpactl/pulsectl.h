#ifndef PaCtl_H
#define PaCtl_H

#include <QObject>

class PaCtlPrivate;
class PaContext;
class PaCtl;

class PaDeferredTask;
class PaTask : public QObject
{
	Q_OBJECT
	friend class PaDeferredTask;
public:
	PaTask(PaCtl *parent = 0);
	~PaTask();

	void run();

signals:
	void ready();

protected:
	void finish();

	PaDeferredTask *d;
};

class PaTaskSources : public PaTask
{
	Q_OBJECT

public:
	PaTaskSources(PaCtl *parent = 0);

	struct Source {
		QString id;
		QString description;
		QString monitorId; // id of monitored sink
	};

	QList<Source> sources() const;
};


class PaCtl : public QObject
{
	Q_OBJECT
public:

	explicit PaCtl(QObject *parent = 0);
	PaContext *context();

signals:

public slots:

private:
	friend class PaCtlPrivate;
	class PaCtlPrivate *d;
};

#endif // PaCtl_H
