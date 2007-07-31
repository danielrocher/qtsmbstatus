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

#include <Q3CString>
#include <QTextStream>
#include <QTimer>
#include <Q3SocketDevice>
#include <QSocketNotifier>

#include "clientsocket.h"

// SSL context
SSL_CTX* ssl_ctx;

int ClientSocket::compteur_objet=0;

//! Time between 2 echo request
int ClientSocket::TimoutTimerEcho=30000;

/**
	\class ClientSocket
	\brief This class dialogs with client. (One object per client)
	\date 2007-06-18
	\version 1.0
	\author Daniel Rocher
	\sa Server
	\param sock Socket number
	\param parent pointer to parent for this object
 */

ClientSocket :: ClientSocket( const int & sock, QObject *parent ) : QObject (parent)
{
	debugQt("Object ClientSocket : "+QString::number(++compteur_objet));
	debugQt("New client - socket : "+ QString::number(sock));

	SSL_init=false;
	AuthUser=false;
	permitDisconnectUser=false;
	permitSendMsg=false;

	timer = new QTimer( this );
	connect( timer, SIGNAL(timeout()), this, SLOT(slot_pam()) );

	pamthread = new PamThread();

	socket = sock;
	socketdevice = new Q3SocketDevice(socket , Q3SocketDevice::Stream );
	int ret = socketdevice->error();
	if (ret!=0)
	{ // TCP Error
		qWarning("TCP Error !");
		Socket_print_error(ret);
		deleteLater ();
	}

	// create a new SSL structure for a connection
	ssl=SSL_new (ssl_ctx);
	if (!ssl)
	{
		qWarning ("Error creating SSL structure" );
		SSL_print_error(SSL_get_error(ssl, 0));
		socketConnectionClose();
		return;
	}
	debugQt ("Create a new SSL structure             OK");

	SSL_set_mode (ssl, SSL_MODE_AUTO_RETRY); //auto retry

	// set the file descriptor for the socket
	if (!SSL_set_fd( ssl , socket ) )
	{
		qWarning( "Error initializing SSL-socket" );
		SSL_print_error(SSL_get_error(ssl, 0));
		socketConnectionClose();
		return;
	}
	debugQt ("Connect SSL object with a socket       OK");

	// wait for a TLS/SSL client to initiate a TLS/SSL handshake
	ret = SSL_accept(ssl);
	if (ret <= 0 )
	{
		qWarning( "Error accept connect" );
		SSL_print_error(SSL_get_error(ssl, ret));
		socketConnectionClose();
		return;
	}
	debugQt ("SSL connection                         OK");
	SSL_init=true;
	// Socket notificier
	sn_read= new QSocketNotifier(socketdevice->socket(), QSocketNotifier::Read,this) ;
	connect( sn_read,SIGNAL(activated(int)),this, SLOT(readClient()) );

	sn_exception= new QSocketNotifier(socketdevice->socket(), QSocketNotifier::Exception ,this) ;
	connect( sn_exception,SIGNAL(activated(int)),this, SLOT(Exception ()) );

	// test if connection is not dead
	// reset timer if we receive data from client
	echo=0; // if echo > 2, disconnect
	echo_timer = new QTimer (this);
	connect( echo_timer, SIGNAL(timeout()), this, SLOT(slot_echo_timer()) );
	echo_timer->start( TimoutTimerEcho, FALSE );
}


ClientSocket ::~ClientSocket()
{
	debugQt("Object ClientSocket : "+QString::number(--compteur_objet));
	delete socketdevice;
	// Blocks until the thread PamThread has finished execution
	pamthread->wait();
	delete pamthread;
}


/**
	There is a TCP error
*/
void ClientSocket ::Exception()
{
	debugQt ("ClientSocket::Exception()");
	int ret = socketdevice->error();
	if (ret!=0)
	{
		// an error occurred
		qWarning("Socket error !");
		Socket_print_error(ret);
		socketConnectionClose();
	}
}


/**
	Receive datas from client
	\sa sendToClient
*/
void ClientSocket ::readClient()
{
	debugQt ("ClientSocket::readClient()");
	if (SSL_init==false) return;
	if (!socketdevice->isOpen()) //if not connected
	{
		socketConnectionClose();
		return;
	}
	Q3CString rcvtxt;
	rcvtxt.resize(16384);
	int ret = SSL_read(ssl, rcvtxt.data(), rcvtxt.size());
	if (ret <=0 )
	{
		// an error occurred
		SSL_print_error(SSL_get_error(ssl, ret));
		socketConnectionClose();
		return;
	}


	if (AuthUser)  // if client authenticated
	{
		// reset echo timer because client has sent data
		echo=0;
		echo_timer->stop();
		echo_timer->start( TimoutTimerEcho, FALSE );
	}

	rcvtxt.resize(ret);  // Sets the size of the byte array to size bytes
	debugQt (rcvtxt);
	core ( rcvtxt );
}

/**
	Send datas to client
	\param inputText datas to send
	\param cmd command
	\sa ClientSocket::command
	\sa readClient
	\sa core_syntax
*/
void ClientSocket ::sendToClient(int cmd,const QString & em_txt)
{
	debugQt ("ClientSocket::sendToClient()");
	if (SSL_init==false) return;
	Q3CString send_txt = ("["+QString::number(cmd)+"]"+em_txt+"\n").utf8 () ;
	int value=SSL_write (ssl, send_txt.data() , send_txt.length ());
	if (!value)
	{
		// an error occurred
		SSL_print_error(SSL_get_error(ssl, 0));
		socketConnectionClose();
		return;
	}
	debugQt(send_txt);
}

/**
	Request from client to disconnect an user
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
			if (((*it)==Username) || ((*it).lower()=="all")) permitDisconnectUser=true;
		}
		// if client is authorized to send popup message
		for ( QStringList::Iterator it = AllowUserSendMsg.begin(); it != AllowUserSendMsg.end(); ++it ) {
			if (((*it)==Username) || ((*it).lower()=="all")) permitSendMsg=true;
		}

		// PAM Request
		pamthread->setAuth(Username,Passwd);
		pamthread->start();
		timer->start( 500, FALSE ); //request every 500ms to know pamthread status (finished)
		return;
	}
	sendToClient(error_auth);
	socketConnectionClose();
}



/**
	test if pamthread has terminated
*/
void ClientSocket ::slot_pam()
{
	debugQt ("ClientSocket::slot_pam()");
	if (pamthread->isFinished ())
	{
		// stop timer
		timer->stop();
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
			socketConnectionClose();
		}
	}
}

/**
	Protocol interpreter.
	analyze the client's answers.
	\param rcv_txt data sent by client
	\sa core_syntax
*/
void ClientSocket :: core (const Q3CString & rcv_txt)
{
	debugQt ("ClientSocket::core()");
	QString texte;
	int commande;
	bool ok;
	core_syntax stx;
	QTextStream ts( rcv_txt, QIODevice::ReadOnly );
	while ( !ts.atEnd () )
	{
		texte=ts.readLine();

		texte=QString::fromUtf8( texte );
		stx.setValue(texte);
		if (stx.returnArg(0) != "")
		{
			commande=(stx.returnArg(0)).toInt(&ok);
			if (ok) // if txt to int conversion is ok
			{
				switch (commande)
				{
					case auth_rq: // auth_rq
							debugQt("["+QString::number(commande)+"] auth_rq");
							CmdAuthRq(texte);
							break;
					case end: // end socket by client
							debugQt("["+QString::number(commande)+"] end");
							socketConnectionClose();
							break;
					case kill_user: // disconnect user
							debugQt("["+QString::number(commande)+"] kill_user");
							CmdKillUser(texte);
							break;
					case send_msg: // send message to user
							debugQt("["+QString::number(commande)+"] send_msg");
							CmdSendMsg(texte);
							break;
					case smb_rq: // smbstatus request
							debugQt("["+QString::number(commande)+"] smb_rq");
							CmdSmbRq();
							break;
					case echo_request:// echo request from client
							debugQt("["+QString::number(commande)+"] echo_request");
							sendToClient(echo_reply);
							break;
					case echo_reply: // echo reply from client, reset echo timer
							debugQt("["+QString::number(commande)+"] echo_reply");
							if (AuthUser) echo=0; // only if client is authenticated
							break;
					case error_command: // Command error ( the last command is not recognized)
							debugQt("["+QString::number(commande)+"] error_command");
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
	Close connection.
*/
void ClientSocket ::socketConnectionClose()
{
	debugQt("ClientSocket ::socketConnectionClose()");
	SSL_init=false;
	SSL_shutdown(ssl);
	debugQt ("SSL_shutdown                  OK");
	SSL_free(ssl);
	debugQt ("SSL_free                      OK");
	socketdevice->close();
	deleteLater();
}


/**
	Slot : Receive smbstatus reply.
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
			sendToClient(smb_data,toSend); // et on l'envoi au client
			 // on rinitialise les variables
			toSend="";
			taille=0;
			start=true;
		}
	}
	sendToClient(end_smb_rq); // end of smbstatus reply. All data has been sent
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
	Timer ending (echo), sends a new echo request. After 3 failures, disconnect.
*/
void ClientSocket::slot_echo_timer()
{
	echo++;
	debugQt("ClientSocket::slot_echo_timer() - echo = "+QString::number(echo));
	if (echo > 2) socketConnectionClose();  //  After 3 failures, disconnect
	else sendToClient(echo_request);  // sends a new echo request
}


