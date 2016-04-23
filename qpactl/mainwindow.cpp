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

	PaTaskSources *t = new PaTaskSources(ctl);
	connect(t, SIGNAL(ready()), SLOT(sourcesFinihed()));
	sleep(1); // Hack!!!
	t->run();

}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::sourcesFinihed()
{
	QList<PaTaskSources::Source> sources = ((PaTaskSources *)sender())->sources();

	foreach (const PaTaskSources::Source &s, sources) {
		qDebug() << s.id << s.description << s.monitorId;
	}
}
