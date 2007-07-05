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

#ifndef MACHINE_H
#define MACHINE_H

#include <QPixmap>
#include <Q3ListView>

#include "user.h"


extern void debugQt(const QString & message);
extern QList<Q3ListViewItem *> Q3ListViewItemList;

class machine : public Q3ListViewItem  {
public: 
	machine(Q3ListViewItem * parent,const QString & PID,const QString & Username, const QString & Group, const QString & MachineName, const QString & MachineIP);
	virtual ~machine();
	void append_user(const QString & PID,const QString & Name,const QString & Group);
	void refresh_childs();
	void mark_childs();
	static int compteur_objet;
	bool mark;
	QString machine_name;
	QString machine_ip;
};

#endif
 
