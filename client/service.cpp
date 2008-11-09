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
	\class service
	\brief Class of service items (share or locked file)
	\date 2008-11-08
	\version 2.0
	\author Daniel Rocher
	\sa machine user server

	'service' is child of 'user'
*/

#include <QtGui>

#include "service.h"


extern void debugQt(const QString & message);
extern QList<QTreeWidgetItem *> QTreeWidgetItemList;

int service::compteur_objet=0;

/**
	Constructor of share
*/
service::service(QTreeWidgetItem * parent,const QString & PID,const QString & Share,const QString & DateOpen ) : QTreeWidgetItem(parent) {
	debugQt("Object service : "+QString::number(++compteur_objet));
	QTreeWidgetItemList.append(this);
	mark=true;
	pid=PID;
	share=Share;
	dateopen=DateOpen;
	this->setText ( 0, Share) ;
	QIcon icon;
	icon.addPixmap(QPixmap(":/icons/folder_open.png"), QIcon::Normal, QIcon::Off);
	this->setIcon( 0, icon ); //icon
	// define object type
	TypeOfService=Tshare;
}

/**
	Constructor of locked file
*/
service::service(QTreeWidgetItem * parent,const QString & PID,const QString & FileName,const QString & DenyMode,const QString & RW,const QString & Oplock,const QString & DateOpen): QTreeWidgetItem(parent) {
	debugQt("Object service : "+QString::number(++compteur_objet));
	QTreeWidgetItemList.append(this);
	mark=true;
	pid=PID;
	filename=FileName;
	denymode=DenyMode;
	rw= RW;
	oplock=Oplock;
	dateopen=DateOpen;
	this->setText ( 0, FileName) ;
	QIcon icon;
	icon.addPixmap(QPixmap(":/icons/document.png"), QIcon::Normal, QIcon::Off);
	this->setIcon( 0, icon ); //icon
	// define object type
	TypeOfService=Tlockedfile;
}

service::~service(){
	debugQt("Object service : "+QString::number(--compteur_objet));
	QTreeWidgetItemList.removeAll (this);
}

