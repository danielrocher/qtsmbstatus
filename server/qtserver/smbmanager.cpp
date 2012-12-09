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

#include <QMutex>
#include <QTimer>
#include "smbmanager.h"

int smbmanager::compteur_objet=0;
QMutex smbmanager::mutex;
smbmanager::Cache smbmanager::cache;

/**
	\class smbmanager
	\brief start %smbstatus process
	\date 2012-12-07
	\version 1.3
	\author Daniel Rocher
	\param parent pointer to parent for this object
 */
smbmanager::smbmanager(QObject *parent, int cachetimeout) : QObject(parent)
{
	debugQt("Object smbmanager : "+ QString::number(++compteur_objet));
	this->cachetimeout=cachetimeout;
	m_textDecoder = QTextCodec::codecForLocale()->makeDecoder();
	QTimer::singleShot(150, this, SLOT(start()));
}

smbmanager::~smbmanager()
{
	debugQt("Object smbmanager : "+ QString::number(--compteur_objet));
	delete m_textDecoder;
}


void smbmanager::start() {
	debugQt("smbmanager::start()");
	datas.clear();
	requestFailed=false;
	if (cacheExpired()) {
		debugQt ("smbmanager - start process");
		mutex.tryLock ( 15000 );
		cache.strList.clear();
		cache.error.clear();
		mutex.unlock();
		connect( &proc, SIGNAL(finished ( int, QProcess::ExitStatus) ),this, SLOT(end_process()) );
		connect( &proc, SIGNAL(readyReadStandardOutput ()),this, SLOT(read_data()) );
		connect( &proc, SIGNAL(readyReadStandardError ()),this, SLOT(ReadStderr()) );
		connect( &proc, SIGNAL(error ( QProcess::ProcessError) ),this, SLOT(error(QProcess::ProcessError)) );

		proc.start("smbstatus",QIODevice::ReadOnly);
	} else {
		debugQt ("smbmanager - Use cache");
		for (int i=0 ; i < cache.error.size() ; i++)
			emit ObjError(cache.error[i]);
		emit signal_std_output(cache.strList);
		deleteLater();
	}
}

/**
	an error occurs with the process
*/
void smbmanager::error(QProcess::ProcessError err) {
	debugQt("smbmanager::error()");
	// error handling
	qWarning("process smbstatus error");

	switch (err) {
		case 0: debugQt("  ==> FailedToStart");
			break;
		case 1: debugQt("  ==> Crashed");
			break;
		case 2: debugQt("  ==> Timedout");
			break;
		case 3: debugQt("  ==> ReadError");
			break;
		case 4: debugQt("  ==> WriteError");
			break;
		case 5: debugQt("  ==> UnknownError");
			break;
	}
	QString str=tr("process smbstatus error");
	// Update Cache
	mutex.tryLock ( 15000 );
	cache.error.append(str);
	mutex.unlock();
	emit ObjError(str);
	requestFailed=true;
	deleteLater ();
}


/**
	Read Std error
*/
void smbmanager::ReadStderr()
{
	debugQt("smbmanager::ReadStderr()");
	QString str=m_textDecoder->toUnicode(proc.readAllStandardError());
	debugQt(str);
	str=tr("Smbstatus request error")+" : "+ str.replace('\n',' ');
	// Update Cache
	mutex.tryLock ( 15000 );
	cache.error.append(str);
	mutex.unlock();
	emit ObjError(str);
}


/**
	Read Std output
*/
void smbmanager::read_data ()
{
	debugQt("smbmanager::read_data ()");
	datas+=m_textDecoder->toUnicode(proc.readAllStandardOutput());
}

/**
	Process terminated
*/
void smbmanager::end_process ()
{
	debugQt("smbmanager::end_process ()");
	QStringList list=datas.split("\n");
	// Update Cache
	mutex.tryLock ( 15000 );
	cache.date=QDateTime::currentDateTime().addSecs(cachetimeout);
	cache.strList=list;
	mutex.unlock();
	if (!requestFailed) emit signal_std_output(list);
	deleteLater();
}

/**
  Return true if Cache has expired
*/
bool smbmanager::cacheExpired() {
	mutex.tryLock ( 15000 );
	bool cache_expired= !(cache.date.isValid() && QDateTime::currentDateTime() < cache.date);
	mutex.unlock();
	return cache_expired;
}
