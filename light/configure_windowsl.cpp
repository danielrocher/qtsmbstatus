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

#include "configure_windowsl.h"

extern uint int_qtsmbstatus_version;

/**
	\class configure_windowsl
	\brief Configure Qtsmbstatus-Light
	\param parent pointer to the parent object
	\date 2008-11-15
	\version 1.0
	\author Daniel Rocher
*/
configure_windowsl::configure_windowsl(QWidget *parent)
 : QDialog(parent)
{
	debugQt("configure_windowsl::configure_windowsl()");
	this->setAttribute(Qt::WA_DeleteOnClose);
	setupUi(this);
	groupBox->setVisible (false);
	QSettings settings;
	settings.beginGroup("/Configuration");
	uint last_known_release=settings.value("lastKnownRelease",int_qtsmbstatus_version).toUInt();
	if (last_known_release>int_qtsmbstatus_version)
	{
		labelNewRelease->setText("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; color:#ff0000;\">"
				+labelNewRelease->text()+" : "
				+"<a href=\"http://qtsmbstatus.free.fr/index.php?page=downloads\">"+
				+"<span style=\" text-decoration: underline; color:#0000ff;\">Downloads</span></a></p>");
		labelNewRelease->setTextFormat ( Qt::RichText );
		labelNewRelease->setTextInteractionFlags (Qt::LinksAccessibleByMouse);
		labelNewRelease->setOpenExternalLinks (true);
	} else
		labelNewRelease->hide();

	spinInterval->setValue (settings.value("interval",interval).toInt());
	checkViewHiddenShares->setChecked (settings.value("viewHiddenShares",view_hidden_shares).toBool());
	checkIcon->setChecked(settings.value("systemTray",iconize).toBool());
	checkNotification->setChecked(settings.value("notificationMessages",show_messages).toBool());
	checkLogActivity->setChecked(settings.value("logActivity",log_activity).toBool());
	limitLogBox->setValue (settings.value("limitLog",limitLog).toInt());
	checkNewRelease->setChecked(settings.value("checkNewRelease",check_new_release).toBool());
	// first time
	on_checkIcon_toggled(checkIcon->isChecked());
	on_checkLogActivity_toggled(checkLogActivity->isChecked());
	settings.endGroup();
}


configure_windowsl::~configure_windowsl()
{
		debugQt("configure_windowsl::~configure_windowsl()");
}

void configure_windowsl::on_checkIcon_toggled(bool checked )
{
	debugQt("configure_windowsl::on_checkIcon_toggled()");
	checkNotification->setEnabled(checked);
}

void configure_windowsl::on_checkLogActivity_toggled(bool checked)
{
	debugQt("configure_windowsl::on_checkLogActivity_toggled()");
	limitLogBox->setEnabled(checked);
}

/**
	Save configuration
*/
void configure_windowsl::on_okButton_clicked()
{
	debugQt("configure_windowsl::on_okButton_clicked() - Save Config file");
	interval = spinInterval->value ();
	view_hidden_shares = checkViewHiddenShares->isChecked ();
	iconize=checkIcon->isChecked();
	show_messages=checkNotification->isChecked();
	log_activity=checkLogActivity->isChecked();
	limitLog=limitLogBox->value ();
	check_new_release=checkNewRelease->isChecked();
	// save configuration
	writeConfigFile();
	emit (configuration_changed());
}

