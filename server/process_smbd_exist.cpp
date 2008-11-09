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

#include "process_smbd_exist.h"

int process_smbd_exist::compteur_objet=0;

/**
	\class process_smbd_exist
	\brief test if PID it's a correct smbd process.
	\date 2008-11-04
	\version 1.1
	\author Daniel Rocher
	\param MyPID PID of process
	\param USER username
	\param parent pointer to parent for this object
	\sa disconnect_manager
 */
process_smbd_exist::process_smbd_exist(const QString & MyPID,const QString & USER,QObject *parent)
 : QObject(parent)
{
	debugQt("Object process_smbd_exist : "+QString::number(++compteur_objet));
	State=begin;

	MyPid=MyPID.simplified();
	UserSamba=USER.simplified();

	connect( &proc, SIGNAL(finished ( int, QProcess::ExitStatus)),this, SLOT(end_process()) );
	connect( &proc, SIGNAL(readyReadStandardOutput ()),this, SLOT(readFromStdout()) );
	connect( &proc, SIGNAL(error ( QProcess::ProcessError) ),this, SLOT(error(QProcess::ProcessError)) );

	QStringList arguments;
	arguments  << "-c" << "ps -f "+MyPid;
	debugQt ("check process : sh " + arguments.join(" "));

	proc.start("sh",arguments,QIODevice::ReadOnly);
}

process_smbd_exist::~process_smbd_exist()
{
	debugQt("Object process_smbd_exist : "+QString::number(--compteur_objet));
}


/**
	an error occurs with the process
*/
void process_smbd_exist::error(QProcess::ProcessError err) {
	debugQt("process_smbd_exist::error()");
	// error handling
	qWarning("process sh error");

	switch (err) {
		case 0: debugQt("  ==> FailedToStart");
			break;
		case 1: debugQt("  ==> Crashed");
			break;
		case 2: debugQt("  ==> Timedout");
			break;
		case 3: debugQt("  ==> ReadError");
			break;
		case 4: debugQt("  ==> WriteError");
			break;
		case 5: debugQt("  ==> UnknownError");
			break;
	}
	emit ObjError("Process sh error. Contact your administrator.");
	deleteLater ();
}


/**
	Read Std output
*/
void process_smbd_exist::readFromStdout(){
	debugQt("process_smbd_exist::readFromStdout()");
	QString str(proc.readAllStandardOutput ());

	debugQt(str);
	if (str.contains (MyPid,Qt::CaseInsensitive) && str.contains ("smbd",Qt::CaseInsensitive))
	{
		// It's a correct process
		State=find;
		debugQt("process is smbd");
		// disconnect user (kill process)
		disconnect_manager * kill_process = new disconnect_manager(MyPid,UserSamba,this);
		connect (kill_process,SIGNAL(ObjError(QString)), this,SIGNAL(ObjError(QString)));
		connect( kill_process, SIGNAL(destroyed()),this, SLOT(slot_EndProcessKill()) );
	}
}

/**
	End process.
*/
void process_smbd_exist::end_process(){
	debugQt("process_smbd_exist::end_process()");
	if (State!=find)
	{
		// It isn't process smbd
		debugQt("process is not smbd  !  Pid: "+MyPid +  " User: "+ UserSamba);
		emit ObjError(tr("Failed to disconnect user")+" "+ UserSamba);
		deleteLater ();
	}
}


/**
	disconnect_manager terminated
*/
void process_smbd_exist::slot_EndProcessKill()
{
	debugQt("process_smbd_exist::slot_EndProcessKill()");
	deleteLater();
}
