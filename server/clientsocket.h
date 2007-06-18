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

#ifndef CLIENTSOCKET_H
#define CLIENTSOCKET_H

//SSL
#include<openssl/ssl.h>
#include <openssl/err.h>

//Qt
#include <QObject>

class Q3SocketDevice;
class QSocketNotifier;
class Q3CString;
class QTimer;

#include "process_smbd_exist.h"
#include "sendmessage_manager.h"
#include "smbmanager.h"
#include "pamthread.h"
#include "../common/core_syntax.h"

extern void debugQt(const QString & message);
extern void SSL_print_error(int errorcode);
extern void Socket_print_error (int errorcode);

extern bool debug_qtsmbstatus;
extern bool daemonize;
extern Q_UINT16 port_server;
extern QString Certificat;
extern QString Private_key;
extern QString ssl_password;
extern QString version_qtsmbstatus;
extern QStringList AllowUserDisconnect;
extern QStringList AllowUserSendMsg;

class ClientSocket: public QObject {
	Q_OBJECT
public:
	ClientSocket( const int & sock, QObject *parent=0);
	virtual ~ClientSocket();
	static int compteur_objet;
private: //attributes
	Q3SocketDevice *socketdevice;
	QSocketNotifier * sn_read;
	QSocketNotifier * sn_exception;
	int echo;
	static int TimoutTimerEcho;
	QTimer * echo_timer;
	int socket;
	//! if user authenticated
	bool AuthUser;
	//! if client is authorized to disconnect user
	bool permitDisconnectUser;
	//! if client is authorized to send popup message
	bool permitSendMsg;
	bool SSL_init;
	SSL* ssl;
	PamThread * pamthread;
	QTimer *timer;
	enum command {auth_rq,auth_ack,end,kill_user,send_msg,smb_rq,smb_data,end_smb_rq,not_imp1,server_info,error_auth,error_command,error_obj,echo_request,echo_reply} ;
private slots:
	void readClient();
	void Exception();
	void ObjError(const QString & error_txt);
	void slot_smbstatus(const QStringList &);
	void slot_pam();
	void slot_echo_timer();
public slots:
	void socketConnectionClose();
private://methods
	void sendToClient(int cmd,const QString & em_txt="");
	void core (const Q3CString & rcv_txt) ;
	void CmdKillUser(const QString & texte);
	void CmdSendMsg(const QString & texte);
	void CmdSmbRq();
	void CmdAuthRq(const QString & texte);
};

#endif
