#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDomElement>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString tidiedFile(const QString &) const;
    void recursiveSectionAdd(const QDomElement &source, QDomElement &destination, QHash<QString, QString> &keywords);
    void addIndex(const QString &fileName, QHash<QString, QString> &keywords);

private slots:
    void on_indexBtn_clicked();
    void on_generateBtn_clicked();
    void on_tidyBtn_clicked();

private:
    Ui::MainWindow *ui;
    QString indexLoc;
    QString tidyLoc;
};

#endif // MAINWINDOW_H
