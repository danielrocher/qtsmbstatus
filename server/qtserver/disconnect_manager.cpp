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

#include "disconnect_manager.h"

int disconnect_manager::compteur_objet=0;


/**
	\class disconnect_manager
	\brief Kill a smb process (disconnect an %user)
	\date 2008-11-10
	\version 1.2
	\author Daniel Rocher
	\param MyPID PID of process
	\param USER username
	\param parent pointer to parent for this object
	\sa process_smbd_exist
 */
disconnect_manager::disconnect_manager(const QString & MyPID,const QString & USER,QObject *parent ) : QObject(parent)
{
	debugQt("Object disconnect_manager : "+QString::number(++compteur_objet));

	m_textDecoder = QTextCodec::codecForLocale()->makeDecoder();
	QRegExp regexp("[\"\'\n\r;|&#><`\\\\]");
	MyPid=MyPID.simplified().replace(regexp, " ");
	user=USER.simplified().replace(regexp, " ");

	connect( &proc, SIGNAL(finished ( int, QProcess::ExitStatus) ),this, SLOT(end_process()) );
	connect( &proc, SIGNAL(readyReadStandardError ()),this, SLOT(ReadStderr()) );
	connect( &proc, SIGNAL(error ( QProcess::ProcessError) ),this, SLOT(error(QProcess::ProcessError)) );

	QStringList arguments;
	arguments << "-c" << "kill -15 "+MyPid;

	debugQt ("Disconnect "+user+ " - sh "+arguments.join(" "));

	proc.start("sh",arguments,QIODevice::ReadOnly);
}

disconnect_manager::~disconnect_manager()
{
	debugQt("Object disconnect_manager : "+QString::number(--compteur_objet));
	delete m_textDecoder;
}


/**
	an error occurs with the process
*/
void disconnect_manager::error(QProcess::ProcessError err) {
	debugQt("disconnect_manager::error()");
	// error handling
	qWarning("process kill error");

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
	emit ObjError(tr("process kill error"));
	deleteLater ();
}

/**
	Process ending
*/
void disconnect_manager::end_process()
{
	debugQt("disconnect_manager::end_process()");
	deleteLater (); // delete this object
}

/**
	Read errors
*/
void disconnect_manager::ReadStderr()
{

	debugQt("disconnect_manager::ReadStderr()");

	QString str=m_textDecoder->toUnicode(proc.readAllStandardError());
	debugQt(str);

	emit ObjError(tr("Failed to disconnect user")+" "+ user + " : "  + str );
}
