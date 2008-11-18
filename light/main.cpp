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
#include <unistd.h>

#include "../client/smbstatus.h"
#include "../client/instances_dlg.h"
#include "../server/process_smbd_exist.h"
#include "../server/sendmessage_manager.h"
#include "../server/smbmanager.h"
#include "lmain_windows.h"


extern void unsupported_options(char *error, const QString & usage);
extern void debugQt(const QString & message);
extern void writeToConsole(const QString & message);

extern uint int_qtsmbstatus_version;


/**
	Save configuration file (create if not exist)
	\sa readConfigFile
*/
void writeConfigFile()
{
	debugQt("WriteConfigFile()");
	// new format
	// qtsmbstatus >= 2.0.1
	QSettings settings;
	settings.beginGroup("/Configuration");
		settings.setValue("qtsmbstatusVersion",int_qtsmbstatus_version);
		settings.setValue("interval",interval);
		settings.setValue("viewHiddenShares",view_hidden_shares);
		settings.setValue("systemTray",iconize);
		settings.setValue("notificationMessages",show_messages);
		settings.setValue("logActivity",log_activity);
		settings.setValue("limitLog",limitLog);
		settings.setValue("checkNewRelease",check_new_release);
	settings.endGroup();
}


/**
	Read configuration file.
	\sa writeConfigFile
*/
void readConfigFile()
{
	debugQt("readConfigFile()");

	QSettings settings;
	settings.beginGroup("/Configuration");
	if (!settings.contains ("uuid"))
		settings.setValue("uuid",QUuid::createUuid().toString ());
		int interval_conf=settings.value("interval",interval).toInt();
		if (interval_conf>2) interval=interval_conf;
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
	QString usage="\n    Usage:  qtsmbstatusl -i <seconds> -m -v --help\n\n"
		"    -h|--help :    Show this help\n"
		"    -i <seconds> : Interval between smbstatus requests in seconds, (interval > 2) - default value = " + QString::number(interval) + "\n"
		"    -v :           Show qtsmbstatusl version\n"
		"    -m :           Show debug messages\n\n"
		"example:           qtsmbstatusl -i 50\n\n";

	#ifdef Q_WS_WIN
	// for windows only - get install directory (register when qtsmbstatus has been installed)
	QSettings settings(QSettings::SystemScope,"qtsmbstatus", "settings");
	if (settings.contains ( "path") )
	{
		QString qtsmbstatus_path = settings.value("path").toString ();
		QDir::setCurrent ( qtsmbstatus_path );
	}
	#endif

	MyApplication app( argc, argv );
	app.setApplicationName("qtsmbstatusl");
	#if QT_VERSION >= 0x040400
	app.setApplicationVersion(version_qtsmbstatus);
	#endif
	app.setOrganizationName("adella.org");
	app.setOrganizationDomain("qtsmbstatus.free.fr");
	
	// read configuration file
	readConfigFile();

	
	int optch;
	extern int opterr;
	opterr = 1; // show getopt errors

	if( app.arguments().contains("--help")) {
		writeToConsole(usage);
		return 0;
	}

	while ((optch = getopt(argc, argv, "hi:vm")) != -1) {
		switch (optch) {
			case 'h':
				writeToConsole(usage);
				return 0;
				break;
			case 'v':
				writeToConsole("QtSmbstatus Light version : " + version_qtsmbstatus); // view qtsmbstatus version
				return 0;
				break;
			case 'i':
				interval=(QString(optarg)).toInt(&ok);
				if (ok==false) // bad conversion, quit
				{
					writeToConsole("\nbad syntax for -i\n" + usage);
					return 1;
				}
				if (interval<3)
				{
					writeToConsole("\nInterval it must be higher than two seconds (option: -i)\n");
					return 1;
				}
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
	main_windowsl fenetre_principale;
	if (debug_qtsmbstatus)
	{
		instances_dlg * tempdlg = new instances_dlg(&fenetre_principale);
		tempdlg->appendObject(&LineCore::compteur_objet,"LineCore");
		tempdlg->appendObject(&server::compteur_objet,"server");
		tempdlg->appendObject(&machine::compteur_objet,"machine");
		tempdlg->appendObject(&user::compteur_objet,"user");
		tempdlg->appendObject(&service::compteur_objet,"service");
		tempdlg->appendObject(&smbstatus::compteur_objet,"disconnect_manager");
		tempdlg->appendObject(&disconnect_manager::compteur_objet,"disconnect_manager");
		tempdlg->appendObject(&process_smbd_exist::compteur_objet,"process_smbd_exist");
		tempdlg->appendObject(&Sendmessage_manager::compteur_objet,"Sendmessage_manager");
		tempdlg->appendObject(&smbmanager::compteur_objet,"smbmanager");
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
	debugQt("disconnect_manager  : "+QString::number(disconnect_manager::compteur_objet));
	debugQt("process_smbd_exist  : "+QString::number(process_smbd_exist::compteur_objet));
	debugQt("Sendmessage_manager : "+ QString::number(Sendmessage_manager::compteur_objet));
	debugQt("smbmanager          : "+ QString::number(smbmanager::compteur_objet));
	if ((LineCore::compteur_objet+server::compteur_objet+disconnect_manager::compteur_objet
		+process_smbd_exist::compteur_objet+Sendmessage_manager::compteur_objet+smbmanager::compteur_objet
		+machine::compteur_objet+user::compteur_objet+service::compteur_objet+smbstatus::compteur_objet)==0)
		debugQt("\nDeleted objects : OK\n"); else debugQt("\nDeleted objects : Error !\n");
	return value_return;
}

