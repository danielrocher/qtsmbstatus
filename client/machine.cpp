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

 /**
	\class machine
	\brief Class of machine items
	\date 2007-07-04
	\version 1.1
	\author Daniel Rocher
	\sa server user service

	'machine' is parent of 'user' and child of 'server'
 */

#include "machine.h"

int machine::compteur_objet=0;


machine::machine(Q3ListViewItem * parent,const QString & PID,const QString & Username, const QString & Group,const QString & MachineName, const QString & MachineIP) : Q3ListViewItem(parent)
{
	debugQt("Object machine : "+QString::number(++compteur_objet));
	Q3ListViewItemList.append(this);
	mark=true;
	machine_name=MachineName;
	machine_ip=MachineIP;
	this->setPixmap( 0, QPixmap::QPixmap(":/icons/machine.png") ); //icone
	this->setText ( 0, machine_name) ;
	// add new user
	new user(this,PID,Username,Group);
}

machine::~machine(){
	debugQt("Object machine : "+QString::number(--compteur_objet));
	Q3ListViewItemList.removeAll (this);
}

/**
	Add user if not exist
*/
void machine::append_user(const QString & PID,const QString & Name,const QString & Group)
{
	user * Child= dynamic_cast<user *>(this->firstChild());
	bool exist=false;
	while ( Child )
	{
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
		Child = dynamic_cast<user *>(Child->nextSibling());
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
	user * item_temp;
	user * Child= dynamic_cast<user *>(this->firstChild());
	while ( Child )
	{
		// if child doesn't exist any more
		if (!Child->mark)
		{
			// delete child
			item_temp= Child;
			Child = dynamic_cast<user *>(Child->nextSibling()); // next
			delete item_temp; // delete item and his children
			continue;
		}
		// if child exist
		Child->refresh_childs();
		Child = dynamic_cast<user *>(Child->nextSibling());
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
	user * Child= dynamic_cast<user *>(this->firstChild());
	 while ( Child )
	{
		Child->mark=false;
		Child->mark_childs();
		Child = dynamic_cast<user *>(Child->nextSibling());
	}
}
