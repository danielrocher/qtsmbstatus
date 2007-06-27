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
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include "log.h"
#include "mysortfilterproxymodel.h"

#define name_of_share "Share"
#define name_of_file  "File"

/**
	\class LogForm
	\brief Form to view CIFS/SMB activities
	\date 2007-06-27
	\version 1.1
	\param parent pointer to the parent object
	\author Daniel Rocher
*/
LogForm::LogForm(QWidget *parent) : QDialog(parent)
{
	debugQt("LogForm::LogForm()");
	setupUi(this);

	model = new QStandardItemModel(0, 5,tableView);

	// sort and filter model
	proxyModel = new MySortFilterProxyModel(name_of_share,name_of_file,tableView);
	proxyModel->setSourceModel(model);

	// populate model
	tableView->setModel ( proxyModel);

	// sort enable
	tableView->setSortingEnabled(true);

	// proxy model is dynamically sorted and filtered whenever the contents of the source model change.
	proxyModel->setDynamicSortFilter ( true);

	//not editable
        tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	tableView->verticalHeader ()->setVisible(false); // hide vertical header
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
	labels << tr("Date") << tr("Machine") << tr("User") << tr("Service") << tr("Type");
	model->setHorizontalHeaderLabels ( labels );
	tableView->setColumnWidth ( 0, 200 );
	tableView->setColumnWidth ( 1, 100 );
	tableView->setColumnWidth ( 2, 100 );
	tableView->setColumnWidth ( 3, 450 );
	tableView->setColumnWidth ( 4, 0);
	tableView->setColumnHidden ( 4, true );
}


/**
	Save log file.
	CSV format
*/
void LogForm::on_SaveLogButton_clicked()
{
	debugQt("LogForm::on_SaveLogButton_clicked()");
	QStandardItem * item;
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
	for (int i=0; i<model->rowCount (); ++i)
	{
		item =model->item ( i, 0 );
		if (item) date=item->text (); else continue;
		item =model->item ( i, 1 );
		if (item) machine=item->text ();  else continue;
		item =model->item ( i, 2 );
		if (item) user=item->text ();  else continue;
		item =model->item ( i, 3 );
		if (item) service=item->text (); else continue;
		item =model->item ( i, 4 );
		if (item) type_of_service=item->text ();  else continue;
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
	QStandardItem * item;
	for (int i=0; i<model->rowCount (); ++i)
	{
		item =model->item ( i, 0 );
		if (item) dateItem=QDateTime::fromString (item->text (),"yyyy-MM-dd hh:mm:ss"); else continue;
		if  (!dateItem.isValid()) continue;
		if (dateItem<QDateTime::currentDateTime().addDays(-limitLog))
		{
			model->removeRow ( i );
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
	model->clear();
	setHeader();
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
	QList<QStandardItem *> listItem;

	// change date format
	QDateTime datetime=QDateTime::fromString ( Tmessage.date );
	if (datetime.isValid ())
		datetimeStr=datetime.toString("yyyy-MM-dd hh:mm:ss");
	else
		datetimeStr=Tmessage.date;
	listItem << new QStandardItem(datetimeStr);

	// machine
	listItem << new QStandardItem(Tmessage.machine);

	// user
	listItem << new QStandardItem(Tmessage.user);

	// service
	if (Tmessage.type_message==0)  listItem << new QStandardItem(icon_share, Tmessage.opened) << new QStandardItem(name_of_share); // a share
	if (Tmessage.type_message==1)  listItem << new QStandardItem(icon_file, Tmessage.opened) << new QStandardItem(name_of_file); // a file

	model->appendRow (listItem);
}

/**
	User apply filter
*/
void LogForm::on_filterEdit_textChanged()
{
	debugQt("LogForm::on_filterEdit_textChanged()");
	QString filter=filterEdit->text();
	proxyModel->setFilterCaseSensitivity ( Qt::CaseInsensitive );
	proxyModel->setFilterFixedString (filter);
}

/**
	view/hide Shares
*/
void LogForm::on_checkShare_stateChanged ( int state)
{
	debugQt("LogForm::on_checkShare_stateChanged ()");
	proxyModel->setFilterShare(state);
}

/**
	view/hide Files
*/
void LogForm::on_checkFile_stateChanged ( int state)
{
	debugQt("LogForm::on_checkFile_stateChanged ()");
	proxyModel->setFilterFile(state);
}


