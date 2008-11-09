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


#include "instances_dlg.h"
#include "linecore.h"
#include "smbstatus.h"
#include "user.h"
#include "server.h"
#include "machine.h"
#include "service.h"

extern void debugQt(const QString & message);

/**
	\class instances_dlg
  	\brief dialog box for debug only
  	\author Daniel Rocher
  	\version 0.9.0
  	\date    2006-10-17
*/


int instances_dlg::compteur_objet=0;

instances_dlg::instances_dlg(QWidget * parent) : QDialog(parent)
{
	debugQt("instances_dlg::instances_dlg()");
	this->setAttribute(Qt::WA_DeleteOnClose);
	compteur_objet++;
	this->setWindowTitle("view instances");
	this->setGeometry(QRect(330, 120, 112, 68));

	layoutV = new QVBoxLayout(this);

	appendObject(&instances_dlg::compteur_objet,"instances_dlg");
	appendObject(&LineCore::compteur_objet,"LineCore");
	appendObject(&server::compteur_objet,"server");
	appendObject(&machine::compteur_objet,"machine");
	appendObject(&user::compteur_objet,"user");
	appendObject(&service::compteur_objet,"service");
	appendObject(&smbstatus::compteur_objet,"smbstatus");
	
	this->setLayout(layoutV);

	timer = new QTimer (this);
	connect(timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
	timer->start(1500);
	slotTimer(); 
}

instances_dlg::~instances_dlg()
{
	debugQt("instances_dlg::~instances_dlg()");
	compteur_objet--;
}

void instances_dlg::appendObject(int * compteur,const QString & name)
{
	QHBoxLayout * tmplayoutH = new QHBoxLayout();

	QLabel * label1 = new QLabel(this);
	QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(5));
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(label1->sizePolicy().hasHeightForWidth());
	label1->setSizePolicy(sizePolicy);
	label1->setMinimumSize(QSize(150, 0));

	QLabel * label2 = new QLabel(this);
	tmplayoutH->addWidget(label1);
	tmplayoutH->addWidget(label2);
	layoutV->addLayout(tmplayoutH);

	struct_object myobject;
	myobject.compteur=compteur;
	myobject.name=name;
	myobject.label1=label1;
	myobject.label2=label2;
	list_object.append(myobject);
}

void instances_dlg::slotTimer()
{
	for (int i= 0 ; i < list_object.size(); ++i)
	{
		list_object[i].label1->setText(list_object[i].name);
		list_object[i].label2->setText(" : "+QString::number(*list_object[i].compteur));
	}
}
