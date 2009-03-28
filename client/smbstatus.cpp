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

/**
	\class smbstatus
	\brief Parse %smbstatus reply
	\date 2008-11-11
	\version 1.1
	\author Daniel Rocher
	\sa LineCore
	\param stringlist %smbstatus reply
	\param parent pointer to parent for this object
*/

#include "smbstatus.h"

int smbstatus::compteur_objet=0;

smbstatus :: smbstatus(const QStringList & stringlist,QObject *parent )  : QObject (parent)
{
	debugQt("Object smbstatus : "+QString::number(++compteur_objet));
	ListSmbstatus=stringlist;
}

smbstatus::~smbstatus()
{
	debugQt("Object smbstatus : "+QString::number(--compteur_objet));
}

/**
	start analysis
*/
void smbstatus::RQ_smbstatus()
{
	debugQt("smbstatus::RQ_smbstatus()");
	QString ligne;

	LineCore * linecore = new LineCore(this);
	readingpart=header_connexions; // section

	int iMachineIP=0;
	int iConnected=0;

	QString strShare;
	QString strUser;
	QString strGroup;
	QString strPid;
	QString strMode;
	QString strRW;
	QString strOplock;
	QString strMachineName;
	QString strMachineIP;
	QString strName;
	QString strConnected;
	QString strDateOpen;

	version_samba=unknown; // default, samba version is unknown

	// read all lines of smbstatus reply
	for ( QStringList::Iterator it = ListSmbstatus.begin(); it != ListSmbstatus.end(); ++it ) {
		ligne = *it; // one line
		// ------------- get samba version --------------------
		if ((ligne.contains("samba version",Qt::CaseInsensitive)) && (version_samba==unknown))
		{
			ligne=ligne.simplified ();
			if (ligne.contains("version 2.2",Qt::CaseInsensitive) ) version_samba=version2;
			if (ligne.contains("version 3.",Qt::CaseInsensitive) ) version_samba=version3;
			emit sambaVersion(ligne);
		}

		// inspired of ksmbstatus and updated for samba V3

		// --------------- CONNECTIONS --------------------------
		if ((readingpart==header_connexions) && ligne.contains("machine",Qt::CaseInsensitive))
		{
			linecore->InitHeader(ligne);
			//version samba 3
			if (version_samba==version3)
			{
				linecore->InitElement("Username");
				linecore->InitElement("Group");
				linecore->InitElement("PID");
				linecore->InitElement("Machine");
			}
			else
			{ //version samba 2.2
				linecore->InitElement("uid");
				linecore->InitElement("gid");
				linecore->InitElement("pid");
				linecore->InitElement("machine");
				linecore->InitElement("Service");
			}
		}

		if ((readingpart==connexions) && (ligne.simplified ().isEmpty ()==false))
		{
			linecore->Analysis(ligne);
			if ((version_samba==version3) ||  (version_samba==version2 ) )
			{ // version 2 and version 3 of samba
					strPid=linecore->ReturnElement("pid");
					strMachineName=linecore->ReturnElement("machine");
					iMachineIP=strMachineName.indexOf("(");
					iConnected=strMachineName.indexOf(")",iMachineIP)+1;
					strMachineIP=(strMachineName.mid(iMachineIP+1,iConnected-iMachineIP-2)).trimmed();
			}
			if (version_samba==version3 )
			{ //version samba 3
				strUser=linecore->ReturnElement("username");
				strGroup=linecore->ReturnElement("group");
				strMachineName=(strMachineName.mid(0,iMachineIP)).trimmed();
			}
			if (version_samba==version2 )
			{ //version samba 2.2
				strUser=linecore->ReturnElement("uid");
				strGroup=linecore->ReturnElement("gid");
				strShare=linecore->ReturnElement("Service");
				strConnected=(strMachineName.mid(iConnected)).trimmed();
				strMachineName=(strMachineName.mid(0,iMachineIP)).trimmed();
			}
			emit add_user(strPid,strUser,strGroup,strMachineName,strMachineIP);
			if (version_samba==version2 )
			{ //version samba 2.2
				// Send user BEFORE share (if not, the user is unknown)
				emit add_share(strPid,strShare,strConnected);
			}
		}

		// --------------- SERVICES ---------------------------------------------
		if ((readingpart==header_services) && ligne.contains("Service",Qt::CaseInsensitive))
		{ // only samba 3.0
			linecore->InitHeader(ligne);
			linecore->InitElement("pid");
			linecore->InitElement("Service");
			linecore->InitElement("Connected at");
			linecore->InitElement("Machine");
		}

		if ((readingpart==services) && (ligne.simplified ().isEmpty ()==false) && (ligne.contains("locked files",Qt::CaseInsensitive)==false))
		// shares
		{
			linecore->Analysis(ligne);
			strShare=linecore->ReturnElement("Service");
			strPid=linecore->ReturnElement("pid");
			strMachineName=linecore->ReturnElement("Machine");
			strConnected=linecore->ReturnElement("Connected at");
			emit add_user(strPid,"nobody","nogroup",strMachineName,strMachineName);
			emit add_share(strPid,strShare,strConnected);
		}

		// --------------- LOCKED FILES ---------------------------------------------
		if ((readingpart==header_locked_files) && ligne.contains("pid",Qt::CaseInsensitive))
		{
			linecore->InitHeader(ligne);
			// samba 2 and 3
			linecore->InitElement("pid");
			linecore->InitElement("DenyMode");
			linecore->InitElement("Access");
			linecore->InitElement("R/W");
			linecore->InitElement("Oplock");
			// for samba >3.0.20
			if (ligne.contains("SharePath   Name   Time",Qt::CaseInsensitive))
			{
				linecore->InitElement("SharePath   Name   Time");
			}
			else if (ligne.contains("SharePath   Name",Qt::CaseInsensitive))
			{
				linecore->InitElement("SharePath   Name");
			}
			else
				linecore->InitElement("Name");
		}

		if ((readingpart==locked_files) && (ligne.simplified ().isEmpty ()==false))
		// locked files
		{
			linecore->Analysis(ligne);
			strPid=linecore->ReturnElement("pid");
			strName=linecore->ReturnElement("Name");
			// for samba >3.0.20
			if (strName.isEmpty()) strName=linecore->ReturnElement("SharePath   Name   Time");
			if (strName.isEmpty()) strName=linecore->ReturnElement("SharePath   Name");
			strMode=linecore->ReturnElement("DenyMode");
			strRW=linecore->ReturnElement("R/W");
			strOplock=linecore->ReturnElement("Oplock");
			strDateOpen=(strName.mid(strName.length()-24)).trimmed();
			strName=(strName.mid(0,strName.length()-24)).trimmed();
			emit add_lockedfile(strPid,strName,strMode,strRW,strOplock,strDateOpen);
		}
	what_part(ligne); // find section
	}
ListSmbstatus.clear(); // clear the QStringList
linecore->deleteLater();
deleteLater();
}

/**
	Find section of %smbstatus
	\sa readingpart

	 Sections: header_connexions, connexions, header_services, services, header_locked_files, locked_files
*/
void smbstatus::what_part(QString part)
{
	if ((readingpart==header_connexions) && (part.contains("----",Qt::CaseInsensitive)))
	// end header, start connection
	{
		readingpart=connexions;
		debugQt("header_connexions->connexions");
	}

	if ((readingpart==connexions) && (part.simplified ().isEmpty ()==true))
	// and connection
	{
		if (version_samba==version3 )
		{ //version samba 3
			readingpart=header_services;
			debugQt("connexions->header_services");
		}
		if (version_samba==version2 )
		{ // version samba 2.2
			// go directly to header_locked_files because there isn't section 'service' on samba 2.2
			readingpart=header_locked_files;
			debugQt("connexions->header_locked_files");
		}
	}

	if ((readingpart==header_services) && (part.contains("----",Qt::CaseInsensitive)))
	// end header_services, start services
	{
		readingpart=services;
		debugQt("header_services->services");
	}

	if ((readingpart==services) && (part.contains("Locked files",Qt::CaseInsensitive)))
	// end services, start header_locked_files
	{
		readingpart=header_locked_files;
		debugQt("services->header_locked_files");
	}

	if ((readingpart==header_locked_files) && (part.contains("----",Qt::CaseInsensitive)))
	// end header_locked_files, start locked_files
	{
		readingpart=locked_files;
		debugQt("header_locked_files->locked_files");
	}
}
