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
	\class machine
	\brief Class of machine items
	\date 2008-11-08
	\version 2.0
	\author Daniel Rocher
	\sa server user service

	'machine' is parent of 'user' and child of 'server'
 */


#include <QtGui>

#include "machine.h"

extern QList<QTreeWidgetItem *> QTreeWidgetItemList;
extern void debugQt(const QString & message);

int machine::compteur_objet=0;


machine::machine(QTreeWidgetItem * parent,const QString & PID,const QString & Username, const QString & Group,const QString & MachineName, const QString & MachineIP) : QTreeWidgetItem(parent)
{
	debugQt("Object machine : "+QString::number(++compteur_objet));
	QTreeWidgetItemList.append(this);
	mark=true;
	machine_name=MachineName;
	machine_ip=MachineIP;
	QIcon icon;
	icon.addPixmap(QPixmap(":/icons/machine.png"), QIcon::Normal, QIcon::Off);
	this->setIcon( 0, icon ); //icone
	this->setText ( 0, machine_name) ;
	// add new user
	new user(this,PID,Username,Group);
}

machine::~machine(){
	debugQt("Object machine : "+QString::number(--compteur_objet));
	QTreeWidgetItemList.removeAll (this);
}

/**
	Add user if not exist
*/
void machine::append_user(const QString & PID,const QString & Name,const QString & Group)
{
	bool exist=false;
	for (int i=0;  i < this->childCount (); ++i )
	{
		user * Child= dynamic_cast<user *>(this->child (i) );
		if (!Child) break;
		// If exist
		if  (Child->pid==PID)
		{
			exist=true;
			// Mark this object
			Child->mark=true;
			// replace nobody by real name
			if   ((Child->username=="nobody") && (Name!="nobody"))
			{
				Child->username=Name;
				Child->group=Group;
				Child->setText(0,Name);
			}
			return; // exit loop
		}
	}
	// if not exist add user
	if (!exist) new user (this,PID,Name,Group);
}


/**
	Refresh view. Delete obsolete objects (mark = false).
	\sa mark_childs
*/
void machine::refresh_childs()
{
	for (int i=0;  i < this->childCount (); ++i )
	{
		user * Child= dynamic_cast<user *>(this->child (i) );
		if (!Child) break;
		
		// if child doesn't exist any more
		if (!Child->mark)
		{
			delete Child;  // delete item and his children
			--i;
			continue;
		}
		// if child exist
		Child->refresh_childs();
	}
}


/**
	Mark all children.
 	- mark = true -> exist
 	- mark = false -> obsolete
	\sa refresh_childs
 */
void machine::mark_childs()
{
	for (int i=0;  i < this->childCount (); ++i )
	{
		user * Child= dynamic_cast<user *>(this->child (i) );
		if (!Child) break;
		Child->mark=false;
		Child->mark_childs();
	}
}
