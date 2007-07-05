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
	\class user
	\brief Class of user items
	\date 2007-07-05
	\version 1.1
	\author Daniel Rocher
	\sa server machine service

	'user' is parent of 'service' and child of 'machine'
 */

#include "user.h"

int user::compteur_objet=0;

user::user(Q3ListViewItem * parent,const QString & PID,const QString & Username, const QString & Group) : Q3ListViewItem(parent)
{
	debugQt("Object user : "+QString::number(++compteur_objet));
	Q3ListViewItemList.append(this);
	mark=true;
	username=Username;
	pid=PID;
	group=Group;
	this->setPixmap( 0,QPixmap(":/icons/user.png") ); //icon
	this->setText ( 0, Username) ;
}

user::~user(){
	debugQt("Object user : "+QString::number(--compteur_objet));
	Q3ListViewItemList.removeAll (this);
}

/**
	add share if not exist
	\retval true already exist
	\retval false is new
*/
bool user::append_share(const QString & PID,const QString & Share,const QString & DateOpen)
{
	bool exist=false;
	service * Child = dynamic_cast<service *>(this->firstChild());
	while ( Child )
	{
		// if child exist
		if  ((Child->pid==PID) && (Child->share==Share) )
		{
			exist=true;
			// Mark this object
			Child->mark=true;
			return true; // exit loop
		}
		Child = dynamic_cast<service *>(Child->nextSibling());
	}
	// if not exist add it
	if (!exist) new service (this,PID,Share,DateOpen);
	return false;
}

/**
	add lockedfile if not exist
	\retval true already exist
	\retval false is new
*/
bool user::append_lockedfile(const QString & PID,const QString & File,const QString & DenyMode,const QString & RW,const QString & Oplock,const QString & DateOpen)
{
	bool exist=false;
	service * Child = dynamic_cast<service *>(this->firstChild());
	while ( Child )
	{
		// if child exist
		if  ((Child->pid==PID) && (Child->filename==File) && (Child->denymode==DenyMode)  && (Child->rw==RW) && (Child->oplock==Oplock) && (Child->dateopen==DateOpen) )
		{
			exist=true;
			// Mark this object
			Child->mark=true;
			return true;  // exit loop
		}
		Child = dynamic_cast<service *>(Child->nextSibling());
	}
	// if not exist add it
	if (!exist) new service (this,PID,File,DenyMode,RW,Oplock,DateOpen);
	return false;
}

/**
	Refresh view. Delete obsolete objects (mark = false).
	\sa mark_childs
*/
void user::refresh_childs()
{
	service * item_temp;
	service * Child= dynamic_cast<service *>(this->firstChild());
	while ( Child )
	{
		// if child doesn't exist any more
		if (!Child->mark)
		{
			// delete child
			item_temp= Child;
			Child = dynamic_cast<service *>(Child->nextSibling()); // next
			delete item_temp; // delete item and his children
			continue;
		}
		Child = dynamic_cast<service *>(Child->nextSibling());
	}
}


/**
	Mark all children.
 	- mark = true -> exist
 	- mark = false -> obsolete
	\sa refresh_childs
 */
void user::mark_childs()
{
	service * Child= dynamic_cast<service *>(this->firstChild());
	 while ( Child )
	{
		Child->mark=false;
		Child = dynamic_cast<service *>(Child->nextSibling());
	}
}
