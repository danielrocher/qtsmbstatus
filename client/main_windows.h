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

#include <QSslSocket>


#include "login_windows.h"
#include "configure_windows.h"
#include "mainwindows.h"


extern bool autoconnect;

class Host {
public:
	QString address;
	QString sha1;
	bool operator == (Host other) const {
		return (other.address == address && other.sha1 == sha1);
	};
};


class main_windows : public MainWindows  {
   Q_OBJECT
public:
	main_windows(QWidget *parent=0);
	virtual ~main_windows();
private slots: // Private slots
	virtual void slot_timer();
	virtual void on_pushButton_connect_clicked();
	virtual void sendMessage(const QString & machine,const QString & message);
	virtual void disconnectUser(const QString & pid,const QString & user);
	virtual void on_configureAction_triggered();
	void socketConnected();
	void socketClosed();
	void core();
	void error(QAbstractSocket::SocketError);
	void SslErrors (const QList<QSslError> &);
	void errorAuth();
private: // Private attributes
	QStringList ListSmbstatus;
	QSslSocket sslSocket;
	QAction * connect_action;
	/**
		These enums describe connection state.
		\sa setWidgetState socketState
	*/
	enum socket_state {UnconnectedState,ConnectingState,ConnectedState};
	/**
		These enums describe protocol between %server and client.
		\sa core_syntax
	*/
	enum command {auth_rq,auth_ack,end,kill_user,send_msg,smb_rq,smb_data,end_smb_rq,whoiam,
		server_info,error_auth,error_command,error_obj,echo_request,echo_reply};
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
};

#endif

