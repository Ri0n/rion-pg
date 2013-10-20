#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class NatPortMapper;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void mapperReady();
private:
    Ui::MainWindow *ui;
    NatPortMapper *mapper;
};

#endif // MAINWINDOW_H
