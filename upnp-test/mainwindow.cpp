#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "natportmapper.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mapper(new NatPortMapper(this))
{
    ui->setupUi(this);
    connect(mapper, SIGNAL(initialized()), SLOT(mapperReady()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mapperReady()
{
    ui->lblStatus->setText("Initialized!!!");
}
