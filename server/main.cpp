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

#include <QtCore>
#include <unistd.h>

#include "server.h"
#include "process_smbd_exist.h"
#include "sendmessage_manager.h"
#include "smbmanager.h"
#include "pamthread.h"

extern void unsupported_options(char *error, const QString & usage);
extern bool validatePort(const int & port);
extern void debugQt(const QString & message);
extern void writeToConsole(const QString & message);

extern quint16 port_server;
extern QString version_qtsmbstatus;
extern bool debug_qtsmbstatus;

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
	writeToConsole ("Stop QtSmbstatusd , please wait ...");
	QTimer::singleShot( 0, myserver, SLOT (stopServer()));
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
		fin=variable.indexOf (",");
		if (fin==-1) fin=variable.length();
		allow_user=(variable.mid(0,fin)).simplified ();

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
		fin=variable.indexOf (",");
		if (fin==-1) fin=variable.length();
		allow_user=(variable.mid(0,fin)).simplified();

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
		writeToConsole("Impossible to read configuration file. Use default settings.");
		return;
	}
	QTextStream t( &f1);
	while ( !t.atEnd() )
	{
		ligne =  t.readLine();
		ligne = ligne.simplified();
		if ((ligne.mid(0,1) == "#") || (ligne.length ()==0)) continue;
		if (ligne.contains("="))
		{
			attr= (ligne.mid(0,ligne.indexOf ("="))).simplified();
			variable = (ligne.mid(ligne.indexOf ("=")+1)).simplified();
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
		"    -h|--help :    Show this help\n"
		"    -d :           Daemonize\n"
		"    -p <port> :    TCP port - default = " + QString::number(port_server) +"\n"
		"    -v :           Show qtsmbstatusd version\n"
		"    -m :           Show debug messages\n\n";

	// read configuration file
	readConfigFile();

	QCoreApplication app(argc, argv); // user interface is unused in this program
	app.setApplicationName("qtsmbstatusd");
	app.setApplicationVersion(version_qtsmbstatus);
	app.setOrganizationName("adella.org");
	app.setOrganizationDomain("qtsmbstatus.free.fr");

	int optch;
	extern int opterr;
	opterr = 1; // show getopt errors

	if( app.arguments().contains("--help")) {
		writeToConsole(usage);
		return 0;
	}

	while ((optch = getopt(argc, argv, "hdp:vm")) != -1) {
		switch (optch) {
			case 'h':
				writeToConsole(usage);
				return 0;
				break;
			case 'v':
				writeToConsole("QtSmbstatusd version : " + version_qtsmbstatus); // view qtsmbstatusd version
				return 0;
				break;
			case 'd':
				daemonize=true;
				break;
			case 'p':
				port_server=(QString(optarg)).toInt(&ok);
				if (ok==false)
				{
					writeToConsole("\nbad syntax for -p\n" + usage);
					return 1;
				}
				if (!validatePort(port_server)) return 1;  // port not valid
				break;
			case 'm':
				debug_qtsmbstatus=true; // view message debug
				break;
			default: // '?'
				writeToConsole(usage);
				return 1;
		}
	}
	
	if (optind < argc) {
		printf("\nUnknown option: %s\n\n",argv[optind]);
		return 1;
	}

	// SSL support
	if (!QSslSocket::supportsSsl())
		writeToConsole("This platform don't supports SSL. The socket will fail in the connection phase.");
	
	myserver = new Server(Certificat, Private_key, ssl_password , &app);
	if (! myserver->listen (  QHostAddress::Any, port_server)) {
		writeToConsole("Failed to bind to port "+QString::number( port_server ));
		exit(1);
	}

	if (daemonize) // if daemonize server (run as server)
	{
		if (daemon(0, 0)==-1)
		{
			writeToConsole("Error in daemon(0,0) call - exiting");
			exit(1);
		}
	}

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

