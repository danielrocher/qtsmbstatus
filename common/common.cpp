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

// include SSL
#include<openssl/ssl.h>
#include <openssl/err.h>

//include Qt
#include <QString>
#include <QTextEncoder>
#include <QTextCodec>

/**
	\mainpage QtSmbstatus
 	\image html samba.png
	\section Introduction Introduction
	QtSmbstatus is a GUI (graphical %user interface) for %smbstatus.

	<b>QtSmbstatus</b> was designed as a <b>client/server</b> system secured with SSL. A login and password is required to log on to %server (PAM). <i>qtsmbstatus-server</i> (qtsmbstatusd) ought to be installed on host where SAMBA executes.

	To use QtSmbstatus only locally (without qtsmbstatus-server), see <b>qtsmbstatusl</b>. qtsmbstatusl only works locally (no remote access).

	<b><i>Commands:</i></b>
	- <b>qtsmbstatus</b> client of qtsmbstatus
	- <b>qtsmbstatusd</b> %server of qtsmbstatus
	- <b>qtsmbstatusl</b> qtsmbstatus light

	\endverbatim
	This program is licensed to you under the terms of the GNU General Public License version 2
*/


// variables environnements
QString version_qtsmbstatus="2.1";
uint int_qtsmbstatus_version=210;
QString date_qtsmbstatus="2008-11-18";
QString auteur_qtsmbstatus="Daniel Rocher";
QString mail_qtsmbstatus="<a href='mailto:daniel.rocher@adella.org'>daniel.rocher@adella.org</a>";
QString web_qtsmbstatus="<a href='http://qtsmbstatus.free.fr'>http://qtsmbstatus.free.fr</a>";

//default value
//! view debug messages
bool debug_qtsmbstatus=false;
//! TCP port
quint16  port_server=4443;

/**
	detect text encodings for locale
	\sa writeToConsole debugQt
*/
QTextEncoder * encoder=QTextCodec::codecForLocale()->makeEncoder();

/**
	Write to console. The message is printed to stderr.
*/
void writeToConsole (const QString & message) { qDebug("%s",encoder->fromUnicode ( message ).data()); }


/**
	view debug messages. The message is printed to stderr only if debug is enabled.
*/
void debugQt(const QString & message) {if (debug_qtsmbstatus) writeToConsole(message); }


//! command line error
void unsupported_options(char *error, const QString & usage) 
{
	writeToConsole(QString("\n      Unsupported option : %1 \n%2").arg(error).arg(usage));
}

/**
	Convert string "true" and "false" to boolean
	\sa BoolToStr
*/
bool StrToBool(QString & value)
{
	if (value.toLower()=="true") return true;
	return false;
}

/**
	Convert bool to string
	\sa StrToBool
*/
QString BoolToStr(bool & value)
{
	if (value) return "true";
	return "false";
}


/**
	Test if TCP port is valid
	\param port port number
	\retval true TCP port is valid
	\retval false TCP port is not valid
*/
bool validatePort(const int & port)
{
	if ((port < 1024) || (port > 49151))
	{
		writeToConsole ( "available port : > 1023 and < 49152");
		return false;
	}
	return true;
}

