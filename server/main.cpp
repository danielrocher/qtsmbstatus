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

#include <signal.h>

#include <QApplication>
#include <QTimer>
#include <QFile>
#include <QTextStream>

#include "server.h"

extern void unsupported_options(char *erreur, const QString & usage);
extern bool validatePort(const int & port);

// default values of configs
QString Certificat = "/etc/qtsmbstatusd/server.pem";
QString Private_key = "/etc/qtsmbstatusd/privkey.pem";
QString ssl_password = "password";
bool daemonize=false;

//! Allow client to disconnect user
QStringList AllowUserDisconnect;

//! Allow client to send message
QStringList AllowUserSendMsg;

Server* myserver;

/**
	Receive SIGTERM, SIGINT or SIGQUIT signal
*/
void signal_handler(int) {
	qDebug ("Stop QtSmbstatusd , please wait ...");
	QTimer::singleShot( 0, myserver, SLOT (deleteLater()));
}

/**
	Add user to a list
	\param variable username
	\sa AllowUserDisconnect
*/
void GetUserDisconnect(QString variable)
{
	QString allow_user;
	int fin=0;
	AllowUserDisconnect.clear();
	while (variable.length() > 0)
	{
		fin=variable.find(",");
		if (fin==-1) fin=variable.length();
		allow_user=(variable.mid(0,fin)).simplifyWhiteSpace();

		if (allow_user!="") AllowUserDisconnect.append(allow_user);
		variable=variable.mid(fin+1);
	}
}

/**
	Add user to a list
	\param variable username
	\sa AllowUserSendMsg
*/
void GetUserSend(QString variable)
{
	QString allow_user;
	int fin=0;
	AllowUserSendMsg.clear();
	while (variable.length() > 0)
	{
		fin=variable.find(",");
		if (fin==-1) fin=variable.length();
		allow_user=(variable.mid(0,fin)).simplifyWhiteSpace();

		if (allow_user!="") AllowUserSendMsg.append(allow_user);
		variable=variable.mid(fin+1);
	}
}

/**
	Read configuration file
*/
void readConfigFile()
{
	QString ligne;
	QString attr;
	QString variable;
	bool ok;
	int port;

	QFile f1( "/etc/qtsmbstatusd/qtsmbstatusd.conf" );
	if ( !f1.open( QIODevice::ReadOnly ) )
	{
		qDebug("Impossible to read configuration file. Use default settings.");
		return;
	}
	QTextStream t( &f1);
	while ( !t.atEnd() )
	{
		ligne =  t.readLine();
		ligne = ligne.simplifyWhiteSpace();
		if ((ligne.mid(0,1) == "#") || (ligne.length ()==0)) continue;
		if (ligne.contains("="))
		{
			attr= (ligne.mid(0,ligne.find("="))).simplifyWhiteSpace();
			variable = (ligne.mid(ligne.find("=")+1)).simplifyWhiteSpace();
			if (attr=="port")
			{
				port=(variable).toInt(&ok);
				// if port is valid
				if ((ok==true) && (validatePort(port))) port_server=port;
			}
			if ((attr=="private_key") && (variable.length()!=0)) Private_key=variable;
			if ((attr=="certificat") && (variable.length()!=0)) Certificat=variable;
			if ((attr=="ssl_password") && (variable.length()!=0)) ssl_password=variable;
			if (attr=="permit_disconnect_user") GetUserDisconnect(variable);
			if (attr=="permit_send_msg") GetUserSend(variable);
		}
	}
   f1.close();
}


int main( int argc,char *argv[] )
{
	// intercept the signals (kill)
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);
	signal(SIGQUIT, signal_handler);

	bool ok;

	// help message (option --help)
	QString usage="\n    Usage:  qtsmbstatusd -d -m -p <port> -v --help\n\n"
		"    --help :       Show this help\n"
		"    -d :           Daemonize\n"
		"    -p <port> :    TCP port - default = " + QString::number(port_server) +"\n"
		"    -v :           Show qtsmbstatusd version\n"
		"    -m :           Show debug messages\n\n";

	// read configuration file
	readConfigFile();

	for ( int i = 1; i < argc; i++ )
	{
		if (QString(argv[i])=="--help")
		{
			qDebug(usage);
			return 0;
		}
		if (argv[i][0] == '-')
		switch (argv[i][1])
		{
			case 'p':    // TCP Port
				if (argv[i][2]=='\0')
				{
					port_server=(QString(argv[++i])).toInt(&ok);
					if (ok==false) 
					{
						qDebug("\n    Missing argument : -p   \n" + usage);
						return 1;
					}
					if (!validatePort(port_server)) return 1;  // port not valid
				}
				else
				{
					unsupported_options(argv[i],usage);
					return 1;
				}
				break;
			case 'v': //version
				if (argv[i][2]=='\0')  qDebug("QtSmbstatusd version : " + version_qtsmbstatus); // view qtsmbstatusd version
				else
				{
					unsupported_options(argv[i],usage);
					return 1;
				}
				return 0;
				break;
			case 'm':    // debug
				if (argv[i][2]=='\0')  debug_qtsmbstatus=true; // view message debug
				else
				{
					unsupported_options(argv[i],usage);
					return 1;
				}
				break;
			case 'd':    // daemonize
				if (argv[i][2]=='\0')  daemonize=true;
				else
				{
					unsupported_options(argv[i],usage);
					return 1;
				}
				break;
			default:     // unsupported option -> show message and exit
				unsupported_options(argv[i],usage);
				return 1;
				break;
		}
	}

	QApplication app(argc, argv, false ); // user interface is unused in this program
	myserver = new Server(&app);

	app.connect( myserver, SIGNAL( destroyed ()), &app, SLOT(quit()) );
	int value_return=app.exec();

	// for debug only
	debugQt("\n\n ___________ OBJECTS _________________");
	debugQt("ClientSocket        : "+QString::number(ClientSocket::compteur_objet));
	debugQt("disconnect_manager  : "+QString::number(disconnect_manager::compteur_objet));
	debugQt("PamThread           : "+QString::number(PamThread::compteur_objet));
	debugQt("process_smbd_exist  : "+QString::number(process_smbd_exist::compteur_objet));
	debugQt("Sendmessage_manager : "+ QString::number(Sendmessage_manager::compteur_objet));
	debugQt("smbmanager          : "+ QString::number(smbmanager::compteur_objet));
	if ((ClientSocket::compteur_objet+disconnect_manager::compteur_objet+PamThread::compteur_objet+process_smbd_exist::compteur_objet
		+Sendmessage_manager::compteur_objet+smbmanager::compteur_objet)==0)
		debugQt("\nDeleted objects : OK\n"); else debugQt("\nDeleted objects : Error !\n");
	return value_return;
}

