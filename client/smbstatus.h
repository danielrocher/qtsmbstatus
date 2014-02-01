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

#ifndef SMBSTATUS_H
#define SMBSTATUS_H

//Qt
#include <QObject>
#include <QStringList>

class QString;


#include "linecore.h"


extern void debugQt(const QString & message);

class smbstatus : public QObject
{
	Q_OBJECT
public:
	smbstatus(const QStringList &,QObject *parent=0);
	virtual ~smbstatus();
	void RQ_smbstatus();
	static int compteur_objet;
signals:
	/** Samba version
	\param version_samba samba version */
	void sambaVersion (const QString & version_samba);
	/** Informations for an %user
	\param strPid PID
	\param strUser User name
	\param strGroup Group name
	\param strMachineName Machine name
	\param strMachineIP IP address  **/
	void add_user (const QString & strPid,const QString & strUser,const QString & strGroup,const QString & strMachineName,const QString & strMachineIP);
	/** Informations for a %share
	\param strPid PID
	\param strShare Share name
	\param strConnected date */
	void add_share(const QString & strPid ,const QString & strShare,const QString & strConnected);
	/** Informations for a file opened (locked file)
	\param strPid PID
	\param strName File name
	\param strMode Mode
	\param strRW RW
	\param strOplock Oplock
	\param strDateOpen date */
	void add_lockedfile(const QString & strPid,const QString & strName,const QString & strMode,const QString & strRW,const QString & strOplock,const QString & strDateOpen);
private: // Private methods
	void what_part(QString part);

private: // Private attributes
	QStringList ListSmbstatus;
	int version_samba;
	/** Enum samba version */
	enum {unknown,version2,version3,version4} enum_version_samba;
	/** Enum section of smbstatus */
	enum {header_connexions, connexions, header_services, services, header_locked_files, locked_files} readingpart;
};

#endif
