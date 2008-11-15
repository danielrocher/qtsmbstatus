/***************************************************************************
 *   Copyright (C) 2004 by Daniel Rocher                                   *
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


#ifndef MAIN_WINDOWSL_H
#define MAIN_WINDOWSL_H

#include <QMainWindow>
#include <QApplication>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QErrorMessage>
#include <QHttp>

#include "build/ui/ui_form_smbstatusl.h"
#include "configure_windowsl.h"
#include "../client/log.h"
#include "../client/server.h"
#include "../client/smbstatus.h"
#include "../server/smbmanager.h"
#include "../server/process_smbd_exist.h"
#include "../server/sendmessage_manager.h"

extern void debugQt(const QString & message);

extern bool autoconnect;
extern QString version_qtsmbstatus;
extern uint int_qtsmbstatus_version;
extern QString date_qtsmbstatus;
extern QString mail_qtsmbstatus;
extern QString auteur_qtsmbstatus;
extern QString web_qtsmbstatus;
extern bool debug_qtsmbstatus;

class MyApplicationL : public QApplication
{
	Q_OBJECT
public:
	MyApplicationL (int & argc, char ** argv );
	virtual void commitData(QSessionManager& manager);
signals:
	void quitMyApp();
};

class main_windowsl : public QMainWindow, public Ui::form_smbstatusl  {
   Q_OBJECT
public:
	main_windowsl(QWidget *parent=0);
	virtual ~main_windowsl();
public slots:
	void beforeQuit();
signals:
	void refreshviewlog(const type_message &);
private slots: // Private slots
	void slot_timer();
	void helpAbout();
	void helpAboutQt();
	void ConfigureSlot();
	void AllSearchSlot();
	void NextSlot();
	void InfoServer();
	void InfoUser();
	void InfoService();
	void InfoMachine();
	void slotSendMessage();
	void slotSendMessageAllUsers();
	void slotDisconnectUser();
	void slotPopupMenu( const QPoint & );
	void on_action_View_log_triggered ();
	void trayicon_activated(QSystemTrayIcon::ActivationReason reason);
	void restore_minimize();
	void configuration_changed();
	void setSambaVersion (const QString &);
	void add_user (const QString &,const QString &,const QString &,const QString &,const QString &);
	void add_share(const QString &,const QString &,const QString &);
	void add_lockedfile(const QString &,const QString &,const QString &,const QString &,const QString &,const QString &);
	void AnalysisSmbDestroyed();
	void InfoSMB();
	void requestHtmlFinished ( bool error );
	void AnalysisSmbstatus(const QStringList & listSmbstatus);
private: // Private attributes
	QErrorMessage * msgError;
	server * item_server;
	QMenu* menuPopup;
	QTreeWidgetItem* currentPopupMenuItem;
	QTreeWidgetItem * FindItem;
	/**
		direction of search 
		\sa search
	*/
	enum T_Direction {to_first,to_preview,to_next};
	QString SearchTxt;
	int currentIndexOfListItem;
	QAction * configure_action;
	QAction * restore_action;
	QAction * viewlog_action;
	QSystemTrayIcon * trayicon;
	QTimer timerSmbRequest;
	QTimer timerinfoSmb;
	bool firstTime;
	LogForm * logform;
	smbstatus * InstanceSmbstatus;
	QHttp * http;
protected:
	virtual void closeEvent(QCloseEvent *e);
private: // Private methods
	void search(T_Direction direction=to_first);
	void selectItem(QTreeWidgetItem *item);
	void restoreWindowSize();
	void saveWindowSize();
	void checkForUpdateOfQtSmbstatus ();
};

#endif
