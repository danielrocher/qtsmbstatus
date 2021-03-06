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

#ifndef SMBMANAGER_H
#define SMBMANAGER_H

#include <QProcess>
#include <QObject>
#include <QTextCodec>
#include <QDateTime>

class QStringList;
class QMutex;

extern void debugQt(const QString & message);

class smbmanager : public QObject  {
   Q_OBJECT
public:
	smbmanager(QObject * parent=0, int cachetimeout=60);
	virtual ~smbmanager();
	static int compteur_objet;
private: // Private attributes
	QProcess proc;
	QString datas;
	bool requestFailed;
	QTextDecoder * m_textDecoder;
	int cachetimeout;
	struct Cache {
		QStringList strList;
		QStringList error;
		QDateTime date;
	};
	static Cache cache;
	static QMutex mutex;
private slots: // Private slots
	void start();
	void end_process ();
	void read_data ();
	void ReadStderr();
	void error(QProcess::ProcessError);
private: // methods
	bool cacheExpired();
signals: // Signals
	void signal_std_output(const QStringList  & );
	void ObjError(const QString & );
};

#endif
