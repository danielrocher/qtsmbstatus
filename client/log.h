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

#ifndef LOG_H
#define LOG_H

#include "ui_log.h"
#include "server.h"


extern void debugQt(const QString & message);
extern bool Log_activity; // Log SMB/CIFS activities

class LogForm: public QDialog, public Ui::logform
{
Q_OBJECT
public:
	LogForm(QWidget *parent = 0);
	virtual ~LogForm();
	void setHeader();
public slots:
	void append(const type_message &);
	void on_clearButton_clicked();
	void on_filterEdit_textChanged(const QString &);
private slots:
	void on_checkShare_stateChanged ( int);
	void on_checkFile_stateChanged ( int);
private: //methods
	void restoreLogFile();
	void saveLogFile();
};

#endif
