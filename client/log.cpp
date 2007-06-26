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
	tableWidget->verticalHeader ()->setVisible(false); // hide vertical header
	setHeader();
	QHeaderView * header= tableWidget->horizontalHeader ();
	connect (header,SIGNAL(sectionClicked ( int )),this,SLOT(sectionHeaderClicked ( int )));
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
	labels << tr("Date") << tr("Machine") << tr("User") << tr("Service");
	tableWidget->setHorizontalHeaderLabels ( labels );
}


/**
	Save log file.
	CSV format
*/
void LogForm::on_SaveLogButton_clicked()
{
	debugQt("LogForm::on_SaveLogButton_clicked()");
	QTableWidgetItem * item;
	QString date;
	QString machine;
	QString user;
	QString type_of_service;
	QString service;
	QFileDialog fileDialog;
	fileDialog.setDefaultSuffix ( "csv" );
	QString fileName = fileDialog.getSaveFileName(this, tr("Open File"), QDir::homeDirPath (),"*.csv");

	if (fileName.isEmpty())
		return;

	if (!fileName.contains(".csv", Qt::CaseInsensitive)) fileName+=".csv";

	QFile f1(fileName);
	if ( !f1.open( QIODevice::WriteOnly | QIODevice::Text ) )
	{
		qDebug("Impossible to create file !");
		QMessageBox::warning ( this, "Error" , "Impossible to create file !");
		return;
	}

	QTextStream t( &f1 );
	t << "Date;Machine;User;Type;Service\n"; // header of CSV
	for (int i=0; i<tableWidget->rowCount (); ++i)
	{
		item =tableWidget->item ( i, 0 );
		if (item) date=item->text (); else continue;
		item =tableWidget->item ( i, 1 );
		if (item) machine=item->text ();  else continue;
		item =tableWidget->item ( i, 2 );
		if (item) user=item->text ();  else continue;
		item =tableWidget->item ( i, 3 );
		if (item->type ()==2000) // if a share
		{
			type_of_service="Share";
		}
		if (item->type ()==2001) // if a file
		{
			type_of_service="File";
		}
		if (item) service=item->text (); else continue;
		t << date << ";" << machine << ";" << user << ";" << type_of_service << ";" << service << "\n";
	}
	f1.close();
}


/**
	erase old log
	\sa limitLog
*/
void LogForm::eraseOldLog()
{
	debugQt("LogForm::eraseOldLog()");
	QDateTime dateItem;
	QTableWidgetItem * item;
	for (int i=0; i<tableWidget->rowCount (); ++i)
	{
		item =tableWidget->item ( i, 0 );
		if (item) dateItem=QDateTime::fromString (item->text (),Qt::ISODate); else continue;
		if  (!dateItem.isValid()) continue;
		if (dateItem<QDateTime::currentDateTime().addDays(-limitLog))
		{
			tableWidget->removeRow ( tableWidget->row ( item )  );
			i--;
		}
	}
}

/**
	Clear tableView
*/
void LogForm::on_clearButton_clicked()
{
	debugQt("LogForm::on_clearButton_clicked()");
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
	if (Tmessage.type_message==0)  newItem = new QTableWidgetItem(icon_share, Tmessage.opened,2000); // a share
	if (Tmessage.type_message==1)  newItem = new QTableWidgetItem(icon_file, Tmessage.opened,2001); // a file
	if ((Tmessage.type_message==0) || (Tmessage.type_message==1)) tableWidget->setItem ( rowcount, 3, newItem );

	tableWidget->setSortingEnabled ( true);
}

/**
	User apply filter
*/
void LogForm::on_filterEdit_textChanged()
{
	debugQt("LogForm::on_filterEdit_textChanged()");
	QString filter=filterEdit->text();
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
				if (item->type ()==2000) // if a share
				{
					if (checkShare->checkState ()==Qt::Unchecked ) hide=true;
				}
				if (item->type ()==2001) // if a file
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
	on_filterEdit_textChanged();
}

/**
	view/hide Files
*/
void LogForm::on_checkFile_stateChanged ( int )
{
	debugQt("LogForm::on_checkFile_stateChanged ()");
	on_filterEdit_textChanged();
}



/**
	Sort changed, refresh filter
*/
void LogForm::sectionHeaderClicked ( int )
{
	debugQt("LogForm::sectionHeaderClicked()");
	// wait, else doesn't work. It's a bad solution but ...
	//! @todo FIXME 
	QTimer::singleShot(5, this, SLOT(on_filterEdit_textChanged()));
}
