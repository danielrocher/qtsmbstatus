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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


/**
	\class ClientSSL
	\brief Control SSL and TCP connections
	\date 2007-06-14
	\version 1.0
	\author Daniel Rocher
*/


#include "clientssl.h"


int ClientSSL::compteur_objet=0;

ClientSSL :: ClientSSL(QObject *parent)  : QObject (parent)
{
	debugQt("Object ClientSSL : "+QString::number(++compteur_objet));
	SSL_init=false;

	// Load error strings
	SSL_load_error_strings();
	// Initialize SSL library by registering algorithms
	OpenSSL_add_ssl_algorithms();
	debugQt ("add_ssl_algorithms            OK");

	// Create a new SSL_CTX object
	ssl_ctx = SSL_CTX_new (SSLv23_client_method());
	debugQt ("Create a new SSL_CTX object   OK");

	// Create a new SSL structure for a connection
	ssl=SSL_new (ssl_ctx);
	if (!ssl)
	{
		qDebug ("Error creating SSL object" );
		SSL_print_error(SSL_get_error(ssl, 0));
		exit(1);
	}
	debugQt ("Create a new SSL structure    OK");

	SSL_set_mode (ssl, SSL_MODE_AUTO_RETRY); //auto retry

}

ClientSSL :: ~ClientSSL()
{
	// shuts down an active TLS/SSL connection
	SSL_shutdown(ssl);
	debugQt ("SSL_shutdown                  OK");
	SSL_free(ssl);
	debugQt ("SSL_free                      OK");
	// Closes the socket
	socketdevice->close();
	if (!socketdevice->isOpen()) socketClosed();
	debugQt("Object ClientSSL : "+QString::number(--compteur_objet));
	delete socketdevice;
}


/**
	Connect to Host
	\param d_host host address or host name
	\param d_port TCP port
	\sa ResolveName
	\sa connectToHost
*/
void ClientSSL ::Connect(const QString & d_host, Q_UINT16 d_port)
{
	debugQt("ClientSSL::Connect()");
	host=d_host;
	port=d_port;
	QHostInfo::lookupHost(d_host,this, SLOT(ResolveName(QHostInfo)));

}

/**
	Result of the lookup
	\param host_info object can then be inspected to get the results of the lookup
*/
void ClientSSL ::ResolveName(QHostInfo host_info)
{
	debugQt("ClientSSL::ResolveName()");
	 if (!host_info.addresses().isEmpty())
	 {
		host = host_info.addresses().first(). toString ();
	 }
	connectToHost();
}

/**
	Start connection
	\sa ResolveName
	\sa Connect
*/
void ClientSSL ::connectToHost()
{
	debugQt("ClientSSL::connectToHost()");
	emit info( tr("Connection in progress on %1").arg(host));
	// connect to server
	socketdevice = new Q3SocketDevice( Q3SocketDevice::Stream );

	QHostAddress addressServer;
	if (!addressServer.setAddress ( host ))
	{
		//  if the address was successfully parsed
		debugQt("Unvalid address");
		emit info( tr("Host not found !") );
		emit UnreachableHost();
		deleteLater();
		return;
	}

	if (socketdevice->connect ( addressServer, port ))
	{
		Socket_print_error(socketdevice->error());
		socketConnected();
	}
	else
	{
		// Connection not established (TCP)
		debugQt("Connection not established");
		Socket_print_error(socketdevice->error());
		emit info( tr("Host not found !") );
		emit UnreachableHost();
		deleteLater();
	}
}


/**
	Close Connection
*/
void ClientSSL :: closeConnection()
{
	debugQt("ClientSSL::closeConnection()");
	deleteLater();
}

/**
	Send datas to server
	\param inputText datas to send
	\sa socketReadyRead
*/
void ClientSSL :: sendToServer(const QString & inputText)
{
	debugQt ("ClientSSL::sendToServer()");
	if (SSL_init==false) return;
	Q3CString send_txt = inputText.utf8 () ;
	if (!SSL_write (ssl, send_txt.data() , send_txt.length ()))
	{
		// an error occurred
		SSL_print_error(SSL_get_error(ssl, 0));
	}
	debugQt(send_txt);
}

/**
	Receive datas from server
	\sa sendToServer
*/
void ClientSSL :: socketReadyRead()
{
	// read from the server
	debugQt ("ClientSSL::socketReadyRead()");
	if (SSL_init==false) return;
	int ret;
	if (!socketdevice->isOpen())
	{
		closeConnection();
		return;
	}
	Q3CString rcv_txt;
	rcv_txt.resize(16384);
	ret = SSL_read(ssl, rcv_txt.data(), rcv_txt.size());
	if (ret <=0 )
	{
		// an error occurred
		SSL_print_error(SSL_get_error(ssl, ret));
		closeConnection();
		return;
	}
	rcv_txt.resize(ret); // Sets the size of the byte array to size bytes
	debugQt (rcv_txt);
	emit readFromHost(rcv_txt);
}


/**
	Socket connected. Run SSL
	\sa connectToHost
*/
void ClientSSL :: socketConnected()
{
	int ret;
	debugQt ("ClientSSL::socketConnected()");
	debugQt ("Socket : " + QString ::number( socketdevice->socket()));
	// sets the file descriptor fd
	if (!SSL_set_fd( ssl , socketdevice->socket() ))
	{
		qDebug( "Error initializing SSL-socket" );
		SSL_print_error(SSL_get_error(ssl, 0));
		exit(1);
	}
	debugQt ("Connect SSL object with a socket                        OK");
	// initiate the TLS/SSL handshake with an TLS/SSL server
	if ((ret = SSL_connect(ssl)) <= 0 )
	{
		qDebug( "SSL Connect error" );
		SSL_print_error(SSL_get_error(ssl, ret));
		exit(1);
	}
	debugQt ("Initiate the TLS/SSL handshake with an TLS/SSL server   OK");
	SSL_init=true;

	sn_read= new QSocketNotifier(socketdevice->socket(), QSocketNotifier::Read,this) ;
	connect( sn_read,SIGNAL(activated(int)),this, SLOT(socketReadyRead()) );

	sn_exception= new QSocketNotifier(socketdevice->socket(), QSocketNotifier::Exception ,this) ;
	connect( sn_exception,SIGNAL(activated(int)),this, SLOT(Exception ()) );

	emit info( tr("Connected to host") );
	emit socketconnected();
}

/**
	There is a TCP error
*/
void ClientSSL ::Exception()
{
	debugQt ("ClientSSL::Exception()");
	int ret = socketdevice->error();
	if (ret!=0)
	{
		// an error occurred
		qWarning("Socket error !");
		Socket_print_error(ret);
		emit info( tr("Socket error") );
		emit Socketerror();
		closeConnection();
	}
}

/**
	Connection closed
	\sa closeConnection
*/
void ClientSSL :: socketClosed()
{
	debugQt ("ClientSSL::socketClosed()");
	emit info( tr("Connection closed") );
	emit socketclosed();
}
