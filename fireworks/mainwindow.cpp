#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "firework.h"

#include <QPainter>
#include <QDateTime>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qsrand(uint(QDateTime::currentSecsSinceEpoch()));

    auto timer = new QTimer(this);
    timer->setInterval(3000);
    timer->setSingleShot(false);
    connect(timer, &QTimer::timeout, this, &MainWindow::newFirework);
    timer->start();

    timer = new QTimer(this);
    timer->setInterval(20);
    timer->setSingleShot(false);
    connect(timer, &QTimer::timeout, this, &MainWindow::doStep);
    timer->start();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newFirework()
{
    auto fw = new Firework();
    fireworks.emplace_back(fw);
}

void MainWindow::doStep()
{
    auto it = fireworks.begin();
    while (it != fireworks.end()) {
        auto &f = *it;
        f->step();
        if (f->isFinished()) {
            fireworks.erase(it++);
        } else {
            ++it;
        }
    }
    repaint();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setBrush(QColor(Qt::black));
    painter.drawRect(event->rect());

    for (auto &f : fireworks) {
        f->paint(&painter);
    }
}
