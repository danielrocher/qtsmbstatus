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
	linecore.InitHeader("PID     Username      Group         Machine                        ");
}

void UtestLineCore::InitElement() {
	QCOMPARE(linecore.InitElement("PID"),0);
	QCOMPARE(linecore.InitElement("Username"),1);
	QCOMPARE(linecore.InitElement("Group"),2);
	QCOMPARE(linecore.InitElement("Machine"),3);
}

void UtestLineCore::ReturnElement_data()
{
	QTest::addColumn<QString>("aRow");
	QTest::addColumn<QString>("col1");
	QTest::addColumn<QString>("col2");
	QTest::addColumn<QString>("col3");
	QTest::addColumn<QString>("col4");

	QTest::newRow("test row 1") << " 3456   robert        etude         etude1       (192.168.1.1)" << "3456" 
		<< "robert" <<  "etude" << "etude1       (192.168.1.1)";
	QTest::newRow("test row 2") << " 3456    robert         etude          etude1       (192.168.1.1)" << "3456" 
		<< "robert" <<  "etude" << "etude1       (192.168.1.1)";
	QTest::newRow("test row 3") << " 3456     robert          etude           etude1       (192.168.1.1)" << "3456" 
		<< "robert" <<  "etude" << "etude1       (192.168.1.1)";
	QTest::newRow("test row 4") << " 3456      robert           etude            etude1       (192.168.1.1)" << "3456" 
		<< "robert" <<  "etude" << "etude1       (192.168.1.1)";
	QTest::newRow("test row 5") << " 3456  robert       etude        etude1       (192.168.1.1)" << "3456" 
		<< "robert" <<  "etude" << "etude1       (192.168.1.1)";
	QTest::newRow("test row 6") << " 3456 robert      etude       etude1       (192.168.1.1)" << "3456" 
		<< "robert" <<  "etude" << "etude1       (192.168.1.1)";
	QTest::newRow("test row 7") << " 3456  robert      etude          etude1       (192.168.1.1)" << "3456" 
		<< "robert" <<  "etude" << "etude1       (192.168.1.1)";
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
	QCOMPARE(linecore.ReturnElement("PID"), col1);
	QCOMPARE(linecore.ReturnElement("Username"), col2);
	QCOMPARE(linecore.ReturnElement("Group"), col3);
	QCOMPARE(linecore.ReturnElement("Machine"), col4);
}


