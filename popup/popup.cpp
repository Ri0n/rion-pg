#include "popup.h"
#include "ui_popup.h"

#include <QUiLoader>
#include <QFile>
#include <QDebug>
#include <QWidget>
#include <QVBoxLayout>
#include <QPalette>
#include <QLabel>

class Popup::Private
{
public:
	QString title;
	QString text;

	static QBuffer uiBuffer;
	static int curPos;
	static Qt::Alignment alignment;
};

QBuffer Popup::Private::uiBuffer;
int Popup::Private::curPos = 0;
Qt::Alignment Popup::Private::alignment = Qt::AlignRight | Qt::AlignBottom;

bool Popup::init(const QString &fileName)
{
	QFile uiFile(fileName);
	if (uiFile.open(QIODevice::ReadOnly)) {
		Private::uiBuffer.setData(uiFile.readAll());
		uiFile.close();
		return true;
	}
	return false;
}

Popup::Popup(QWidget *parent, const QString &title, const QString &text) :
	QWidget(parent),
	ui(new Ui::Popup)
{
    ui->setupUi(this);
	d = new Private;
	d->text = text;
	d->title = title;

	QUiLoader loader;
	Private::uiBuffer.open(QIODevice::ReadOnly);
	QWidget *popupWidget = loader.load(&Private::uiBuffer, this);
	Private::uiBuffer.close();
	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);
	layout->addWidget(popupWidget);

	QLabel *lblTitle = popupWidget->findChild<QLabel *>("lblTitle");
	QLabel *lblText = popupWidget->findChild<QLabel *>("lblText");
	QLabel *lblImage = popupWidget->findChild<QLabel *>("lblImage");
	if (lblTitle) {
		lblTitle->setText(title);
	}
	if (lblText) {
		lblText->setText(text);
	}
	if (lblImage) {
		lblImage->setStyleSheet("width:48px; height:48px;");
		lblImage->setText("<img src=\":/juick.png\" width='48' height='48' />");
	}

	QPalette palette;
	palette.setBrush(QPalette::Base, Qt::transparent);
	setPalette(palette);
	setAttribute(Qt::WA_TranslucentBackground, true);
	//qDebug() << popupWidget->metaObject()->className();
}

Popup::~Popup()
{
    delete ui;
	delete d;
}

void Popup::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Popup::mousePressEvent(QMouseEvent *event)
{
	close();
}
