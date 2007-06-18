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


#include <QtGui>

#include "log.h"

/**
	\class LogForm
	\brief Form to view CIFS/SMB activities
	\date 2007-06-15
	\version 1.0
	\param parent pointer to the parent object
	\author Daniel Rocher
*/
LogForm::LogForm(QWidget *parent) : QDialog(parent)
{
	debugQt("LogForm::LogForm()");
	setupUi(this);
	//not editable
        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tableWidget->setColumnCount ( 4);
	tableWidget->setColumnWidth ( 0, 200 );
	tableWidget->setColumnWidth ( 1, 100 );
	tableWidget->setColumnWidth ( 2, 100 );
	tableWidget->setColumnWidth ( 3, 450 );
	setHeader();
}


LogForm::~LogForm()
{
	debugQt("LogForm::~LogForm()");
}

/**
	Set tableView's header
*/
void LogForm::setHeader()
{
	debugQt("LogForm::setHeader()");
	QStringList labels;
	labels << tr("Date") << tr("Machine") << tr("User") << "Service";
	tableWidget->setHorizontalHeaderLabels ( labels );
}



/**
	Clear tableView
*/
void LogForm::on_clearButton_clicked()
{
	tableWidget->clear();
	setHeader();
	tableWidget->setRowCount (0);
}


/**
	Add new data
	\sa type_message
*/
void LogForm::append(const type_message & Tmessage)
{

	QIcon icon_file(":/icons/document.png");
	QIcon icon_share(":/icons/folder_open.png");
	QString datetimeStr;
	// not to sort temporarily before insert
	tableWidget->setSortingEnabled ( false);
	int rowcount=tableWidget->rowCount ();
	tableWidget->insertRow (rowcount);

	// change date format
	QDateTime datetime=QDateTime::fromString ( Tmessage.date );
	if (datetime.isValid ())
		datetimeStr=datetime.toString(Qt::ISODate);
	else 
		datetimeStr=Tmessage.date;
	QTableWidgetItem *newItem = new QTableWidgetItem(datetimeStr);
	tableWidget->setItem ( rowcount, 0, newItem );

	// machine
	newItem = new QTableWidgetItem(Tmessage.machine);
	tableWidget->setItem ( rowcount, 1, newItem );

	// user
	newItem = new QTableWidgetItem(Tmessage.user);
	tableWidget->setItem ( rowcount, 2, newItem );

	// service
	if (Tmessage.type_message==0)  newItem = new QTableWidgetItem(icon_share, Tmessage.opened,2000);
	if (Tmessage.type_message==1)  newItem = new QTableWidgetItem(icon_file, Tmessage.opened,2001);
	if ((Tmessage.type_message==0) || (Tmessage.type_message==1)) tableWidget->setItem ( rowcount, 3, newItem );

	tableWidget->setSortingEnabled ( true);
}

/**
	User apply filter
	\param filter filter to apply
*/
void LogForm::on_filterEdit_textChanged(const QString & filter)
{
	debugQt("LogForm::on_filterEdit_textChanged()");
	bool hide=false;
	QTableWidgetItem * item;
	// not to sort temporarily before filtering
	tableWidget->setSortingEnabled ( false);
	int rowcount=tableWidget->rowCount ();
	for (int i=0; i<rowcount; ++i)
	{
		hide=true;
		for (int j=0 ; j<4 ; ++j)
		{
			item=tableWidget->item( i, j );
			if (item)
			{
				if (item->text().contains(filter,Qt::CaseInsensitive)) hide=false;
				if (item->type ()==2000)
				{
					if (checkShare->checkState ()==Qt::Unchecked ) hide=true;
				}
				if (item->type ()==2001)
				{
					if (checkFile->checkState ()==Qt::Unchecked ) hide=true;
				}
			}
		}
		tableWidget->setRowHidden ( i, hide );
	}

	tableWidget->setSortingEnabled ( true);
}

/**
	view/hide Shares
*/
void LogForm::on_checkShare_stateChanged ( int )
{
	debugQt("LogForm::on_checkShare_stateChanged ()");
	on_filterEdit_textChanged(filterEdit->text());
}

/**
	view/hide Files
*/
void LogForm::on_checkFile_stateChanged ( int )
{
	debugQt("LogForm::on_checkFile_stateChanged ()");
	on_filterEdit_textChanged(filterEdit->text());
}


