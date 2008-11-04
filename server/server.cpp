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

#include "server.h"

/**
	\class Server
	\brief The Server class handles new connections to the server.
	\param parent pointer to the parent object
	\date 2008-11-03
	\version 1.1
	\author Daniel Rocher
	For every client that connects, it creates a new ClientSocket
*/
Server :: Server( QObject* parent ) : QTcpServer ( parent )
{
	if (daemonize) // if daemonize server (run as server)
	{
        if (daemon(0, 0)==-1)
		{
			qWarning("Error in daemon(0,0) call - exiting");
			exit(1);
		}
	}
}

Server :: ~Server()
{
	debugQt("~Server()");
}

/**
	New client
	create new Clientsocket
	\param socket socket number
*/
void Server :: incomingConnection( int socket )
{
	debugQt ("Server :: incomingConnection()");
	ClientSocket *s = new ClientSocket( socket, this );
	connect (this,SIGNAL(destroyed()),s,SLOT(socketConnectionClose()));
}


