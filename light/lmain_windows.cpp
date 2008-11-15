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

#include "lmain_windows.h"

extern void writeToConsole(const QString & message);
extern QList<QTreeWidgetItem *> QTreeWidgetItemList;

/**
	\class MyApplicationL
	\brief reimplementing QApplication::commitData()
	\date 2007-07-26
	\version 1.0
	\author Daniel Rocher
*/
MyApplicationL::MyApplicationL(int & argc, char ** argv ) : QApplication(argc,argv) { }

/**
	reimplementing QApplication::commitData().
	This function deals with session management. It is invoked when the QSessionManager wants the application to commit all its data.
*/
void MyApplicationL::commitData(QSessionManager& manager)
{
	debugQt("MyApplicationL::commitData()");
	if (manager.allowsInteraction())
	{
		manager.release();
		emit quitMyApp();
	}
}

/**
	\class main_windowsl
	\brief Main Window for QtSmbstatus Light
	\date 2008-11-07
	\version 2.0
	\author Daniel Rocher
*/
main_windowsl::main_windowsl(QWidget *parent) : QMainWindow(parent)
{
	debugQt("main_windowsl::main_windowsl()");
	setupUi(this);

	if (check_new_release) checkForUpdateOfQtSmbstatus();
	
	msgError=new QErrorMessage(this); // an error message display dialog
	
	trayicon =new QSystemTrayIcon(QIcon (":/icons/trayicon.png"),this);
	trayicon->setToolTip ( tr("QtSmbstatus Client") );
	QMenu * menuApp=new QMenu( this );
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
	trayicon->setContextMenu ( menuApp );
	connect(trayicon,SIGNAL(activated ( QSystemTrayIcon::ActivationReason ) ),this,SLOT(trayicon_activated(QSystemTrayIcon::ActivationReason)));
	restoreWindowSize();
	this->setWindowTitle ( "QtSmbstatus light "+version_qtsmbstatus); // forms title
	// create statusBar

	// treeWidget
	treeWidget->setSortingEnabled ( true );
	treeWidget->setRootIsDecorated ( false );
	treeWidget->setHeaderLabel ( "");
	treeWidget->setContextMenuPolicy (Qt::CustomContextMenu );
	// create popup menu
	menuPopup = new QMenu( treeWidget );
	connect(treeWidget, SIGNAL( customContextMenuRequested ( const QPoint & ) ), this, SLOT( slotPopupMenu( const QPoint & ) ) );
	
	// first time
	configuration_changed();

	// form view CIFS/SMB activities
	logform = new LogForm(this);
	connect(this ,SIGNAL(refreshviewlog(const type_message &)), logform,SLOT(append(const type_message &)));

	connect(&timerinfoSmb, SIGNAL(timeout()), this, SLOT(InfoSMB()));
	connect(&timerSmbRequest, SIGNAL(timeout()), this, SLOT(slot_timer()));

	currentIndexOfListItem=0;
	QString SearchTxt="";

	searchNextAction->setEnabled ( false );
	firstTime=true;
	timerinfoSmb.start(10000);

	// start smbstatus request
	slot_timer(); // first time
	timerSmbRequest.start (interval*1000);

	statusBar()->showMessage( tr("Connected to host") );

	// create icon server on treeWidget
	item_server = new server( treeWidget );

}

main_windowsl::~main_windowsl()
{
	debugQt("main_windowsl::~main_windowsl()");
}

/**
	Before quit qtsmbstatus-client
*/
void main_windowsl::beforeQuit()
{
	debugQt("main_windowsl::beforeQuit()");
	trayicon->setVisible(false);
	this->close();
}


/**
	When %user restore or minimize windows
*/
void main_windowsl::restore_minimize()
{
	debugQt("main_windowsl::restore_minimize()");
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
	Connected to signal when the %user activates the system tray icon
	\param reason describes the reason the system tray was activated
	\sa restore_minimize
*/
void main_windowsl::trayicon_activated(QSystemTrayIcon::ActivationReason reason)
{
	debugQt("main_windowsl::trayicon_activated()");
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
void main_windowsl::restoreWindowSize()
{
	debugQt("main_windowsl::restoreWindowSize()");
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
void main_windowsl::saveWindowSize()
{
	debugQt("main_windowsl::saveWindowSize()");
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
void main_windowsl::closeEvent(QCloseEvent *e)
{
	debugQt("main_windowsl::closeEvent(QCloseEvent *e)");
	if (trayicon->isVisible())
	{
		this->hide ();
		logform->hide();
		e->ignore();
	}
	else
	{
		saveWindowSize();
		logform->close();
		logform->deleteLater();
		treeWidget->clear();
		timerinfoSmb.stop();
		timerSmbRequest.stop();
		QTimer::singleShot(800, qApp, SLOT(quit())); // wait before quit
		e->accept();
	}
}


/**
	When configuration changed
	\sa configure_windows
*/
void main_windowsl::configuration_changed()
{
	debugQt("main_windowsl::configuration_changed()");
	if (!iconize) this->setVisible(true);
	action_View_log->setEnabled(log_activity);
	viewlog_action->setEnabled(log_activity);
	trayicon->setVisible(iconize);
	timerSmbRequest.setInterval(interval*1000);
}



/**
	menu Help/helpAbout
*/
void main_windowsl::helpAbout()
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
void main_windowsl::helpAboutQt()
{
	QMessageBox::aboutQt( this, "Qt Application" );
}

/**
	Configure qtsmbstatus client (open form)
	\sa configure_windows
*/
void main_windowsl::ConfigureSlot()
{
	debugQt("main_windowsl::ConfigureSlot()");
	configure_windowsl * configurate = new configure_windowsl(this);
	connect (configurate,SIGNAL(configuration_changed()),this, SLOT(configuration_changed()));
	configurate->exec();
}

/**
	View log
	\sa LogForm
*/
void main_windowsl::on_action_View_log_triggered ()
{
	debugQt("main_windowsl::on_action_View_log_triggered ()");
	logform->show();
}


/**
	Find items
	\sa search NextSlot
 */
void main_windowsl::AllSearchSlot()
{
	debugQt("main_windowsl::AllSearchSlot()");
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
void main_windowsl::NextSlot()
{
	debugQt("main_windowsl::NextSlot()");
	search(to_next);
}


/**
	Select item
	\param item pointer on QListViewItem
	\sa NextSlot search
*/
void main_windowsl::selectItem(QTreeWidgetItem *item)
{
	debugQt("main_windowsl::selectItem()");
	treeWidget->setCurrentItem(item);
}


/**
	Search item.
	\param direction
	\sa T_Direction selectItem
*/
void main_windowsl::search(T_Direction direction)
{
	debugQt("main_windowsl::search()");
	
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
void main_windowsl::InfoSMB()
{
	debugQt("main_windowsl::InfoSMB()");
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
	Slot %smbstatus timer
	Request %smbstatus to %server
*/
void main_windowsl::slot_timer()
{
	debugQt("main_windowsl::slot_timer()");
	smbmanager* processus_smbstatus = new  smbmanager(this);
	connect(processus_smbstatus,SIGNAL(ObjError(const QString &)),msgError, SLOT(showMessage (const QString &)) );
	connect( processus_smbstatus, SIGNAL(signal_std_output(const QStringList &)),this, SLOT(AnalysisSmbstatus(const QStringList &)) );
}



/**
	Interpret %smbstatus reply.
	\param listSmbstatus a QStringList
	\sa smbstatus
*/
void main_windowsl::AnalysisSmbstatus(const QStringList & listSmbstatus)
{
	debugQt("main_windowsl::AnalysisSmbstatus()");
	// mark items of listview first
	item_server->mark_childs();
	InstanceSmbstatus = new smbstatus(listSmbstatus,this);

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
void main_windowsl::AnalysisSmbDestroyed ()
{
	debugQt("main_windowsl::AnalysisSmbDestroyed ()");
	item_server->refresh_childs(); // erase items obsoleted
}


/**
	InstanceSmbstatus sends samba version.
	\param version_samba samba version
	\sa smbstatus
*/
void main_windowsl::setSambaVersion (const QString & version_samba)
{
	debugQt("main_windowsl::setSambaVersion ()");
	item_server->setSambaVersion(version_samba);
}

/**
	InstanceSmbstatus has found an %user
	\param strPid PID
	\param strUser User name
	\param strGroup Group name
	\param strMachineName Machine name
	\param strMachineIP IP address
	\sa smbstatus
**/
void main_windowsl::add_user (const QString & strPid,const QString & strUser,const QString & strGroup,const QString & strMachineName,const QString & strMachineIP)
{
	debugQt("main_windowsl::add_user ()");
	item_server->add_user(strPid,strUser,strGroup,strMachineName,strMachineIP);
}

/**
	InstanceSmbstatus has found a %share
	\param strPid PID
	\param strShare Share name
	\param strConnected date
	\sa smbstatus
*/
void main_windowsl::add_share(const QString & strPid ,const QString & strShare,const QString & strConnected)
{
	debugQt("main_windowsl::add_share ()");
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
void main_windowsl::add_lockedfile(const QString & strPid,const QString & strName,const QString & strMode,const QString & strRW,const QString & strOplock,const QString & strDateOpen)
{
	debugQt("main_windowsl::add_lockedfile ()");
	item_server->add_lockedfile(strPid,strName,strMode,strRW,strOplock,strDateOpen);

}


/**
	popup menu
*/
void main_windowsl::slotPopupMenu(  const QPoint & point )
{
	currentPopupMenuItem = treeWidget->itemAt ( point );
	if( currentPopupMenuItem )
	{
		// create popup menu
		menuPopup->clear();

		if (currentPopupMenuItem==item_server) // if a server item
		{
			menuPopup->addAction ( tr( "Properties"),this,SLOT(InfoServer() ) );
			menuPopup->addAction ( tr( "Send out message to all users"),this,SLOT(slotSendMessageAllUsers() ) );
			menuPopup->exec( QCursor::pos() );
			return;
		}
		if (currentPopupMenuItem->parent()==item_server) // if a machine item
		{
			machine * myItem=dynamic_cast<machine *>(currentPopupMenuItem);
			if (!myItem) return;
			menuPopup->addAction ( tr( "Properties"),this,SLOT(InfoMachine() ) );
			menuPopup->addAction ( tr( "Send out message to")+ " " + myItem->machine_name,this,SLOT(slotSendMessage() ) );
			menuPopup->exec( QCursor::pos() );
			return;
		}
		if (currentPopupMenuItem->parent()->parent()==item_server) // if an user item
		{
			user * myItem=dynamic_cast<user *>(currentPopupMenuItem);
			if (!myItem) return;
			menuPopup->addAction ( tr( "Properties"),this,SLOT(InfoUser() ) );
			menuPopup->addAction ( tr( "Disconnect user")+ " " + myItem->username,this,SLOT(slotDisconnectUser() ) );
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
	\sa server::ViewInfoServer
*/
void main_windowsl::InfoServer()
{
	// if object is dead
	if (!QTreeWidgetItemList.contains(currentPopupMenuItem)) return;
	QString message=item_server->ViewInfoServer();
	QMessageBox::information(this, "QtSmbstatus",message,QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
}

/**
	View %user informations
	\sa slotPopupMenu
	\sa server::ViewInfoUser
*/
void main_windowsl::InfoUser()
{
	// if object is dead
	if (!QTreeWidgetItemList.contains(currentPopupMenuItem)) return;
	QString message=item_server->ViewInfoUser(currentPopupMenuItem);
	QMessageBox::information(this, "QtSmbstatus",message,QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
}

/**
	View %service informations (locked file or %share)
	\sa slotPopupMenu
	\sa server::ViewInfoService
*/
void main_windowsl::InfoService()
{
	// if object is dead
	if (!QTreeWidgetItemList.contains(currentPopupMenuItem)) return;
	QString message=item_server->ViewInfoService(currentPopupMenuItem);
	QMessageBox::information(this, "QtSmbstatus",message,QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
}

/**
	View %machine informations
	\sa slotPopupMenu
	\sa server::ViewInfoMachine
*/
void main_windowsl::InfoMachine()
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
void main_windowsl::slotSendMessageAllUsers() {
	machine* Item;
	bool ok;
	QString message = QInputDialog::getText(this,
            "QtSmbstatus", tr("Message to send")+":", QLineEdit::Normal,
            QString::null, &ok);

	if ( !ok || message.isEmpty() ) return;
	
	for (int i=0;  i < item_server->childCount () ; ++i )
	{
		Item= dynamic_cast<machine *>(item_server->child (i) );
		if (!Item) break;
		Sendmessage_manager* send_msg = new Sendmessage_manager(Item->machine_name,message,this);
		connect(send_msg,SIGNAL(ObjError(const QString &)),msgError, SLOT(showMessage (const QString &)) );
	}
}

/**
	send out messages (popupwindows) to one %user
	\sa slotPopupMenu
*/
void main_windowsl::slotSendMessage() {
	bool ok;
	// if object is dead
	if (!QTreeWidgetItemList.contains(currentPopupMenuItem)) return;

	machine* Item=dynamic_cast<machine *>(currentPopupMenuItem);
	if (!Item) return;

	QString message = QInputDialog::getText(this,
		"QtSmbstatus", tr("Message to send to %1:").arg(Item->machine_name), QLineEdit::Normal,
            QString::null, &ok);
	if ( ok && !message.isEmpty() )
	{
		// user entered something and pressed OK
		// if object is dead
		if (!QTreeWidgetItemList.contains(currentPopupMenuItem)) return;

		Sendmessage_manager* send_msg = new Sendmessage_manager(Item->machine_name,message,this);
		connect(send_msg,SIGNAL(ObjError(const QString &)),msgError, SLOT(showMessage (const QString &)) );
	}
}

/**
	Disconnect an %user
	\sa slotPopupMenu
*/
void main_windowsl::slotDisconnectUser() {
	
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
		 tr("&Yes"), tr("&No"),QString::null, 1, 1 ) )
	{
		// if object is dead
		if (!QTreeWidgetItemList.contains(currentPopupMenuItem)) return;
		process_smbd_exist* killSmbProc = new process_smbd_exist(pid,username,this);
		connect(killSmbProc,SIGNAL(ObjError(const QString &)),msgError, SLOT(showMessage (const QString &)) );
	}
}

/**
	Check for update of qtsmbstatus
	\sa requestHtmlFinished
*/
void main_windowsl::checkForUpdateOfQtSmbstatus () {
	debugQt("main_windowsl::checkForUpdateOfQtSmbstatus ()");
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
void main_windowsl::requestHtmlFinished (  bool error ) {
	debugQt("main_windowsl::requestHtmlFinished ()");
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
