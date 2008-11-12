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

 

#include "login_windows.h"


/**
	\class login_windows
	\brief InputBox login and password
	\date 2008-11-11
	\version 1.1
	\param parent pointer to the parent object
	\author Daniel Rocher
*/ 
login_windows::login_windows(QWidget *parent)
 : QDialog(parent)
{
	debugQt("login_windows::login_windows()");
	this->setAttribute(Qt::WA_DeleteOnClose);
	setupUi(this);
	input_username->setText ( username_login );
	input_password->setText ( passwd_login );
}


login_windows::~login_windows()
{
	debugQt("login_windows::~login_windows()");
}

void login_windows::on_bt_login_clicked()
{
	debugQt("login_windows::on_bt_login_clicked()");
	username_login=input_username->text();
	passwd_login=input_password->text();
}


