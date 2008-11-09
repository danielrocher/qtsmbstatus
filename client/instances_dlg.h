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

#ifndef instances_dlg_H
#define instances_dlg_H

#include <QtGui>

class instances_dlg : public QDialog
{
    Q_OBJECT
public:
	instances_dlg(QWidget * parent = 0);
	virtual ~instances_dlg();
	static int compteur_objet;
protected slots:
	virtual void slotTimer();
private: //methods
	void appendObject(int * compteur,const QString & name);
private: //attributes
	struct struct_object
	{
		int * compteur;
		QString name;
		QLabel * label1;
		QLabel * label2;
	};
	QTimer * timer;
	QVBoxLayout *layoutV;
	QList<struct_object> list_object;
};

#endif

