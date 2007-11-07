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

#ifndef SENDMESSAGE_MANAGER_H
#define SENDMESSAGE_MANAGER_H

#include <QObject>
#include <Q3Process>

class QString;


extern void debugQt(const QString & message);

class Sendmessage_manager : public QObject  {
   Q_OBJECT
public: 
	Sendmessage_manager(const QString & machine,const QString & message,QObject *parent=0);
	virtual ~Sendmessage_manager();
	static int compteur_objet;
private: // Private attributes
	Q3Process* proc;
	QString my_message;
	QString to_machine;
	bool error_proc;
	enum {begin,error} State;
private slots: // Private slots
  void readFromStdout();
  void ReadStderr();
  void end_process();
signals:
	void ObjError(const QString & );
};

#endif
