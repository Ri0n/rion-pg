#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "firework.h"

#include <QMainWindow>
#include <QScopedPointer>
#include <memory>

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void newFirework();
    void doStep();
private:
    Ui::MainWindow *ui;
    std::list<std::unique_ptr<Firework>> fireworks;

};

#endif // MAINWINDOW_H
