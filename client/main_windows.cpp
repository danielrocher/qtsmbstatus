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


#include <QtGui>
#include <Q3PtrList>

#include "main_windows.h"

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
	\date 2007-06-17
	\version 1.0
	\author Daniel Rocher
*/
main_windows::main_windows(QWidget *parent) : QMainWindow(parent)
{
	debugQt("main_windows::main_windows()");
	setupUi(this);
	trayicon =new QSystemTrayIcon(QIcon (":/icons/trayicon.png"),this);
	trayicon->setToolTip ( tr("QtSmbstatus Client") );
	QMenu * menu=new QMenu( this );
	connect_action= menu->addAction ( QIcon (":/icons/connect_no.png"),tr("Connect") );
	connect(connect_action,SIGNAL( triggered ()),this , SLOT(Slot_connect()));
	menu->addSeparator ();
	viewlog_action=menu->addAction ( tr("Show CIFS/SMB activities") );
	connect(viewlog_action,SIGNAL( triggered ()),this , SLOT(on_action_View_log_triggered()));
	configure_action= menu->addAction ( QIcon (":/icons/configure.png"),tr("Configure") );
	connect(configure_action,SIGNAL( triggered ()),this , SLOT(ConfigureSlot()));
	menu->addSeparator ();
	restore_action=menu->addAction (tr("Minimize") );
	connect(restore_action,SIGNAL( triggered ()),this , SLOT(restore_minimize()));
 	QAction * exit_action=menu->addAction (  QIcon (":/icons/exit.png"),tr("Quit") );
 	connect(exit_action,SIGNAL( triggered ()),this , SLOT(beforeQuit () ));
	connect(action_Quit,SIGNAL( triggered ()),this , SLOT(beforeQuit () ));
	connect(action_Connect,SIGNAL( triggered ()),this , SLOT(Slot_connect()));
	trayicon->setContextMenu ( menu );
	connect(trayicon,SIGNAL(activated ( QSystemTrayIcon::ActivationReason ) ),this,SLOT(trayicon_activated(QSystemTrayIcon::ActivationReason)));
	restoreWindowSize();
	this->setCaption ( "QtSmbstatus client "+version_qtsmbstatus); // forms title
	// create statusBar
	statusBar()->showMessage (tr("Impossible to know samba version")); //status bar

	// read the address history
	readHistoryFile();

	comboBox_hostaddr->setCurrentText (host);

	//listview
	listView->addColumn ( "",550);

	socketclosed();

	// first time
	configuration_changed();

	// form view CIFS/SMB activities
	logform = new LogForm(this);
	connect(this ,SIGNAL(refreshviewlog(const type_message &)), logform,SLOT(append(const type_message &)));

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
	QSettings settings(QSettings::UserScope,"adella.org", "qtsmbstatus");
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
	QSettings settings(QSettings::UserScope,"adella.org", "qtsmbstatus");
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
		if (connected) clientsocket->Disconnect(); //if connected, disconnect
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
	QSettings settings(QSettings::UserScope,"adella.org", "qtsmbstatus");
	settings.beginGroup("/MainWindow");
		settings.beginGroup("/History");
			int index=0;
			while (index<comboBox_hostaddr->count ())
			{
				settings.setValue("recentAddress"+QString::number(index),comboBox_hostaddr->text(index) );
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
	QSettings settings(QSettings::UserScope,"adella.org", "qtsmbstatus");
	settings.beginGroup("/MainWindow");
		settings.beginGroup("/History");
			for ( int i = 0; i < 20 ; ++i )
			{
				value=settings.value( "recentAddress" +QString::number( i) ).toString();
				if (!value.isEmpty())
					comboBox_hostaddr->insertItem( value );
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
	comboBox_hostaddr->insertItem (texte,0);
	while (index<comboBox_hostaddr->count ())
	{
		if ((comboBox_hostaddr->text(index) == texte) || (index > 19) )
			comboBox_hostaddr->removeItem (index);
		else index++;
	}
	comboBox_hostaddr->setCurrentItem(0);
}


/**
	Slot connect. If connected, disconnect.
*/
void main_windows::Slot_connect()
{
	debugQt("main_windows::Slot_connect()");
	if (!connected) open_dialog_for_login(); // open authentication dialogbox
	else clientsocket->Disconnect();
}

/**
	Open authentication dialogbox
	\sa login_windows ClientSocket
*/
void main_windows::open_dialog_for_login()
{
	debugQt("main_windows::open_dialog_for_login()");
	login_windows * login_passwd = new login_windows(this);
	if (login_passwd->exec()==QDialog::Accepted)
	{
		pushButton_connect->setPixmap( QPixmap(":/icons/connect_creating.png") );
		connect_action->setIcon ( QPixmap(":/icons/connect_creating.png") );
		action_Connect->setIcon ( QPixmap(":/icons/connect_creating.png") );
		host=comboBox_hostaddr->currentText ();

		// new clientsocket
		clientsocket = new ClientSocket(listView,this);
		connect(clientsocket, SIGNAL( SignalConnected() ), this, SLOT(socketconnected() ) );
		connect(clientsocket, SIGNAL( SignalClosed() ), this, SLOT(socketclosed() ) );
		connect(clientsocket, SIGNAL( SignalErrorAuth()), this, SLOT(SignalErrorAuth()));
		connect(clientsocket, SIGNAL( SignalShortMessage(const QString &) ), statusBar(), SLOT(showMessage (const QString & )));
		QTimer *timer = new QTimer(clientsocket);
		connect(timer, SIGNAL(timeout()), this, SLOT(InfoSMB()));
		firstTime=true;
		timer->start(10000);
		// clear form log
		logform->on_clearButton_clicked();
		clientsocket->connectionToServer(username_login, passwd_login);
	}
}

/**
	Slot connected. When client is connected to server
	\sa Slot_connect ClientSocket
*/
void main_windows::socketconnected()
{
	debugQt("main_windows::socketconnected()");
	connected=true;
	// server address is valid, add to combobox
	comboBox_valid();

	pushButton_connect->setPixmap(QPixmap(":/icons/connect_established.png") );
	connect_action->setIcon (QPixmap(":/icons/connect_established.png") );
	action_Connect->setIcon ( QPixmap(":/icons/connect_established.png") );
	connect_action->setText(tr("Disconnect"));
	action_Connect->setText(tr("Disconnect"));
	searchAll->setEnabled ( true );
	searchMachineAction->setEnabled ( true );
	searchUserAction->setEnabled ( true );
	searchShare_openAction->setEnabled ( true );
	searchFile_openAction->setEnabled ( true );
}

/**
	Slot socketclosed. When client is disconnected
	\sa ClientSocket
*/
void main_windows::socketclosed()
{
	debugQt("main_windows::socketclosed()");

	pushButton_connect->setPixmap( QPixmap(":/icons/connect_no.png") );
	connect_action->setIcon(QPixmap(":/icons/connect_no.png") );
	action_Connect->setIcon(QPixmap(":/icons/connect_no.png") );
	connect_action->setText(tr("Connect"));
	action_Connect->setText(tr("Connect"));
	connected=false;
	searchAll->setEnabled ( false );
	searchMachineAction->setEnabled ( false );
	searchUserAction->setEnabled ( false );
	searchShare_openAction->setEnabled ( false );
	searchFile_openAction->setEnabled ( false );
	searchNextAction->setEnabled ( false );
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
}


/**
	Bad login or bad password.
	\sa ClientSocket::SignalErrorAuth
*/
void main_windows::SignalErrorAuth()
{
	debugQt("main_windows::SignalErrorAuth()");
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
		"<table>"
		"<tr><td>Polish translation :</td><td>Krzysztof Bzowski <a href='mailto:sh4dow4@gmail.com'>sh4dow4@gmail.com</a></td></tr>"
		"<tr><td></td><td>Pawel Dyda</td></tr>"
		"<tr><td>Spanish translation :</td><td>Mauro</td></tr>"
		"<tr><td>German translation :</td><td>Alex Djekic</td></tr>"
		"<tr><td>RPM packages for FC3 :</td><td>CG (cyberkoa)</td></tr>"
		"<tr><td>Second reading :</td><td>Eric Bresson <a href='mailto:eric.public@wanadoo.fr'>eric.public@wanadoo.fr</a></td></tr>"
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
	search(T_All,getSearchStr(tr("Find text")));
}


/**
	Find a machine

*/
void main_windows::MachineSearchSlot()
{
	debugQt("main_windows::MachineSearchSlot()");
	search(T_Machine,getSearchStr(tr("Search machine")));
}


/**
	Find an user
*/
void main_windows::UserSearchSlot()
{
	debugQt("main_windows::UserSearchSlot()");
	search(T_User,getSearchStr(tr("Search user")));
}

/**
	Find a share
*/
void main_windows::ShareSearchSlot()
{
	debugQt("main_windows::ShareSearchSlot()");
	search(T_Share,getSearchStr(tr("Search share")));
}

/**
	Find a file
*/
void main_windows::FileSearchSlot()
{
	debugQt("main_windows::FileSearchSlot()");
	search(T_File,getSearchStr(tr("Search locked file")));
}

/**
	Find next.
	\sa AllSearchSlot MachineSearchSlot UserSearchSlot ShareSearchSlot FileSearchSlot
*/
void main_windows::NextSlot()
{
	debugQt("main_windows::NextSlot()");
	search(currentSearchItem,currentSearchStr,to_next);
}


/**
	input dialogbox. Find a machine, an user, ...
	\param msg  label of dialogbox
	\return criteria of search
	\sa AllSearchSlot MachineSearchSlot UserSearchSlot ShareSearchSlot FileSearchSlot
*/
QString main_windows::getSearchStr(const QString & msg)
{
	bool ok;
	QString text = QInputDialog::getText(
	"Search", msg+":", QLineEdit::Normal,QString::null, &ok, this );
	if ( ok && !text.isEmpty() )
        	// user entered something and pressed OK
        	return text;
        return "";
}


/**
	Select item and open parents (if it's possible)
	\param item pointer on QListViewItem
	\sa nextItem search
*/
void main_windows::selectItem(Q3ListViewItem *item)
{
	debugQt("main_windows::selectItem()");
	Q3ListViewItem * parent;
	// open parent, grandparent, ...
	parent=item;
	while (parent->parent() )
	{
		parent=parent->parent();
		parent->setOpen ( true ); // open parent
	}
	listView->setSelected ( item, true ) ;
	listView->ensureVisible ( 0, item->itemPos () );
}

/**
	Find next item.
	\param item pointer on item
	\return pointer on next item
	\sa selectItem search
*/
Q3ListViewItem * main_windows::nextItem(Q3ListViewItem * item)
{
	debugQt("main_windows::nextItem()");
	Q3ListViewItem * tempNextItem;
	Q3ListViewItem * parent;
	Q3ListViewItem * tempItem=dynamic_cast<Q3ListViewItem *>(item);
	// if item is incorrect
	if (!tempItem)
		return listView->firstChild ();
	// first child of this item
	tempNextItem=tempItem->firstChild();
	// if child exist
	if (tempNextItem)
	{
		return tempNextItem;
	}
	else // no child
	{
		// next
		tempNextItem=tempItem->nextSibling();
		// if next exist
		if (tempNextItem)
		{
			return tempNextItem;
		}
		else // if next doesn't exist
		{
			parent=tempItem;
			// find next from parent
			while (parent->parent() )
			{
				parent=parent->parent();
				// find next
				tempNextItem=parent->nextSibling();
				// find oncle
				if (tempNextItem) return tempNextItem;
			}
			// nothing found
		}
	}
	// nothing found , return first child of listview
	return listView->firstChild ();
}

/**
	Search item.
	\param typeOfSearch see T_Item
	\param str string to search
	\param direction see T_direction
	\sa T_Item T_direction selectItem
*/
void main_windows::search(T_Item typeOfSearch,const QString & str,T_Direction direction)
{
	debugQt("main_windows::search()");
	if (str.isEmpty()) return;
	searchNextAction->setEnabled ( true );
	currentSearchStr=str;
	currentSearchItem=typeOfSearch;
	service * currentItemService;
	user * currentItemUser;
	machine * currentItemMachine;
	Q3PtrList<Q3ListViewItem> list;
	bool noText=false;

	if (direction==to_next)
	{
		// check if exist
		FindItem=listView->currentItem();
		if (FindItem)
		{
			FindItem=nextItem(FindItem); // find next
			if (!FindItem) return;
		}
		else direction=to_first; // go to begin
	}

	if (direction==to_first)
	{
		FindItem=dynamic_cast<server *>(listView->firstChild());
		if (!FindItem) return; // nothing
	}

	while (FindItem)
	{

		listView->setCurrentItem(FindItem);
		// find string char
		FindItem=listView->findItem ( str, 0,Q3ListView::Contains );

		if (!FindItem) noText=true;

		if (!list.containsRef (FindItem))
			list.append(FindItem);
		else noText=true;

		if (noText==true)
		{
			listView->clearSelection ();
			statusBar ()->showMessage  ( tr("Text not found."), 2000 );
			QApplication::beep ();
			searchNextAction->setEnabled ( false );
			return;
		}

		switch (typeOfSearch)
		{
			case T_All:
				selectItem(FindItem);
				return; break;
			case T_Machine:
				currentItemMachine= dynamic_cast<machine *>(FindItem);
				if (currentItemMachine)
				{
					selectItem(currentItemMachine);
					return;
				}
				break;
			case T_User:
				currentItemUser= dynamic_cast<user *>(FindItem);
				if (currentItemUser)
				{
					selectItem(currentItemUser);
					return;
				}
				break;
			case T_Share:
				currentItemService= dynamic_cast<service *>(FindItem);
				if (currentItemService)
				{
					// if search share
					if  (currentItemService->TypeOfService==service::Tshare)
					{
						selectItem(currentItemService);
						return;
					}
				}
				break;
			case T_File:
				currentItemService= dynamic_cast<service *>(FindItem);
				if (currentItemService)
				{
					// if search file
					if  (currentItemService->TypeOfService==service::Tlockedfile)
					{
						selectItem(currentItemService);
						return;
					}
				}
				break;
			default: // nothing
				return; break;
		}
		FindItem=nextItem(FindItem); // find next
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
	server * item =dynamic_cast<server *>(listView->firstChild ());
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


