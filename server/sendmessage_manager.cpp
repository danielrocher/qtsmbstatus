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

#include "sendmessage_manager.h"

/**
	\class Sendmessage_manager
	\brief Send message popup (smbclient required)
	\date 2007-06-18
	\version 1.0
	\author Daniel Rocher
	\param machine Machine name
	\param message message to be sent
	\param parent pointer to parent for this object
 */

int Sendmessage_manager::compteur_objet=0;

Sendmessage_manager::Sendmessage_manager(const QString & machine, const QString & message,QObject *parent) : QObject(parent) {
	debugQt("Object Sendmessage_manager : "+ QString::number(++compteur_objet));
	error_proc=false;

	to_machine=machine.stripWhiteSpace();
	my_message=message.stripWhiteSpace();

	my_message=my_message.replace( QChar('"'), " " );

	my_message="echo \"" + my_message +  "\" | smbclient -M " + to_machine;

	proc = new Q3Process( this );
	proc->addArgument( "sh");   // run a shell
	debugQt ("Send message at "+machine+ " - " + my_message);

	connect( proc, SIGNAL(processExited()),
	this, SLOT(end_process()) );

	connect( proc, SIGNAL(readyReadStdout ()),
	this, SLOT(readFromStdout()) );

	connect( proc, SIGNAL(readyReadStderr ()),
	this, SLOT(ReadStderr()) );

	State=begin;
	if ( !proc->launch (my_message) ) {
		// error handling
		qWarning("process smbclient error");
		error_proc=true;
		deleteLater();
	}
}

Sendmessage_manager::~Sendmessage_manager(){
	debugQt("Object Sendmessage_manager : "+ QString::number(--compteur_objet));
	if (error_proc) emit ObjError(tr("process smbclient error"));
}

/**
	Read Std error
*/
void Sendmessage_manager::ReadStderr(){
	debugQt("Sendmessage_manager::ReadStderr()");
	QString ligne;

	while ( proc->canReadLineStderr () )
	{
		ligne=proc->readLineStderr (); // Read one line
		debugQt(ligne);

		emit ObjError(tr("Failed to send message")+" : "+ ligne );
		return;
	}
}


/**
	Read Std output
*/
void Sendmessage_manager::readFromStdout(){

	debugQt("Sendmessage_manager::readFromStdout()");
	QString ligne;

	while ( proc->canReadLineStdout () )  
	{
		ligne=proc->readLineStdout (); // Read one line

		if (ligne.contains ("Cannot resolve",false) or ligne.contains (" failed",false) or ligne.contains ("ERRSRV",false))
		{
			State = error;
			emit ObjError(tr("Could not send message to") + " " + to_machine);
			deleteLater ();
			return;
		}
	}
}


/**
	Process terminated
*/
void Sendmessage_manager::end_process(){
	debugQt("Sendmessage_manager::end_process()");
	if (State!=error) deleteLater ();
}
