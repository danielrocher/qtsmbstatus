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


#ifndef MAIN_WINDOWSL_H
#define MAIN_WINDOWSL_H


#include "configure_windowsl.h"
#include "../client/mainwindows.h"
#include "../server/smbmanager.h"
#include "../server/sendmessage_manager.h"
#include "../server/process_smbd_exist.h"
#include "../server/disconnect_manager.h"

class main_windowsl : public MainWindows {
   Q_OBJECT
public:
	main_windowsl(QWidget *parent=0);
	virtual ~main_windowsl();
private slots: // Private slots
	virtual void slot_timer();
	virtual void sendMessage(const QString & machine,const QString & message);
	virtual void disconnectUser(const QString & pid,const QString & user);
	virtual void on_configureAction_triggered();
protected:
	virtual void closeEvent(QCloseEvent *e);
};

#endif


