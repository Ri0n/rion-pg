#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QDomDocument>
#include <QTextDocument>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
#ifdef Q_OS_LINUX
    const char *linuxTidy = "/usr/bin/tidy";
    if (QFileInfo(linuxTidy).exists()) {
        tidyLoc = linuxTidy;
        ui->tidyLe->setText(linuxTidy);
    }
	const char *linuxQHelpgen = "/usr/bin/qhelpgenerator";
	if (QFileInfo(linuxQHelpgen).exists()) {
		helpgenLoc = linuxQHelpgen;
		ui->helpgenLe->setText(linuxQHelpgen);
	}
#endif
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::tidiedFile(const QString &fileName) const
{
    QProcess p;
    p.start(tidyLoc, QStringList() << "-asxml" << "-numeric");
    if (!p.waitForStarted()) {
        qDebug("failed to start tidy process");
        return QString::null;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug("failed to open html file");
        return QString::null;
    }
    p.write(file.readAll());
    p.closeWriteChannel();
    if (!p.waitForFinished()) {
        qDebug("tidy process does not finish or something else broken..");
        return QString::null;
    }

    return p.readAll();
}

void MainWindow::on_indexBtn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose index.html file from glibc documentation"),
                                 QDir::homePath(),
                                 tr("Html (index.html)"));
    if (!fileName.isNull() && QFileInfo(fileName).exists()) {
        indexLoc = fileName;
        ui->indexLe->setText(fileName);
    }
}

void MainWindow::on_tidyBtn_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose tidy executable"),
                                 QDir::homePath());
    if (!fileName.isNull() && QFileInfo(fileName).exists()) {
        tidyLoc = fileName;
        ui->tidyLe->setText(fileName);
    }
}

void MainWindow::on_helpgenBtn_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Choose qhelpgenerator executable"),
								 QDir::homePath());
	if (!fileName.isNull() && QFileInfo(fileName).exists()) {
		helpgenLoc = fileName;
		ui->helpgenLe->setText(fileName);
	}
}

void MainWindow::on_generateBtn_clicked()
{
    if (!QFileInfo(indexLoc).exists() or !QFileInfo(tidyLoc).exists()) {
        QMessageBox::critical(this, "Can't generate", "Please fill all fields");
        return;
    }
    QString indexStr = tidiedFile(indexLoc);
    if (indexStr.isNull()) {
        return;
    }
    QDomDocument indexDoc;
    if (!indexDoc.setContent(indexStr)) {
        return;
    }

	// prepare temporary directory
	QString tempDir = QFileInfo(indexLoc).absolutePath() + "/temp";
	if (QFileInfo(tempDir).exists()) {
		foreach (const QFileInfo &entry, QDir(tempDir).entryInfoList(QDir::Files)) {
			if (!QFile::remove(entry.filePath())) {
				QMessageBox::warning(this, "Error", "Failed to remove files from temporary dir " + tempDir);
				return;
			}
		}
	}
	else {
		if (!QDir(QFileInfo(indexLoc).absolutePath()).mkdir("temp")) {
			QMessageBox::warning(this, "Error", "Failed to create temporary dir " + tempDir);
			return;
		}
	}

    QDomNodeList divList = indexDoc.elementsByTagName("div");
    QDomElement rootUl;
    for (unsigned int i = 0; i < divList.length(); i++) {
        QDomElement divEl = divList.item(i).toElement();
        if (divEl.attribute("class") == "contents") {
            for (unsigned int j = 0; j < divEl.childNodes().length(); j++) {
                QDomElement divChild = divEl.childNodes().item(j).toElement();
                if (!divChild.isNull() && divChild.nodeName() == "ul") {
                    rootUl = divChild;
                    break;
                }
            }
            if (rootUl.isNull()) {
                qDebug("Invalid index file");
                return;
            } else {
                break;
            }
        }
    }
    if (rootUl.isNull()) {
        qDebug("Contents is not found in index file");
        return;
    }

    QString rootSectionTitle = "Glibc Manual";
    QString version = "1.0";
    QString filterName = "glibc";
    QString filterDesc = "Glibc Doc";
    QString docNamespace = QString("ru.skydns.%1.%2").arg(filterName).arg(version);

    QDomDocument out;
    QDomElement prohectEl = out.appendChild(out.createElement("QtHelpProject")).toElement();
    prohectEl.setAttribute("version", version);
    prohectEl.appendChild(out.createElement("namespace")).appendChild(out.createTextNode(docNamespace));
    prohectEl.appendChild(out.createElement("virtualFolder")).appendChild(out.createTextNode(filterName));

    QDomElement customFilter = prohectEl.appendChild(out.createElement("customFilter")).toElement();
    customFilter.setAttribute("name", filterDesc);
    customFilter.appendChild(out.createElement("filterAttribute")).appendChild(out.createTextNode(filterName));
    customFilter.appendChild(out.createElement("filterAttribute")).appendChild(out.createTextNode(version));

    QDomElement filter = prohectEl.appendChild(out.createElement("filterSection")).toElement();
    filter.setAttribute("name", filterDesc);
    filter.appendChild(out.createElement("filterAttribute")).appendChild(out.createTextNode(filterName));
    filter.appendChild(out.createElement("filterAttribute")).appendChild(out.createTextNode(version));

    QDomElement rootSection = filter.appendChild(out.createElement("toc"))
            .appendChild(out.createElement("section")).toElement();
    rootSection.setAttribute("title", rootSectionTitle);
    rootSection.setAttribute("ref", "index.html");

    QHash<QString, QString> keywords;
    recursiveSectionAdd(rootUl, rootSection, keywords);
    addIndex(QFileInfo(indexLoc).absolutePath() + "/Concept-Index.html", keywords);
    addIndex(QFileInfo(indexLoc).absolutePath() + "/Type-Index.html", keywords);
    addIndex(QFileInfo(indexLoc).absolutePath() + "/Function-Index.html", keywords);
    addIndex(QFileInfo(indexLoc).absolutePath() + "/Variable-Index.html", keywords);
    addIndex(QFileInfo(indexLoc).absolutePath() + "/File-Index.html", keywords);

    QDomElement keywordsEl = filter.appendChild(out.createElement("keywords")).toElement();
    foreach (const QString &title, keywords.keys()) {
        QDomElement keywordEl = keywordsEl.appendChild(out.createElement("keyword")).toElement();
        keywordEl.setAttribute("name", title);
        keywordEl.setAttribute("id", title);
        keywordEl.setAttribute("ref", keywords.value(title));
    }

	QDomElement filesEl = filter.appendChild(out.createElement("files")).toElement();
	filesEl.appendChild(out.createElement("file")).appendChild(out.createTextNode("*.html"));
	filesEl.appendChild(out.createElement("file")).appendChild(out.createTextNode("main.css"));

	QString qhpFileName = tempDir + "/index.qhp";
    QFile qhpFile(qhpFileName);
	if (!qhpFile.open(QIODevice::WriteOnly)) {
		qDebug("Failed to open %s for write", qPrintable(qhpFileName));
		return;
	}
	qhpFile.write(out.toString().toUtf8());
	qhpFile.close();
	if (helpgenLoc.isEmpty()) {
		QMessageBox::information(this, "Qt help project saved.", qhpFileName +
								 "\n\nYou can manually start qhelpgenerator to convert it into qch file");
		return;
	}

	// insert styles into each html file
	QFile css(tempDir + "/main.css");
	css.open(QIODevice::WriteOnly);
	css.write("html, body { background-color:#ffffff; color:#000000; }");
	css.close();
	foreach (const QFileInfo &entry, QDir(QFileInfo(indexLoc).absolutePath()).entryInfoList(QDir::Files)) {
		if (entry.suffix() != "html") {
			QFile(entry.filePath()).copy(tempDir + "/" + entry.fileName());
			continue;
		}
		QFile f(entry.filePath());
		f.open(QIODevice::ReadOnly);
		QString content = QString::fromAscii(f.readAll());
		f.close();
		int pos = content.indexOf("<head>", 0, Qt::CaseInsensitive);
		if (pos != -1) {
			content.insert(pos + 6, "\n<link type=\"text/css\" rel=\"stylesheet\" href=\"main.css\">\n");
		}
		QFile of(tempDir + "/" + entry.fileName());
		of.open(QIODevice::WriteOnly);
		of.write(content.toUtf8());
		of.close();
	}

	QString qchFileName = tempDir + "/glibc.qch";
	QProcess p;
	p.start(helpgenLoc, QStringList() << qhpFileName << "-o" << qchFileName);
	if (!p.waitForFinished()) {
		QMessageBox::warning(this, "Generation failed", "Failed to generate qch file. The process didn't finish shortly");
		return;
	}
	if (!QFileInfo(qchFileName).exists()) {
		QMessageBox::warning(this, "Generation failed", "Failed to generate qch file.\n" + QString::fromLocal8Bit(p.readAll()));
		return;
	}
	QMessageBox::information(this, "Qt help generated and saved", qchFileName);
}

void MainWindow::recursiveSectionAdd(const QDomElement &source, QDomElement &destination, QHash<QString, QString> &keywords)
{
    unsigned int i, j;
    for (i = 0; i < source.childNodes().length(); i++) {
        QDomElement li = source.childNodes().item(i).toElement();
        if (li.isNull() || li.tagName() != "li") {
            continue;
        }
        QDomElement a, ul;
        for (j = 0; j < li.childNodes().length(); j++) {
            QDomElement el = li.childNodes().item(j).toElement();
            if (el.isNull()) continue;
            if (el.tagName() == "a") {
                a = el;
            } else if (el.tagName() == "ul") {
                ul = el;
            }
        }
        if (a.isNull()) continue;
        QDomElement section = destination.appendChild(destination.ownerDocument().createElement("section")).toElement();
        QString title = a.text().replace('\n', ' ').trimmed();
        QRegExp re("^(?:[A-Z]|\\d+)(?:\\.\\d+)* (.*)$");
        if (re.exactMatch(title))
            title = re.cap(1);
        section.setAttribute("title", title);
        section.setAttribute("ref", a.attribute("href"));

        keywords[title] = a.attribute("href");

        if (!ul.isNull()) {
            recursiveSectionAdd(ul, section, keywords);
        }
    }
}

void MainWindow::addIndex(const QString &fileName, QHash<QString, QString> &keywords)
{
    QString index = tidiedFile(fileName);
    if (index.isNull()) {
        return;
    }
    QDomDocument doc;
    if (doc.setContent(index)) {
        QDomNodeList ulList = doc.elementsByTagName("ul");
        if (!ulList.length()) {
            qDebug("%s", qPrintable("invalid index " + fileName));
            return;
        }
        QDomNodeList liList = ulList.item(0).childNodes();
        for (unsigned int i = 0; i < liList.length(); i++ ) {
            QDomElement li = liList.item(i).toElement();
            if (!li.isNull()) {
                for (unsigned int j = 0; j < li.childNodes().length(); j++) {
                    QDomElement a = li.childNodes().item(j).toElement();
                    if (!a.isNull() && a.tagName() == "a") {
                        keywords[a.text().replace('\n', '0').trimmed()] = a.attribute("href");
                        break;
                    }
                }
            }
        }
    }
}
