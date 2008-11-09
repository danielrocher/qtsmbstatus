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
	\class server
	\brief Class of server items
	\date 2008-11-08
	\version 2.0
	\author Daniel Rocher
	\sa machine user service

	'server' is parent of 'machine' and child of listview
 */

#include <QtGui>

#include "server.h"

// know instancies
QList<QTreeWidgetItem *> QTreeWidgetItemList;

extern void debugQt(const QString & message);

int server::compteur_objet=0;

server::server(QTreeWidget * parent) : QTreeWidgetItem(parent)
{
	debugQt("Object server : "+QString::number(++compteur_objet));
	QTreeWidgetItemList.append(this);
	SambaVersion="";
	QIcon icon;
	icon.addPixmap(QPixmap(":/icons/server.png"), QIcon::Normal, QIcon::Off);
	this->setIcon( 0, icon ); //icon
	this->setExpanded ( true ); // Sets item to be open
}
server::~server()
{
	debugQt("Object server : "+QString::number(--compteur_objet));
	QTreeWidgetItemList.removeAll (this);
}

/**
	Refresh view. Delete obsolete objects (mark = false).
	\sa mark_childs
*/
void server::refresh_childs()
{
	debugQt("server::refresh_childs()");
	for (int i=0;  i < this->childCount (); ++i )
	{
		machine * Child= dynamic_cast<machine *>(this->child (i) );
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
	this->setExpanded ( true ); // Sets item to be open
}

/**
	Mark all children.
 	- mark = true -> exist
 	- mark = false -> obsolete
	\sa refresh_childs
 */
void server::mark_childs()
{
	debugQt("server::mark_childs()");
	for (int i=0;  i < this->childCount (); ++i )
	{
		machine * Child= dynamic_cast<machine *>(this->child (i) );
		if (!Child) break;
		Child->mark=false;
		Child->mark_childs();
	}
}

/**
	add machine and user if not exist
	\sa add_share add_lockedfile
 */
void server::add_user(const QString & PID,const QString & Name,const QString & Group,const QString & MachineName,const QString & MachineIP)
{
	bool exist=false;
	for (int i=0; i < this->childCount (); ++i )
	{
		machine * Child= dynamic_cast<machine *>(this->child (i) );
		if (!Child) break;
		// if child exist
		if  (Child->machine_name==MachineName)
		{
			exist=true;
			// mark this child
			Child->mark=true;
			Child->append_user(PID,Name,Group);
			return; // exit loop
		}
	}
	// if not exist add machine
	if (!exist) new machine (this,PID,Name,Group,MachineName,MachineIP);
}

/**
	add share if not exist
	\sa add_user add_lockedfile
**/
void server::add_share(const QString & PID,const QString & Share,const QString & DateOpen)
{
	for (int i=0; i < this->childCount (); ++i )
	{
		machine * Child= dynamic_cast<machine *>(this->child (i) );
		if (!Child) break;
		for (int j=0; j < Child->childCount (); ++j )
		{
			user * grandChildren = dynamic_cast<user *>(Child->child (j));
			if (!grandChildren) break;
			
			// if child exist
			if  (grandChildren->pid==PID)
			{
				grandChildren->mark=true;
				// send message (log and "balloon message") only if new
				if (!grandChildren-> append_share(PID,Share,DateOpen))
				{
					type_message tempMessage;
					tempMessage.date=DateOpen;
					tempMessage.user=grandChildren->username;
					tempMessage.opened=Share;
					tempMessage.machine=Child->machine_name;
					tempMessage.type_message=0;
					// add to a list
					listMessages << tempMessage;
				}
				return; // exit loop
			}
		}
	}
}

/**
	add lockedfile if not exist
	\sa add_user add_share
**/
void server::add_lockedfile(const QString & PID,const QString & File,const QString & DenyMode,const QString & RW,const QString & Oplock,const QString & DateOpen)
{
	for (int i=0; i < this->childCount (); ++i )
	{
		machine * Child= dynamic_cast<machine *>(this->child (i) );
		if (!Child) break;
		for (int j=0; j < Child->childCount (); ++j )
		{
			user * grandChildren = dynamic_cast<user *>(Child->child (j));
			if (!grandChildren) break;
			// if child exist
			if  (grandChildren->pid==PID)
			{
				grandChildren->mark=true;
				// send message (log and "balloon message") only if new
				if (!grandChildren-> append_lockedfile(PID,File,DenyMode,RW,Oplock,DateOpen))
				{
					type_message tempMessage;
					tempMessage.date=DateOpen;
					tempMessage.user=grandChildren->username;
					tempMessage.opened=File;
					tempMessage.machine=Child->machine_name;
					tempMessage.type_message=1;
					// add to a list
					listMessages << tempMessage;
				}
				return; // exit loop
			}
		}
	}
}

/**
	Return server informations
	\sa ClientSocket::InfoServer
*/
QString server::ViewInfoServer()
{
	debugQt("server::ViewInfoServer()");
	return getSambaVersion();
}

/**
	Return user informations
	\sa ClientSocket::InfoUser
*/
QString server::ViewInfoUser(QTreeWidgetItem* Item)
{
	debugQt("server::ViewInfoUser()");
	QString message;
	user * myItem = dynamic_cast<user *>(Item);
	if (myItem)
	{
		message = "<hr>User : <i>"+
		myItem->username+"</i><br><br>Group: <i>"+
		myItem->group+"</i><br><br>PID: <i>"+
		myItem->pid+"</i><br><br><hr>";
	}
	return message;
}

/**
	Return machine informations
	\sa ClientSocket::InfoMachine
*/
QString server::ViewInfoMachine(QTreeWidgetItem* Item)
{
	debugQt("server::ViewInfoMachine()");
	QString message;
	machine * myItem = dynamic_cast<machine *>(Item);
	if (myItem)
	{
		message = "<hr>Machine : <i>"+
		myItem->machine_name+"</i><br><br>Address: <i>"+
		myItem->machine_ip+"</i><br><hr>";
	}
	return message;
}

/**
	Return service informations (locked file or share)
	\sa  ClientSocket::InfoService
	\sa TypeService
*/
QString server::ViewInfoService(QTreeWidgetItem* Item)
{
	debugQt("server::ViewInfoService()");
	QString message;
	service * myItem = dynamic_cast<service *>(Item);
	if (myItem)
	{
		// if a locked file or share
		switch (myItem->TypeOfService)
		{
			case Tshare: // partage
				message="<hr>Service: <i>"+
				myItem->share+"</i><br><br>Date: <i>"+
				myItem->dateopen+"</i><br><br>PID: <i>"+
				myItem->pid+"</i><br><hr>";
				break;
			case Tlockedfile: // fichier ouvert
				message = "<hr>File: <i>"+
				myItem->filename+"</i><br><br>Date: <i>"+
				myItem->dateopen+"</i><br><br>DenyMode: <i>"+
				myItem->denymode+"</i><br><br>R/W: <i>"+
				myItem->rw+"</i><br><br>"+"Oplock: <i>"+
				myItem->oplock+"</i><br><br>"+"PID: <i>"+
				myItem->pid+"</i><br><hr>";
				break;
		}
	}
	return message;
}

/**
	Set samba version
*/
void server::setSambaVersion(const QString & sambaversion)
{
	SambaVersion=sambaversion;
}

/**
	Return samba version
*/
QString server::getSambaVersion()
{
	return SambaVersion;
}
