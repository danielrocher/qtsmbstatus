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
#include "mainwindows.h"

extern void writeToConsole(const QString & message);
extern QList<QTreeWidgetItem *> QTreeWidgetItemList;

//default value
//! Interval, in seconds, between every request to %smbstatus
int interval=10;
//!  View hidden shares (share$)
bool view_hidden_shares = true;
//! Iconize QtSmbstatus on system tray
bool iconize=true;
//! show status notification messages (balloon messages)
bool show_messages=true;
//! log SMB/CIFS activities
bool log_activity=false;
//! limit log (number of days)
int limitLog=1;
//!  check for new release of qtsmbstatus
bool check_new_release=true;


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
	\class MainWindows
	\brief Main Window for QtSmbstatus Client et Light
	\date 2008-11-16
	\version 2.1
	\author Daniel Rocher
 */
MainWindows::MainWindows(QWidget *parent) : QMainWindow(parent)
{
	debugQt("MainWindows::MainWindows()");
	setupUi(this);
	if (check_new_release) checkForUpdateOfQtSmbstatus();
	
	msgError=new QErrorMessage(this); // an error message display dialog
	
	trayicon =new QSystemTrayIcon(QIcon (":/icons/qtsmbstatus.png"),this);
	menuApp=new QMenu( this );
	viewlog_action=menuApp->addAction ( tr("Show CIFS/SMB activities") );
	connect(viewlog_action,SIGNAL( triggered ()),this , SLOT(on_action_View_log_triggered()));
	configure_action= menuApp->addAction ( QIcon (":/icons/configure.png"),tr("Configure") );
	connect(configure_action,SIGNAL( triggered ()),this , SLOT(on_configureAction_triggered()));
	menuApp->addSeparator ();
	restore_action=menuApp->addAction (tr("Minimize") );
	connect(restore_action,SIGNAL( triggered ()),this , SLOT(restore_minimize()));
	QAction * exit_action=menuApp->addAction (  QIcon (":/icons/exit.png"),tr("Quit") );
	connect(exit_action,SIGNAL( triggered ()),this , SLOT(beforeQuit () ));
	connect(action_Quit,SIGNAL( triggered ()),this , SLOT(beforeQuit () ));
	
	trayicon->setContextMenu ( menuApp );
	connect(trayicon,SIGNAL(activated ( QSystemTrayIcon::ActivationReason ) ),this,SLOT(trayicon_activated(QSystemTrayIcon::ActivationReason)));
	restoreWindowSize();
	
	// create statusBar
	statusBar()->showMessage (""); //status bar
	
	smbstatus_rq_started=false;
	
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
}

MainWindows::~MainWindows()
{
	debugQt("MainWindows::~MainWindows()");
}

/**
	Before quit qtsmbstatus
 */
void MainWindows::beforeQuit()
{
	debugQt("MainWindows::beforeQuit()");
	trayicon->setVisible(false);
	this->close();
}



/**
	When %user restore or minimize windows
 */
void MainWindows::restore_minimize()
{
	debugQt("MainWindows::restore_minimize()");
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
void MainWindows::trayicon_activated(QSystemTrayIcon::ActivationReason reason)
{
	debugQt("MainWindows::trayicon_activated()");
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
void MainWindows::restoreWindowSize()
{
	debugQt("MainWindows::restoreWindowSize()");
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
void MainWindows::saveWindowSize()
{
	debugQt("MainWindows::saveWindowSize()");
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
	When configuration changed
	\sa configure_windows
 */
void MainWindows::configuration_changed()
{
	debugQt("MainWindows::configuration_changed()");
	if (!iconize) this->setVisible(true);
	action_View_log->setEnabled(log_activity);
	viewlog_action->setEnabled(log_activity);
	trayicon->setVisible(iconize);
	timerSmbRequest.setInterval(interval*1000);
}



/**
	menu Help/helpAbout
 */
void MainWindows::helpAbout()
{
	QMessageBox::about( this, "QtSmbstatus","<b>QtSmbstatus : </b>"+web_qtsmbstatus+"<br>Version: <b>"+version_qtsmbstatus+"</b><br>"+date_qtsmbstatus+"<br>"
			"<b>Developer : </b> "+auteur_qtsmbstatus+" "+mail_qtsmbstatus+"<br><hr><br>"
					"<b>"+tr("Thanks to:")+QString::fromUtf8("</b><br/>"
					"<table border='1'>"
					"<tr><td>Polish translation </td><td>Inox, Jarosław Ogrodnik, Pawel Dyda, Sh4dow</td></tr>"
					"<tr><td>Spanish translation </td><td>Abel O'Rian, Amaeth, DiegoJ, Eduardo Alberto Calvo, Fernando Muñoz, Ian Springer, Jorge Dardón, Monkey</td></tr>"
					"<tr><td>Italian translation </td><td>Martino Barbon, Naldini Paolo, Salvatore Udda, simone.sandri</td></tr>"
					"<tr><td>German translation </td><td>Alex Djekic, Dennis Baudys, Sascha Biermanns, Till Freier</td></tr>"
					"<tr><td>Hungarian translation </td><td>Muszela Balázs, Pittmann Tamás, Szenográdi Norbert Péter, Szilveszter Kovács</td></tr>"
					"<tr><td>Russian translation </td><td>Motsyo Gennadi, Eugene Marshal</td></tr>"
					"<tr><td>Ukrainian translation </td><td>Motsyo Gennadi, Андрей Стрепетов</td></tr>"
					"<tr><td>Swedish translation </td><td>Zirro</td></tr>"
					"<tr><td>Brazilian Portuguese translation </td><td>Muriel de Souza Godoi, Rafael Porto Rodrigues</td></tr>"
					"<tr><td>Dutch translation </td><td>Jogai, Elco, Robert Wubs</td></tr>"
					"<tr><td>RPM packages for FC3 </td><td>CG (cyberkoa)</td></tr>"
					"<tr><td>Second reading </td><td>Eric Bresson</td></tr>"
					"</table><br><hr>")+
					tr("This program is licensed to you under the terms of the GNU General Public License version 2"));
}

/**
	menu Help/helpAboutQt
 */
void MainWindows::helpAboutQt()
{
	QMessageBox::aboutQt( this, "Qt Application" );
}


/**
	View log
	\sa LogForm
 */
void MainWindows::on_action_View_log_triggered ()
{
	debugQt("MainWindows::on_action_View_log_triggered ()");
	logform->show();
}


/**
	Find items
	\sa search NextSlot
 */
void MainWindows::AllSearchSlot()
{
	debugQt("MainWindows::AllSearchSlot()");
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
void MainWindows::NextSlot()
{
	debugQt("MainWindows::NextSlot()");
	search(to_next);
}


/**
	Select item
	\param item pointer on QListViewItem
	\sa NextSlot search
 */
void MainWindows::selectItem(QTreeWidgetItem *item)
{
	debugQt("MainWindows::selectItem()");
	treeWidget->setCurrentItem(item);
}


/**
	Search item.
	\param direction
	\sa T_Direction selectItem
 */
void MainWindows::search(T_Direction direction)
{
	debugQt("MainWindows::search()");
	
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
void MainWindows::InfoSMB()
{
	debugQt("MainWindows::InfoSMB()");
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
	Interpret %smbstatus reply.
	\param listSmbstatus a QStringList
	\sa smbstatus
 */
void MainWindows::AnalysisSmbstatus(const QStringList & listSmbstatus)
{
	debugQt("MainWindows::AnalysisSmbstatus()");
	// mark items of listview first
	item_server->mark_childs();
	smbstatus * InstanceSmbstatus = new smbstatus(listSmbstatus,this);

	// InstanceSmbstatus sends samba version
	connect (InstanceSmbstatus,SIGNAL(sambaVersion (const QString &)),this,SLOT(setSambaVersion (const QString &)));
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
void MainWindows::AnalysisSmbDestroyed ()
{
	debugQt("MainWindows::AnalysisSmbDestroyed ()");
	item_server->refresh_childs(); // erase items obsoleted
	smbstatus_rq_started=false;
}


/**
	InstanceSmbstatus sends samba version.
	\param version_samba samba version
	\sa smbstatus
 */
void MainWindows::setSambaVersion (const QString & version_samba)
{
	debugQt("MainWindows::setSambaVersion ()");
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
void MainWindows::add_user (const QString & strPid,const QString & strUser,const QString & strGroup,const QString & strMachineName,const QString & strMachineIP)
{
	debugQt("MainWindows::add_user ()");
	item_server->add_user(strPid,strUser,strGroup,strMachineName,strMachineIP);
}

/**
	InstanceSmbstatus has found a %share
	\param strPid PID
	\param strShare Share name
	\param strConnected date
	\sa smbstatus
 */
void MainWindows::add_share(const QString & strPid ,const QString & strShare,const QString & strConnected)
{
	debugQt("MainWindows::add_share ()");
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
void MainWindows::add_lockedfile(const QString & strPid,const QString & strName,const QString & strMode,const QString & strRW,const QString & strOplock,const QString & strDateOpen)
{
	debugQt("MainWindows::add_lockedfile ()");
	item_server->add_lockedfile(strPid,strName,strMode,strRW,strOplock,strDateOpen);

}



/**
	send out messages (popupwindows) to all users
	\sa slotPopupMenu
 */
void MainWindows::slotSendMessageAllUsers() {
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
		// if object is dead
		if (!QTreeWidgetItemList.contains(Item)) break;
		sendMessage(Item->machine_name,message);
	}
}

/**
	send out messages (popupwindows) to one %user
	\sa slotPopupMenu
 */
void MainWindows::slotSendMessage() {
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
		sendMessage(Item->machine_name,message);
	}
}

/**
	Disconnect an %user
	\sa slotPopupMenu
 */
void MainWindows::slotDisconnectUser() {
	
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
		disconnectUser(pid,username);
	}
}




/**
	popup menu
 */
void MainWindows::slotPopupMenu(  const QPoint & point )
{
	currentPopupMenuItem = treeWidget->itemAt ( point );
	// if object is dead
	if (!QTreeWidgetItemList.contains(currentPopupMenuItem)) return;
	
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
	\sa server::ViewInfoServer
 */
void MainWindows::InfoServer()
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
void MainWindows::InfoUser()
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
void MainWindows::InfoService()
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
void MainWindows::InfoMachine()
{
	// if object is dead
	if (!QTreeWidgetItemList.contains(currentPopupMenuItem)) return;
	QString message=item_server->ViewInfoMachine(currentPopupMenuItem);
	QMessageBox::information(this, "QtSmbstatus",message,QMessageBox::Ok,QMessageBox::NoButton,QMessageBox::NoButton);
}


/**
	Check for update of qtsmbstatus
	\sa requestHtmlFinished
 */
void MainWindows::checkForUpdateOfQtSmbstatus () {
	debugQt("MainWindows::checkForUpdateOfQtSmbstatus ()");
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
	http->get("/last_version.php?uuid="+uuid+"&app="+qApp->applicationName ());
}

/**
	request HTML finished
	\sa checkForUpdateOfQtSmbstatus
 */
void MainWindows::requestHtmlFinished (  bool error ) {
	debugQt("MainWindows::requestHtmlFinished ()");
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
			if (available_release>last_known_release && available_release>int_qtsmbstatus_version)
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
				labelPixmap->setPixmap(qApp->windowIcon().pixmap(QSize (32,32)) );
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





