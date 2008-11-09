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

#ifndef LOG_H
#define LOG_H

#include "build/ui/ui_log.h"
#include "server.h"

class QStandardItemModel;
class MySortFilterProxyModel;

extern void debugQt(const QString & message);
extern bool Log_activity; // Log SMB/CIFS activities
extern int limitLog; // limit log (number of days)

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
	void eraseOldLog();
	void on_filterEdit_textChanged();
private slots:
	void on_checkShare_stateChanged ( int state);
	void on_checkFile_stateChanged ( int state);
	void on_SaveLogButton_clicked();
private: // attributes
	QStandardItemModel * model;
	MySortFilterProxyModel *proxyModel;
};

#endif
