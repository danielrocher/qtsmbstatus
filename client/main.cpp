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

#include <QtGui>

#include "smbstatus.h"
#include "instances_dlg.h"
#include "main_windows.h"


extern void unsupported_options(char *error, const QString & usage);
extern bool validatePort(const int & port);
extern bool StrToBool(QString & value);
extern QString BoolToStr(bool & value);
extern void debugQt(const QString & message);
extern void writeToConsole(const QString & message);

extern uint int_qtsmbstatus_version;

//default value
QString host="127.0.0.1";
//! Interval, in seconds, between every request to smbstatus
int interval=10;
QString username_login="root";
QString passwd_login="";
//! Autoconnect when qtsmbstatus start
bool autoconnect=false;
//!  View hidden shares (share$)
bool view_hidden_shares = true;
//! Iconize QtSmbstatus on system tray
bool iconize=true;
//! show status notification messages (balloon messages)
bool show_messages=true;
//! log SMB/CIFS activities
bool log_activity=false;
//! limit log (number of days)
int limitLog=1;
//!  check for new release of qtsmbstatus
bool check_new_release=false;

/**
	Convert QtSmbstatus config file to new format 2.0.1.
	We never use .qtsmbstatus.conf (replaced by QSetting).
	Read old file and save setting
*/
void convertto201()
{
	debugQt("convertto201()");
	QString ligne;
	QString attr;
	QString variable;
	bool ok;
	int port_conf;
	int interval_conf;
	int limit_log_conf;
	QString username_login_conf;

	QFile f1( QDir::homePath () + "/.qtsmbstatus.conf" );
	if ( !f1.open( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		writeToConsole("Create configuration file, Use default settings.");
		writeConfigFile(); // create configuration file, use default value
		return;
	}
	QTextStream t( &f1 );
	while ( !t.atEnd() )
	{
		ligne =  t.readLine();
		ligne = ligne.simplified();
		if ((ligne.mid(0,1) == "#") || (ligne.length ()==0)) continue;
		if (ligne.contains("="))
		{
			attr= (ligne.mid(0,ligne.indexOf("="))).simplified();
			variable = (ligne.mid(ligne.indexOf("=")+1)).simplified();
			if (attr=="port")
			{
				port_conf=(variable).toInt(&ok);
				// if port is valid
				if ((ok==true) && (validatePort(port_conf))) port_server=port_conf;
			}
			if ((attr=="tempo") || (attr=="interval"))
			{
				interval_conf=(variable).toInt(&ok);
				// if interval is valid
				if ((ok==true) && (interval_conf>2)) interval=interval_conf;
			}
			if ((attr=="host_address") && (variable.length()!=0)) host=variable;
			if ((attr=="username") && (variable.length()!=0)) username_login=variable;
			if (attr=="autoconnect")  autoconnect=StrToBool(variable);
			if (attr=="view_hidden_shares")  view_hidden_shares=StrToBool(variable);
			if (attr=="system_tray")  iconize=StrToBool(variable);
			if (attr=="notification_messages")  show_messages=StrToBool(variable);
			if (attr=="log_activity")  log_activity=StrToBool(variable);
			if (attr=="limit_log")
			{
				limit_log_conf=(variable).toInt(&ok);
				if (ok==true) limitLog=limit_log_conf;
			}
		}
	}
	f1.close();
	// save change
	writeConfigFile();
}

/**
	Save configuration file (create if not exist)
	\sa readConfigFile
*/
void writeConfigFile()
{
	debugQt("WriteConfigFile()");
	// new format
	// qtsmbstatus >= 2.0.1
	QSettings settings(QSettings::UserScope,"adella.org", "qtsmbstatus");
	settings.beginGroup("/Configuration");
		settings.setValue("qtsmbstatusVersion",int_qtsmbstatus_version);
		settings.setValue("port",port_server);
		settings.setValue("interval",interval);
		settings.setValue("hostAddress",host);
		settings.setValue("username",username_login);
		settings.setValue("autoconnect",autoconnect);
		settings.setValue("viewHiddenShares",view_hidden_shares);
		settings.setValue("systemTray",iconize);
		settings.setValue("notificationMessages",show_messages);
		settings.setValue("logActivity",log_activity);
		settings.setValue("limitLog",limitLog);
		settings.setValue("checkNewRelease",check_new_release);
	settings.endGroup();

	// old format (.qtsmbstatus.conf)
	// qtsmbstatus < 2.0.1
	/*
	QFile f1( QDir::homeDirPath () + "/.qtsmbstatus.conf" );
	if ( !f1.open( QIODevice::WriteOnly | QIODevice::Text ) )
	{
		writeToConsole("Impossible to create configuration file");
		return;
	}
	QTextStream t( &f1 );
	t <<
	"# This is the configuration file for qtsmbstatus (client)\n" <<
	"# Port destination\n" <<
	"# default :  port = 4443\n" <<
	"port = " << QString::number(port_server) << "\n\n" <<
	"# Interval, in seconds, between every request to smbstatus (interval > 2)\n" <<
	"# default :  interval = 10\n" <<
	"interval = " << QString::number(interval) << "\n\n" <<
	"# Host Address\n" <<
	"# default :  host_address = 127.0.0.1\n" <<
	"host_address = " << host << "\n\n" <<
	"# Username\n" <<
	"# default :  username = root\n" <<
	"username = " << username_login << "\n\n" <<
	"# Autoconnect when qtsmbstatus start\n" <<
	"# default :  autoconnect = false\n" <<
	"autoconnect = " << BoolToStr(autoconnect) << "\n\n" <<
	"#View Hidden Shares\n"
	"# default :  view_hidden_shares = true\n" <<
	"view_hidden_shares = " << BoolToStr(view_hidden_shares) << "\n\n" <<
	"#Iconize QtSmbstatus on system tray\n" <<
	"# default :  system_tray = true\n" <<
	"system_tray = " << BoolToStr(iconize) << "\n\n" <<
	"#Show status notification messages\n" <<
	"# default :  notification_messages = true\n" <<
	"notification_messages = " << BoolToStr(show_messages) << "\n\n" <<
	"#Log SMB/CIFS activities\n" <<
	"# default :  log_activity = false\n" <<
	"log_activity = " << BoolToStr(log_activity) << "\n\n" <<
	"#Limit log SMB/CIFS (number of days)\n" <<
	"# default :  limit_log = 1\n" <<
	"limit_log = " << QString::number(limitLog) << "\n\n" <<
	"# end file configuration\n";
	f1.close(); */
}


/**
	Read configuration file.
	\sa writeConfigFile
*/
void readConfigFile()
{
	debugQt("readConfigFile()");

	QSettings settings(QSettings::UserScope,"adella.org", "qtsmbstatus");
	settings.beginGroup("/Configuration");
	if (!settings.contains ("uuid"))
		settings.setValue("uuid",QUuid::createUuid().toString ());
	// if qtsmbstatus < 2.0.1 or first time
	if  ((settings.value("qtsmbstatusVersion",-1).toInt())==-1)
	{
		// convert to new format
		convertto201();
		return;
	}
		int port_conf=settings.value("port",port_server).toInt();
 		if (validatePort(port_conf)) port_server=port_conf;
		int interval_conf=settings.value("interval",interval).toInt();
		if (interval_conf>2) interval=interval_conf;
		host=settings.value("hostAddress",host).toString();
		username_login=settings.value("username",username_login).toString();
		autoconnect=settings.value("autoconnect",autoconnect).toBool();
		view_hidden_shares=settings.value("viewHiddenShares",view_hidden_shares).toBool();
		iconize=settings.value("systemTray",iconize).toBool();
		show_messages=settings.value("notificationMessages",show_messages).toBool();
		log_activity=settings.value("logActivity",log_activity).toBool();
		limitLog=settings.value("limitLog",limitLog).toInt();
		check_new_release=settings.value("checkNewRelease",check_new_release).toBool();
	settings.endGroup();
}

int main(int argc, char *argv[])
{
	bool ok;
	QString usage="\n    Usage:  qtsmbstatus -i <seconds> -m -p <port> -v --help\n\n"
		"    --help :       Show this help\n"
		"    -i <seconds> : Interval between smbstatus requests in seconds, (interval > 2) - default value = " + QString::number(interval) + "\n"
		"    -v :           Show qtsmbstatus version\n"
		"    -p <port> :    TCP port - default = " + QString::number(port_server) + "\n"
		"    -m :           Show debug messages\n\n"
		"exemple:           qtsmbstatus -i 50\n\n";

	#ifdef Q_WS_WIN
	// for windows only - get install directory (register when qtsmbstatus has been installed)
	QSettings settings(QSettings::SystemScope,"qtsmbstatus", "settings");
	if (settings.contains ( "path") )
	{
		QString qtsmbstatus_path = settings.value("path").toString ();
		QDir::setCurrent ( qtsmbstatus_path );
	}
	#endif

	// read configuration file
	readConfigFile();

	// get args
	for ( int i = 1; i < argc; i++ )
	{
		if (QString(argv[i])=="--help")
		{
			writeToConsole(usage);
			return 0;
		}
		if (argv[i][0] == '-')
		switch (argv[i][1])
		{
			case 'i':    // timer interval
				if (argv[i][2]=='\0')
				{
					interval=(QString(argv[++i])).toInt(&ok);
					if (ok==false) // bad conversion, quit
					{
						writeToConsole("\n    Missing argument : -i   \n" + usage);
						return 1;
					}
					if (interval<3)
					{
						writeToConsole("\n    Interval it must be higher than two seconds (option: -i)  \n");
						return 1;
					}
				}
				else
				{
					unsupported_options(argv[i],usage);
					return 1;
				}
				break;
			case 'p':    // TCP Port
				if (argv[i][2]=='\0')
				{
					port_server=(QString(argv[++i])).toInt(&ok);
					if (ok==false)  // bad conversion, quit
					{
						writeToConsole("\n    Missing argument : -p   \n" + usage);
						return 1;
					}
					if (!validatePort(port_server)) return 1; // port not valid
				}
				else
				{
					unsupported_options(argv[i],usage);
					return 1;
				}
				break;
			case 'm':    // debug
				if (argv[i][2]=='\0')  debug_qtsmbstatus=true; // view message debug
				else
				{
					unsupported_options(argv[i],usage);
					return 1;
				}
				break;
			case 'v': //version
				if (argv[i][2]=='\0')  writeToConsole("QtSmbstatus version : " + version_qtsmbstatus); // view qtsmbstatus version
				else
				{
					unsupported_options(argv[i],usage);
					return 1;
				}
				return 0;
				break;
			default:     // unsupported option -> show message and exit
				unsupported_options(argv[i],usage);
				return 1;
				break;
		}
	}

	MyApplication app( argc, argv );

	//  < translate >

	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name());
	app.installTranslator(&qtTranslator);


	QString translate_file;
	QTranslator myappTranslator;
	// only windows
	#ifdef Q_WS_WIN
	translate_file= QDir::currentPath () + QString(  "/tr/qtsmbstatus_");
	myappTranslator.load(translate_file+ QLocale::system().name());
	#else
	translate_file=QString("/usr/share/qtsmbstatus/qtsmbstatus_");
	myappTranslator.load(translate_file+ QLocale::system().name());
	if (myappTranslator.isEmpty())
	{
		translate_file=QString("/usr/local/share/qtsmbstatus/qtsmbstatus_");
		myappTranslator.load(translate_file+ QLocale::system().name());
	}
	#endif

	app.installTranslator( &myappTranslator );
	//  < /translate >

	app.setQuitOnLastWindowClosed ( false );
	main_windows fenetre_principale;
	if (debug_qtsmbstatus)
	{
		instances_dlg * tempdlg = new instances_dlg(&fenetre_principale);
		tempdlg->show();
	}
	app.connect (&app, SIGNAL(quitMyApp()),&fenetre_principale,SLOT(beforeQuit()));
	fenetre_principale.show();
	int value_return=app.exec();

	// for debug only
	debugQt("\n\n ___________ OBJECTS _________________");
	debugQt("LineCore            : "+QString::number(LineCore::compteur_objet));
	debugQt("server              : "+QString::number(server::compteur_objet));
	debugQt("machine             : "+ QString::number(machine::compteur_objet));
	debugQt("user                : "+ QString::number(user::compteur_objet));
	debugQt("service             : "+ QString::number(service::compteur_objet));
	debugQt("smbstatus           : "+ QString::number(smbstatus::compteur_objet));
	if ((LineCore::compteur_objet+server::compteur_objet
		+machine::compteur_objet+user::compteur_objet+service::compteur_objet+smbstatus::compteur_objet)==0)
		debugQt("\nDeleted objects : OK\n"); else debugQt("\nDeleted objects : Error !\n");
	return value_return;
}
