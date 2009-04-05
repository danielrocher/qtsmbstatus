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

#include <QtCore>

#include "customsmbstatus.h"

void CustomSmbstatus::clear() {
	m_version_samba="";
	m_user.clear();
	m_share.clear();
	m_lockedfile.clear();
}

void CustomSmbstatus::setSambaVersion (const QString & version_samba)
{
	m_version_samba=version_samba;
}

void CustomSmbstatus::add_user (const QString & strPid,const QString & strUser,const QString & strGroup,const QString & strMachineName,const QString & strMachineIP)
{
	if (m_user.isEmpty () || strUser!="nobody")
		m_user << strPid << strUser << strGroup << strMachineName << strMachineIP;
}

void CustomSmbstatus::add_share(const QString & strPid ,const QString & strShare,const QString & strConnected)
{
	m_share <<  strPid << strShare << strConnected;
}

void CustomSmbstatus::add_lockedfile(const QString & strPid,const QString & strName,const QString & strMode,const QString & strRW,const QString & strOplock,const QString & strDateOpen)
{
	m_lockedfile << strPid << strName << strMode << strRW << strOplock << strDateOpen;
}

