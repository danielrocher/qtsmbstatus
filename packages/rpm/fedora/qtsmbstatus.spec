# SPECS - create RPM
#    by Daniel Rocher
# Thanks:
#    Mike A. Harris <mharris@redhat.com>
#    Stephane URBANOVSKI
#    Johan Cwiklinski


Summary:                A GUI for smbstatus with Qt
Name:                   qtsmbstatus
Group:                  Applications/System
Version:                2.0.5
Release:                1%{?dist}
License:                GPLv2
Vendor:                 ADELLA
Packager:               Daniel Rocher <daniel.rocher@adella.org>
URL:                    http://qtsmbstatus.free.fr
Source0:                http://qtsmbstatus.free.fr/files/qtsmbstatus-%{version}.tar.gz
Patch0:                 %{name}-%{version}-init.patch
BuildRoot:              %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:          qt4-devel >= 4.2 desktop-file-utils
BuildRequires:          pam-devel openssl-devel

%description
QtSmbstatus is a smbstatus graphical interface (GUI). It is meant to provide
the possibility of administering remote machines. QtSmbstatus was designed
as a client/server system secured with SSL.

%package client
Group:                  Applications/System
Summary:                A GUI for smbstatus with Qt (client)
Requires:               openssl qt4-x11 >= 4.2

%description client
QtSmbstatus is a smbstatus graphical interface (GUI). It is meant to provide
the possibility of administering remote machines. QtSmbstatus was designed
as a client/server system secured with SSL. A login and password is required
to log on to server.

%package server
Group:                  Applications/System
Summary:                A GUI for smbstatus with Qt (server)
Requires:               openssl pam samba samba-client
Requires:               qt4 >= 4.2

%description server
QtSmbstatus is a smbstatus graphical interface (GUI). It is meant to provide
the possibility of administering remote machines. QtSmbstatus was designed
as a client/server system secured with SSL. A login and password is required
to log on to server. The server's application ought to be installed on host
where SAMBA executes.


%prep
%setup -q -n %{name}-%{version}
%patch0 -p1 -b .init

%build
# create Makefile
qmake-qt4

# Internationalization
lrelease-qt4 client/tr/*.ts

# build
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{_bindir}
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/qtsmbstatus
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/pixmaps
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/applications
mkdir -p $RPM_BUILD_ROOT/%{_mandir}/man7
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/pam.d
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/qtsmbstatusd
mkdir -p $RPM_BUILD_ROOT/%{_initrddir}


# bin
cp -a client/bin/qtsmbstatus $RPM_BUILD_ROOT/%{_bindir}
cp -a server/bin/qtsmbstatusd $RPM_BUILD_ROOT/%{_bindir}

# server
cp -a server/pam.d/qtsmbstatusd $RPM_BUILD_ROOT/%{_sysconfdir}/pam.d/
cp -a server/etc/qtsmbstatusd $RPM_BUILD_ROOT/%{_initrddir}
cp -a server/etc/privkey.pem server/etc/qtsmbstatusd.conf \
        server/etc/server.pem server/etc/qtsmbstatusd.users \
        $RPM_BUILD_ROOT/%{_sysconfdir}/qtsmbstatusd/

# translations
cp -a client/tr/*.qm $RPM_BUILD_ROOT/%{_datadir}/qtsmbstatus/
#icons
cp -a client/qtsmbstatus.xpm $RPM_BUILD_ROOT/%{_datadir}/pixmaps/
#menu
desktop-file-install --vendor=""  \
       --dir=$RPM_BUILD_ROOT/%{_datadir}/applications client/qtsmbstatus.desktop
#manpage
cp -a client/qtsmbstatus.7.gz $RPM_BUILD_ROOT/%{_mandir}/man7/
cp -a server/qtsmbstatusd.7.gz $RPM_BUILD_ROOT/%{_mandir}/man7/

%clean
rm -rf $RPM_BUILD_ROOT

%post server
# postinstall: service
chkconfig --add qtsmbstatusd
%{_initrddir}/qtsmbstatusd start

%preun server
# If uninstall
if [ $1 = 0 ]; then
# stop qtsmbstatus server and remove service
%{_initrddir}/qtsmbstatusd stop
# service
chkconfig --del qtsmbstatusd 2>/dev/null
fi


%files client
%defattr(755,root,root)
%{_bindir}/qtsmbstatus
%dir %{_datadir}/qtsmbstatus

%defattr(644,root,root)
%{_datadir}/qtsmbstatus/*.qm
%attr(-,root,root) %doc README README-* INSTALL COPYING changelog
%{_datadir}/pixmaps/*.xpm
%{_datadir}/applications/qtsmbstatus.desktop
%{_mandir}/man7/qtsmbstatus.7.gz


%files server
%defattr(755,root,root)
%{_bindir}/qtsmbstatusd
%{_initrddir}/qtsmbstatusd
%dir %{_sysconfdir}/qtsmbstatusd/

%defattr(644,root,root)
%config(noreplace) %{_sysconfdir}/pam.d/*
%attr(640,root,root) %config(noreplace) %{_sysconfdir}/qtsmbstatusd/*
%attr(-,root,root) %doc README README-* INSTALL COPYING changelog
%{_mandir}/man7/qtsmbstatusd.7.gz

%changelog
* Sun Aug 31 2008 Daniel Rocher <daniel.rocher@adella.org> 2.0.5-1
- Initial release

