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


#ifndef MAIN_WINDOWS_H
#define MAIN_WINDOWS_H

#include <QMainWindow>
#include <QApplication>
#include <QSslSocket>
#include <QTimer>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QErrorMessage>
#include <QHttp>

#include "build/ui/ui_form_smbstatus.h"
#include "login_windows.h"
#include "configure_windows.h"
#include "log.h"
#include "server.h"
#include "smbstatus.h"
#include "../common/core_syntax.h"

extern void debugQt(const QString & message);

extern bool autoconnect;
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
	void slot_timer();
	void helpAbout();
	void helpAboutQt();
	void ConfigureSlot();
	void AllSearchSlot();
	void NextSlot();
	void Slot_connect();
	void socketConnected();
	void socketClosed();
	void core();
	void InfoServer();
	void InfoUser();
	void InfoService();
	void InfoMachine();
	void slotSendMessage();
	void slotSendMessageAllUsers();
	void slotDisconnectUser();
	void slotPopupMenu( const QPoint & );
	void error(QAbstractSocket::SocketError);
	void SslErrors (const QList<QSslError> &);
	void errorAuth();
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
private: // Private attributes
	QErrorMessage * msgError;
	QStringList ListSmbstatus;
	server * item_server;
	bool permitDisconnectUser;
	bool permitSendMsg;
	QMenu* menuPopup;
	QTreeWidgetItem* currentPopupMenuItem;
	QSslSocket sslSocket;
	QTreeWidgetItem * FindItem;
	//! direction of search 
	enum T_Direction {to_first,to_preview,to_next};
	QString SearchTxt;
	int currentIndexOfListItem;
	QAction * configure_action;
	QAction * restore_action;
	QAction * connect_action;
	QAction * viewlog_action;
	QSystemTrayIcon * trayicon;
	QTimer timerSmbRequest;
	QTimer timerinfoSmb;
	bool firstTime;
	LogForm * logform;
	smbstatus * InstanceSmbstatus;
	/**
		These enums describe connection state.
		\sa setWidgetState socketState
	*/
	enum socket_state {UnconnectedState,ConnectingState,ConnectedState};
	/**
		These enums describe protocol between server and client.
		\sa core_syntax
	*/
	enum command {auth_rq,auth_ack,end,kill_user,send_msg,smb_rq,smb_data,end_smb_rq,whoiam,server_info,error_auth,error_command,error_obj,echo_request,echo_reply};
	QHttp * http;
protected:
	virtual void closeEvent(QCloseEvent *e);
private: // Private methods
	void infoserver(const QString & text);
	void setWidgetState();
	void Disconnect();
	void sendToServer(int cmd,const QString & inputArg1="",const QString & inputArg2="");
	short unsigned int socketState();
	void writeHistoryFile();
	void readHistoryFile();
	void comboBox_valid();
	void open_dialog_for_login();
	void search(T_Direction direction=to_first);
	void selectItem(QTreeWidgetItem *item);
	void restoreWindowSize();
	void saveWindowSize();
	void AnalysisSmbstatus();
	void checkNewVersionOfQtSmbstatus ();
};

#endif
