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

#include "disconnect_manager.h"

int disconnect_manager::compteur_objet=0;


/**
	\class disconnect_manager
	\brief Kill a process
	\date 2007-06-18
	\version 1.0
	\author Daniel Rocher
	\param MyPID PID of process
	\param USER username
	\param parent pointer to parent for this object
	\sa process_smbd_exist
 */
disconnect_manager::disconnect_manager(const QString & MyPID,const QString & USER,QObject *parent ) : QObject(parent)
{
	debugQt("Object disconnect_manager : "+QString::number(++compteur_objet));
	MyPid=MyPID.stripWhiteSpace();
	user=USER.stripWhiteSpace();
	QString kill_argument;
	error_proc=false;

	proc = new Q3Process( this );
	proc->addArgument( "sh");   // run a shell
	kill_argument="kill -15 " + MyPid;
	debugQt ("Disconnect "+user+ " - "+kill_argument);

	connect( proc, SIGNAL(processExited()),
	this, SLOT(end_process()) );

	connect( proc, SIGNAL(readyReadStderr ()),
	this, SLOT(ReadStderr()) );

	State=begin;
	if ( !proc->launch (kill_argument) )
	{
		// error handling
		qWarning("process kill error");
		error_proc=true;
		deleteLater();
	}
}

disconnect_manager::~disconnect_manager()
{
	debugQt("Object disconnect_manager : "+QString::number(--compteur_objet));
	if (error_proc) emit ObjError(tr("process kill error"));
}

/**
	Process ending
*/
void disconnect_manager::end_process()
{
	debugQt("disconnect_manager::end_process()");
	if (State!=error) deleteLater (); // delete this object
}

/**
	Read errors
*/
void disconnect_manager::ReadStderr()
{

	debugQt("disconnect_manager::ReadStderr()");
	QString ligne;

	while ( proc->canReadLineStderr () )
	{
		ligne=proc->readLineStderr ();
		debugQt(ligne);
		State = error;

		emit ObjError(tr("Failed to disconnect user")+" "+ user + " : "  + ligne );
		deleteLater ();
		return;
	}
}
