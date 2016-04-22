#include <pulse/introspect.h>
#include <pulse/mainloop.h>
#include <pulse/mainloop-signal.h>
#include <pulse/error.h>

#include <QExplicitlySharedDataPointer>

#include "pulsectl.h"


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

	static void get_source_info_callback(pa_context *c, const pa_source_info *i, int is_last, void *userdata)
	{
		static_cast<PaContext*>(userdata)->onSourceInfoReady(c, i, is_last);
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
				// TODO implement all stuff here
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

	QString server;
	PaMainloop mainloop;

	PaContext(const QString &server = QString::null) :
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

	void querySourceInfoList()
	{
		pa_operation *o = NULL;
		o = pa_context_get_source_info_list(context, get_source_info_callback, this);
	}
};


class PulseCtlPrivate
{
public:
	PulseCtlPrivate()
	{
		context = new PaContext();
	}

	~PulseCtlPrivate()
	{
		delete context;
	}

	PaContext *context;
};




PulseCtl::PulseCtl(QObject *parent) :
    QObject(parent),
	d(new PulseCtlPrivate)
{

}

PATask *PulseCtl::sourcesList()
{

}



