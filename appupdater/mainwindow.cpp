#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "appupdater.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	updater = new AppUpdater("MyApp Name", QUrl("http://localhost/test.php"), "1.2.3", this);
	updater->setType(AppUpdater::TypeVersionHeader);
	connect(updater, SIGNAL(checkFinished()), SLOT(checkFinished()));
	connect(updater, SIGNAL(finished()), SLOT(updateFinished()));
	connect(updater, SIGNAL(checkEnabled(bool)), SLOT(checksEnabled(bool)));
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
	updater->setDetails("Hello world");
}

void MainWindow::updateFinished()
{
	if (!updater->filename().isEmpty()) { // no errors or cancel
		ui->lblProposedFilename->setText(updater->proposedFilename());
		ui->lblTempFilename->setText(updater->filename());
	} else {
		ui->lblCheckStatus->setText(updater->error());
		ui->lblProposedFilename->setText("");
		ui->lblTempFilename->setText("");
	}
}

void MainWindow::checksEnabled(bool s)
{
	ui->lblCheckStatus->setText(s?"checks enable requested":"checks disable requested");
}
