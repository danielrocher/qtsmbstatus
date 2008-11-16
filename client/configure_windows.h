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

#ifndef CONFIGURE_WINDOWS_H
#define CONFIGURE_WINDOWS_H

#include "build/ui/ui_configure.h"

extern void debugQt(const QString & message);
extern void writeConfigFile();

extern QString host;
extern quint16  port_server; //tcp port
extern QString username_login;
extern QString passwd_login;
extern bool autoconnect; //Autoconnect when qtsmbstatus start
extern int interval; // Interval, in seconds, between every request to smbstatus
extern bool view_hidden_shares; // View hidden shares (share$)
extern bool iconize; // Iconize QtSmbstatus on system tray
extern bool show_messages; // show status notification messages
extern bool log_activity; // log SMB/CIFS activities
extern int limitLog; // limit log (number of days)
extern bool check_new_release; // check for new release of qtsmbstatus

class configure_windows : public QDialog, public Ui::configure
{
Q_OBJECT
public:
	configure_windows(QWidget *parent = 0);
	virtual ~configure_windows();
signals:
	void configuration_changed();
private slots: // Private slots
	virtual void on_okButton_clicked();
	virtual void on_checkIcon_toggled(bool);
	virtual void on_checkLogActivity_toggled(bool checked);
};

#endif
