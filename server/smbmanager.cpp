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


#include "smbmanager.h"

int smbmanager::compteur_objet=0;

smbmanager::smbmanager(QObject *parent ) : QObject(parent)
{
	debugQt("Object smbmanager : "+ QString::number(++compteur_objet));
	error_proc=false; 
	stError=false; 

	proc = new Q3Process( this );
	proc->addArgument( "smbstatus" );

	connect( proc, SIGNAL(processExited()),
	this, SLOT(end_process()) );

	connect( proc, SIGNAL(readyReadStdout ()),
	this, SLOT(read_data()) );

	connect( proc, SIGNAL(readyReadStderr ()),
	this, SLOT(ReadStderr()) );

	if ( !proc->start() ) {
		// error handling
		qWarning("process smbstatus error");
		error_proc=true;
		deleteLater ();
	}
}

smbmanager::~smbmanager()
{
	debugQt("Object smbmanager : "+ QString::number(--compteur_objet));
	if (error_proc) emit ObjError(tr("process smbstatus error"));
}

/**
	Read Std error
*/
void smbmanager::ReadStderr()
{
	debugQt("smbmanager::ReadStderr()");
	QString ligne;

	while ( proc->canReadLineStderr () ) 
	{
		ligne=proc->readLineStderr (); // read one line
		debugQt(ligne);

		emit ObjError(tr("Smbstatus request error")+" : "+ ligne );
		stError=true;
		return;
	}
}


/**
	Read Std output
*/
void smbmanager::read_data ()
{
	debugQt("smbmanager::read_data ()");
	while (proc->canReadLineStdout ())
	{
		data << (proc->readLineStdout ());
	}
}

/**
	Process terminated
*/
void smbmanager::end_process ()
{
	debugQt("smbmanager::end_process ()");
	if ((!stError) && (!error_proc)) emit signal_std_output(data);
	deleteLater();
}
