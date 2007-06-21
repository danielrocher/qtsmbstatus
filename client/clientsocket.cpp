/***************************************************************************
 *   Copyright (C) 2004 by Daniel rocher                                   *
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

/**
	\class ClientSocket
	\brief This class dialogs with server
	\date 2007-06-14
	\version 1.0
	\author Daniel Rocher
	\sa ClientSocket::command
*/


#include <Q3CString>
#include <Q3PopupMenu>
#include <QTextStream>

#include "clientsocket.h"


int ClientSocket::compteur_objet=0;

//! Time between 2 echo request
int ClientSocket::TimoutTimerEcho=30000;

/**
	\param mylistView pointer to listview
	\param parent pointer to parent for this object
*/
ClientSocket :: ClientSocket(Q3ListView * mylistView,QObject *parent)  : QObject (parent)
{
	debugQt("Object ClientSocket : "+QString::number(++compteur_objet));
	permitDisconnectUser=false; // permit (or not) client to disconnect an user
	permitSendMsg=false; // permit (or not) client to send popup messages
	listView=mylistView;
	// create popup menu
	menu = new Q3PopupMenu( listView );
	// and connect it with listview
	connect(listView, SIGNAL( contextMenuRequested( Q3ListViewItem *, const QPoint& , int ) ), this, SLOT( slotPopupMenu( Q3ListViewItem *, const QPoint &, int ) ) );
	// interval for smbstatus request
	timer=new QTimer(this);

	// test if connection is not dead
	// reset timer if we receive data from server
	echo=0; // if echo > 2, disconnect
	echo_timer = new QTimer (this);
	connect( echo_timer, SIGNAL(timeout()), this, SLOT(slot_echo_timer()) );
}

ClientSocket::~ClientSocket()
{
	debugQt("Object ClientSocket : "+QString::number(--compteur_objet));
}

/**
	Close TCP connection
*/
void ClientSocket::Disconnect()
{
	sendToServer(end); // inform server
	socketclient->closeConnection(); // close SSL/TCP connection
}

/**
	Connect to server
	\param username username (a valid login on server)
	\param passwd password (a valid password on server)
*/
void ClientSocket::connectionToServer(const QString & username, const QString & passwd)
{
	username_login=username;
	passwd_login=passwd;

	debugQt("ClientSocket::connectionToServer()");
	socketclient = new ClientSSL(this);
	// socketclient can post a message on statusBar
	connect (socketclient,SIGNAL(info(const QString & )),this,SIGNAL(SignalShortMessage(const QString & )) );
	// when server sends data, send it to core methods
	connect (socketclient,SIGNAL(readFromHost(const Q3CString& )),this,SLOT(core(const Q3CString& )) );
	// connect to slots, signals connected and closed
	connect (socketclient,SIGNAL(socketconnected()),this,SLOT(socketconnected()) );
	connect (socketclient,SIGNAL(socketclosed()),this,SLOT(socketclosed()) );
	// host unreachable
	connect (socketclient,SIGNAL(UnreachableHost()),this,SLOT(UnreachableHost()) );
	// connect on server
	socketclient->Connect(host,port_server);
}

/**
	SLot connection is OK
*/
void ClientSocket::socketconnected()
{
	debugQt("ClientSocket::socketconnected()");
	ssl_connected=true;
	emit(SignalConnected());
	// create icon server on listview
	item_server = new server( listView );
	sendToServer(auth_rq,username_login+";"+passwd_login); // send username and password to server
	echo=0;
	echo_timer->start( TimoutTimerEcho, FALSE ); // start echo timer
}


/**
	Slot connection is closed
*/
void ClientSocket::socketclosed()
{
	debugQt("ClientSocket::socketclosed()");
	timer->stop();
	ListSmbstatus.clear();
	// clear listview
	listView->clear();
	// clear popumenu
	menu->clear();
	ssl_connected=false;
	echo_timer->stop(); // stop echo timer
	emit(SignalClosed());
	deleteLater(); // delete this class
}


/**
	Host is unreachable
*/
void ClientSocket::UnreachableHost()
{
	debugQt("ClientSocket::UnreachableHost()");
	QMessageBox::warning ( 0, "QtSmbstatus",tr("Host not found !"));
}

/**
	Slot smbstatus timer
	Request smbstatus to server
*/
void ClientSocket::slot_timer()
{
	debugQt("ClientSocket::slot_timer()");
	sendToServer(smb_rq);
	timer->singleShot(interval*1000, this, SLOT(slot_timer()));  // restart timer
}

/**
	Receive informations from server (right for current client)
	\param text client's right:
	\verbatim
	0000 0001 : permit client to disconnect an user
	0000 0010 : permit client to send popup messages (popupwindows)
	\endverbatim
*/
void ClientSocket::infoserver(const QString & text)
{
	debugQt("ClientSocket::infoserver()");
	bool ok;
	permitDisconnectUser=false;
	permitSendMsg=false;
	int value = (text.mid(text.find("]")+1)).toInt(&ok);
	debugQt("Info Serveur : "+QString::number(value));
	if ( ok )
	{
		if (value & 1) permitDisconnectUser=true;
		if (value & 2) permitSendMsg=true;
	}
}

/**
	Protocol interpreter.
	analyze the server's answers.
	\param rcv_txt data sent by server
	\sa ClientSSL core_syntax
*/
void ClientSocket::core(const Q3CString & rcv_txt)
{
	debugQt ("ClientSocket::core()");
	QString texte;
	int reponse;
	bool ok;
	core_syntax stx;

	// reset echo timer because server has sent data
	echo=0;
	echo_timer->stop();
	echo_timer->start( TimoutTimerEcho, FALSE );

	QTextStream ts( rcv_txt, QIODevice::ReadOnly );
	while (!ts.atEnd () )
	{
		texte=ts.readLine();
		texte=QString::fromUtf8( texte );
		stx.setValue(texte);
		if (stx.returnArg(0) != "")
		{
			reponse=(stx.returnArg(0)).toInt(&ok);
			if (ok) // if txt to int conversion is ok
			{
				switch (reponse)
				{
					case auth_ack: // authentication ok
							debugQt("["+QString::number(reponse)+"] auth_ack");
							// start smbstatus timer
							timer->singleShot(interval*1000, this, SLOT(slot_timer()));
							slot_timer(); // first time
							break;
					case end: // end socket by server
							debugQt("["+QString::number(reponse)+"] end");
							socketclient->closeConnection(); //on ferme la connection
							break;
					case smb_data: // smbstatus datas
							debugQt("["+QString::number(reponse)+"] smb_data");
							ListSmbstatus.append(stx.returnArg(1));
							break;
					case end_smb_rq: // smbstatus command is finished
							debugQt("["+QString::number(reponse)+"] end_smb_rq");
							AnalysisSmbstatus(); // interpret smbstatus reply
							break;
					case error_auth: // authentication error
							debugQt("["+QString::number(reponse)+"] error_auth");
							socketclient->closeConnection();
							emit (SignalErrorAuth());
							break;
					case error_command: // Command error ( the last command is not recognized)
							debugQt("["+QString::number(reponse)+"] error_command");
							break;
					case echo_request: // echo request from server
							debugQt("["+QString::number(reponse)+"] echo_request");
							sendToServer(echo_reply);
							break;
					case echo_reply: // echo reply from server, reset echo timer
							debugQt("["+QString::number(reponse)+"] echo_reply");
							echo=0;
							break;
					case error_obj: // Server wants to open a dialogbox to visualize an error
							debugQt("["+QString::number(reponse)+"] error_obj");
							QMessageBox::warning ( 0, "QtSmbstatus",stx.returnArg(1));
							break;
					case server_info: // informations from server
							debugQt("["+QString::number(reponse)+"] server_info");
							infoserver(texte);
							break;
					default: // not implemented
							debugQt("["+QString::number(reponse)+"] not implemented");
							sendToServer(error_command,"["+QString::number(reponse)+"] not implemented");
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
	Interpret smbstatus reply.
	\sa smbstatus
*/
void ClientSocket::AnalysisSmbstatus()
{
	debugQt("ClientSocket::AnalysisSmbstatus()");
	// mark items of listview first
	item_server->mark_childs();
	InstanceSmbstatus = new smbstatus(ListSmbstatus,this);

	// InstanceSmbstatus sends samba version
	connect (InstanceSmbstatus,SIGNAL(setSambaVersion (const QString &)),this,SLOT(setSambaVersion (const QString &)));
	// InstanceSmbstatus has found a share
	connect (InstanceSmbstatus,SIGNAL(add_share(const QString &,const QString &,const QString &)),this,SLOT(add_share(const QString &,const QString &,const QString &)));
	// InstanceSmbstatus has found an user
	connect (InstanceSmbstatus,SIGNAL(add_user (const QString &,const QString &,const QString &,const QString &,const QString &)),this,SLOT(add_user (const QString &,const QString &,const QString &,const QString &,const QString &)));
	// InstanceSmbstatus has found a file opened (locked file)
	connect (InstanceSmbstatus,SIGNAL(add_lockedfile(const QString &,const QString &,const QString &,const QString &,const QString &,const QString &)),this,SLOT(add_lockedfile(const QString &,const QString &,const QString &,const QString &,const QString &,const QString &)));
	//  InstanceSmbstatus has finished his work
	connect (InstanceSmbstatus,SIGNAL(destroyed ()),this,SLOT(AnalysisSmbDestroyed()));
	// Start analysis
	InstanceSmbstatus->RQ_smbstatus();
}

/**
	InstanceSmbstatus has finished his work
	\sa smbstatus
*/
void ClientSocket::AnalysisSmbDestroyed ()
{
	debugQt("ClientSocket::AnalysisSmbDestroyed ()");
	item_server->refresh_childs(); // erase items obsoleted
	ListSmbstatus.clear(); // clear QStringList
}

/**
	InstanceSmbstatus sends samba version.
	\param version_samba samba version
	\sa smbstatus
*/
void ClientSocket::setSambaVersion (const QString & version_samba)
{
	debugQt("ClientSocket::setSambaVersion ()");
	item_server->setSambaVersion(version_samba);
}

/**
	InstanceSmbstatus has found an user
	\param strPid PID
	\param strUser User name
	\param strGroup Group name
	\param strMachineName Machine name
	\param strMachineIP IP address
	\sa smbstatus
**/
void ClientSocket::add_user (const QString & strPid,const QString & strUser,const QString & strGroup,const QString & strMachineName,const QString & strMachineIP)
{
	debugQt("ClientSocket::add_user ()");
	item_server->add_user(strPid,strUser,strGroup,strMachineName,strMachineIP);
}

/**
	InstanceSmbstatus has found a share
	\param strPid PID
	\param strShare Share name
	\param strConnected date
	\sa smbstatus
*/
void ClientSocket::add_share(const QString & strPid ,const QString & strShare,const QString & strConnected)
{
	debugQt("ClientSocket::add_share ()");
	// if hidden shares
	if (!view_hidden_shares && (strShare.find(QRegExp("\\$$"))!=-1)) return;
	item_server->add_share(strPid,strShare,strConnected);
}

/**
	InstanceSmbstatus has found a file opened (locked file)
	\param strPid PID
	\param strName File name
	\param strMode Mode
	\param strRW RW
	\param strOplock Oplock
	\param strDateOpen date
	\sa smbstatus
*/
void ClientSocket::add_lockedfile(const QString & strPid,const QString & strName,const QString & strMode,const QString & strRW,const QString & strOplock,const QString & strDateOpen)
{
	debugQt("ClientSocket::add_lockedfile ()");
	item_server->add_lockedfile(strPid,strName,strMode,strRW,strOplock,strDateOpen);

}

/**
	Send data to server.
	\param cmd command
	\sa ClientSocket::command
	\sa ClientSSL core_syntax
*/
void ClientSocket::sendToServer(int cmd,const QString & inputText)
{
	debugQt("ClientSocket::sendToServer()");
	if (!ssl_connected) return; // if deconnected
	socketclient->sendToServer("["+QString::number(cmd)+"]"+inputText+"\n");
}

/**
	popup menu
*/
void ClientSocket::slotPopupMenu( Q3ListViewItem* Item, const QPoint & point, int )
{
	int id;
	currentPopupMenuItem = Item;
	// if deconnected, no popup menu
	if (!ssl_connected) return;
	if( Item )
	{
		// create popup menu
		menu->clear();

		if (Item==item_server) // if a server item
		{
			menu->insertItem( tr( "Properties"),this,SLOT(InfoServer() ) );
			id=menu->insertItem( tr( "Send out message to all users"),this,SLOT(slotSendMessageAllUsers() ) );
			menu->setItemEnabled(id,permitSendMsg);
			menu->popup( point );
			return;
		}
		if (Item->parent()==item_server) // if a machine item
		{
			machine * myItem=dynamic_cast<machine *>(Item);
			if (!myItem) return;
			menu->insertItem( tr( "Properties"),this,SLOT(InfoMachine() ) );
			id=menu->insertItem( tr( "Send out message to")+ " " + myItem->machine_name,this,SLOT(slotSendMessage() ) );
			menu->setItemEnabled(id,permitSendMsg); // if client can send popup messages
			menu->popup( point );
			return;
		}
		if (Item->parent()->parent()==item_server) // if an user item
		{
			user * myItem=dynamic_cast<user *>(Item);
			if (!myItem) return;
			menu->insertItem( tr( "Properties"),this,SLOT(InfoUser() ) );
			id=menu->insertItem( tr( "Disconnect user")+ " " + myItem->username,this,SLOT(slotDisconnectUser() ) );
			menu->setItemEnabled(id,permitDisconnectUser); //  if client can disconnect an user
			menu->popup( point );
			return;
		}
		// it's a locked file or share
		menu->insertItem( tr( "Properties"),this,SLOT(InfoService() ) );
		menu->popup( point );
	}
}

/**
	View samba version
	\sa slotPopupMenu
	\sa server::ViewInfoUser
*/
void ClientSocket::InfoServer()
{
	QString message=item_server->ViewInfoServer();
	QMessageBox::information(0, "QtSmbstatus",message,QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
}

/**
	View user informations
	\sa slotPopupMenu
	\sa server::ViewInfoUser
*/
void ClientSocket::InfoUser()
{
	QString message=item_server->ViewInfoUser(currentPopupMenuItem);
	QMessageBox::information(0, "QtSmbstatus",message,QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
}

/**
	View service informations (locked file or share)
	\sa slotPopupMenu
	\sa server::ViewInfoService
*/
void ClientSocket::InfoService()
{
	QString message=item_server->ViewInfoService(currentPopupMenuItem);
	QMessageBox::information(0, "QtSmbstatus",message,QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
}

/**
	View machine informations
	\sa slotPopupMenu
	\sa server::ViewInfoMachine
*/
void ClientSocket::InfoMachine()
{
	QString message=item_server->ViewInfoMachine(currentPopupMenuItem);
	QMessageBox::information(0, "QtSmbstatus",message,QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
}

/**
	send out messages (popupwindows) to all users
	\sa slotPopupMenu
*/
void ClientSocket::slotSendMessageAllUsers() {
	machine* Item;
	bool ok;
	QString message = QInputDialog::getText(
            "QtSmbstatus", tr("Message to send")+":", QLineEdit::Normal,
            QString::null, &ok, 0 );

	if ( !ok || message.isEmpty() ) return;
	if (!ssl_connected) return;
	Item=dynamic_cast<machine *>(item_server->firstChild ());

	while( Item ) // all machines
	{
		if (!ssl_connected) return;
		sendToServer(send_msg,Item->machine_name+";"+message);
		Item=dynamic_cast<machine *>(Item->nextSibling ()); // next
	}
}

/**
	send out messages (popupwindows) to one user
	\sa slotPopupMenu
*/
void ClientSocket::slotSendMessage() {
	bool ok;
	machine* Item=dynamic_cast<machine *>(currentPopupMenuItem);
	if (!Item) return;
	QString message = QInputDialog::getText(
		"QtSmbstatus", tr("Message to send to %1:").arg(Item->machine_name), QLineEdit::Normal,
            QString::null, &ok, 0 );
	if ( ok && !message.isEmpty() )
	{
		// user entered something and pressed OK
		if (!ssl_connected) return;
		sendToServer(send_msg,Item->machine_name+";"+message);
	}
}

/**
	Disconnect an user
	\sa slotPopupMenu
*/
void ClientSocket::slotDisconnectUser() {
	QString username;
	QString pid;
	user* Item=dynamic_cast<user *>(currentPopupMenuItem);
	if (!Item) return;
	// get PID
	pid=Item->pid;

	// get user name
	username=Item->username;

	if ( !QMessageBox::information(0,"QtSmbstatus",
	      tr("Do you really want to disconnect user %1 ?").arg(username),
		tr("&Yes"), tr("&No"),QString::null, 1, 1 ) )
	{
		//if true
		if (!ssl_connected) return;
		sendToServer(kill_user,pid+";"+username);
	}
}

/**
	Timer ending (echo), sends a new echo request. After 3 failures, disconnect.
*/
void ClientSocket::slot_echo_timer()
{
	echo++;
	if (!ssl_connected) return;
	debugQt("ClientSocket::slot_echo_timer() - echo = "+QString::number(echo));
	if (echo > 2) socketclient->closeConnection(); //  After 3 failures, disconnect
	else sendToServer(echo_request); // sends a new echo request
}


