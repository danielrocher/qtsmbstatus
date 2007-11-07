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

#include "pamthread.h"

/**
	\class PamThread
	\brief Authenticate with PAM
	\date 2007-06-18
	\version 1.0
	\author Daniel Rocher

	Use thread (don't block Mainwindow event loop)
*/

int PamThread::compteur_objet=0;

PamThread::PamThread() : QThread()
{
	debugQt("Object PamThread : "+QString::number(++compteur_objet));
	auth_resu=false;

}

PamThread::~PamThread()
{
	debugQt("Object PamThread : "+QString::number(--compteur_objet));
}

/**
	set Username and password
	\param name username
	\param passwd passord
*/
void PamThread::setAuth(const QString & name,const QString & passwd)
{
	debugQt ("PamThread::setAuth()");
	Name=name;
	Passwd=passwd;
}

/**
	PAM Request
*/
void PamThread::run()
{
	debugQt ("PamThread::run()");
	// try to authenticate using username and password
	if (auth((char *) Name.latin1() , (char *) Passwd.latin1() ) ) 
	{
		// authentication OK
		debugQt ("PAM AUTH : OK !");
		auth_resu=true;
	}
	else
	{
		// bad authentication
		debugQt ("PAM AUTH : FALSE !");
	}
}

