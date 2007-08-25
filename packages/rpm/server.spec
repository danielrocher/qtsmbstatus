# SPECS - create RPM
#    by Daniel Rocher
# Thanks:
#    Mike A. Harris <mharris@redhat.com>
#    Stephane URBANOVSKI


%define instaldir_bin  /usr/bin
%define instaldir_pam  /etc/pam.d
%define instaldir_ini  /etc/init.d
%define instaldir_qts  /etc/qtsmbstatusd

# file .pro
%define qmake_pro      server/server.pro

Summary:		A GUI for smbstatus with Qt
Name:			qtsmbstatus-server
Version:		2.0
Release:		1
License:		GPL
Vendor:			ADELLA
Packager:		Daniel ROCHER <daniel.rocher@adella.org>
Group:			Networking/Other
URL:			http://qtsmbstatus.free.fr
Source0:		%{name}-%{version}.tar.gz
BuildRoot:		%{_tmppath}/%{name}-buildroot
BuildArchitectures:	i586

%if "%{_distribution}" == "Mandriva" || "%{_distribution}" == "Mandrake"
# Mandrake / Mandriva requirements
BuildRequires:		gcc-c++, XFree86-devel
BuildRequires:		libqt4-dev >= 4.2
BuildRequires:		libopenssl-devel
BuildRequires:		pam-devel
Requires:		openssl pam samba-server samba-client
Requires:		libqt4-core >=4.2, libqt4-gui >=4.2, libqt4-sql >=4.2, libqt4-qt3support >=4.2
%else
%if  "%{_distribution}" == "Suse"
# Suse requirements
BuildRequires:		gcc-c++, XFree86-devel
BuildRequires:		qt4-dev >= 4.2
BuildRequires:		openssl-devel
BuildRequires:		pam-devel
Requires:		openssl pam samba samba-client
Requires:		libqt4-core >=4.2, libqt4-gui >=4.2, libqt4-sql >=4.2, libqt4-qt3support >=4.2
%else
%if  "%{_distribution}" == "Fedora"
# Fedora requirements
BuildRequires:          gcc-c++,libX11-devel
BuildRequires:          qt4-devel >= 4.2
BuildRequires:          openssl-devel
BuildRequires:		pam-devel
Requires:		openssl pam samba samba-client
Requires:		libqt4-core >=4.2, libqt4-gui >=4.2, libqt4-sql >=4.2, libqt4-qt3support >=4.2
%else
#Unknown Distribution !
%endif
%endif
%endif


%description
QtSmbstatus is a smbstatus graphical interface (GUI). It is meant to provide
the possibility of administering remote machines. QtSmbstatus was designed
as a client/server system secured with SSL. A login and password is required
to log on to server. The server's application ought to be installed on host
where SAMBA executes.


%prep
# delete temporary files
rm %{_builddir}/* -r -f
rm %{_tmppath}/* -r -f

%setup -q

%build
# create Makefile
qmake-qt4 %{qmake_pro}

# build
make

%install

mkdir -p $RPM_BUILD_ROOT/%{instaldir_bin}
mkdir -p $RPM_BUILD_ROOT/%{instaldir_pam}
mkdir -p $RPM_BUILD_ROOT/%{instaldir_ini}
mkdir -p $RPM_BUILD_ROOT/%{instaldir_qts}

cp -a bin/qtsmbstatusd $RPM_BUILD_ROOT/%{instaldir_bin}
cp -a server/pam.d/qtsmbstatusd $RPM_BUILD_ROOT/%{instaldir_pam}
cp -a server/etc/qtsmbstatusd $RPM_BUILD_ROOT/%{instaldir_ini}
cp -a server/etc/privkey.pem server/etc/qtsmbstatusd.conf server/etc/server.pem server/etc/qtsmbstatusd.users $RPM_BUILD_ROOT/%{instaldir_qts}

#====[ postinstall :
%post

%if "%{_distribution}" == "Mandriva" || "%{_distribution}" == "Mandrake" || "%{_distribution}" == "Fedora"
# service for mdk/mdv/fedora
chkconfig --add qtsmbstatusd
%else
%if  "%{_distribution}" == "Suse"
# service for suse
insserv qtsmbstatusd
%else
#Unknown Distribution !
%endif
%endif
/etc/init.d/qtsmbstatusd start

%clean

#====[ pre uninstall :
%preun
# If uninstall
if [ $1 = 0 ]; then
# stop qtsmbstatus server and remove service
/etc/init.d/qtsmbstatusd stop
%if "%{_distribution}" == "Mandriva" || "%{_distribution}" == "Mandrake" || "%{_distribution}" == "Fedora"
# service for mdk/mdv/fedora
chkconfig --del qtsmbstatusd 2>/dev/null
%else
%if  "%{_distribution}" == "Suse"
# service for suse
insserv -r qtsmbstatusd 2>/dev/null
%else
#Unknown Distribution !
%endif
%endif
fi

#====[ post uninstall :
%postun
if [ $1 = 0 ]; then
# If uninstall, remove directory
test -d $RPM_BUILD_ROOT/%{instaldir_qts} && rmdir $RPM_BUILD_ROOT/%{instaldir_qts} 2>/dev/null
fi

%files
%defattr(750,root,root)
%{instaldir_bin}/qtsmbstatusd
%{instaldir_ini}/qtsmbstatusd

%defattr(640,root,root)
%config(noreplace) %{instaldir_pam}/*
%config(noreplace) %{instaldir_qts}/*

%attr(-,root,root) %doc README README-FR INSTALL COPYING CHANGELOG

%changelog

