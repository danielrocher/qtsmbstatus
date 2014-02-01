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
#include "utestsmbstatus.h"
#include "customsmbstatus.h"
#include "../client/smbstatus.h"

/// Samba version 2.2.8a
QString status228a="Samba version 2.2.8a \n\
Service      uid      gid      pid     machine \n\
---------------------------------------------- \n\
travail      jacques  jacques   8135   atelier2 (100.83.118.21) Mon Aug 22 10:41:47 2005 \n\
 \n\
Locked files: \n\
Pid    DenyMode   Access      R/W        Oplock           Name \n\
-------------------------------------------------------------- \n\
8135   DENY_WRITE 0x20089     RDONLY     EXCLUSIVE+BATCH  /home/serveur/LIGH.ppt   Mon Aug 22 10:45:39 2005 \n";



/// Samba version 3.0.2a
QString status302a="Samba version 3.0.2a \n\
PID     Username      Group         Machine                        \n\
------------------------------------------------------------------- \n\
 3456   robert        etude         etude1       (192.168.1.1) \n\
 \n\
 \n\
Service      pid     machine       Connected at \n\
------------------------------------------------------- \n\
robert        3456   etude1        Fri Sep 10 07:45:05 2004 \n\
\n\
Locked files: \n\
Pid    DenyMode   Access      R/W        Oplock           Name \n\
-------------------------------------------------------------- \n\
3456   DENY_NONE  0x1         RDONLY     NONE             /home/villou/projet_top.fde   Fri Sep 10 19:31:38 2004 \n";



/// Samba version 3.0.4
QString status304="Samba version 3.0.4 \n\
PID     Username      Group         Machine \n\
------------------------------------------------------------------- \n\
7793    daniel        users         pc513-3      (192.168.1.130) \n\
 \n\
Service      pid     machine       Connected at \n\
------------------------------------------------------- \n\
hv05727      7793    pc513-3       Thu May 27 18:10:11 2004 \n\
 \n\
Locked files: \n\
Pid    DenyMode   Access      R/W        Oplock           Name \n\
-------------------------------------------------------------- \n\
7793   DENY_WRITE 0x20        RDONLY     EXCLUSIVE+BATCH  /users/cycle1_1/2003/hv05727/SHDOCVW.DLL   Thu May 27 18:10:38 2004 \n";



/** Samba version 3.0.22
    lockedfile header with "SharePath Name" */
QString status3022="Samba version 3.0.22-13.27-1162-SUSE-SLES10 \n\
PID     Username      Group         Machine                        \n\
------------------------------------------------------------------- \n\
24158   user1         group-admin   my861      (192.168.1.31) \n\
 \n\
Service      pid     machine       Connected at \n\
------------------------------------------------------- \n\
sys          24158   my861       Thu Apr  2 11:58:16 2009 \n\
 \n\
Locked files: \n\
Pid          DenyMode   Access      R/W        Oplock           SharePath           Name \n\
---------------------------------------------------------------------------------------- \n\
24158        DENY_NONE  0x100001    RDONLY     NONE             /XXone   user/bginfo   Fri Apr  3 08:46:03 2009 \n";



/** Samba version 3.0.23d
    lockedfile header with "SharePath   Name   Time" */
QString status3023d="Samba version 3.0.23d-6-1083-SUSE-SL10.2 \n\
PID     Username      Group         Machine                         \n\
------------------------------------------------------------------- \n\
 4528   villou        users         localhost    (192.168.1.150) \n\
 \n\
Service      pid     machine       Connected at \n\
------------------------------------------------------- \n\
users        4528   localhost     Wed May  9 16:35:57 2007 \n\
 \n\
Locked files: \n\
Pid          Uid        DenyMode   Access      R/W        Oplock           SharePath   Name   Time \n\
-------------------------------------------------------------------------------------------------- \n\
4528         1000       DENY_NONE  0x120116    WRONLY     NONE             /home   villou/Documents/kubuntu-7.04-desktop-i386.iso   Wed May  9 16:37:13 2007 \n";



/** Samba version 3.0.30
    test with user="nobody"  and group="nogroup"  */
QString status3030="Samba version 3.0.30-0.fc8 \n\
PID     Username      Group         Machine                         \n\
------------------------------------------------------------------- \n\
 \n\
Service      pid     machine       Connected at \n\
------------------------------------------------------- \n\
F            32459   filomenap     Fri Sep 12 08:38:55 2008 \n\
 \n\
Locked files: \n\
Pid          Uid        DenyMode   Access      R/W        Oplock           SharePath   Name   Time \n\
-------------------------------------------------------------------------------------------------- \n\
32459        99         DENY_NONE  0x12019f    RDWR       EXCLUSIVE+BATCH  /F   Fac/EMPRESA.DBF   Fri Sep 12 09:43:09 2008 \n";


/** Samba version 4.1.4 */
QString status414="Samba version 4.1.4-SerNet-RedHat-7.el6\n\
PID     Username      Group         Machine\n\
-------------------------------------------------------------------\n\
24107     3000029       users         10.10.10.8  (ipv4:10.10.10.8:1099)\n\
\n\
\n\
Service      pid     machine       Connected at\n\
-------------------------------------------------------\n\
user1        24107   10.10.10.8    Fri Jan 31 08:13:06 2014\n\
\n\
Locked files:\n\
Pid          Uid        DenyMode   Access      R/W        Oplock           SharePath   Name   Time\n\
--------------------------------------------------------------------------------------------------\n\
24107        3000029    DENY_NONE  0x2019f     RDWR       NONE             /opt/user1   target/clients/CLI6.cdx   Fri Jan 31 09:04:12 2014\n\
\n";

void UtestSmbstatus::cleanup() {
	m_customsmbstatus.clear();
}

void UtestSmbstatus::SambaVersion_data() {
	QTest::addColumn<QString>("smbstatus_rq");
	QTest::addColumn<QString>("result");

	QTest::newRow("Samba version 2.2.8a") << status228a << "Samba version 2.2.8a";
	QTest::newRow("Samba version 3.0.2a") << status302a << "Samba version 3.0.2a";
	QTest::newRow("Samba version 3.0.4") << status304 << "Samba version 3.0.4";
	QTest::newRow("Samba version 3.0.22") << status3022 << "Samba version 3.0.22-13.27-1162-SUSE-SLES10";
	QTest::newRow("Samba version 3.0.23d") << status3023d << "Samba version 3.0.23d-6-1083-SUSE-SL10.2";
	QTest::newRow("Samba version 3.0.30") << status3030 << "Samba version 3.0.30-0.fc8";
	QTest::newRow("Samba version 4.1.4") << status414 << "Samba version 4.1.4-SerNet-RedHat-7.el6";
}

void UtestSmbstatus::SambaVersion() {
	QFETCH(QString, smbstatus_rq);
	QFETCH(QString, result);

	smbstatus m_smbstatus(smbstatus_rq.split('\n'));
	connect (&m_smbstatus,SIGNAL(sambaVersion (const QString &)),&m_customsmbstatus, SLOT(setSambaVersion (const QString &)));
	m_smbstatus.RQ_smbstatus();
	
	QCOMPARE(m_customsmbstatus.m_version_samba,result);
}

void UtestSmbstatus::addUser_data() {
	QTest::addColumn<QString>("smbstatus_rq");
	QTest::addColumn<QStringList>("result");

	QTest::newRow("Samba version 2.2.8a") << status228a << (QStringList() << "8135" << "jacques" << "jacques" << "atelier2" << "100.83.118.21");
	QTest::newRow("Samba version 3.0.2a") << status302a << (QStringList() << "3456" << "robert" << "etude" << "etude1" << "192.168.1.1");
	QTest::newRow("Samba version 3.0.4") << status304 << (QStringList() << "7793" << "daniel" << "users" << "pc513-3" << "192.168.1.130");
	QTest::newRow("Samba version 3.0.22") << status3022 <<  (QStringList() << "24158"<< "user1" << "group-admin" << "my861" << "192.168.1.31");
	QTest::newRow("Samba version 3.0.23d") << status3023d << (QStringList() << "4528" << "villou" << "users" << "localhost" << "192.168.1.150");
	QTest::newRow("Samba version 3.0.30") << status3030 << (QStringList() << "32459" << "nobody" << "nogroup" << "filomenap" << "filomenap");
	QTest::newRow("Samba version 4.1.4") << status414 << (QStringList() << "24107" << "3000029" << "users" << "10.10.10.8" << "ipv4:10.10.10.8:1099");
}

void UtestSmbstatus::addUser() {
	QFETCH(QString, smbstatus_rq);
	QFETCH(QStringList, result);

	smbstatus m_smbstatus(smbstatus_rq.split('\n'));
	connect (&m_smbstatus,SIGNAL(add_user (const QString &,const QString &,const QString &,const QString &,const QString &)),&m_customsmbstatus, SLOT(add_user (const QString &,const QString &,const QString &,const QString &,const QString &)));
	m_smbstatus.RQ_smbstatus();
	QCOMPARE(m_customsmbstatus.m_user,result);
}

void UtestSmbstatus::addShare_data() {
	QTest::addColumn<QString>("smbstatus_rq");
	QTest::addColumn<QStringList>("result");

	QTest::newRow("Samba version 2.2.8a") << status228a << (QStringList() << "8135" << "travail" << "Mon Aug 22 10:41:47 2005");
	QTest::newRow("Samba version 3.0.2a") << status302a << (QStringList() << "3456" << "robert" << "Fri Sep 10 07:45:05 2004");
	QTest::newRow("Samba version 3.0.4") << status304 << (QStringList() << "7793" << "hv05727" << "Thu May 27 18:10:11 2004");
	QTest::newRow("Samba version 3.0.22") << status3022 <<  (QStringList() << "24158"<< "sys" << "Thu Apr  2 11:58:16 2009");
	QTest::newRow("Samba version 3.0.23d") << status3023d << (QStringList() << "4528" << "users" << "Wed May  9 16:35:57 2007");
	QTest::newRow("Samba version 3.0.30") << status3030 << (QStringList() << "32459" << "F" << "Fri Sep 12 08:38:55 2008");
	QTest::newRow("Samba version 4.1.4") << status414 << (QStringList() << "24107" << "user1" << "Fri Jan 31 08:13:06 2014");
}

void UtestSmbstatus::addShare() {
	QFETCH(QString, smbstatus_rq);
	QFETCH(QStringList, result);

	smbstatus m_smbstatus(smbstatus_rq.split('\n'));
	connect (&m_smbstatus,SIGNAL(add_share(const QString &,const QString &,const QString &)),&m_customsmbstatus, SLOT(add_share(const QString &,const QString &,const QString &)));
	m_smbstatus.RQ_smbstatus();
	QCOMPARE(m_customsmbstatus.m_share,result);
}

void UtestSmbstatus::addLockedfile_data() {
	QTest::addColumn<QString>("smbstatus_rq");
	QTest::addColumn<QStringList>("result");

	QTest::newRow("Samba version 2.2.8a") << status228a << (QStringList() << "8135" << "/home/serveur/LIGH.ppt" << "DENY_WRITE" << "RDONLY" << "EXCLUSIVE+BATCH" << "Mon Aug 22 10:45:39 2005");
	QTest::newRow("Samba version 3.0.2a") << status302a << (QStringList() << "3456" << "/home/villou/projet_top.fde" << "DENY_NONE" << "RDONLY" <<  "NONE" << "Fri Sep 10 19:31:38 2004");
	QTest::newRow("Samba version 3.0.4") << status304 << (QStringList() << "7793" << "/users/cycle1_1/2003/hv05727/SHDOCVW.DLL" << "DENY_WRITE" << "RDONLY" << "EXCLUSIVE+BATCH" << "Thu May 27 18:10:38 2004");
	QTest::newRow("Samba version 3.0.22") << status3022 <<  (QStringList() << "24158"<< "/XXone   user/bginfo" << "DENY_NONE" << "RDONLY" << "NONE" << "Fri Apr  3 08:46:03 2009");
	QTest::newRow("Samba version 3.0.23d") << status3023d << (QStringList() << "4528" << "/home   villou/Documents/kubuntu-7.04-desktop-i386.iso" << "DENY_NONE" << "WRONLY" << "NONE" << "Wed May  9 16:37:13 2007");
	QTest::newRow("Samba version 3.0.30") << status3030 << (QStringList() << "32459" << "/F   Fac/EMPRESA.DBF" << "DENY_NONE" << "RDWR" << "EXCLUSIVE+BATCH" << "Fri Sep 12 09:43:09 2008");
	QTest::newRow("Samba version 4.1.4") << status414 << (QStringList() << "24107" << "/opt/user1   target/clients/CLI6.cdx" << "DENY_NONE" << "RDWR" << "NONE" << "Fri Jan 31 09:04:12 2014");
}

void UtestSmbstatus::addLockedfile() {
	QFETCH(QString, smbstatus_rq);
	QFETCH(QStringList, result);

	smbstatus m_smbstatus(smbstatus_rq.split('\n'));
	connect (&m_smbstatus,SIGNAL(add_lockedfile(const QString &,const QString &,const QString &,const QString &,const QString &,const QString &)),&m_customsmbstatus, SLOT(add_lockedfile(const QString &,const QString &,const QString &,const QString &,const QString &,const QString &)));
	m_smbstatus.RQ_smbstatus();
	QCOMPARE(m_customsmbstatus.m_lockedfile,result);
}

