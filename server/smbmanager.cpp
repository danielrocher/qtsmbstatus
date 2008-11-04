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

/**
	\class smbmanager
	\brief start smbstatus process
	\date 2008-11-03
	\version 1.0
	\author Daniel Rocher
	\param parent pointer to parent for this object
 */
smbmanager::smbmanager(QObject *parent ) : QObject(parent)
{
	debugQt("Object smbmanager : "+ QString::number(++compteur_objet));
	requestFailed=false;

	connect( &proc, SIGNAL(finished ( int, QProcess::ExitStatus) ),this, SLOT(end_process()) );
	connect( &proc, SIGNAL(readyReadStandardOutput ()),this, SLOT(read_data()) );
	connect( &proc, SIGNAL(readyReadStandardError ()),this, SLOT(ReadStderr()) );
	connect( &proc, SIGNAL(error ( QProcess::ProcessError) ),this, SLOT(error(QProcess::ProcessError)) );

	proc.start("smbstatus",QIODevice::ReadOnly);
}

smbmanager::~smbmanager()
{
	debugQt("Object smbmanager : "+ QString::number(--compteur_objet));
}


/**
	an error occurs with the process
*/
void smbmanager::error(QProcess::ProcessError err) {
	debugQt("smbmanager::error()");
	// error handling
	qWarning("process smbstatus error");

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
	emit ObjError(tr("process smbstatus error"));
	requestFailed=true;
	deleteLater ();
}


/**
	Read Std error
*/
void smbmanager::ReadStderr()
{
	debugQt("smbmanager::ReadStderr()");
	QString str(proc.readAllStandardError());
	debugQt(str);

	emit ObjError(tr("Smbstatus request error")+" : "+ str );
	requestFailed=true;
}


/**
	Read Std output
*/
void smbmanager::read_data ()
{
	debugQt("smbmanager::read_data ()");
	data.append(proc.readAllStandardOutput ());
}

/**
	Process terminated
*/
void smbmanager::end_process ()
{
	debugQt("smbmanager::end_process ()");
	QStringList list=QString(data).split('\n');

	if (!requestFailed) emit signal_std_output(list);
	deleteLater();
}
