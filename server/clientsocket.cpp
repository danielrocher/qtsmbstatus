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

#include "clientsocket.h"
#include "process_smbd_exist.h"
#include "sendmessage_manager.h"
#include "smbmanager.h"
#include "../common/core_syntax.h"

extern void debugQt(const QString & message);

extern QStringList AllowUserDisconnect;
extern QStringList AllowUserSendMsg;
extern uint int_qtsmbstatus_version;
extern QString version_qtsmbstatus;
extern QString date_qtsmbstatus;

int ClientSocket::compteur_objet=0;

/**
	\class ClientSocket
	\brief This class dialogs with client. (One object/client)
	\date 2008-11-06
	\version 2.0
	\author Daniel Rocher
	\sa Server
	\param parent pointer to parent for this object
 */

ClientSocket :: ClientSocket( QObject* parent ) : QSslSocket ( parent )
{
	debugQt("ClientSocket::ClientSocket(): "+QString::number(++compteur_objet));

	AuthUser=false;
	permitDisconnectUser=false;
	permitSendMsg=false;

	pamthread = new PamThread();
	pamTimer = new QTimer( this );
	connect( pamTimer, SIGNAL(timeout()), this, SLOT(slot_pam()) );

	ignoreSslErrors ();
	connect (this, SIGNAL(readyRead()),this, SLOT(core()));
	connect (this, SIGNAL(disconnected()),this, SLOT(deleteLater()));
	connect (this, SIGNAL( error ( QAbstractSocket::SocketError )),this,SLOT(SocketError()));
	connect (this, SIGNAL(sslErrors ( const QList<QSslError> & )),this,SLOT(SslErrors(const QList<QSslError> &)));
	connect (this, SIGNAL(encrypted ()),this,SLOT(socketEncrypted ()));
}

ClientSocket :: ~ClientSocket()
{
	debugQt("ClientSocket::~ClientSocket(): "+QString::number(--compteur_objet));
	// Blocks until the thread PamThread has finished execution
	pamthread->wait();
	delete pamthread;
}

/** Socket error */
void ClientSocket::SocketError () {
	debugQt("ClientSocket::SocketError()");
	qWarning() << errorString();
	deleteLater();
}

/** SSL error */
void ClientSocket::SslErrors (const QList<QSslError> & listErrors) {
	debugQt("ClientSocket::SslErrors()");
	for (int i = 0; i < listErrors.size(); ++i)
             qWarning() <<listErrors.at(i).errorString ();
}


/**
	now, socket is encrypted
*/
void ClientSocket::socketEncrypted () {
	debugQt("ClientSocket::socketEncrypted()");
	// who i am 
	sendToClient(whoiam,QString::number(int_qtsmbstatus_version),"QtSmbstatus server "+version_qtsmbstatus+" - "+date_qtsmbstatus);

}

/**
	Protocol interpreter.
	analyze the client's answers.
	\sa core_syntax
*/
void ClientSocket :: core ()
{
	debugQt ("ClientSocket::core()");
	QString line;
	QByteArray lineArray;
	int commande;
	bool ok;
	core_syntax stx;

	while ( this->canReadLine ())
	{
		lineArray = this->readLine();
		line=QString::fromUtf8( lineArray.data() ).trimmed ();
		debugQt(line);
		
		stx.setValue(line);
		if (stx.returnArg(0) != "")
		{
			commande=(stx.returnArg(0)).toInt(&ok);
			if (ok) // if txt to int conversion is ok
			{
				switch (commande)
				{
					case auth_rq: // auth_rq
							debugQt("["+QString::number(commande)+"] auth_rq");
							CmdAuthRq(line);
							break;
					case end: // end socket by client
							debugQt("["+QString::number(commande)+"] end");
							deleteLater();
							break;
					case kill_user: // disconnect user
							debugQt("["+QString::number(commande)+"] kill_user");
							CmdKillUser(line);
							break;
					case send_msg: // send message to user
							debugQt("["+QString::number(commande)+"] send_msg");
							CmdSendMsg(line);
							break;
					case smb_rq: // smbstatus request
							debugQt("["+QString::number(commande)+"] smb_rq");
							CmdSmbRq();
							break;
					case echo_request:// echo request from client
							debugQt("["+QString::number(commande)+"] echo_request");
							sendToClient(echo_reply);
							break;
					case echo_reply: // echo reply from client
							debugQt("["+QString::number(commande)+"] echo_reply");
							// now, ignore it (compatibility to qtsmbstatus =< 2.0.6)
							break;
					case error_command: // Command error ( the last command is not recognized)
							debugQt("["+QString::number(commande)+"] error_command");
							break;
					case whoiam:
							debugQt("["+QString::number(commande)+"] whoiam");
							break;
					default: // not implemented
							debugQt("["+QString::number(commande)+"] not implemented");
							sendToClient(error_command,"["+QString::number(commande)+"] not implemented");
							break;
				}
			}
			else
			{ //conversion error string->int
				debugQt("Command error !");
			}
		}
		else
		{ // core_syntax sends an error
			debugQt("Command error !");
		}
	}
}


/**
	Send datas to client
	\param cmd command
	\param inputArg1 argument 1
	\param inputArg2 argument 2
	\sa ClientSocket::command
	\sa core_syntax
*/
void ClientSocket ::sendToClient(int cmd,const QString & inputArg1,const QString & inputArg2)
{
	debugQt ("ClientSocket::sendToClient()");
	QString MyTxt;
	if (!inputArg1.isEmpty()) MyTxt=addEscapeKeys(inputArg1);
	if (!inputArg2.isEmpty()) MyTxt+=";"+addEscapeKeys(inputArg2);
	QString send_txt = "["+QString::number(cmd)+"]"+MyTxt+"\n" ;
	this->write(send_txt.toUtf8());
	debugQt(send_txt);
}


/**
	Request from client to disconnect an %user
	\param texte arg1=PID, arg2=username
	\sa ClientSocket::permitDisconnectUser
	\sa process_smbd_exist
*/
void ClientSocket ::CmdKillUser(const QString & texte)
{
	debugQt(" ClientSocket :: CmdKillUser()");
	core_syntax stx(texte);
	if (!AuthUser) // if not authenticated
	{
		debugQt("Client not authenticated !");
		sendToClient(error_auth,"Not authenticated !");
	}
	else // if authenticated
	{
		if (!permitDisconnectUser) // if client is authorized
		{
			debugQt("Client is unauthorized to disconnect an user !");
			sendToClient(error_obj,tr("Unauthorized request!"));
			return;
		}
		QString pid=stx.returnArg(1);
		if (pid!="")
		{
			QString user=stx.returnArg(2);
			if (user!="")
			{
				process_smbd_exist* killSmbProc = new process_smbd_exist(pid,user,this);
				connect(killSmbProc,SIGNAL(ObjError(const QString &)),this, SLOT(ObjError (const QString &)) );
			} else sendToClient(error_command,"Missing arguments");
		} else sendToClient(error_command,"Missing arguments");
	}
}

/**
	Request from client to send popup message
	\param texte arg1=machine, arg2=message to be sent
	\sa ClientSocket::permitSendMsg
	\sa Sendmessage_manager
*/
void ClientSocket ::CmdSendMsg(const QString & texte)
{
	debugQt(" ClientSocket :: CmdSendMsg()");
	core_syntax stx(texte);
	if (!AuthUser) // if not authenticated
	{
		debugQt("Client not authenticated !");
		sendToClient(error_auth,"Not authenticated !");
	}
	else // if authenticated
	{
		if (!permitSendMsg) // if client is authorized
		{
			debugQt("Client is unauthorized to send popup message !");
			sendToClient(error_obj,tr("Unauthorized request!"));
			return;
		}
		QString machine=stx.returnArg(1);
		if (machine!="")
		{
			QString message=stx.returnArg(2);
			if (message!="")
			{
				Sendmessage_manager* send_msg = new Sendmessage_manager(machine,message,this);
				connect(send_msg,SIGNAL(ObjError(const QString &)),this, SLOT(ObjError (const QString &)) );
			} else sendToClient(error_command,"Missing arguments");
		} else sendToClient(error_command,"Missing arguments");
	}
}

/**
	Run Smbstatus request
	\sa slot_smbstatus
*/
void ClientSocket ::CmdSmbRq()
{
	debugQt(" ClientSocket :: CmdSmbRq()");
	if (!AuthUser)  // if not authenticated
	{
		debugQt("Client not authenticated !");
		sendToClient(error_auth,"Not authenticated !");
	}
	else // if authenticated
	{
		smbmanager* processus_smbstatus = new  smbmanager(this);
		connect(processus_smbstatus,SIGNAL(ObjError(const QString &)),this, SLOT(ObjError (const QString &)) );
		connect( processus_smbstatus, SIGNAL(signal_std_output(const QStringList &)),this, SLOT(slot_smbstatus(const QStringList &)) );
	}
}



/**
	Slot : Receive %smbstatus reply.
	\sa CmdSmbRq
*/
void ClientSocket ::slot_smbstatus(const QStringList & rcv_smb)
{

	debugQt("ClientSocket::slot_smbstatus()");
	QString ligne;
	QString toSend="";
	bool start=true;
	int taille=0;
	QStringList data = rcv_smb;

	QStringList::Iterator it = data.begin();
	while ( it != data.end())
	{
		if (start==true)
		{ // first time
			start=false;
		}
		else
		{ // else, send [smb_data]
			toSend+="\n["+QString::number(smb_data)+"]";
		}

		ligne = *it; // read one line
		toSend+=ligne;
		taille=toSend.length (); // calculate size

		++it;
		// if size > 2048 char or if it's the end of stringlist, send data
		if ((taille>2048) || (it == data.end()))
		{
			sendToClient(smb_data,toSend);
			toSend="";
			taille=0;
			start=true;
		}
	}
	sendToClient(end_smb_rq); // end of smbstatus reply. All datas has been sent
}


/**
	Slot : receive objects errors and send it to client.
	\param error_txt error to be sent
*/
void ClientSocket ::ObjError(const QString & error_txt)
{
	debugQt("ClientSocket::ObjError()");
	sendToClient(error_obj,error_txt); // send to client
}


/**
	Authentication Request
	\param texte arg=1=username, arg2=password
	\sa core_syntax PamThread
*/
void ClientSocket :: CmdAuthRq(const QString & texte)
{
	debugQt(" ClientSocket :: CmdAuthRq()");
	core_syntax stx(texte);
	QString Username=stx.returnArg(1);
	QString Passwd=stx.returnArg(2);
	if ( (!Username.isEmpty()) && (Username.length () < 50) && (!Passwd.isEmpty()) && (Passwd.length () < 50))
	{
		// if client is authorized to disconnect user
		for ( QStringList::Iterator it = AllowUserDisconnect.begin(); it != AllowUserDisconnect.end(); ++it ) {
			if (((*it)==Username) || ((*it).toLower()=="all")) permitDisconnectUser=true;
		}
		// if client is authorized to send popup message
		for ( QStringList::Iterator it = AllowUserSendMsg.begin(); it != AllowUserSendMsg.end(); ++it ) {
			if (((*it)==Username) || ((*it).toLower ()=="all")) permitSendMsg=true;
		}

		// PAM Request
		pamthread->setAuth(Username,Passwd);
		pamthread->start();
		pamTimer->start( 500); //request every 500ms to know pamthread status (finished)
		return;
	}
	sendToClient(error_auth);
	QTimer::singleShot(100,this,SLOT(deleteLater()));
}



/**
	test if pamthread has terminated
	\sa CmdAuthRq
*/
void ClientSocket ::slot_pam()
{
	debugQt ("ClientSocket::slot_pam()");
	if (pamthread->isFinished ())
	{
		// stop timer
		pamTimer->stop();
		if (pamthread->auth_resu)
		{
			// client authenticated
			AuthUser=true;
			sendToClient(auth_ack);
			/*
			infoserver : right for current client
			0000 0001 : permit client to disconnect an user
			0000 0010 : permit client to send popup messages (popupwindows)
			*/
			int infoserver=0;
			if (permitDisconnectUser) infoserver=1;
			if (permitSendMsg) infoserver+=2;
			// send client's rights
			sendToClient(server_info,QString::number(infoserver));
		}
		else
		{
			// client is not authenticated. disconnect it
			sendToClient(error_auth);
			QTimer::singleShot(100,this,SLOT(deleteLater()));
		}
	}
}

