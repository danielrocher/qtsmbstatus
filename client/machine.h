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

#ifndef MACHINE_H
#define MACHINE_H

#include <QPixmap>
#include <QTreeWidgetItem>

#include "user.h"

class machine : public QTreeWidgetItem  {
public: 
	machine(QTreeWidgetItem * parent,const QString & PID,const QString & Username, const QString & Group, const QString & MachineName, const QString & MachineIP);
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
 
