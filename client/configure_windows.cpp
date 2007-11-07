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


#include <QtGui>

#include "configure_windows.h"

/**
	\class configure_windows
	\brief Configure Qtsmbstatus-client
	\param parent pointer to the parent object
	\date 2007-06-21
	\version 1.4
	\author Daniel Rocher
*/
configure_windows::configure_windows(QWidget *parent)
 : QDialog(parent)
{
	debugQt("configure_windows::configure_windows()");
	this->setAttribute(Qt::WA_DeleteOnClose);
	setupUi(this);
	// read configuration
	readConfigFile();

	spinInterval->setValue (interval);
	EditHost->setText (host);
	EditUsername->setText(username_login);
	checkAutoconnect->setChecked (autoconnect);
	checkViewHiddenShares->setChecked (view_hidden_shares);
	checkIcon->setChecked(iconize);
	checkNotification->setChecked(show_messages);
	checkLogActivity->setChecked(log_activity);
	limitLogBox->setValue (limitLog);
	// first time
	on_checkIcon_toggled(iconize);
	on_checkLogActivity_toggled(log_activity);
}


configure_windows::~configure_windows()
{
		debugQt("configure_windows::~configure_windows()");
}

void configure_windows::on_checkIcon_toggled(bool checked )
{
	debugQt("configure_windows::on_checkIcon_toggled()");
	checkNotification->setEnabled(checked);
}

void configure_windows::on_checkLogActivity_toggled(bool checked)
{
	debugQt("configure_windows::on_checkLogActivity_toggled()");
	limitLogBox->setEnabled(checked);
}

/**
	Save configuration
*/
void configure_windows::SlotOk()
{
	debugQt("configure_windows::SlotOk() - Save Config file");
	interval = spinInterval->value ();
	host= EditHost->text ();
	username_login = EditUsername->text();
	autoconnect = checkAutoconnect->isChecked ();
	view_hidden_shares = checkViewHiddenShares->isChecked ();
	iconize=checkIcon->isChecked();
	show_messages=checkNotification->isChecked();
	log_activity=checkLogActivity->isChecked();
	limitLog=limitLogBox->value ();
	// save configuration
	writeConfigFile();
	emit (configuration_changed());
}

