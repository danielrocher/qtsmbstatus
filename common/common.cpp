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

// include SSL
#include<openssl/ssl.h>
#include <openssl/err.h>

//include Qt
#include <QString>
#include <Q3SocketDevice>

/**
	\mainpage QtSmbstatus
	\section Introduction Introduction
	QtSmbstatus is a GUI (graphical user interface) for smbstatus.

	QtSmbstatus was designed as a client/server system secured with SSL. A login and password is required to log on to server (PAM). Qtsmbstatusd ought to be installed on host where SAMBA executes.

	This program is licensed to you under the terms of the GNU General Public License version 2
*/


// variables environnements
QString version_qtsmbstatus="2.0.2";
uint int_qtsmbstatus_version=202;
QString date_qtsmbstatus="2007-11-07";
QString auteur_qtsmbstatus="Daniel Rocher";
QString mail_qtsmbstatus="<a href='mailto:daniel.rocher@adella.org'>daniel.rocher@adella.org</a>";
QString web_qtsmbstatus="<a href='http://qtsmbstatus.free.fr'>http://qtsmbstatus.free.fr</a>";

//default value
//! view debug messages
bool debug_qtsmbstatus=false;
//! TCP port
Q_UINT16  port_server=4443;

/**
	view debug messages. The message is printed to stderr.
*/
void debugQt(const QString & message) {if (debug_qtsmbstatus) qDebug ("%s",message.latin1()); }

//! command line error
void unsupported_options(char *erreur, const QString & usage) 
{
	qDebug("\n      Unsupported option : %s \n"+ usage , erreur);
}

/**
	Convert string "true" and "false" to boolean
	\sa BoolToStr
*/
bool StrToBool(QString & value)
{
	if (value.lower()=="true") return true;
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
		qDebug ( "available port : > 1023 and < 49152");
		return false;
	}
	return true;
}

/**
	Print to stderr, QSocketDevice error
	\param errorcode code error
	\sa SSL_print_error
*/
void Socket_print_error (int errorcode) 
{ 
	switch (errorcode) {
	case 0:
		return;
		break;
	case Q3SocketDevice::AlreadyBound:
		qDebug  ("The device is already bound, according to bind().") ;
		break;
	case Q3SocketDevice::Inaccessible:
		qDebug  ( "The operating system or firewall prohibited the action.") ;
		break;
	case Q3SocketDevice::NoResources:
		qDebug  ( " The operating system ran out of a resource. ") ;
		break;
	case Q3SocketDevice::InternalError:
		qDebug  ( "An internal error occurred in QSocketDevice.") ;
		break;
	case Q3SocketDevice::Impossible:
		qDebug  ( "An attempt was made to do something which makes no sense") ;
		break;
	case Q3SocketDevice::NoFiles:
		qDebug  ( "The operating system will not let QSocketDevice open another file.") ;
		break;
	case Q3SocketDevice::ConnectionRefused:
		qDebug  ( " A connection attempt was rejected by the peer.") ;
		break;
	case Q3SocketDevice::NetworkFailure:
		qDebug  ( " There is a network failure. " );
		break;
	case Q3SocketDevice::UnknownError:
		qDebug  ( " The operating system did something unexpected. ") ;
		break;
	default:
		qDebug  ( "Socket error");
	}
}

/**
	Print to stderr, TLS/SSL error
	\param errorcode code error
	\sa Socket_print_error
*/
void SSL_print_error(int errorcode) 
{
	switch (errorcode) {
	case SSL_ERROR_NONE:
		qDebug  (  "SSL Operation successful");
		break;
	case SSL_ERROR_ZERO_RETURN:
		qDebug  (  "SSL socket shutdown cleanly") ;
		break;
	case SSL_ERROR_WANT_READ:
		qDebug  ( "SSL want read") ;
		break;
	case SSL_ERROR_WANT_WRITE:
		qDebug  ( "SSL want write") ;
		break;
	case SSL_ERROR_WANT_CONNECT:
		qDebug  ( "SSL want connect") ;
		break;
	case SSL_ERROR_WANT_X509_LOOKUP:
		qDebug  ( "SSL want x509 lookup" );
		break;
	case SSL_ERROR_SYSCALL:
		qDebug  ( "SSL io error") ;
		break;
	case SSL_ERROR_SSL:
		qDebug  ( "SSL protocol error" );
		break;
	default:
		qDebug  ( "SSL Unknown error");
	}
}

