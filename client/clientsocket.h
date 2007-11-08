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


#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

#include <Q3CString>

#include "server.h"
#include "clientssl.h"
#include "smbstatus.h"
#include "../common/core_syntax.h"

extern void debugQt(const QString & message);

extern int interval;
extern QString host;
extern Q_UINT16 port_server;
extern bool view_hidden_shares;

class ClientSocket : public QObject
{
	Q_OBJECT
public:
	ClientSocket(Q3ListView * mylistView,QObject *parent=0);
	virtual ~ClientSocket();
	void Disconnect();
	void connectionToServer(const QString & username, const QString & passwd);
	static int compteur_objet;
private slots:
	void slot_timer();
	void socketconnected();
	void socketclosed();
	void UnreachableHost();
	void slotPopupMenu( Q3ListViewItem*, const QPoint &, int );
	void slotSendMessage();
	void slotSendMessageAllUsers();
	void slotDisconnectUser();
	void InfoServer();
	void InfoUser();
	void InfoService();
	void InfoMachine();
	void core(const Q3CString & rcv_txt);
	void setSambaVersion (const QString &);
	void add_user (const QString &,const QString &,const QString &,const QString &,const QString &);
	void add_share(const QString &,const QString &,const QString &);
	void add_lockedfile(const QString &,const QString &,const QString &,const QString &,const QString &,const QString &);
	void AnalysisSmbDestroyed();
	void slot_echo_timer();
signals:
	//! Client connected
	void SignalConnected();
	//! Connection closed
	void SignalClosed();
	//! Authentication error
	void SignalErrorAuth();
	//! Short messages for statusBar
	void SignalShortMessage(const QString &);

private: // Private methods
	void sendToServer(int cmd,const QString & inputArg1="",const QString & inputArg2="");
	void infoserver(const QString & text);
	void AnalysisSmbstatus();

private: // Private attributes
	bool permitDisconnectUser;
	bool permitSendMsg;
	ClientSSL* socketclient;
	bool ssl_connected;
	int echo;
	static int TimoutTimerEcho;
	QTimer * echo_timer;
	Q3PopupMenu* menu;
	Q3ListView * listView;
	QString passwd_login;
	QString username_login;
	server * item_server;
	QStringList ListSmbstatus;
	smbstatus * InstanceSmbstatus;
	QTimer* timer;
	QErrorMessage * msgError;
	Q3ListViewItem* currentPopupMenuItem;
	/**
		Protocol between server and client.
		\sa core_syntax
	*/
	enum command {auth_rq,auth_ack,end,kill_user,send_msg,smb_rq,smb_data,end_smb_rq,not_imp1,server_info,error_auth,error_command,error_obj,echo_request,echo_reply};
};

#endif
