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

#ifndef SERVER_H
#define SERVER_H

#include <QPixmap>
#include <Q3ListView>
#include <QTextStream>

#include "machine.h"


extern void debugQt(const QString & message);

struct type_message
{
	QString date;
	QString user;
	QString machine;
	QString opened;
 	/** 	- 0 = share
	 	- 1 = file */
	short unsigned int type_message;
};

class server : public Q3ListViewItem  {
public:
	server(Q3ListView * parent);
	virtual ~server();
	void refresh_childs();
	void mark_childs();
	void add_user(const QString & PID,const QString & Name,const QString & Group,const QString & MachineName,const QString & MachineIP);
	void add_share(const QString & PID,const QString & Share,const QString & DateOpen);
	void add_lockedfile(const QString & PID,const QString & File,const QString & DenyMode,const QString & RW,const QString & Oplock,const QString & DateOpen);
	void setSambaVersion (const QString &);
	QString getSambaVersion();
	QString ViewInfoServer();
	QString ViewInfoUser(Q3ListViewItem* Item);
	QString ViewInfoService(Q3ListViewItem* Item);
	QString ViewInfoMachine(Q3ListViewItem* Item);
	static int compteur_objet;
	QList<type_message> listMessages;
private: //private attributes
	QString SambaVersion;
	/**
		Enum type of service:
		- Tshare :  0   ->  share
		- Tlockedfile : 1 -> locked file
	*/
	enum {Tshare,Tlockedfile} TypeService;
};

#endif
