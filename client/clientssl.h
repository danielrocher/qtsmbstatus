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


#ifndef CLIENTSSL_H
#define CLIENTSSL_H


 //SSL
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <QtGui>
#include <QtNetwork>
#include <Qt3Support>

extern void debugQt(const QString & message);
extern void SSL_print_error(int errorcode);
extern void Socket_print_error (int errorcode);

class ClientSSL : public QObject
{
	Q_OBJECT
public:
	ClientSSL(QObject *parent=0);
	virtual ~ClientSSL();
	void sendToServer(const QString & inputText="");
	void Connect(const QString & d_host, Q_UINT16 d_port);
	void closeConnection();
	static int compteur_objet;
signals:
	void info(const QString &);
	void readFromHost(const QByteArray &);
	void socketconnected();
	void socketclosed();
	void Socketerror();
	void UnreachableHost();
private slots:
	void connectToHost();
	void ResolveName(QHostInfo host_info);
	void socketReadyRead();
	void Exception();
	void socketConnected();
	void socketClosed();
private: //attributes
	Q3SocketDevice *socketdevice;
	QSocketNotifier * sn_read;
	QSocketNotifier * sn_exception;
	bool SSL_init;
	SSL_CTX* ssl_ctx;
	SSL* ssl;
	QString host;
	Q_UINT16 port;
};

#endif
