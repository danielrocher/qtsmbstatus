/***************************************************************************
 *   Copyright (C) 2008 by Daniel Rocher                                   *
 *   daniel.rocher@adella.org                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/
 
 
 
#ifndef MAINWINDOWS_H
#define MAINWINDOWS_H


#include <QMainWindow>
#include <QApplication>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QErrorMessage>
#include <QHttp>

#include "build/ui/ui_form_smbstatus.h"
#include "log.h"
#include "server.h"
#include "smbstatus.h"
#include "../common/core_syntax.h"

extern void debugQt(const QString & message);

extern QString version_qtsmbstatus;
extern uint int_qtsmbstatus_version;
extern QString date_qtsmbstatus;
extern QString mail_qtsmbstatus;
extern QString auteur_qtsmbstatus;
extern QString web_qtsmbstatus;
extern bool debug_qtsmbstatus;

class MyApplication : public QApplication
{
  Q_OBJECT
public:
	MyApplication (int & argc, char ** argv );
	virtual void commitData(QSessionManager& manager);
signals:
	void quitMyApp();
};


class MainWindows : public QMainWindow, public Ui::form_smbstatus  {
  Q_OBJECT
public:
	MainWindows(QWidget *parent=0);
	virtual ~MainWindows();
signals:
	void refreshviewlog(const type_message &);
public slots:
	void beforeQuit();
protected slots:
	virtual void slot_timer() = 0;
	virtual void sendMessage(const QString & machine,const QString & message) = 0;
	virtual void disconnectUser(const QString & pid,const QString & user) = 0;
	virtual void on_configureAction_triggered()=0;
	virtual void on_pushButton_connect_clicked() {printf ("not implemented here.\n");}
	void AnalysisSmbstatus(const QStringList & listSmbstatus);
private slots: // Private slots
	void helpAbout();
	void helpAboutQt();
	void on_action_View_log_triggered ();
	void AllSearchSlot();
	void NextSlot();
	void requestHtmlFinished ( bool error );
	void trayicon_activated(QSystemTrayIcon::ActivationReason reason);
	void restore_minimize();
	void configuration_changed();
	void setSambaVersion (const QString &);
	void add_user (const QString &,const QString &,const QString &,const QString &,const QString &);
	void add_share(const QString &,const QString &,const QString &);
	void add_lockedfile(const QString &,const QString &,const QString &,const QString &,const QString &,const QString &);
	void AnalysisSmbDestroyed();
	void InfoSMB();
	void InfoServer();
	void InfoUser();
	void InfoService();
	void InfoMachine();
	void slotPopupMenu(const QPoint &);
	void slotSendMessage();
	void slotSendMessageAllUsers();
	void slotDisconnectUser();
protected: // protected attributes
	QMenu * menuApp;
	bool permitDisconnectUser;
	bool permitSendMsg;
	server * item_server;
	QHttp * http;
	QErrorMessage * msgError;
	QSystemTrayIcon * trayicon;
	QAction * configure_action;
	QAction * restore_action;
	QAction * viewlog_action;
	QMenu* menuPopup;
	QTreeWidgetItem* currentPopupMenuItem;
	/**
	direction of search 
	\sa search
	 */
	enum T_Direction {to_first,to_preview,to_next};
	QString SearchTxt;
	int currentIndexOfListItem;
	QTimer timerSmbRequest;
	QTimer timerinfoSmb;
	bool smbstatus_rq_started;
	bool firstTime;
	LogForm * logform;
protected: // Protected methods
	void restoreWindowSize();
	void saveWindowSize();
	void search(T_Direction direction=to_first);
	void selectItem(QTreeWidgetItem *item);
	void checkForUpdateOfQtSmbstatus ();
};

#endif

