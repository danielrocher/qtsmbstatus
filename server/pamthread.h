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

#ifndef PAMTHREAD_H
#define PAMTHREAD_H

#include <QThread>

class QString;

//authentication
#include "pam.h"

extern void debugQt(const QString & message);

class PamThread : public QThread {
public:
	PamThread();
	virtual ~PamThread();
	static int compteur_objet;
	bool auth_resu;
	virtual void run();
	void setAuth(const QString & name,const QString & passwd);
private:
	QString Name;
	QString Passwd;
};

#endif
