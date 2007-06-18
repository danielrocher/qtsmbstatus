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


#ifndef LINECORE_H
#define LINECORE_H

#include <QObject>
#include <Q3ValueList>

extern void debugQt(const QString & message);

class LineCore : public QObject
{
Q_OBJECT
public:
	LineCore(QObject *parent=0);
	~LineCore();
	void InitHeader(const QString & OneHeader);
	bool InitElement(QString element);
	QString ReturnElement(QString element);
	void Analysis(const QString & OneLine);
	void SortElements();

public: //attributes
	static int compteur_objet;

private: //attributes
	QString line;
	QString header;
	bool analysisProc;
	bool initHead;
	static const short int back;

struct record {
	Q_INT32 Begin;
	Q_INT32 End;
	Q_INT32 New_Begin;
	Q_INT32 New_End;
	QString Ident;
	QString Value;
};

typedef Q3ValueList<record> recordList;
recordList listElement;
};

#endif
