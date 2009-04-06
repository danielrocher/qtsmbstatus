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


#include <QtTest/QtTest>
#include "utestlinecore.h"

void UtestLineCore::initTestCase() {
	col m_cols0= { "PID", "3456",  8};
	col m_cols1= { "Username", "robert  Gas", 14};
	col m_cols2= { "Group", "etude  5519", 14};
	col m_cols3= { "Machine", "etude1       (192.168.1.1)", 31};

	cols <<  m_cols0 << m_cols1 << m_cols2 << m_cols3;
	
	QString header;
	for (int i = 0; i < cols.size(); ++i)
		header.append(QString("%1").arg(cols.at(i).name, - cols.at(i).fieldWidth));

	// qDebug() << header;
	linecore.InitHeader(header);
}

void UtestLineCore::InitElement() {
	for (int i = 0; i < cols.size(); ++i)
		QCOMPARE(linecore.InitElement(cols.at(i).name),i);
}

void UtestLineCore::ReturnElement_data()
{
	QTest::addColumn<QString>("aRow");
	QTest::addColumn<QString>("col1");
	QTest::addColumn<QString>("col2");
	QTest::addColumn<QString>("col3");
	QTest::addColumn<QString>("col4");

	QString line;
	for (int j = -9; j <= 2 ; ++j) { // change fields size
		line.clear();
		for (int i = 0; i < cols.size(); ++i)
			line.append(QString("%1").arg(cols.at(i).data, j - (cols.at(i).fieldWidth)));
		// qDebug() << line;
		QTest::newRow("test row") << line << cols.at(0).data << cols.at(1).data << cols.at(2).data << cols.at(3).data;
	}
}

void UtestLineCore::ReturnElement()
{
	QFETCH(QString, aRow);
	QFETCH(QString, col1);
	QFETCH(QString, col2);
	QFETCH(QString, col3);
	QFETCH(QString, col4);

	linecore.Analysis(aRow);
	QCOMPARE(linecore.ReturnElement(0), col1);
	QCOMPARE(linecore.ReturnElement(1), col2);
	QCOMPARE(linecore.ReturnElement(2), col3);
	QCOMPARE(linecore.ReturnElement(3), col4);
	QCOMPARE(linecore.ReturnElement(cols.at(0).name), col1);
	QCOMPARE(linecore.ReturnElement(cols.at(1).name), col2);
	QCOMPARE(linecore.ReturnElement(cols.at(2).name), col3);
	QCOMPARE(linecore.ReturnElement(cols.at(3).name), col4);
}


