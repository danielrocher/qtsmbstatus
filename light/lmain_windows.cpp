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

extern QList<QTreeWidgetItem *> QTreeWidgetItemList;

/**
	\class main_windowsl
	\brief Main Window for QtSmbstatus Light
	\date 2008-11-16
	\version 2.1
	\author Daniel Rocher
*/
main_windowsl::main_windowsl(QWidget *parent) : MainWindows(parent)
{
	debugQt("main_windowsl::main_windowsl()");
	this->setWindowTitle ( "QtSmbstatus light "+version_qtsmbstatus); // forms title
	qApp->setWindowIcon ( QIcon (":/icons/qtsmbstatusl.png") );
	trayicon->setToolTip ( windowTitle ());
	trayicon->setIcon (qApp->windowIcon());
	helpAboutAction->setIcon(qApp->windowIcon());
	
	currentIndexOfListItem=0;
	QString SearchTxt="";
	
	//delete unused widgets
	action_Connect->deleteLater();
	textLabel1->deleteLater();
	comboBox_hostaddr->deleteLater();
	pushButton_connect->deleteLater();
	horizontalLayout->deleteLater();
	
	permitDisconnectUser=true;
	permitSendMsg=true;
	
	searchNextAction->setEnabled ( false );
	firstTime=true;
	timerinfoSmb.start(10000);

	// start smbstatus request
	slot_timer(); // first time
	timerSmbRequest.start (interval*1000);

	// create icon server on treeWidget
	item_server = new server( treeWidget );

}

main_windowsl::~main_windowsl()
{
	debugQt("main_windowsl::~main_windowsl()");
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
		treeWidget->clear();
		timerinfoSmb.stop();
		timerSmbRequest.stop();
		saveWindowSize();
		logform->close();
		logform->deleteLater();
		QTimer::singleShot(800, qApp, SLOT(quit())); // wait before quit
		e->accept();
	}
}


/**
	Slot %smbstatus timer
	Request %smbstatus to %server
*/
void main_windowsl::slot_timer()
{
	debugQt("main_windowsl::slot_timer()");
	if (! smbstatus_rq_started) {
		smbstatus_rq_started=true;
		smbmanager* processus_smbstatus = new  smbmanager(this);
		connect(processus_smbstatus,SIGNAL(ObjError(const QString &)),msgError, SLOT(showMessage (const QString &)) );
		connect( processus_smbstatus, SIGNAL(signal_std_output(const QStringList &)),this, SLOT(AnalysisSmbstatus(const QStringList &)) );
	}
}


/**
	send message to an %user
	\sa MainWindows::slotSendMessage MainWindows::slotSendMessageAllUsers
*/
void main_windowsl::sendMessage(const QString & machine,const QString & message) {
	debugQt("main_windowsl::sendMessage()");
	Sendmessage_manager* send_msg = new Sendmessage_manager(machine,message,this);
	connect(send_msg,SIGNAL(ObjError(const QString &)),msgError, SLOT(showMessage (const QString &)) );
}

/**
	disconnect an %user
	\sa MainWindows::slotDisconnectUser
*/
void main_windowsl::disconnectUser(const QString & pid,const QString & user) {
	debugQt("main_windowsl::disconnectUser()");
	process_smbd_exist* killSmbProc = new process_smbd_exist(pid,user,this);
	connect(killSmbProc,SIGNAL(ObjError(const QString &)),msgError, SLOT(showMessage (const QString &)) );
}

/**
	Configure qtsmbstatus light (open form)
	\sa configure_windowsl
 */
void main_windowsl::on_configureAction_triggered()
{
	debugQt("main_windowsl::on_configureAction_triggered()");
	configure_windowsl * configurate = new configure_windowsl(this);
	connect (configurate,SIGNAL(configuration_changed()),this, SLOT(configuration_changed()));
	configurate->exec();
}



