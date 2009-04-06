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

#ifndef UTESTLINECORE_H
#define UTESTLINECORE_H

#include <QObject>
#include <QList>
#include <QString>

#include "../client/linecore.h"


class UtestLineCore: public QObject
{
Q_OBJECT
private slots:
	void initTestCase();
	void InitElement();
	void ReturnElement_data();
	void ReturnElement();

private:
	LineCore linecore;
	struct col {
		QString name;
		QString data;
		//!  fieldWidth value specifies the minimum amount of space that a is padded to and filled with spaces
		qint32 fieldWidth;
	};
	QList<col> cols;
};

#endif
