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


#include <QtGui>

#include "linecore.h"


/**
	\class LineCore
	\brief Analysis the lines of smbstatus reply.
	\date 2008-11-11
	\version 1.1
	\author Daniel Rocher
	\sa smbstatus
*/

int LineCore::compteur_objet=0;

// tolerance in the research of the beginning or the end of a field - advised: 2
const short int LineCore::back=2;

LineCore::LineCore(QObject *parent)
 : QObject(parent)
{
	debugQt("Object LineCore : "+QString::number(++compteur_objet));
	initHead=false; // true if InitHeader has been processed
	// true if Analysis has been processed
	analysisProc=false;
}


LineCore::~LineCore()
{
	debugQt("Object LineCore : "+QString::number(--compteur_objet));
	listElement.clear();
}

/**
	Init header. Received header for one section.
	\param OneHeader header of one section (connections, shares, locked files, ...).

	Example:
	\verbatim
	Pid    DenyMode   Access      R/W        Oplock           Name
	\endverbatim
*/
void LineCore::InitHeader(const QString & OneHeader)
{
	debugQt("LineCore::InitHeader("+OneHeader+")");
	initHead=true;
	header=OneHeader;
	analysisProc=false;
	// Clear valuelist
	listElement.clear();
}


/**
	Find field on header
	\param element field
	\retval true field exist
	\retval false field doesn't exist
	\sa InitHeader
	Example: "Pid", "DenyMode", ...
*/
bool LineCore::InitElement(const QString & element)
{
	debugQt("LineCore::InitElement("+element+")");
	if (!initHead) return false;
	record mon_record;

	mon_record.Begin=header.indexOf (element,0,Qt::CaseInsensitive);
	// field doesn't exist
	if (mon_record.Begin==-1) return false;

	// find first space+characters after 'element'
	mon_record.End=header.indexOf(QRegExp("[\\s][\\S]"),mon_record.Begin+element.length());
	mon_record.New_Begin=0;
	mon_record.New_End=0;
	mon_record.Value="";
	mon_record.Ident=element;

	// add field characteristics on a record
	listElement.append(mon_record);
	return true;
}

/**
	Analysis one line
	\param OneLine one line of smbstatus (without the header).

	Example:
	\verbatim
	3456   DENY_NONE  0x1         RDONLY     NONE             /home/villou/Utilitaires/qtsmbstatus.pdf   Fri Sep 10 14:14:12 2004
	\endverbatim
*/
void LineCore::Analysis(const QString & OneLine)
{
	debugQt("LineCore::Analysis()");
	if (!initHead) return;
	// if first time in this section, sort
	if (!analysisProc) SortElements();

	analysisProc=true;
	line=OneLine;
	int resu=0;
	int CumulDelta=0;
	QList<record>::iterator itI;

	for (itI = listElement.begin(); itI != listElement.end(); ++itI )
	{
		(*itI).New_Begin=(*itI).Begin;
		(*itI).New_End=(*itI).End;
		// if not first field
		if ((*itI).Begin!=0)
		{
			resu=line.indexOf(QRegExp("[\\s][\\S]"),-back+(*itI).Begin+CumulDelta);
			(*itI).New_Begin=resu+1;
			CumulDelta=(*itI).New_Begin-(*itI).Begin;
		}
		// if last field
		if ((*itI).End==-1)
		{
			(*itI).New_End=line.length();
		}
		else
		{
			resu=line.indexOf(QRegExp("[\\s][\\S]"),-back+(*itI).End+CumulDelta);
			(*itI).New_End=resu;
			CumulDelta=(*itI).New_End-(*itI).End;
		}
		(*itI).Value=(line.mid((*itI).New_Begin,(*itI).New_End-(*itI).New_Begin)).trimmed();

		 /*debugQt( "  Begin:"+QString::number((*itI).Begin)+
			"  End:"+QString::number((*itI).End)+
			"  New_Begin:"+QString::number((*itI).New_Begin)+
			"  New_End:"+QString::number((*itI).New_End)+
			"  CumulDelta:"+QString::number(CumulDelta)+
			"  Ident:"+(*itI).Ident+
			"  Value:"+(*itI).Value); */
	}
}

/**
	Return one field contents
	\param element field name
	\return one field contents
*/
QString LineCore::ReturnElement(const QString & element)
{
	debugQt("LineCore::ReturnElement("+element+")");
	if (!initHead || !analysisProc) return "";
	QList<record>::iterator itI;
	for (itI = listElement.begin(); itI != listElement.end(); ++itI )
	{
		if (((*itI).Ident).toLower()==element.toLower ()) return ((*itI).Value);
	}
	// if doesn't exist
	return "";
}


/**
	Sort records
**/
void LineCore::SortElements()
{
	debugQt("LineCore::SortElements()");
	if (!initHead) return;
	int plus_petit;

	QList<record>::iterator itI;
	QList<record>::iterator itJ;
	QList<record>::iterator position;
	record mon_record;
	for (itI = listElement.begin(); itI != listElement.end(); ++itI )
	{
		plus_petit=(*itI).Begin;
		position=itI;
		for (itJ = itI; itJ != listElement.end(); ++itJ)
		{
			if ((*itJ).Begin < plus_petit) {
				plus_petit=(*itJ).Begin;
				position=itJ;
			}
		}
		if (position!=itI)
		{
			mon_record=*itI;
			*itI=*position;
			*position=mon_record;
		}
	}
}
