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
	password callback for ssl key usage
*/

int pem_passwd_cb(char *buf, int size, int rwflag, void *password)
{
	strncpy(buf, ssl_password, size);
	buf[size - 1] = '\0';
	return(strlen(buf));
}


/**
	\class Server
	\brief The Server class handles new connections to the server.
	\param port_server Port where the server should listen at
	\param parent pointer to the parent object
	\date 2007-06-15
	\version 1.0
	\author Daniel Rocher
	For every client that connects, it creates a new ClientSocket
*/
Server :: Server( QObject* parent ) : Q3ServerSocket( port_server, 1, parent )
{
	// ***************** initialize TLS/SSL **************************

	// Load error strings
	SSL_load_error_strings();
	// Initialize SSL library by registering algorithms
	OpenSSL_add_ssl_algorithms();
	debugQt ("add ssl algorithms    OK");

	// Create a new SSL_CTX object
	ssl_ctx = SSL_CTX_new (SSLv23_server_method());
	debugQt ("SSL CTX new           OK");

	// set passwd callback for encrypted PEM file handling
	SSL_CTX_set_default_passwd_cb(ssl_ctx,pem_passwd_cb);
	debugQt ("set default passwd    OK");

	// loads the first certificate stored in file into ctx
	if (SSL_CTX_use_certificate_file(ssl_ctx, Certificat, SSL_FILETYPE_PEM) <= 0)
	{
		qWarning("Error in use_certificate_file");
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	debugQt ("use certificate file  OK");

	// adds the first private key found in file to ctx
	if (SSL_CTX_use_PrivateKey_file(ssl_ctx, Private_key , SSL_FILETYPE_PEM) <= 0)
	{
		qWarning("Error in use_privatekey_file");
		ERR_print_errors_fp(stderr);
		exit(1);
	}
	debugQt ("use PrivateKey file   OK");

	// load certificate and key data
	if (!SSL_CTX_check_private_key(ssl_ctx))
	{
		qWarning("Error loading ssl certificate: Private key does not match the certificate public key");
		exit(1);
	}
	debugQt ("check private key     OK");

	if ( !ok() )
	{
		qWarning("Failed to bind to port "+QString::number( port_server ));
		exit(1);
	}

	// here, SSL is ready

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
void Server :: newConnection( int socket )
{
	debugQt ("Server :: newConnection()");
	ClientSocket *s = new ClientSocket( socket, this );
	connect (this,SIGNAL(destroyed()),s,SLOT(socketConnectionClose()));
}

/**
	Stop server (kill , CTRL+C, ...)
*/
void Server ::stopserver()
{
	debugQt ("Server :: stopserver()");
	deleteLater();
}

