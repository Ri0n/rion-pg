#include <pulse/introspect.h>
#include <pulse/mainloop.h>
#include <pulse/mainloop-signal.h>
#include <pulse/error.h>

#include <QExplicitlySharedDataPointer>

#include "pulsectl.h"


class PaDeferredTask
{
public:
	PaContext *context;
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

class PaMainloopPrivate : public QSharedData
{
public:
	PaMainloopPrivate() :
	    m(0), mainloop_api(0) {}

	pa_mainloop *m;
	pa_mainloop_api *mainloop_api;
};

class PaMainloop
{
public:
	QExplicitlySharedDataPointer<PaMainloopPrivate> d;


	PaMainloop() :
	    d(new PaMainloopPrivate)
	{
		if (!(d->m = pa_mainloop_new())) {
	        qDebug("pa_mainloop_new() failed.");
	        return;
	    }

		d->mainloop_api = pa_mainloop_get_api(d->m);

	//	Q_ASSERT(pa_signal_init(mainloop_api) == 0);
	//    pa_signal_new(SIGINT, exit_signal_callback, NULL);
	//    pa_signal_new(SIGTERM, exit_signal_callback, NULL);
	//    pa_disable_sigpipe();

	}

	~PaMainloop()
	{
		if (d->m) {
	        pa_signal_done();
	        pa_mainloop_free(d->m);
	    }
	}

	void quit(int ret) {
	    Q_ASSERT(d->mainloop_api);
	    d->mainloop_api->quit(d->mainloop_api, ret);
	}

	void run()
	{
		int ret = 1;

		if (pa_mainloop_run(d->m, &ret) < 0) {
	        qDebug("pa_mainloop_run() failed.");
	        return;
	    }
	}

	inline pa_mainloop_api *api() const { return d->mainloop_api; }
};

class PaContext
{

private:
	static void context_state_callback(pa_context *c, void *userdata)
	{
		Q_ASSERT(c);
		Q_ASSERT(userdata);
		static_cast<PaContext*>(userdata)->onStateCallback();
	}

protected:
	void onStateCallback()
	{

		switch (pa_context_get_state(context)) {
	        case PA_CONTEXT_CONNECTING:
	        case PA_CONTEXT_AUTHORIZING:
	        case PA_CONTEXT_SETTING_NAME:
	            break;

	        case PA_CONTEXT_READY:
				ready = true;
				PaDeferredTask *t;
				while ((t = taskQueue.takeFirst())) {
					t->run();
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

	void onSourceInfoReady(pa_context *c, const pa_source_info *i, int is_last)
	{
		if (is_last < 0) {
	        qDebug("Failed to get source information: %s", pa_strerror(pa_context_errno(c)));
	        mainloop.quit(1);
	        return;
	    }

		if (is_last) {
	        complete_action();
	        return;
	    }

		Q_ASSERT(i);
	}

	static void context_drain_complete(pa_context *c, void *userdata) {
		Q_UNUSED(userdata);
	    pa_context_disconnect(c);
	}

	void drain(void) {
	    pa_operation *o;

	    if (!(o = pa_context_drain(context, context_drain_complete, NULL)))
	        pa_context_disconnect(context);
	    else
	        pa_operation_unref(o);
	}

	void complete_action(void) {
	    Q_ASSERT(actions > 0);

	    if (!(--actions))
	        drain();
	}

public:

	pa_context *context;
	pa_proplist *proplist;
	int actions;
	bool ready;

	QString server;
	PaMainloop mainloop;
	QList<PaDeferredTask*> taskQueue;

	PaContext(const QString &server = QString::null) :
	    ready(false),
	    server(server)
	{
		proplist = pa_proplist_new();

		if (!(context = pa_context_new_with_proplist(mainloop.api(), NULL, proplist))) {
			qDebug("pa_context_new() failed.");
			return;
		}

		pa_context_set_state_callback(context, context_state_callback, this);
	    if (pa_context_connect(context, server.toLocal8Bit().data(), PA_CONTEXT_NOFLAGS, NULL) < 0) {
	        qDebug("pa_context_connect() failed: %s", pa_strerror(pa_context_errno(context)));
	        return;
	    }

		mainloop.run();
	}

	~PaContext()
	{
		if (context)
	        pa_context_unref(context);

		if (proplist)
	        pa_proplist_free(proplist);
	}

	void tryExec(PaDeferredTask *t)
	{
		if (ready) {
			t->run();
		} else {
			taskQueue.append(t);
		}
	}
};





//------------------------------------------------------------
// PaDeferredTask
//------------------------------------------------------------
PaDeferredTask::PaDeferredTask(PaTask *task) :
	op(0),
	errorCode(0),
	task(task)
{
	context = static_cast<PaCtl*>(task->parent())->context();
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
	d->context->tryExec(d);
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
			setError(pa_context_errno(context->context));
			return;
		}

		if (is_last) {
			setSuccess();
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
		op = pa_context_get_source_info_list(context->context, get_source_info_callback, this);
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
class PaCtlPrivate
{
public:
	PaCtlPrivate()
	{
		context = new PaContext();
	}

	~PaCtlPrivate()
	{
		delete context;
	}

	PaContext *context;
};




PaCtl::PaCtl(QObject *parent) :
    QObject(parent),
	d(new PaCtlPrivate)
{

}

PaContext *PaCtl::context()
{
	return d->context;
}
