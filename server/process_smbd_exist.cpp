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

#include "process_smbd_exist.h"

int process_smbd_exist::compteur_objet=0;

/**
	\class process_smbd_exist
	\brief test if PID it's a correct smbd process.
	\date 2007-06-18
	\version 1.0
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

	MyPid=MyPID.stripWhiteSpace();
	UserSamba=USER.stripWhiteSpace();
	QString argument;

	proc = new Q3Process( this );
	proc->addArgument( "sh");   // run a shell
	argument="ps -f " + MyPid;
	debugQt ("verifie coherence process :" + argument);

	connect( proc, SIGNAL(processExited()),
	this, SLOT(end_process()) );

	connect( proc, SIGNAL(readyReadStdout ()),
	this, SLOT(readFromStdout()) );

	State=begin;
	if ( !proc->launch (argument) ) {
		// error handling
		qWarning("process 'ps -f pid' error");
		deleteLater();
	}
}

process_smbd_exist::~process_smbd_exist()
{
	debugQt("Object process_smbd_exist : "+QString::number(--compteur_objet));
}


/**
	Read Std output
*/
void process_smbd_exist::readFromStdout(){
	debugQt("process_smbd_exist::readFromStdout()");
	QString ligne;

	while ( proc->canReadLineStdout ())
	{
		ligne=proc->readLineStdout (); // Read one line
		if (ligne.contains (MyPid,false) && ligne.contains ("smbd",false))
		{  
			// It's a correct process
			State = find;
			debugQt("process is smbd  : "+ ligne);
			// disconnect user (kill process)
			disconnect_manager * kill_process = new disconnect_manager(MyPid,UserSamba,this);

			connect( kill_process, SIGNAL(destroyed()),this, SLOT(slot_EndProcessKill()) );
			return;
		}
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
		emit ObjError("Failed to disconnect user "+ UserSamba);
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
