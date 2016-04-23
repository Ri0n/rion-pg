#include <pulse/introspect.h>
#include <pulse/mainloop.h>
#include <pulse/mainloop-signal.h>
#include <pulse/error.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <QExplicitlySharedDataPointer>
#include <QThread>
#include <QCoreApplication>
#include <QDebug>

#include "pulsectl.h"


class PaDeferredTask
{
public:
	PaCtlPrivate *ctl;
	pa_operation *op;
	int errorCode;
	PaTask *task;

	PaDeferredTask(PaTask *task);

	virtual ~PaDeferredTask();
	virtual void run() = 0;
	void finish();
	void setError(int code);
	void setSuccess();
};


class PaMainloop
{
public:
	pa_mainloop *mainloop;
	pa_mainloop_api *api;

	PaMainloop();
	~PaMainloop();
	void quit(int ret);
	void run();
};


class PaContext : public QObject
{
	Q_OBJECT
private:
	static void context_state_callback(pa_context *c, void *userdata);
	static void processIPC(pa_mainloop_api *mainloop_api, pa_io_event *stdio_event,
	                           int fd, pa_io_event_flags_t f, void *userdata);

protected:

	void onStateCallback();
	static void context_drain_complete(pa_context *c, void *userdata);
	void drain(void);
	void complete_action(void);

public:

	pa_context *context;
	pa_proplist *proplist;
	int actions;
	bool ready;

	QString server;
	PaMainloop mainloop;
	QList<PaDeferredTask*> taskQueue;
	int ipcFd; // event file descriptor

	PaContext(int fd, const QString &server = QString::null);
	~PaContext();



public slots:
	void newTask(void*);
	void init();
};


class PaCtlPrivate
{
public:
	PaCtlPrivate();
	~PaCtlPrivate();

	QThread contextThread;
	int ipcFd;
	PaContext *context;
};


//------------------------------------------------------------
// PaMainloop
//------------------------------------------------------------
PaMainloop::PaMainloop()
{
	if (!(mainloop = pa_mainloop_new())) {
		qDebug("pa_mainloop_new() failed.");
		return;
	}

	api = pa_mainloop_get_api(mainloop);

//	Q_ASSERT(pa_signal_init(mainloop_api) == 0);
//    pa_signal_new(SIGINT, exit_signal_callback, NULL);
//    pa_signal_new(SIGTERM, exit_signal_callback, NULL);
//    pa_disable_sigpipe();

}

PaMainloop::~PaMainloop()
{
	if (mainloop) {
		pa_signal_done();
		pa_mainloop_free(mainloop);
	}
}

void PaMainloop::quit(int ret) {
	Q_ASSERT(api);
	api->quit(api, ret);
}

void PaMainloop::run()
{
	int ret = 1;

	if (pa_mainloop_run(mainloop, &ret) < 0) {
		qDebug("pa_mainloop_run() failed.");
		return;
	}
}


//------------------------------------------------------------
// PaContext
//------------------------------------------------------------
void PaContext::context_state_callback(pa_context *c, void *userdata)
{
	Q_ASSERT(c);
	Q_ASSERT(userdata);
	static_cast<PaContext*>(userdata)->onStateCallback();
}

void PaContext::processIPC(pa_mainloop_api *mainloop_api, pa_io_event *stdio_event,
						   int fd, pa_io_event_flags_t f, void *userdata)
{
	Q_UNUSED(mainloop_api)
	Q_UNUSED(stdio_event)
	Q_UNUSED(f)
	Q_UNUSED(userdata)
	char buf[8];
	read(fd, buf, 8);
	qDebug() << "process ipc" << QThread::currentThread();
	QCoreApplication::processEvents();
}

void PaContext::onStateCallback()
{

	switch (pa_context_get_state(context)) {
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
			break;

		case PA_CONTEXT_READY:
			ready = true;
			while (taskQueue.size()) {
				taskQueue.takeFirst()->run();
			}
			break;

		case PA_CONTEXT_TERMINATED:
			mainloop.quit(0);
			break;

		case PA_CONTEXT_FAILED:
		default:
			qDebug("Connection failure: %s", pa_strerror(pa_context_errno(context)));
			mainloop.quit(1);
	}

}

void PaContext::context_drain_complete(pa_context *c, void *userdata) {
	Q_UNUSED(userdata);
	pa_context_disconnect(c);
}

void PaContext::drain(void) {
	pa_operation *o;

	if (!(o = pa_context_drain(context, context_drain_complete, NULL)))
		pa_context_disconnect(context);
	else
		pa_operation_unref(o);
}

void PaContext::complete_action(void) {
	Q_ASSERT(actions > 0);

	if (!(--actions))
		drain();
}


PaContext::PaContext(int fd, const QString &server) :
	ready(false),
	server(server),
	ipcFd(fd)
{}


PaContext::~PaContext()
{
	if (context)
		pa_context_unref(context);

	if (proplist)
		pa_proplist_free(proplist);
}

void PaContext::newTask(void *t)
{
	PaDeferredTask *task = (PaDeferredTask *)t;
	if (ready) {
		task->run();
	} else {
		taskQueue.append(task);
	}
}

void PaContext::init()
{
	proplist = pa_proplist_new();

	if (!(context = pa_context_new_with_proplist(mainloop.api, NULL, proplist))) {
		qDebug("pa_context_new() failed.");
		return;
	}

	pa_context_set_state_callback(context, context_state_callback, this);
	if (pa_context_connect(context, server.isEmpty()? NULL : server.toLocal8Bit().data(), PA_CONTEXT_NOFLAGS, NULL) < 0) {
		qDebug("pa_context_connect() failed: %s", pa_strerror(pa_context_errno(context)));
		return;
	}

	mainloop.api->io_new(mainloop.api, ipcFd, PA_IO_EVENT_INPUT, processIPC, this);

	mainloop.run();
}





//------------------------------------------------------------
// PaDeferredTask
//------------------------------------------------------------
PaDeferredTask::PaDeferredTask(PaTask *task) :
    ctl(static_cast<PaCtl*>(task->parent())->d),
	op(0),
	errorCode(0),
	task(task)
{
}

PaDeferredTask::~PaDeferredTask()
{
	if (op) {
		pa_operation_cancel(op);
		pa_operation_unref(op);
	}
}

void PaDeferredTask::finish()
{
	emit task->ready();
	if (op) {
		pa_operation_unref(op);
		op = 0;
	}
	task->deleteLater();
}

void PaDeferredTask::setError(int code)
{
	errorCode = code;
	finish();
}

void PaDeferredTask::setSuccess()
{
	errorCode = 0;
	finish();
}

//------------------------------------------------------------
// PaTask
//------------------------------------------------------------
PaTask::PaTask(PaCtl *parent) :
    QObject(parent)
{

}

PaTask::~PaTask()
{
	delete d;
}

void PaTask::run()
{
	qDebug() << "task run" << QThread::currentThread();
	QMetaObject::invokeMethod(d->ctl->context, "newTask", Qt::QueuedConnection, Q_ARG(void*, d));
	quint64 cnt = 1;
	write(d->ctl->ipcFd, &cnt, 8);
}



//------------------------------------------------------------
// PaTaskSources
//------------------------------------------------------------
class PaTaskSourcesPrivate : public PaDeferredTask
{
public:
	QList<PaTaskSources::Source> sources;

	PaTaskSourcesPrivate(PaTask *task) :
	    PaDeferredTask(task) {}

	void onSourceInfo(const pa_source_info *i, int is_last)
	{
		if (is_last < 0) {
			setError(pa_context_errno(ctl->context->context));
			return;
		}

		if (is_last) {
			setSuccess();
			return;
		}

		PaTaskSources::Source s;
		s.id = QString::fromLatin1(i->name);
		s.description = QString::fromLocal8Bit(i->description);
		s.monitorId = QString::fromLatin1(i->monitor_of_sink_name);

		sources.append(s);
	}

	static void get_source_info_callback(pa_context *c, const pa_source_info *i, int is_last, void *userdata)
	{
		Q_UNUSED(c);
		static_cast<PaTaskSourcesPrivate*>(userdata)->onSourceInfo(i, is_last);
	}

	void run()
	{
		op = pa_context_get_source_info_list(ctl->context->context, get_source_info_callback, this);
	}

};

PaTaskSources::PaTaskSources(PaCtl *parent) :
    PaTask(parent)
{
	d = new PaTaskSourcesPrivate(this);
}

QList<PaTaskSources::Source> PaTaskSources::sources() const
{
	return static_cast<PaTaskSourcesPrivate*>(d)->sources;
}


//------------------------------------------------------------
// PaCtl
//------------------------------------------------------------
PaCtlPrivate::PaCtlPrivate()
{
	ipcFd = eventfd(0, 0);
	context = new PaContext(ipcFd);
	context->moveToThread(&contextThread);
	contextThread.start();
	QMetaObject::invokeMethod(context, "init", Qt::QueuedConnection);
}

PaCtlPrivate::~PaCtlPrivate()
{
	delete context;
}




PaCtl::PaCtl(QObject *parent) :
    QObject(parent),
	d(new PaCtlPrivate)
{

}

#include "pulsectl.moc"
