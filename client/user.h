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

#ifndef USER_H
#define USER_H

#include <QPixmap>
#include <Q3ListView>

#include "service.h"


extern void debugQt(const QString & message);

class user : public Q3ListViewItem  {
public:
	user(Q3ListViewItem * parent,const QString & PID,const QString & Username, const QString & Group);
	virtual ~user();
	bool append_share(const QString & PID,const QString & Share,const QString & DateOpen);
	bool append_lockedfile(const QString & PID,const QString & File,const QString & DenyMode,const QString & RW,const QString & Oplock,const QString & DateOpen);
	void refresh_childs();
	void mark_childs();
	static int compteur_objet;
	bool mark;
	QString username;
	QString group;
	QString pid;
};

#endif
