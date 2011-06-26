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


#include <QtGui>

#include "../common/core_syntax.h"

#include "main_windows.h"

extern void writeToConsole(const QString & message);
extern QList<QTreeWidgetItem *> QTreeWidgetItemList;

//default value
QString host="127.0.0.1";
QString username_login="root";
QString passwd_login="";
//! Autoconnect when qtsmbstatus start
bool autoconnect=false;


/**
	\class main_windows
	\brief Main Window for QtSmbstatus Client
	\date 2008-11-16
	\version 2.0
	\author Daniel Rocher
*/
main_windows::main_windows(QWidget *parent) : MainWindows(parent)
{
	debugQt("main_windows::main_windows()");
	this->setWindowTitle ( "QtSmbstatus client "+version_qtsmbstatus); // forms title
	qApp->setWindowIcon ( QIcon (":/icons/qtsmbstatus.png") );
	trayicon->setToolTip ( windowTitle ());
	trayicon->setIcon (qApp->windowIcon());

	connect_action= new QAction ( QIcon (":/icons/connect_no.png"),tr("Connect"),menuApp );
	menuApp->insertAction ( viewlog_action, connect_action );
	menuApp->insertSeparator ( viewlog_action );
	connect(connect_action,SIGNAL( triggered ()),this , SLOT(on_pushButton_connect_clicked()));
	connect(action_Connect,SIGNAL( triggered ()),this , SLOT(on_pushButton_connect_clicked()));

	comboBox_hostaddr->setDuplicatesEnabled ( false );
	// read the address history
	readHistoryFile();
	comboBox_hostaddr->setEditText (host);

	// Socket SSL/TLS
	connect(&sslSocket, SIGNAL( encrypted() ), this, SLOT(socketConnected() ) );
	connect(&sslSocket, SIGNAL( disconnected() ), this, SLOT(socketClosed() ) );
	connect(&sslSocket,SIGNAL(readyRead()), this, SLOT(core()) );
	connect(&sslSocket,SIGNAL( error ( QAbstractSocket::SocketError )),this,SLOT(error(QAbstractSocket::SocketError)));
	connect(&sslSocket, SIGNAL(sslErrors ( const QList<QSslError> & )),this,SLOT(SslErrors(const QList<QSslError> &)));
	// initial state
	setWidgetState();

	//autoconnect
	if (autoconnect) on_pushButton_connect_clicked();

}

main_windows::~main_windows()
{
	debugQt("main_windows::~main_windows()");
}



/**
	On close event.
*/
void main_windows::closeEvent(QCloseEvent *e)
{
	debugQt("main_windows::closeEvent(QCloseEvent *e)");
	if (trayicon->isVisible())
	{
		this->hide ();
		logform->hide();
		e->ignore();
	}
	else
	{
		// write address history
		writeHistoryFile();
		if (socketState()!=UnconnectedState) Disconnect(); //if connected, disconnect
		saveWindowSize();
		logform->close();
		logform->deleteLater();
		QTimer::singleShot(800, qApp, SLOT(quit())); // wait before quit
		e->accept();
	}
}

/**
	Write address history.
	Before quit, write history.
*/
void main_windows::writeHistoryFile()
{
	debugQt ("main_windows::writeHistoryFile()");
	QSettings settings;
	settings.beginGroup("/MainWindow");
		settings.beginGroup("/History");
			int index=0;
			while (index<comboBox_hostaddr->count ())
			{
				settings.setValue("recentAddress"+QString::number(index),comboBox_hostaddr->itemText(index) );
				index++;
			}
		settings.endGroup();
	settings.endGroup();
}

/**
	Read the address history.
	On startup, read history.
*/
void main_windows::readHistoryFile()
{
	debugQt ("main_windows::readHistoryFile()");
	QString value;
	QSettings settings;
	settings.beginGroup("/MainWindow");
		settings.beginGroup("/History");
			for ( int i = 0; i < 20 ; ++i )
			{
				value=settings.value( "recentAddress" +QString::number( i) ).toString();
				if (!value.isEmpty())
					comboBox_hostaddr->addItem (  value );
			}
		settings.endGroup();
	settings.endGroup();
}

/**
	address lists
	20 Max addresses
*/
void main_windows::comboBox_valid()
{
	debugQt ("main_windows::comboBox_valid()");
	int index=1;
	QString texte = comboBox_hostaddr->currentText ();
	comboBox_hostaddr->insertItem (0,texte);
	while (index<comboBox_hostaddr->count ())
	{
		if ((comboBox_hostaddr->itemText(index) == texte) || (index > 19) )
			comboBox_hostaddr->removeItem (index);
		else index++;
	}
	comboBox_hostaddr->setCurrentIndex(0);
}


/**
	Slot connect. If connected, disconnect.
*/
void main_windows::on_pushButton_connect_clicked()
{
	debugQt("main_windows::on_pushButton_connect_clicked()");
	if (socketState()==UnconnectedState) open_dialog_for_login(); // open authentication dialogbox
	else Disconnect();
}

/**
	Open authentication dialogbox
	\sa login_windows
*/
void main_windows::open_dialog_for_login()
{
	debugQt("main_windows::open_dialog_for_login()");
	login_windows * login_passwd = new login_windows(this);
	if (login_passwd->exec()==QDialog::Accepted)
	{

		host=comboBox_hostaddr->currentText ();
		// clear form log
		logform->on_clearButton_clicked();

		// now, try to connect ...
		statusBar()->showMessage( tr("Connection in progress on %1").arg(host));
		sslSocket.connectToHostEncrypted(host,port_server);
		sslSocket.ignoreSslErrors ();
		setWidgetState();
	}
}

/**
	Slot connected. When client is connected to %server
	\sa on_pushButton_connect_clicked
*/
void main_windows::socketConnected()
{
	debugQt("main_windows::socketConnected()");
	// server address is valid, add to combobox
	comboBox_valid();
	setWidgetState();
	permitDisconnectUser=false; // permit (or not) client to disconnect an user
	permitSendMsg=false; // permit (or not) client to send popup messages
	currentIndexOfListItem=0;
	QString SearchTxt="";
	
	//check certificate
	QSslCertificate cert=sslSocket.peerCertificate ();
	if ( cert.isNull() || ! cert.isValid()) {
		QMessageBox::warning ( this, "QtSmbstatus",tr("The server certificate doesn't contain valid data or expired !"));
		sslSocket.close();
		return;
	}

	// get hash
	QByteArray digest=cert.digest (QCryptographicHash::Sha1);
	QStringList resu;
	for ( int i=0;i<digest.size();++i )
	{
		QString hx;
		hx.sprintf ( "%02X", ( unsigned char ) digest[i] );
		resu<<hx;
	}
	QString sha1= resu.join ( ":" );

	debugQt("SHA1 : "+sha1);
	QList<Host> values;
	QSettings settings;

	settings.beginGroup("/knownHosts");
		int size = settings.beginReadArray("host");
			for (int i = 0; i < size; ++i) {
				settings.setArrayIndex(i);
				Host knowhost;
				knowhost.address=settings.value("address").toString();
				knowhost.sha1 =settings.value("sha1").toString();
				values.append(knowhost);
			}
			settings.endArray();
	settings.endGroup();

	Host knowhost;
	knowhost.address=comboBox_hostaddr->currentText ();
	knowhost.sha1=sha1;

	if ( ! values.contains(knowhost) ) {
		if (QMessageBox::question ( this, "QtSmbstatus",
			tr("The authenticity of host '%1' can't be established.").arg(knowhost.address)+"\n"+
			tr("SHA1 Fingerprint: %2.").arg(knowhost.sha1)+"\n"+
			tr("Are you sure you want to continue connecting ?"),
			QMessageBox::Yes|QMessageBox::No, QMessageBox::No ) ==QMessageBox::Yes)
		{
			values.append(knowhost);
	
			// save config
			settings.beginGroup("/knownHosts");
				settings.beginWriteArray("host");
					for (int i = 0; i < values.size(); ++i) {
						settings.setArrayIndex(i);
						settings.setValue("address", values.at(i).address);
						settings.setValue("sha1", values.at(i).sha1);
					}
				settings.endArray();
			settings.endGroup();
		} else {
			// unknown host and user want exit
			sslSocket.close();
			return;
		}
	}

	firstTime=true;
	timerinfoSmb.start(10000);

	statusBar()->showMessage( tr("Connected to host") );

	// create icon server on treeWidget
	item_server = new server( treeWidget );

	// who i am 
	sendToServer(whoiam,QString::number(int_qtsmbstatus_version),"QtSmbstatus client "+version_qtsmbstatus+" - "+date_qtsmbstatus);

	//Authentication
	sendToServer(auth_rq,username_login,passwd_login); // send username and password to server
}

/**
	Slot socketClosed. When client is disconnected
*/
void main_windows::socketClosed()
{
	debugQt("main_windows::socketClosed()");
	statusBar()->showMessage( tr("Connection closed") );
	smbstatus_rq_started=false;
	ListSmbstatus.clear();
	// clear listview
	treeWidget->clear();
	timerinfoSmb.stop();
	timerSmbRequest.stop();
	menuPopup->clear ();
	setWidgetState();
}

/**
	socket error
	\param socketError
*/
void main_windows::error (QAbstractSocket::SocketError socketError) {
	debugQt("main_windows::error()");
	statusBar()->showMessage(sslSocket.errorString());
	if (socketError!=QAbstractSocket::RemoteHostClosedError)
		QMessageBox::warning ( this, "QtSmbstatus",sslSocket.errorString());
	qWarning() << sslSocket.errorString();
	Disconnect();
}

/**
	SSL error
*/
void main_windows::SslErrors (const QList<QSslError> & listErrors) {
	debugQt("main_windows::SslErrors()");
	for (int i = 0; i < listErrors.size(); ++i)
             qWarning() << listErrors.at(i).errorString ();
}


/**
	Close TCP connection
*/
void main_windows::Disconnect()
{
	debugQt("main_windows::Disconnect()");
	sendToServer(end); // inform server
	sslSocket.close(); // close SSL/TCP connection
	setWidgetState();
}


/**
	Enabled/disabled widget when connected/unconnected/...
	\sa socketState
*/
void main_windows::setWidgetState() {
	debugQt("main_windows::setWidgetState()");
	QPixmap pixmap;
	QString label;
	bool actionIsEnabled=false;

	switch (socketState()) {
		case ConnectingState:
			pixmap=QPixmap(":/icons/connect_creating.png") ;
			label=tr("Disconnect");
			actionIsEnabled=false;
			break;
		case ConnectedState:
			pixmap=QPixmap(QPixmap(":/icons/connect_established.png"));
			label=tr("Disconnect");
			actionIsEnabled=true;
			break;
		default:
			pixmap=QPixmap(":/icons/connect_no.png");
			label=tr("Connect");
			actionIsEnabled=false;
			break;
	}
	pushButton_connect->setIcon(QIcon(pixmap));
	connect_action->setIcon(QIcon(pixmap));
	action_Connect->setIcon(QIcon(pixmap));
	connect_action->setText( label );
	action_Connect->setText( label );
	searchAll->setEnabled ( actionIsEnabled );
	searchMachineAction->setEnabled ( actionIsEnabled );
	searchUserAction->setEnabled ( actionIsEnabled );
	searchShare_openAction->setEnabled ( actionIsEnabled );
	searchFile_openAction->setEnabled ( actionIsEnabled );
	searchNextAction->setEnabled ( false );
}

/**
	Send data to %server.
	\param cmd command
	\param inputArg1 argument 1
	\param inputArg2 argument 2
	\sa command
	\sa core_syntax
*/
void main_windows::sendToServer(int cmd,const QString & inputArg1,const QString & inputArg2)
{
	debugQt("main_windows::sendToServer()");
	if (socketState()!=ConnectedState) return; // if unconnected
	QString MyTxt;
	if (!inputArg1.isEmpty()) MyTxt=addEscapeKeys(inputArg1);
	if (!inputArg2.isEmpty()) MyTxt+=";"+addEscapeKeys(inputArg2);
	QString send_txt="["+QString::number(cmd)+"]"+MyTxt+"\n";
	sslSocket.write(send_txt.toUtf8());
	debugQt(send_txt);
}




/**
	Bad login or bad password.
*/
void main_windows::errorAuth()
{
	debugQt("main_windows::errorAuth()");
	QMessageBox::warning ( this, "QtSmbstatus",tr("Invalid login or password !"));
	on_pushButton_connect_clicked();
}



/**
	Return states in which a socket can be
	\return state for socket
	\sa socket_state
*/
short unsigned int main_windows::socketState() {
	// connected and encrypted
	if (sslSocket.state()==QAbstractSocket::ConnectedState && sslSocket.isEncrypted())
		return ConnectedState;
	// unconnected
	if (sslSocket.state()==QAbstractSocket::UnconnectedState)
		return UnconnectedState;
	// try to connect
	return ConnectingState;
}


/**
	Slot %smbstatus timer
	Request %smbstatus to %server
*/
void main_windows::slot_timer()
{
	debugQt("main_windows::slot_timer()");
	if (! smbstatus_rq_started) {
		ListSmbstatus.clear();
		smbstatus_rq_started=true;
		sendToServer(smb_rq);
	} else writeToConsole("WARNING: smbstatus request is already started.");
}

/**
	Receive informations from %server (right for current client)
	\param text client's right:
	\verbatim
	0000 0001 : permit client to disconnect an user
	0000 0010 : permit client to send popup messages (popupwindows)
	\endverbatim
*/
void main_windows::infoserver(const QString & text)
{
	debugQt("main_windows::infoserver()");
	bool ok;
	permitDisconnectUser=false;
	permitSendMsg=false;
	int value = (text.mid(text.indexOf("]")+1)).toInt(&ok);
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
	\sa core_syntax
*/
void main_windows::core()
{
	debugQt ("main_windows::core()");
	QString line;
	QByteArray lineArray;
	int reponse;
	bool ok;
	core_syntax stx;

	while (sslSocket.canReadLine ())
	{
		lineArray = sslSocket.readLine();
		line=QString::fromUtf8( lineArray.data() ).trimmed ();
		debugQt(line);
		
		stx.setValue(line);
		if (stx.returnArg(0) != "")
		{
			reponse=(stx.returnArg(0)).toInt(&ok);
			if (ok) // if txt to int conversion is ok
			{
				switch (reponse)
				{
					case auth_ack: // authentication ok
							debugQt("["+QString::number(reponse)+"] auth_ack");
							slot_timer(); // first time
							timerSmbRequest.start (interval*1000);
							break;
					case end: // end socket by server
							debugQt("["+QString::number(reponse)+"] end");
							sslSocket.close();
							break;
					case smb_data: // smbstatus datas
							debugQt("["+QString::number(reponse)+"] smb_data");
							ListSmbstatus.append(stx.returnArg(1));
							break;
					case end_smb_rq: // smbstatus command is finished
							debugQt("["+QString::number(reponse)+"] end_smb_rq");
							AnalysisSmbstatus(ListSmbstatus); // interpret smbstatus reply
							break;
					case error_auth: // authentication error
							debugQt("["+QString::number(reponse)+"] error_auth");
							// socketclient->closeConnection(); (don't close here, server will do it)
							QTimer::singleShot(200,this,SLOT(errorAuth())); // don't block loop
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
							// ignore it (for compatibility with qtsmbstatus =< 2.0.6)
							break;
					case error_obj: // Server wants to open a dialogbox to visualize an error
							debugQt("["+QString::number(reponse)+"] error_obj");
							// QMessageBox::warning ( 0, "QtSmbstatus",stx.returnArg(1));
							msgError->showMessage ( stx.returnArg(1) ) ;
							break;
					case server_info: // informations from server
							debugQt("["+QString::number(reponse)+"] server_info");
							infoserver(line);
							break;
					case whoiam:
							debugQt("["+QString::number(reponse)+"] whoiam");
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
	send message to an user
	\sa MainWindows::slotSendMessage MainWindows::slotSendMessageAllUsers
 */
void main_windows::sendMessage(const QString & machine,const QString & message) {
	debugQt("main_windows::sendMessage()");
	sendToServer(send_msg,machine,message);
}



/**
	disconnect an user
	\sa MainWindows::slotDisconnectUser
 */
void main_windows::disconnectUser(const QString & pid,const QString & user) {
	debugQt("main_windows::disconnectUser()");
	sendToServer(kill_user,pid,user);
}

/**
	Configure qtsmbstatus client (open form)
	\sa configure_windows
 */
void main_windows::on_configureAction_triggered()
{
	debugQt("main_windows::on_configureAction_triggered()");
	configure_windows * configurate = new configure_windows(this);
	connect (configurate,SIGNAL(configuration_changed()),this, SLOT(configuration_changed()));
	configurate->exec();
}

