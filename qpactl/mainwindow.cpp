#include <QtDBus/QtDBus>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "pulsectl.h"
#include <unistd.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	PaCtl *ctl = new PaCtl(this);

	PaTaskSources *t1 = new PaTaskSources(ctl);
	connect(t1, SIGNAL(ready()), SLOT(sourcesFinihed()));
	PaTaskSinks *t2 = new PaTaskSinks(ctl);
	connect(t2, SIGNAL(ready()), SLOT(sinksFinihed()));
	PaTaskServerInfo *t3 = new PaTaskServerInfo(ctl);
	connect(t3, SIGNAL(ready()), SLOT(serverInfoFinihed()));
	t1->run();
	t2->run();
	t3->run();

}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::sourcesFinihed()
{
	QList<PaTaskSources::Source> sources = ((PaTaskSources *)sender())->sources();
	QString ts;

	foreach (const PaTaskSources::Source &s, sources) {
		ts += (QString("Source: ") + s.id + " " + s.description + " " + s.monitorId + " " + "\n");
	}

	ui->teLog->appendPlainText(QString("Sources:\n") + ts + "\n");
}

void MainWindow::sinksFinihed()
{
	QList<PaTaskSinks::Sink> sinks = ((PaTaskSinks *)sender())->sinks();
	QString ts;
	foreach (const PaTaskSinks::Sink &s, sinks) {
		ts += (QString("Sink: ") + s.id + " " + s.description + " " + s.monitorId + " " + "\n");
	}

	ui->teLog->appendPlainText(QString("Sinks:\n") + ts + "\n");
}

void MainWindow::serverInfoFinihed()
{
	const PaTaskServerInfo::ServerInfo &info = ((PaTaskServerInfo *)sender())->info();
	QString s = "Server info:\n";
	s += ("Default sink: " + info.defaultSinkName + "\n");
	s += ("Default source: " + info.defaultSourceName + "\n");

	ui->teLog->appendPlainText(s);
}
