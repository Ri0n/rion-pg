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

#ifndef PaCtl_H
#define PaCtl_H

#include <QObject>


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

class PaTaskSinks : public PaTask
{
	Q_OBJECT

public:
	PaTaskSinks(PaCtl *parent = 0);

	struct Sink {
		QString id;
		QString description;
		QString monitorId; // id of monitor (a source we mirror into)
	};

	QList<Sink> sinks() const;
};

class PaTaskServerInfo : public PaTask
{
	Q_OBJECT

public:
	PaTaskServerInfo(PaCtl *parent = 0);

	struct ServerInfo {
		QString hostName;
		QString serverVersion;
		QString serverName;
		QString defaultSinkName;
		QString defaultSourceName;
	};

	const ServerInfo &info() const;
};

class PaCtlPrivate;
class PaCtl : public QObject
{
	Q_OBJECT
public:

	explicit PaCtl(QObject *parent = 0);
	~PaCtl();

signals:

public slots:

private:
	friend class PaCtlPrivate;
	friend class PaDeferredTask;
	class PaCtlPrivate *d;
};

#endif // PaCtl_H
