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

#include "sendmessage_manager.h"

/**
	\class Sendmessage_manager
	\brief Send message popup (smbclient required)
	\date 2008-11-10
	\version 1.2
	\author Daniel Rocher
	\param machine Machine name
	\param message message to be sent
	\param parent pointer to parent for this object
 */

int Sendmessage_manager::compteur_objet=0;

Sendmessage_manager::Sendmessage_manager(const QString & machine, const QString & message,QObject *parent) : QObject(parent) {
	debugQt("Object Sendmessage_manager : "+ QString::number(++compteur_objet));

	m_textDecoder = QTextCodec::codecForLocale()->makeDecoder();
	
	to_machine=machine.simplified();
	my_message=message.simplified();

	my_message.replace( "\"", " ").replace( "\\", " ");
	my_message="echo " + my_message +  " | smbclient -M " + to_machine + " -N";

	connect( &proc, SIGNAL(finished ( int, QProcess::ExitStatus)),this, SLOT(end_process()) );
	connect( &proc, SIGNAL(readyReadStandardOutput ()),this, SLOT(readFromStdout()) );
	connect( &proc, SIGNAL(readyReadStandardError ()),this, SLOT(ReadStderr()) );
	connect( &proc, SIGNAL(error ( QProcess::ProcessError) ),this, SLOT(error(QProcess::ProcessError)) );

	QByteArray str;
	str.append ( "sh -c \"" + my_message + "\"");
	debugQt ("Send message to "+machine+ " - "+ str);
	proc.start(str,QIODevice::ReadOnly);
}

Sendmessage_manager::~Sendmessage_manager(){
	debugQt("Object Sendmessage_manager : "+ QString::number(--compteur_objet));
	delete m_textDecoder;
}

/**
	an error occurs with the process
*/
void Sendmessage_manager::error(QProcess::ProcessError err) {
	debugQt("Sendmessage_manager::error()");
	// error handling
	qWarning("process smbclient error");

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
	emit ObjError(tr("process smbclient error"));
	deleteLater ();
}

/**
	Read Std error
*/
void Sendmessage_manager::ReadStderr(){
	debugQt("Sendmessage_manager::ReadStderr()");
	QString str=m_textDecoder->toUnicode(proc.readAllStandardError());
	debugQt(str);

	emit ObjError(tr("Failed to send message")+" : "+ str );
}


/**
	Read Std output
*/
void Sendmessage_manager::readFromStdout(){

	debugQt("Sendmessage_manager::readFromStdout()");
	QString str(m_textDecoder->toUnicode(proc.readAllStandardOutput()));
	debugQt(str);
	
	if (str.contains ("Cannot resolve",Qt::CaseInsensitive) or str.contains (" failed",Qt::CaseInsensitive) or str.contains ("ERRSRV",Qt::CaseInsensitive))
		emit ObjError(tr("Could not send message to") + " " + to_machine);
}


/**
	Process terminated
*/
void Sendmessage_manager::end_process(){
	debugQt("Sendmessage_manager::end_process()");
	deleteLater ();
}
