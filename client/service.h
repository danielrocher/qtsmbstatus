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

#ifndef SERVICE_H
#define SERVICE_H

#include <QPixmap>
#include <Q3ListView>


extern void debugQt(const QString & message);
extern QList<Q3ListViewItem *> Q3ListViewItemList;

class service : public Q3ListViewItem  {
public:
	service(Q3ListViewItem * parent,const QString & PID,const QString & Share,const QString & DateOpen);
	service(Q3ListViewItem * parent,const QString & PID,const QString & FileName,const QString & DenyMode,const QString & RW,const QString & Oplock,const QString & DateOpen);
	virtual ~service();
	static int compteur_objet;
	bool mark;
	QString share;
	QString pid;
	QString filename;
	QString denymode;
	QString rw;
	QString oplock;
	QString dateopen;
	int TypeOfService;
	/**
		Enum type of service:
		- Tshare :  0   ->  share
		- Tlockedfile : 1 -> locked file
	*/
	enum {Tshare,Tlockedfile} TypeService;
};

#endif
