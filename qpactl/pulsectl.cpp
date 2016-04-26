/***
  Pulseaudio Qt wrapper

  Copyright 2016 Sergey Il'inykh <rion4ik@gmail.com>

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2.1 of the License,
  or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with PulseAudio; if not, see <http://www.gnu.org/licenses/>.
***/

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
#include <QMetaMethod>

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
	static void context_drain_complete(pa_context *c, void *userdata);

protected:

	void onStateCallback();
	void drain();

public:

	pa_context *context;
	pa_proplist *proplist;
	int actions;
	bool ready;

	QString server;
	PaMainloop mainloop;
	int ipcFd; // event file descriptor

	PaContext(int fd, const QString &server = QString::null);
	~PaContext();



public slots:
	void newTask(void*);
	void init();
	void deinit();

signals:
	void contextReady();
	void contextTerminated();
};


class PaCtlPrivate : public QObject
{
	Q_OBJECT
	void kickContext();
public:
	bool contextReady;
	QThread contextThread;
	int ipcFd;
	PaContext *context;
	QList<PaDeferredTask*> tasksQueue;
	QMetaMethod newTaskMethod;

	PaCtlPrivate();
	~PaCtlPrivate();
	void tryExec(PaDeferredTask *task);
public slots:
	void handleContextReady();
	void handleContextTerminated();
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
			emit contextReady();
			break;

		case PA_CONTEXT_TERMINATED:
			mainloop.quit(0);
			emit contextTerminated();
			break;

		case PA_CONTEXT_FAILED:
		default:
			qDebug("Connection failure: %s", pa_strerror(pa_context_errno(context)));
			mainloop.quit(1);
			emit contextTerminated();
	}
}

void PaContext::context_drain_complete(pa_context *c, void *userdata) {
	Q_UNUSED(userdata);
	pa_context_disconnect(c);
}

void PaContext::drain() {
	pa_operation *o;

	if (!(o = pa_context_drain(context, context_drain_complete, NULL)))
		pa_context_disconnect(context);
	else
		pa_operation_unref(o);
}

PaContext::PaContext(int fd, const QString &server) :
	ready(false),
	server(server),
	ipcFd(fd)
{
	proplist = pa_proplist_new();

	if (!(context = pa_context_new_with_proplist(mainloop.api, NULL, proplist))) {
		qDebug("pa_context_new() failed.");
		return;
	}

	pa_context_set_state_callback(context, context_state_callback, this);
	mainloop.api->io_new(mainloop.api, ipcFd, PA_IO_EVENT_INPUT, processIPC, this);
}


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
	task->run();
}

void PaContext::init()
{
	if (pa_context_connect(context, server.isEmpty()? NULL : server.toLocal8Bit().data(), PA_CONTEXT_NOFLAGS, NULL) < 0) {
		qDebug("pa_context_connect() failed: %s", pa_strerror(pa_context_errno(context)));
		return;
	}
	mainloop.run();
}

void PaContext::deinit()
{
	drain();
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
	d->ctl->tryExec(d);
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
// PaTaskSinks
//------------------------------------------------------------
class PaTaskSinksPrivate : public PaDeferredTask
{
public:
	QList<PaTaskSinks::Sink> sinks;

	PaTaskSinksPrivate(PaTask *task) :
	    PaDeferredTask(task) {}

	void onSinkInfo(const pa_sink_info *i, int is_last)
	{
		if (is_last < 0) {
			setError(pa_context_errno(ctl->context->context));
			return;
		}

		if (is_last) {
			setSuccess();
			return;
		}

		PaTaskSinks::Sink s;
		s.id = QString::fromLatin1(i->name);
		s.description = QString::fromLocal8Bit(i->description);
		s.monitorId = QString::fromLatin1(i->monitor_source_name);

		sinks.append(s);
	}

	static void get_sink_info_callback(pa_context *c, const pa_sink_info *i, int is_last, void *userdata)
	{
		Q_UNUSED(c);
		static_cast<PaTaskSinksPrivate*>(userdata)->onSinkInfo(i, is_last);
	}

	void run()
	{
		op = pa_context_get_sink_info_list(ctl->context->context, get_sink_info_callback, this);
	}

};

PaTaskSinks::PaTaskSinks(PaCtl *parent) :
    PaTask(parent)
{
	d = new PaTaskSinksPrivate(this);
}

QList<PaTaskSinks::Sink> PaTaskSinks::sinks() const
{
	return static_cast<PaTaskSinksPrivate*>(d)->sinks;
}


//------------------------------------------------------------
// PaCtl
//------------------------------------------------------------
PaCtlPrivate::PaCtlPrivate() :
    contextReady(false)
{
	ipcFd = eventfd(0, 0);
	context = new PaContext(ipcFd);
	int ntIndex = context->metaObject()->indexOfSlot(QMetaObject::normalizedSignature("newTask(void*)"));
	newTaskMethod = context->metaObject()->method(ntIndex);

	context->moveToThread(&contextThread);
	connect(context, SIGNAL(contextReady()), SLOT(handleContextReady()));
	connect(context, SIGNAL(contextTerminated()), SLOT(handleContextTerminated()));
	contextThread.start();
	QMetaObject::invokeMethod(context, "init", Qt::QueuedConnection);
}

PaCtlPrivate::~PaCtlPrivate()
{
	QMetaObject::invokeMethod(context, "deinit", Qt::QueuedConnection);
	kickContext();
	contextThread.exit(0);
	contextThread.wait();
	delete context;
	// we don't care aout task queue here. just allow Qt do its job with parent objects
	// in either case deleting this object is supposed to be on app termination.
}

void PaCtlPrivate::kickContext()
{
	quint64 cnt = 1;
	write(ipcFd, &cnt, 8);
}

void PaCtlPrivate::tryExec(PaDeferredTask *task)
{
	if (contextReady) {
		newTaskMethod.invoke(context, Qt::QueuedConnection, Q_ARG(void*, task));
		kickContext();
	} else {
		tasksQueue.append(task);
	}
}

void PaCtlPrivate::handleContextReady()
{
	contextReady = true;
	while (tasksQueue.size()) {
		newTaskMethod.invoke(context, Qt::QueuedConnection, Q_ARG(void*, tasksQueue.takeFirst()));
	}
	kickContext();
}

void PaCtlPrivate::handleContextTerminated()
{
	// Notice this method won't be called in case of deletion of PaCtlPrivate object
	// So it's only in case something wrong pulseaudio connection
	while (tasksQueue.size()) {
		tasksQueue.takeFirst()->setError(PA_ERR_CONNECTIONTERMINATED); // we have refused as well btw. we can check mainloop exit code for that
	}
}



PaCtl::PaCtl(QObject *parent) :
    QObject(parent),
	d(new PaCtlPrivate)
{

}

PaCtl::~PaCtl()
{
	delete d;
}

#include "pulsectl.moc"
