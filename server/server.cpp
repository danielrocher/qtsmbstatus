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

#include <QFile>
#include "server.h"

extern void debugQt(const QString & message);

int Server::compteur_objet=0;

/**
	\class Server
	\brief The Server class handles new connections to the server.
	\param certificatFile certificat file name
	\param privateKeyFile private key file name
	\param sslPassword passphras for private key
	\param parent pointer to the parent object
	\sa ClientSocket
	\date 2008-11-06
	\version 2.0
	\author Daniel Rocher
	For every client that connects, it creates a new ClientSocket
*/
Server :: Server( const QString & certificatFile , const QString & privateKeyFile ,const QString & sslPassword , QObject* parent ) : QTcpServer ( parent )
{
	debugQt("Server::Server(): "+QString::number(++compteur_objet));


	// Certificat
	QFile f_certif(certificatFile);

	if (!f_certif.open(QIODevice::ReadOnly | QIODevice::Text)) {
        	qWarning() <<  "Impossible to open certificat file : " << certificatFile << " !\n\n";
		exit (1);
	}

	certif=QSslCertificate( &f_certif );
	if (! certif.isValid()) {
		qWarning() << "Bad certificate !\n\n";
		exit (1);
	}
	debugQt("Certificate      OK");


	// private key
	QByteArray sslPassPhras;
	sslPassPhras.append(sslPassword);

	QFile f_pKey(privateKeyFile);
	if (!f_pKey.open(QIODevice::ReadOnly | QIODevice::Text)) {
        	qWarning() <<  "Impossible to open private key file : " << privateKeyFile << " !\n\n";
		exit (1);
	}
	
	s_key=QSslKey( &f_pKey, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey, sslPassPhras );
	if (s_key.isNull()) {
		qWarning() << "the private key or password are bad !\n\n";
		exit (1);
	}
	debugQt("Private key      OK");
}

Server :: ~Server()
{
	debugQt("Server::~Server(): "+QString::number(--compteur_objet));
}

/**
	New client
	create new Clientsocket
	\param socketId socket number
*/
void Server :: incomingConnection( int socketId )
{
	debugQt ("Server :: incomingConnection()");
	ClientSocket *socket = new ClientSocket( this );
	// set protocol (SSLv2,v3 and TLS)
	socket->setProtocol (QSsl::AnyProtocol);
        socket->setLocalCertificate(certif);
        socket->setPrivateKey(s_key);
	if( socket->setSocketDescriptor( socketId ) ) {
		debugQt( "Calling sslSocket->startServerEncryption()");
		socket->startServerEncryption();
	} else {
		qWarning() << "Couldn't setSocketDescriptor(" << socketId << ") for this connection";
		delete socket;
	}
}

/**
	Stop server
*/
void Server ::stopServer()
{
	debugQt("Server::stopServer()");
	deleteLater();
}
