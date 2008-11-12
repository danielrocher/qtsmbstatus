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

#include <QSslSocket>
#include <QTimer>
#include "pamthread.h"

class ClientSocket: public QSslSocket
{
    Q_OBJECT
public:
	ClientSocket(QObject* parent=0);
	virtual ~ClientSocket();
	static int compteur_objet;
private slots:
	void core ();
	void SocketError();
	void SslErrors (const QList<QSslError> & listError);
	void socketEncrypted();
	void slot_pam();
	void ObjError(const QString & error_txt);
	void slot_smbstatus(const QStringList &);
private:
	//! if user authenticated
	bool AuthUser;
	//! if client is authorized to disconnect user
	bool permitDisconnectUser;
	//! if client is authorized to send popup message
	bool permitSendMsg;
	PamThread * pamthread;
	QTimer *pamTimer;
	enum command {auth_rq,auth_ack,end,kill_user,send_msg,smb_rq,smb_data,end_smb_rq,whoiam,server_info,error_auth,error_command,error_obj,echo_request,echo_reply} ;
	void sendToClient(int cmd,const QString & inputArg1="",const QString & inputArg2="");
	void CmdKillUser(const QString & texte);
	void CmdSendMsg(const QString & texte);
	void CmdSmbRq();
	void CmdAuthRq(const QString & texte);
};

#endif

