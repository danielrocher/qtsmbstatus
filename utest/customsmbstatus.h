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

#ifndef CUSTOMSMBSTATUS_H
#define CUSTOMSMBSTATUS_H

#include <QObject>
#include <QStringList>

class QString;


class CustomSmbstatus: public QObject
{
Q_OBJECT
public slots:
	void clear();
	void setSambaVersion (const QString &);
	void add_user (const QString &,const QString &,const QString &,const QString &,const QString &);
	void add_share(const QString &,const QString &,const QString &);
	void add_lockedfile(const QString &,const QString &,const QString &,const QString &,const QString &,const QString &);
public:
	QString m_version_samba;
	QStringList m_user;
	QStringList m_share;
	QStringList m_lockedfile;
};



#endif

