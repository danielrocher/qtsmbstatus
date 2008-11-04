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
 
#ifndef PROCESS_SMBD_EXIST_H
#define PROCESS_SMBD_EXIST_H

#include <QObject>
#include <QProcess>

class QString;

#include "disconnect_manager.h"


extern void debugQt(const QString & message);

class process_smbd_exist : public QObject
{
Q_OBJECT
public:
    process_smbd_exist(const QString & MyPID, const QString & USER,QObject *parent = 0);
    virtual ~process_smbd_exist();
	static int compteur_objet;
private: // Private attributes
	QProcess proc;
	QString MyPid;
	QString UserSamba;
	enum {begin,find} State;
private slots: // Private slots
	void end_process();
	void readFromStdout();
	void error(QProcess::ProcessError);
protected slots:
    virtual void slot_EndProcessKill();
signals: // Signals
	void ObjError(const QString & );
};

#endif
