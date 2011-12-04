#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class AppUpdater;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);

private slots:
	void on_pbCheckUpdates_clicked();
	void checkFinished();
	void updateFinished();
	void checksEnabled(bool s);

private:
    Ui::MainWindow *ui;
	AppUpdater *updater;
};

#endif // MAINWINDOW_H
