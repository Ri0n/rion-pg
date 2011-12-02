#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "appupdater.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	updater = new AppUpdater("MyApp Name", QUrl("http://localhost/version.file"), "1.2.3", this);
	connect(updater, SIGNAL(checkFinished()), SLOT(checkFinished()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_pbCheckUpdates_clicked()
{
	updater->check();
}

void MainWindow::checkFinished()
{
	QString noUpdates = updater->error().isEmpty()? "No updates" : updater->error();
	ui->lblCheckStatus->setText(updater->version() == updater->newVersion()?
									noUpdates : QString("Update found: %1").arg(updater->newVersion()));
}
