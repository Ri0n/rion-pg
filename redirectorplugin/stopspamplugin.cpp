/*
 * stopspamplugin.cpp - plugin
 * Copyright (C) 2009-2011  Khryukin Evgeny
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <QDomElement>

#include "psiplugin.h"
#include "optionaccessor.h"
#include "optionaccessinghost.h"
#include "stanzafilter.h"
#include "stanzasender.h"
#include "stanzasendinghost.h"
#include "accountinfoaccessor.h"
#include "accountinfoaccessinghost.h"
#include "applicationinfoaccessor.h"
#include "applicationinfoaccessinghost.h"
#include "popupaccessor.h"
#include "popupaccessinghost.h"
#include "iconfactoryaccessor.h"
#include "iconfactoryaccessinghost.h"
#include "plugininfoprovider.h"
#include "eventfilter.h"
#include "contactinfoaccessinghost.h"
#include "contactinfoaccessor.h"

#include "ui_options.h"
#include "deferredstanzasender.h"

#define cVer "0.0.1"

class Redirector: public QObject, public PsiPlugin, public OptionAccessor, public StanzaSender,  public StanzaFilter,
public AccountInfoAccessor, public ApplicationInfoAccessor,
public PluginInfoProvider, public EventFilter, public ContactInfoAccessor
{
	Q_OBJECT
	Q_INTERFACES(PsiPlugin OptionAccessor StanzaSender StanzaFilter AccountInfoAccessor ApplicationInfoAccessor
				 PluginInfoProvider EventFilter ContactInfoAccessor)

public:
	inline Redirector() : enabled(false)
	  , psiOptions(0)
	  , stanzaHost(0)
	  , accInfoHost(0)
	  , appInfoHost(0)
	  , contactInfo(0) {}
	QString name() const { return "Stop Spam Plugin"; }
	QString shortName() const { return "stopspam"; }
	QString version() const { return cVer; }
	//PsiPlugin::Priority priority() {return PriorityNormal;}
	QWidget* options();
	bool enable();
	bool disable();
	void applyOptions();
	void restoreOptions();
	void setOptionAccessingHost(OptionAccessingHost* host) { psiOptions = host; }
	void optionChanged(const QString& ) {}
	void setStanzaSendingHost(StanzaSendingHost *host);
	bool incomingStanza(int account, const QDomElement& xml);
	bool outgoingStanza(int account, QDomElement& xml);
	void setAccountInfoAccessingHost(AccountInfoAccessingHost* host) { accInfoHost = host; }
	void setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host);
	void setContactInfoAccessingHost(ContactInfoAccessingHost* host);
	QString pluginInfo();

	bool processEvent(int , QDomElement& ) { return false; }
	bool processMessage(int , const QString& , const QString& , const QString& ) { return false; }
	bool processOutgoingMessage(int account, const QString& fromJid, QString& body, const QString& type, QString& subject);
	void logout(int ) {}

private slots:

private:


	bool enabled;
	OptionAccessingHost* psiOptions;
	DefferedStanzaSender* stanzaHost;
	AccountInfoAccessingHost *accInfoHost;
	ApplicationInfoAccessingHost *appInfoHost;
	ContactInfoAccessingHost* contactInfo;

	Ui::Options ui_;
};

Q_EXPORT_PLUGIN(Redirector);


bool Redirector::enable() {
	if (psiOptions) {
		enabled = true;
	}
	return enabled;
}

bool Redirector::disable() {
	enabled = false;
	return true;
}

void Redirector::applyOptions() {
	if (!options_)
		return;

	psiOptions->setPluginOption("jid", ui_.le_jid->text());
}

void Redirector::restoreOptions() {
	if (!options_)
		return;

	ui_.le_jid->setText(psiOptions->getPluginOption("jid").toString());
}

QWidget* Redirector::options() {
	if (!enabled) {
		return 0;
	}
	options_ = new QWidget();
	ui_.setupUi(options_);

	restoreOptions();

	return options_;
}

void Redirector::setStanzaSendingHost(StanzaSendingHost *host) {
	stanzaHost = new DefferedStanzaSender(host);
}

void Redirector::setAccountInfoAccessingHost(AccountInfoAccessingHost* host) {
	accInfoHost = host;
}

void Redirector::setApplicationInfoAccessingHost(ApplicationInfoAccessingHost* host) {
	appInfoHost = host;
}

void Redirector::setContactInfoAccessingHost(ContactInfoAccessingHost *host) {
	contactInfo = host;
}

bool Redirector::incomingStanza(int account, const QDomElement& stanza) {
	if (enabled) {
		if(stanza.tagName() == "iq") {
			QDomElement query = stanza.firstChildElement("query");
			if(!Unblocked.isEmpty()
				&& !query.isNull()
				&& query.attribute("xmlns") == "jabber:iq:roster") {
				QStringList Roster = accInfoHost->getRoster(account);
				QStringList UnblockedList = Unblocked.split("\n");
				while(!Roster.isEmpty()) {
					QString jid = Roster.takeFirst();
					UnblockedList.removeOne(jid);
				}
				Unblocked = "";
				while(!UnblockedList.isEmpty()) {
					QString jid = UnblockedList.takeFirst();
					if(jid != "") {
						Unblocked += jid + "\n";
					}
				}
				psiOptions->setPluginOption(constUnblocked, QVariant(Unblocked));
			}
		}


		QString from = stanza.attribute("from");
		QString to = stanza.attribute("to");
		QString valF = from.split("/").takeFirst();
		QString valT = to.split("/").takeFirst();

		if(valF.toLower() == valT.toLower()
			|| valF.toLower() == accInfoHost->getJid(account).toLower())
			return false;

		if(!from.contains("@"))
			return false;

		// Нам необходимо сделать эту проверку здесь,
		// иначе мы рискуем вообще ее не сделать
		if (stanza.tagName() == "message") {
			bool findInvite = false;
			QString invFrom;
			QDomElement x = stanza.firstChildElement("x");
			while(!x.isNull()) {
				QDomElement invite = x.firstChildElement("invite");
				if(!invite.isNull()) {
					findInvite = true;
					invFrom = invite.attribute("from");
					break;
				}
				x = x.nextSiblingElement("x");
			}
			if(findInvite) {  // invite to MUC
				QStringList r = accInfoHost->getRoster(account);
				if(r.contains(invFrom.split("/").first(), Qt::CaseInsensitive))
					return false;
				else {
					bool findRule = false;
					for(int i = 0; i < Jids.size(); i++) {
						QString jid_ = Jids.at(i);
						if(jid_.isEmpty())
							continue;
						if(invFrom.contains(jid_, Qt::CaseInsensitive)) {
							findRule = true;
							if(!selected[i].toBool())
								return false;
							break;
						}
					}
					if(!findRule && DefaultAct)
						return false;
					else {
						updateCounter(stanza, false);
						return true;
					}
				}
			}
		}

		if(contactInfo->isConference(account, valF)
			|| contactInfo->isPrivate(account, from)
			|| findMucNS(stanza))
			{
			if(UseMuc)
				return processMuc(account, stanza);
			else
				return false;
		}

		QStringList Roster = accInfoHost->getRoster(account);
		if(Roster.isEmpty() || Roster.contains("-1"))
			return false;
		if(Roster.contains(valF, Qt::CaseInsensitive))
			return false;

		QStringList UnblockedJids = Unblocked.split("\n");
		if(UnblockedJids.contains(valF, Qt::CaseInsensitive))
			return false;

		bool findRule = false;
		for(int i = 0; i < Jids.size(); i++) {
			QString jid_ = Jids.at(i);
			if(jid_.isEmpty())
				continue;
			if(from.contains(jid_, Qt::CaseInsensitive)) {
				findRule = true;
				if(!selected[i].toBool())
					return false;
				break;
			}
		}
		if(!findRule && DefaultAct)
			return false;

		if (stanza.tagName() == "message") {
			QString subj = stanza.firstChildElement("subject").text();
			QString type = "";
			type = stanza.attribute("type");
			if(type == "error" && subj == "Redirector Question") {
				updateCounter(stanza, false);
				return true;
			}

			if (subj == "AutoReply" || subj == "Redirector" || subj == "Redirector Question")
				return false;

			if(type == "groupchat" || type == "error")
				return false;

			QDomElement captcha = stanza.firstChildElement("captcha");
			if(!captcha.isNull() && captcha.attribute("xmlns") == "urn:xmpp:captcha")
				return false; // CAPTCHA

			QDomElement Body = stanza.firstChildElement("body");
			if(!Body.isNull()) {
				QString BodyText = Body.text();
				if(BodyText == Answer) {
					Unblocked += valF + "\n";
					psiOptions->setPluginOption(constUnblocked, QVariant(Unblocked));
					psiOptions->setPluginOption(constLastUnblock, QVariant(QDate::currentDate().toString("yyyyMMdd")));
					stanzaHost->sendMessage(account, from, Congratulation, "Redirector", "chat");
					updateCounter(stanza, true);
					if(LogHistory)
						logHistory(stanza);
					return true;
				}
				else {
					int i = BlockedJids.size();
					if(findAcc(account, valF, i)) {
						Blocked &B = BlockedJids[i];
						if(B.count < Times) {
							stanzaHost->sendMessage(account, from,  Question, "Redirector Question", "chat");
							updateCounter(stanza, false);
							if(LogHistory)
								logHistory(stanza);
							B.count++;
							B.LastMes = QDateTime::currentDateTime();
							return true;
						}
						else {
							if(QDateTime::currentDateTime().secsTo(B.LastMes) >= -ResetTime*60) {
								updateCounter(stanza, false);
								if(LogHistory)
									logHistory(stanza);
								return true;
							}
							else {
								B.count = 1;
								B.LastMes = QDateTime::currentDateTime();
								stanzaHost->sendMessage(account, from,  Question, "Redirector Question", "chat");
								updateCounter(stanza, false);
								if(LogHistory)
									logHistory(stanza);
								return true;
							}
						}
					}
					else {
						Blocked B = { account, valF, 1, QDateTime::currentDateTime() };
						BlockedJids << B;
						stanzaHost->sendMessage(account, from,  Question, "Redirector Question", "chat");
						updateCounter(stanza, false);
						if(LogHistory)
							logHistory(stanza);
						return true;
					}
				}
			}
			updateCounter(stanza, false);
			return true;
		}

		if (stanza.tagName() == "presence") {
			QString type = stanza.attribute("type");
			if(type == "subscribe") {
				stanzaHost->sendMessage(account, from,  Question, "Redirector Question", "chat");
				stanzaHost->sendStanza(account, "<presence type=\"unsubscribed\" to=\"" + valF + "\" />");
				updateCounter(stanza, false);
				if(LogHistory)
					logHistory(stanza);
				return true;
			}
			else
				return false;
		}

		if (stanza.tagName() == "iq" && stanza.attribute("type") == "set") {
			QString msg = QString("<iq type=\"error\" id=\"%1\" ").arg(stanza.attribute("id"));
			if(!from.isEmpty())
				msg += QString("to=\"%1\"").arg(from);
			msg += " />";
			stanzaHost->sendStanza(account, msg);
			updateCounter(stanza, false);
			return true;
		}

		return false;
	}
	return false;
}

bool Redirector::outgoingStanza(int /*account*/, QDomElement& /*xml*/) {
	return false;
}

bool Redirector::processOutgoingMessage(int acc, const QString &fromJid, QString &body, const QString &type, QString &/*subject*/) {
	if(enabled && type != "groupchat" && !body.isEmpty()) {
		QString bareJid;
		if(contactInfo->isPrivate(acc, fromJid)) {
			bareJid = fromJid;
		}
		else {
			bareJid =  fromJid.split("/").first();
			if(contactInfo->inList(acc, bareJid))
				return false;
		}
		if(!Unblocked.split("\n").contains(bareJid, Qt::CaseInsensitive)) {
			Unblocked += bareJid + "\n";
			psiOptions->setPluginOption(constUnblocked, QVariant(Unblocked));
			psiOptions->setPluginOption(constLastUnblock, QVariant(QDate::currentDate().toString("yyyyMMdd")));
		}
	}
	return false;
}


QString Redirector::pluginInfo() {
	return tr("Author: ") +  "rion\n"
			+ tr("Email: ") + "rion4ik@gmail.com\n\n"
			+ trUtf8("Redirects all incoming messages to some jid and allows to redirect messages back.");
}

#include "stopspamplugin.moc"
