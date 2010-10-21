#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "popup.h"

#include <QApplication>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	if (!Popup::init(":/defaulttheme.ui")) {
		qWarning("Failed to load Popup theme");
	}
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

void MainWindow::on_btnPopup_clicked()
{
	Popup *popup = new Popup(this, ui->leTitle->text(), ui->teText->toPlainText());
	popup->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint | Qt::Window);
	popup->show();
	//popup->setParent(this);
}
