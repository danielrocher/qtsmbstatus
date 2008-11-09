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


#include <QRegExp>
#include <QtCore>
#include "core_syntax.h"

/**
	\class core_syntax
	\brief This class manage the dialog between client and server
	\date 2008-11-06
	\version 1.2
	\author Daniel Rocher

		Client and server use a protocol :

	\verbatim
	command  client server   definition
	- auth_rq       ->         Authentication request (login+password)
	- auth_ack      <-         Authentication OK
	- end           <->        Stop connection
	- kill_user     ->         Disconnect an user
	- send_msg      ->         Send a popup windows
	- smb_rq        ->         Sambstatus request
	- smb_data      <-         smbstatus reply
	- end_smb_rq    <-         smbstatus command finish
	- not_imp1                 Not implemented	
	- server_info   <-         Server Information (right for current client : permit disconnect an user,...)
	- error_auth    <-         Authentication error (bad password,...)
	- error_command <-         Command error (server informs client : the last command is not recognized)
	- error_obj     <-         Server wants to open a dialogbox to visualize an error
	- echo_request  <->        Test if connection is correct (is not dead)
	- echo_reply    <->        connection is correct
	\endverbatim

	<b>syntax:</b>
	\verbatim
	[command]field1;field2
	\endverbatim

	- command is an integer
	- field1 and field2 are optional

	<b>exemple:</b>
	\verbatim
	[auth_rq]login;password
	[end]
	[smb_rq]
	\endverbatim
*/




core_syntax::core_syntax(const QString & mte)
{
	myTxt=mte;
}

core_syntax::~core_syntax()
{

}

/**
	\param mte line to be analyzed
*/
void core_syntax::setValue(const QString & mte)
{
	myTxt=mte;
}

/**
	Return value of an argument
	\param index index of argument
	\return value of argument <i>index</i>
*/
QString core_syntax::returnArg(int index)
{
	int debut;
	int fin;
	int i=0;

	if (index==0)
	{
		QRegExp rx( "^\\[\\d\\d?\\]" );
		debut=rx.indexIn(myTxt);
		if (debut != -1)
		{
			fin = myTxt.indexOf("]");
			return (myTxt.mid(debut+1 , fin-debut -1 ));
		}
	}

	//same length without escape key
	QString withoutEscapeKeyTxt=myTxt;
	withoutEscapeKeyTxt.replace("\\\\","AA").replace("\\;","AA");

	debut = myTxt.indexOf("]");
	if (debut==-1) return ("");
	for (i=1 ; i<=index ; ++i)
	{
		debut+=1;

		fin=withoutEscapeKeyTxt.indexOf(";",debut);
		if (index==i)
		{
			if (fin!=-1) return (removeEscapeKeys(myTxt.mid(debut , fin-debut )));
			else return (removeEscapeKeys(myTxt.mid(debut )));
		}
		if (fin==-1)
		{
			return (""); // nothing to do
		}
		debut=fin;
	}
	return ("");
}


/**
	return string with escape keys
	\sa core_syntax removeEscapeKeys
*/
QString addEscapeKeys(QString txt)
{
	return txt.replace("\\","\\\\").replace(";","\\;");
}

/**
	return string without escape keys
	\sa core_syntax addEscapeKeys
*/
QString removeEscapeKeys(QString txt)
{
	return txt.replace("\\\\","\\").replace("\\;",";");
}

