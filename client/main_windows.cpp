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

/**
	\class MyApplication
	\brief reimplementing QApplication::commitData()
	\date 2007-07-26
	\version 1.0
	\author Daniel Rocher
*/
MyApplication::MyApplication(int & argc, char ** argv ) : QApplication(argc,argv) { }

/**
	reimplementing QApplication::commitData().
	This function deals with session management. It is invoked when the QSessionManager wants the application to commit all its data.
*/
void MyApplication::commitData(QSessionManager& manager)
{
	debugQt("MyApplication::commitData()");
	if (manager.allowsInteraction())
	{
		manager.release();
		emit quitMyApp();
	}
}

/**
	\class main_windows
	\brief Main Window
	\date 2008-11-07
	\version 2.0
	\author Daniel Rocher
*/
main_windows::main_windows(QWidget *parent) : QMainWindow(parent)
{
	debugQt("main_windows::main_windows()");
	setupUi(this);
	if (check_new_release) checkForUpdateOfQtSmbstatus();
	
	msgError=new QErrorMessage(this); // an error message display dialog
	
	trayicon =new QSystemTrayIcon(QIcon (":/icons/trayicon.png"),this);
	trayicon->setToolTip ( tr("QtSmbstatus Client") );
	QMenu * menuApp=new QMenu( this );
	connect_action= menuApp->addAction ( QIcon (":/icons/connect_no.png"),tr("Connect") );
	connect(connect_action,SIGNAL( triggered ()),this , SLOT(Slot_connect()));
	menuApp->addSeparator ();
	viewlog_action=menuApp->addAction ( tr("Show CIFS/SMB activities") );
	connect(viewlog_action,SIGNAL( triggered ()),this , SLOT(on_action_View_log_triggered()));
	configure_action= menuApp->addAction ( QIcon (":/icons/configure.png"),tr("Configure") );
	connect(configure_action,SIGNAL( triggered ()),this , SLOT(ConfigureSlot()));
	menuApp->addSeparator ();
	restore_action=menuApp->addAction (tr("Minimize") );
	connect(restore_action,SIGNAL( triggered ()),this , SLOT(restore_minimize()));
	QAction * exit_action=menuApp->addAction (  QIcon (":/icons/exit.png"),tr("Quit") );
	connect(exit_action,SIGNAL( triggered ()),this , SLOT(beforeQuit () ));
	connect(action_Quit,SIGNAL( triggered ()),this , SLOT(beforeQuit () ));
	connect(action_Connect,SIGNAL( triggered ()),this , SLOT(Slot_connect()));
	trayicon->setContextMenu ( menuApp );
	connect(trayicon,SIGNAL(activated ( QSystemTrayIcon::ActivationReason ) ),this,SLOT(trayicon_activated(QSystemTrayIcon::ActivationReason)));
	restoreWindowSize();
	this->setWindowTitle ( "QtSmbstatus client "+version_qtsmbstatus); // forms title
	// create statusBar
	statusBar()->showMessage (tr("Impossible to know samba version")); //status bar

	// treeWidget
	treeWidget->setSortingEnabled ( true );
	treeWidget->setRootIsDecorated ( false );
	treeWidget->setHeaderLabel ( "");
	treeWidget->setContextMenuPolicy (Qt::CustomContextMenu );
	// create popup menu
	menuPopup = new QMenu( treeWidget );
	connect(treeWidget, SIGNAL( customContextMenuRequested ( const QPoint & ) ), this, SLOT( slotPopupMenu( const QPoint & ) ) );
	
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

	// first time
	configuration_changed();

	// form view CIFS/SMB activities
	logform = new LogForm(this);
	connect(this ,SIGNAL(refreshviewlog(const type_message &)), logform,SLOT(append(const type_message &)));

	connect(&timerinfoSmb, SIGNAL(timeout()), this, SLOT(InfoSMB()));
	connect(&timerSmbRequest, SIGNAL(timeout()), this, SLOT(slot_timer()));

	//autoconnect
	if (autoconnect) Slot_connect();

}

main_windows::~main_windows()
{
	debugQt("main_windows::~main_windows()");
}

/**
	Before quit qtsmbstatus-client
*/
void main_windows::beforeQuit()
{
	debugQt("main_windows::beforeQuit()");
	trayicon->setVisible(false);
	this->close();
}


/**
	When user restore or minimize windows
*/
void main_windows::restore_minimize()
{
	debugQt("main_windows::restore_minimize()");
	if ((this->isMinimized ()) || !(this->isVisible()))
	{
		this->setVisible(true);
		this->showNormal ();
	}
	else
	{
		this->setVisible(false);
		logform->setVisible(false);
	}
}

/**
	Connected to signal when the user activates the system tray icon
	\param reason describes the reason the system tray was activated
	\sa restore_minimize
*/
void main_windows::trayicon_activated(QSystemTrayIcon::ActivationReason reason)
{
	debugQt("main_windows::trayicon_activated()");
	if ((this->isMinimized ()) || !(this->isVisible()))
		restore_action->setText(tr("Restore"));
	else
		restore_action->setText(tr("Minimize"));

	if (reason == QSystemTrayIcon::Trigger)
		restore_minimize();
}

/**
	restore window size
	\sa saveWindowSize

	On startup, restore windows size.
*/
void main_windows::restoreWindowSize()
{
	debugQt("main_windows::restoreWindowSize()");
	QSettings settings;
	settings.beginGroup("/MainWindow");
		settings.beginGroup("/Geometry");
			QRect rec= this->geometry ();
			int left=settings.value( "left",20 ).toInt();
			int top=settings.value( "top", 20 ).toInt();
			int width=settings.value( "width",300 ).toInt();
			int height=settings.value( "height", 600 ).toInt();
		settings.endGroup();
	settings.endGroup();
	this->setGeometry(QRect( left, top, width, height ));
}

/**
	save window size
	\sa restoreWindowSize

	Before quit, save window size
*/
void main_windows::saveWindowSize()
{
	debugQt("main_windows::saveWindowSize()");
	QSettings settings;
	settings.beginGroup("/MainWindow");
		settings.beginGroup("/Geometry");
			QRect rec= this->geometry ();
			settings.setValue( "left", rec.left () );
			settings.setValue( "top", rec.top () );
			settings.setValue( "width", rec.width () );
			settings.setValue( "height", rec.height () );
		settings.endGroup();
	settings.endGroup();
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
		saveWindowSize();
		logform->close();
		logform->deleteLater();
		if (socketState()!=UnconnectedState) Disconnect(); //if connected, disconnect
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
void main_windows::Slot_connect()
{
	debugQt("main_windows::Slot_connect()");
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
	Slot connected. When client is connected to server
	\sa Slot_connect
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
	
	firstTime=true;
	timerinfoSmb.start(10000);

	statusBar()->showMessage( tr("Connected to host") );

	// create icon server on treeWidget
	item_server = new server( treeWidget );
	//treeWidget->addTopLevelItem (item_server);

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
	Send data to server.
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
	When configuration changed
	\sa configure_windows
*/
void main_windows::configuration_changed()
{
	debugQt("main_windows::configuration_changed()");
	if (!iconize) this->setVisible(true);
	action_View_log->setEnabled(log_activity);
	viewlog_action->setEnabled(log_activity);
	trayicon->setVisible(iconize);
	timerSmbRequest.setInterval(interval*1000);
}


/**
	Bad login or bad password.
*/
void main_windows::errorAuth()
{
	debugQt("main_windows::errorAuth()");
	QMessageBox::warning ( this, "QtSmbstatus",tr("Invalid login or password !"));
	Slot_connect();
}

/**
	menu Help/helpAbout
*/
void main_windows::helpAbout()
{
	QMessageBox::about( this, "QtSmbstatus","<b>QtSmbstatus : </b>"+web_qtsmbstatus+"<br>Version: <b>"+version_qtsmbstatus+"</b><br>"+date_qtsmbstatus+"<br>"
		"<b>Developer : </b> "+auteur_qtsmbstatus+" "+mail_qtsmbstatus+"<br><hr><br>"
		"<b>"+tr("Thanks to:")+"</b><br/>"
		"<table border='1'>"
		"<tr><td>Polish translation </td><td>Krzysztof Bzowski <a href='mailto:sh4dow4@gmail.com'>sh4dow4@gmail.com</a><br/>Pawel Dyda</td></tr>"
		"<tr><td>Spanish translation </td><td>Mauro</td></tr>"
		"<tr><td>Italian translation </td><td>Naldini Paolo</td></tr>"
		"<tr><td>German translation </td><td>Alex Djekic</td></tr>"
		"<tr><td>Hungarian translation </td><td>Szenogradi Norbert Peter <a href='mailto:sevoir75@gmail.com'>sevoir75@gmail.com</a></td></tr>"
		"<tr><td>Russian and Ukrainian translation </td><td>Motsyo Gennadi <a href='mailto:drool@altlinux.ru'>drool@altlinux.ru</a></td></tr>"
		"<tr><td>Swedish translation </td><td>Zirro</td></tr>"
		"<tr><td>Brazilian Portuguese translation </td><td>Rafael Porto Rodrigues <a href='mailto:rafporto@gmail.com'>rafporto@gmail.com</a></td></tr>"
		"<tr><td>Dutch translation </td><td>Robert Wubs <a href='mailto:wubsieonline@gmail.com'>wubsieonline@gmail.com</a></td></tr>"
		"<tr><td>RPM packages for FC3 </td><td>CG (cyberkoa)</td></tr>"
		"<tr><td>Second reading </td><td>Eric Bresson <a href='mailto:eric.public@wanadoo.fr'>eric.public@wanadoo.fr</a></td></tr>"
		"</table><br><hr>"+
		tr("This program is licensed to you under the terms of the GNU General Public License version 2"));
}

/**
	menu Help/helpAboutQt
*/
void main_windows::helpAboutQt()
{
	QMessageBox::aboutQt( this, "Qt Application" );
}

/**
	Configure qtsmbstatus client (open form)
	\sa configure_windows
*/
void main_windows::ConfigureSlot()
{
	debugQt("main_windows::ConfigureSlot()");
	configure_windows * configurate = new configure_windows(this);
	connect (configurate,SIGNAL(configuration_changed()),this, SLOT(configuration_changed()));
	configurate->exec();
}

/**
	View log
	\sa LogForm
*/
void main_windows::on_action_View_log_triggered ()
{
	debugQt("main_windows::on_action_View_log_triggered ()");
	logform->show();
}


/**
	Find items
 */
void main_windows::AllSearchSlot()
{
	debugQt("main_windows::AllSearchSlot()");
	bool ok;
	currentIndexOfListItem=0;
	QString txt = QInputDialog::getText(this,"Search", tr("Find text")+":", QLineEdit::Normal,QString::null, &ok );
	if ( ok && !txt.isEmpty() ) {
		SearchTxt=txt;
		search();
	}
}


/**
	Find next.
	\sa AllSearchSlot
*/
void main_windows::NextSlot()
{
	debugQt("main_windows::NextSlot()");
	search(to_next);
}


/**
	Select item
	\param item pointer on QListViewItem
	\sa nextItem search
*/
void main_windows::selectItem(QTreeWidgetItem *item)
{
	debugQt("main_windows::selectItem()");
	treeWidget->setCurrentItem(item);
}


/**
	Search item.
	\param direction see T_direction
	\sa T_direction selectItem
*/
void main_windows::search(T_Direction direction)
{
	debugQt("main_windows::search()");
	
	QList<QTreeWidgetItem *> listItemFound=treeWidget->findItems (SearchTxt, Qt::MatchContains | Qt::MatchRecursive);
	
	if (direction==to_next)
		currentIndexOfListItem++;
	
	if (direction==to_first || currentIndexOfListItem>=listItemFound.size())
		currentIndexOfListItem=0;
	
	if (listItemFound.size()==0)
	{
		treeWidget->clearSelection ();
		statusBar ()->showMessage  ( tr("Text not found."), 2000 );
		QApplication::beep ();
		searchNextAction->setEnabled ( false );
	} else {
		QTreeWidgetItem *FindItem=listItemFound.at(currentIndexOfListItem);
		selectItem(FindItem);
		searchNextAction->setEnabled ( true );
	}
}


/**
	Info CIFS/SMB for log and balloon messages
*/
void main_windows::InfoSMB()
{
	debugQt("main_windows::InfoSMB()");
	QString user;
	QString opened;
	QString machine;
	short unsigned int type_message=0;
	QString message;
	int occurence=0;
	int nb_occurences=3; // number of occurences on "balloon messages"
	server * item =dynamic_cast<server *>(treeWidget->topLevelItem(0));
	if (item)
	{
		while (!item->listMessages.isEmpty())
		{
			occurence++;
			if (occurence>1 && occurence<=nb_occurences) message+="\n"; // LineFeed
			user=item->listMessages.first().user;
			opened=item->listMessages.first().opened;
			// limit size of text
			if (opened.size()>60) opened=opened.left(7)+"..."+opened.right(50);
			machine=item->listMessages.first().machine;
			type_message=item->listMessages.first().type_message;
			//view 3 occurences max
			if (type_message==0 && occurence<=nb_occurences) message+=tr("User %1 has opened share %2 (%3)").arg(user).arg(opened).arg(machine);
			if (type_message==1 && occurence<=nb_occurences) message+=tr("User %1 has opened file %2 (%3)").arg(user).arg(opened).arg(machine);
			// log
			if (log_activity) emit (refreshviewlog(item->listMessages.first()));
			item->listMessages.removeFirst ();
		}
		if (occurence>nb_occurences) message+="\n  ..."; // hide others datas
		// refresh filter
		if (occurence>0) logform->on_filterEdit_textChanged();
		// erase old log
		logform->eraseOldLog();
		// just see 1 "balloon" (3 last occurences) if show_messages=true
		if (!message.isEmpty() && show_messages && !firstTime) trayicon->showMessage ( tr("CIFS/SMB activities"), message, QSystemTrayIcon::Information ) ;
	}
	firstTime=false;
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
	Slot smbstatus timer
	Request smbstatus to server
*/
void main_windows::slot_timer()
{
	debugQt("main_windows::slot_timer()");
	sendToServer(smb_rq);
}

/**
	Receive informations from server (right for current client)
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
							deleteLater();
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
	Interpret smbstatus reply.
	\sa smbstatus
*/
void main_windows::AnalysisSmbstatus()
{
	debugQt("main_windows::AnalysisSmbstatus()");
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
void main_windows::AnalysisSmbDestroyed ()
{
	debugQt("main_windows::AnalysisSmbDestroyed ()");
	item_server->refresh_childs(); // erase items obsoleted
	ListSmbstatus.clear(); // clear QStringList
}


/**
	InstanceSmbstatus sends samba version.
	\param version_samba samba version
	\sa smbstatus
*/
void main_windows::setSambaVersion (const QString & version_samba)
{
	debugQt("main_windows::setSambaVersion ()");
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
void main_windows::add_user (const QString & strPid,const QString & strUser,const QString & strGroup,const QString & strMachineName,const QString & strMachineIP)
{
	debugQt("main_windows::add_user ()");
	item_server->add_user(strPid,strUser,strGroup,strMachineName,strMachineIP);
}

/**
	InstanceSmbstatus has found a share
	\param strPid PID
	\param strShare Share name
	\param strConnected date
	\sa smbstatus
*/
void main_windows::add_share(const QString & strPid ,const QString & strShare,const QString & strConnected)
{
	debugQt("main_windows::add_share ()");
	// if hidden shares
	if (!view_hidden_shares && (strShare.indexOf(QRegExp("\\$$"))!=-1)) return;
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
void main_windows::add_lockedfile(const QString & strPid,const QString & strName,const QString & strMode,const QString & strRW,const QString & strOplock,const QString & strDateOpen)
{
	debugQt("main_windows::add_lockedfile ()");
	item_server->add_lockedfile(strPid,strName,strMode,strRW,strOplock,strDateOpen);

}


/**
	popup menu
*/
void main_windows::slotPopupMenu(  const QPoint & point )
{
	currentPopupMenuItem = treeWidget->itemAt ( point );
	// if deconnected, no popup menu
	if (socketState()!=ConnectedState) return;
	if( currentPopupMenuItem )
	{
		// create popup menu
		menuPopup->clear();

		if (currentPopupMenuItem==item_server) // if a server item
		{
			menuPopup->addAction ( tr( "Properties"),this,SLOT(InfoServer() ) );
			QAction * action=menuPopup->addAction ( tr( "Send out message to all users"),this,SLOT(slotSendMessageAllUsers() ) );
			action->setEnabled(permitSendMsg);
			menuPopup->exec( QCursor::pos() );
			return;
		}
		if (currentPopupMenuItem->parent()==item_server) // if a machine item
		{
			machine * myItem=dynamic_cast<machine *>(currentPopupMenuItem);
			if (!myItem) return;
			menuPopup->addAction ( tr( "Properties"),this,SLOT(InfoMachine() ) );
			QAction * action=menuPopup->addAction ( tr( "Send out message to")+ " " + myItem->machine_name,this,SLOT(slotSendMessage() ) );
			action->setEnabled(permitSendMsg); // if client can send popup messages
			menuPopup->exec( QCursor::pos() );
			return;
		}
		if (currentPopupMenuItem->parent()->parent()==item_server) // if an user item
		{
			user * myItem=dynamic_cast<user *>(currentPopupMenuItem);
			if (!myItem) return;
			menuPopup->addAction ( tr( "Properties"),this,SLOT(InfoUser() ) );
			QAction * action=menuPopup->addAction ( tr( "Disconnect user")+ " " + myItem->username,this,SLOT(slotDisconnectUser() ) );
			action->setEnabled(permitDisconnectUser); //  if client can disconnect an user
			menuPopup->exec( QCursor::pos() );
			return;
		}
		// it's a locked file or share
		menuPopup->addAction ( tr( "Properties"),this,SLOT(InfoService() ) );
		menuPopup->exec( QCursor::pos() );
	}
}

/**
	View samba version
	\sa slotPopupMenu
	\sa server::ViewInfoUser
*/
void main_windows::InfoServer()
{
	// if object is dead
	if (!QTreeWidgetItemList.contains(currentPopupMenuItem)) return;
	QString message=item_server->ViewInfoServer();
	QMessageBox::information(this, "QtSmbstatus",message,QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
}

/**
	View user informations
	\sa slotPopupMenu
	\sa server::ViewInfoUser
*/
void main_windows::InfoUser()
{
	// if object is dead
	if (!QTreeWidgetItemList.contains(currentPopupMenuItem)) return;
	QString message=item_server->ViewInfoUser(currentPopupMenuItem);
	QMessageBox::information(this, "QtSmbstatus",message,QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
}

/**
	View service informations (locked file or share)
	\sa slotPopupMenu
	\sa server::ViewInfoService
*/
void main_windows::InfoService()
{
	// if object is dead
	if (!QTreeWidgetItemList.contains(currentPopupMenuItem)) return;
	QString message=item_server->ViewInfoService(currentPopupMenuItem);
	QMessageBox::information(this, "QtSmbstatus",message,QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
}

/**
	View machine informations
	\sa slotPopupMenu
	\sa server::ViewInfoMachine
*/
void main_windows::InfoMachine()
{
	// if object is dead
	if (!QTreeWidgetItemList.contains(currentPopupMenuItem)) return;
	QString message=item_server->ViewInfoMachine(currentPopupMenuItem);
	QMessageBox::information(this, "QtSmbstatus",message,QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
}


/**
	send out messages (popupwindows) to all users
	\sa slotPopupMenu
*/
void main_windows::slotSendMessageAllUsers() {
	machine* Item;
	bool ok;
	QString message = QInputDialog::getText(this,
            "QtSmbstatus", tr("Message to send")+":", QLineEdit::Normal,
            QString::null, &ok);

	if ( !ok || message.isEmpty() || socketState()!=ConnectedState ) return;
	
	for (int i=0;  i < item_server->childCount () && socketState()==ConnectedState ; ++i )
	{
		Item= dynamic_cast<machine *>(item_server->child (i) );
		if (!Item) break;
		sendToServer(send_msg,Item->machine_name,message);
	}
}

/**
	send out messages (popupwindows) to one user
	\sa slotPopupMenu
*/
void main_windows::slotSendMessage() {
	bool ok;
	// if object is dead
	if (!QTreeWidgetItemList.contains(currentPopupMenuItem)) return;

	machine* Item=dynamic_cast<machine *>(currentPopupMenuItem);
	if (!Item) return;

	QString message = QInputDialog::getText(this,
		"QtSmbstatus", tr("Message to send to %1:").arg(Item->machine_name), QLineEdit::Normal,
            QString::null, &ok);
	if ( ok && !message.isEmpty() && socketState()==ConnectedState )
	{
		// user entered something and pressed OK
		// if object is dead
		if (!QTreeWidgetItemList.contains(currentPopupMenuItem)) return;

		sendToServer(send_msg,Item->machine_name,message);
	}
}

/**
	Disconnect an user
	\sa slotPopupMenu
*/
void main_windows::slotDisconnectUser() {
	
	QString username;
	QString pid;
	// if object is dead
	if (!QTreeWidgetItemList.contains(currentPopupMenuItem)) return;

	user* Item=dynamic_cast<user *>(currentPopupMenuItem);
	if (!Item) return;
	// get PID
	pid=Item->pid;

	// get user name
	username=Item->username;

	if ( !QMessageBox::information(this,"QtSmbstatus",
	      tr("Do you really want to disconnect user %1 ?").arg(username),
		 tr("&Yes"), tr("&No"),QString::null, 1, 1 ) && socketState()==ConnectedState )
	{
		// if object is dead
		if (!QTreeWidgetItemList.contains(currentPopupMenuItem)) return;

		sendToServer(kill_user,pid,username);
	}
}

/**
	Check for update of qtsmbstatus
	\sa requestHtmlFinished
*/
void main_windows::checkForUpdateOfQtSmbstatus () {
	debugQt("main_windows::checkForUpdateOfQtSmbstatus ()");
	QSettings settings;
	settings.beginGroup("/Configuration");
	QDateTime dateTimeForlastCheck=settings.value("dateTimeForlastCheck").toDateTime ();
	QString uuid=settings.value("uuid").toString().remove(QRegExp("[{}]"));
	settings.endGroup();
	if (dateTimeForlastCheck.isValid()) {
		if (dateTimeForlastCheck>QDateTime::currentDateTime().addDays(-1))
			return; // wait 1 day 
	}
	http = new QHttp(this);
	connect(http, SIGNAL(done(bool)), this, SLOT(requestHtmlFinished ( bool )));
	http->setHost("qtsmbstatus.free.fr");
	http->get("/last_version.php?uuid="+uuid);
}

/**
	request HTML finished
	\sa checkForUpdateOfQtSmbstatus
*/
void main_windows::requestHtmlFinished (  bool error ) {
	debugQt("main_windows::requestHtmlFinished ()");
	if (!error) {
		bool ok;
		uint available_release=QString(http->readAll()).simplified().toUInt (&ok);
		if (ok) {
			debugQt("Available release: "+QString::number(available_release));
			QSettings settings;
			settings.beginGroup("/Configuration");
			uint last_known_release=settings.value("lastKnownRelease",int_qtsmbstatus_version).toUInt();
			settings.setValue("lastKnownRelease",available_release);
			settings.setValue("dateTimeForlastCheck",QDateTime::currentDateTime());
			if (available_release>last_known_release)
			{
				debugQt("A new release is available :-)");
				// New version available
				QDialog * dialog= new QDialog(this);
				dialog->setAttribute(Qt::WA_DeleteOnClose);
				QDialogButtonBox * dialogButton=new QDialogButtonBox ( QDialogButtonBox::Ok, Qt::Horizontal,dialog);
				dialog->setWindowTitle ( tr("Update is available"));
				QGridLayout *layout = new QGridLayout(dialog);
				QLabel * label= new QLabel("<p align='center'>"+
					tr("A new release of %1 is available").arg("qtsmbstatus")+
					".</p><p align='center'>"+
					tr("You can download it here: %1").arg("<br>"+web_qtsmbstatus)+
					"</p><hr>",dialog);
				label->setTextFormat (Qt::RichText);
				label->setTextInteractionFlags (Qt::LinksAccessibleByMouse);
				label->setOpenExternalLinks (true);
				QLabel * labelPixmap= new QLabel(dialog);
				labelPixmap->setPixmap(QPixmap(":/icons/samba.png"));
				layout->addWidget(labelPixmap, 0, 0, 5, 1);
				layout->addWidget(label, 0, 1, 5, 1);
				layout->addWidget(dialogButton, 6, 0, 1, 2, Qt::AlignHCenter);
				dialog->setLayout(layout);
				connect(dialogButton,SIGNAL(accepted ()),dialog,SLOT(accept()));
				dialog->show();
			} else debugQt("No new release");
			settings.endGroup();
		} else writeToConsole("impossible to convert release number");
	} else writeToConsole("impossible to check for update");
	http->deleteLater();
}
