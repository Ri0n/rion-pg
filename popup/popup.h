#ifndef POPUP_H
#define POPUP_H

#include <QWidget>
#include <QBuffer>

namespace Ui {
    class Popup;
}

class Popup : public QWidget
{
    Q_OBJECT

public:
	static bool init(const QString &);

	explicit Popup(QWidget *parent = 0, const QString &title = QString(), const QString &text = QString());
    ~Popup();

protected:
    void changeEvent(QEvent *e);
	void mousePressEvent(QMouseEvent *event);

private:
    Ui::Popup *ui;
	class Private;
	Private *d;

};

#endif // POPUP_H
