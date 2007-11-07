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



#include "mysortfilterproxymodel.h"


/**
	\class MySortFilterProxyModel
	\brief Reimplement QSortFilterProxyModel
	\date 2007-06-27
	\version 1.0
	\author Daniel Rocher
	\param share_label the label of share used in LogForm
	\param file_label the label of file used in LogForm
	\param parent parent of this object
	\sa LogForm 
*/
MySortFilterProxyModel::MySortFilterProxyModel(QString share_label,QString file_label,QObject *parent) : QSortFilterProxyModel(parent)
{
	fileLabel = file_label;
	shareLabel = share_label;
	MShare=true;
	MFile=true;
}


MySortFilterProxyModel::~MySortFilterProxyModel()
{
}

/**
	Show/hide shares
*/
void MySortFilterProxyModel::setFilterShare(bool value)
{
    MShare = value;
    filterChanged();
}

/**
	Show/hide files
*/
void MySortFilterProxyModel::setFilterFile(bool value)
{
    MFile = value;
    filterChanged();
}

/**
	reimplement filterAcceptsRow
*/
bool MySortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);
	QModelIndex index1 = sourceModel()->index(sourceRow, 1, sourceParent);
	QModelIndex index2 = sourceModel()->index(sourceRow, 2, sourceParent);
	QModelIndex index3 = sourceModel()->index(sourceRow, 3, sourceParent);
	QModelIndex index4 = sourceModel()->index(sourceRow, 4, sourceParent);

	return ((sourceModel()->data(index0).toString().contains(filterRegExp())
		|| sourceModel()->data(index1).toString().contains(filterRegExp())
		|| sourceModel()->data(index2).toString().contains(filterRegExp())
		|| sourceModel()->data(index3).toString().contains(filterRegExp()))

		&& ((sourceModel()->data(index4).toString().contains(fileLabel) && MFile==true)
		|| (sourceModel()->data(index4).toString().contains(shareLabel) && MShare==true)));
}
