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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef MAIN_WINDOWS_H
#define MAIN_WINDOWS_H

#include <QMainWindow>
#include <QApplication>
#include <QSystemTrayIcon>

#include "ui_form_smbstatus.h"

#include "clientsocket.h"
#include "login_windows.h"
#include "configure_windows.h"
#include "log.h"

extern void debugQt(const QString & message);

extern bool autoconnect;
extern QString version_qtsmbstatus;
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

class main_windows : public QMainWindow, public Ui::form_smbstatus  {
   Q_OBJECT
public:
	main_windows(QWidget *parent=0);
	virtual ~main_windows();
public slots:
	void beforeQuit();
signals:
	void refreshviewlog(const type_message &);
private slots: // Private slots
	virtual void helpAbout();
	virtual void helpAboutQt();
	virtual void ConfigureSlot();
	virtual void MachineSearchSlot();
	virtual void UserSearchSlot();
	virtual void ShareSearchSlot();
	virtual void FileSearchSlot();
	virtual void AllSearchSlot();
	virtual void NextSlot();
	virtual void Slot_connect();
	virtual void socketconnected();
	virtual void socketclosed();
	virtual void SignalErrorAuth();
	void on_action_View_log_triggered ();
	void trayicon_activated(QSystemTrayIcon::ActivationReason reason);
	void restore_minimize();
	void configuration_changed();
	//! Info CIFS/SMB for log and balloon messages
	void InfoSMB();
private: // Private attributes
	bool connected;
	ClientSocket * clientsocket;
	Q3ListViewItem * FindItem;
	//! type of search (a machine, an user, ...)
	enum T_Item{T_All,T_Machine,T_User,T_Share,T_File};
	//! direction of search 
	enum T_Direction {to_first,to_preview,to_next};
	QString currentSearchStr;
	T_Item currentSearchItem;
	QAction * configure_action;
	QAction * restore_action;
	QAction * connect_action;
	QAction * viewlog_action;
	QSystemTrayIcon * trayicon;
	bool firstTime;
	LogForm * logform;
protected:
	virtual void closeEvent(QCloseEvent *e);
private: // Private methods
	void writeHistoryFile();
	void readHistoryFile();
	void comboBox_valid();
	void open_dialog_for_login();
	void search(T_Item typeOfSearch,const QString & str,T_Direction direction=to_first);
	void selectItem(Q3ListViewItem *item);
	QString getSearchStr(const QString & msg);
	Q3ListViewItem * nextItem(Q3ListViewItem * item);
	void restoreWindowSize();
	void saveWindowSize();

};

#endif
