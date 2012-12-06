# SPECS - create RPM
#    by Daniel Rocher
# Thanks:
#    Mike A. Harris <mharris@redhat.com>
#    Stephane URBANOVSKI
#    Johan Cwiklinski


Summary:                A GUI for smbstatus with Qt
Name:                   qtsmbstatus
Group:                  Applications/System
Version:                2.3.0
Release:                1%{?dist}
License:                GPLv2
Vendor:                 ADELLA
Packager:               Daniel Rocher <daniel.rocher@adella.org>
URL:                    http://qtsmbstatus.free.fr
Source0:                http://qtsmbstatus.free.fr/files/qtsmbstatus-%{version}.tar.gz
Patch0:                 %{name}-init.patch
BuildRoot:              %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:          libstdc++-devel gcc-c++
BuildRequires:          qt4-devel >= 4.3 desktop-file-utils
BuildRequires:          pam-devel

%description
QtSmbstatus is a smbstatus graphical interface (GUI). It is meant to provide
the possibility of administering remote machines. QtSmbstatus was designed
as a client/server system secured with SSL.

%package language
Group:                  Applications/System
Summary:                QtSmbstatus languages package
Conflicts:              qtsmbstatus-client < 2.1

%description language
This package will install additional languages for qtsmbstatus-client
 and qtsmbstatus-light.

%package light
Group:                  Applications/System
Summary:                A GUI for smbstatus with Qt
Requires:               qtsmbstatus-language qt4-x11 >= 4.3

%description light
QtSmbstatus is a smbstatus graphical interface (GUI). Qtsmbstatus Light works
 only locally and doesn't require qtsmbstatus-server.

%package client
Group:                  Applications/System
Summary:                A GUI for smbstatus with Qt (client)
Requires:               qtsmbstatus-language qt4-x11 >= 4.3

%description client
QtSmbstatus is a smbstatus graphical interface (GUI). It is meant to provide
the possibility of administering remote machines. QtSmbstatus was designed
as a client/server system secured with SSL. A login and password is required
to log on to server.

%package server
Group:                  Applications/System
Summary:                A GUI for smbstatus with Qt (server)
Requires:               pam samba samba-client
Requires:               qt4 >= 4.3

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
cp -a server/qtserver/bin/qtsmbstatusd $RPM_BUILD_ROOT/%{_bindir}
cp -a light/bin/qtsmbstatusl $RPM_BUILD_ROOT/%{_bindir}

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
cp -a light/qtsmbstatusl.xpm $RPM_BUILD_ROOT/%{_datadir}/pixmaps/
#menu
desktop-file-install --vendor=""  \
       --dir=$RPM_BUILD_ROOT/%{_datadir}/applications client/qtsmbstatus.desktop
desktop-file-install --vendor=""  \
       --dir=$RPM_BUILD_ROOT/%{_datadir}/applications light/qtsmbstatusl.desktop
#manpage
cp -a client/qtsmbstatus.7.gz $RPM_BUILD_ROOT/%{_mandir}/man7/
cp -a server/qtsmbstatusd.7.gz $RPM_BUILD_ROOT/%{_mandir}/man7/
cp -a light/qtsmbstatusl.7.gz $RPM_BUILD_ROOT/%{_mandir}/man7/

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

%files language
%defattr(755,root,root)
%dir %{_datadir}/qtsmbstatus

%defattr(644,root,root)
%{_datadir}/qtsmbstatus/*.qm
%attr(-,root,root) %doc README README-* INSTALL COPYING changelog

%files light
%defattr(755,root,root)
%{_bindir}/qtsmbstatusl

%defattr(644,root,root)
%attr(-,root,root) %doc README README-* INSTALL COPYING changelog
%{_datadir}/pixmaps/qtsmbstatusl.xpm
%{_datadir}/applications/qtsmbstatusl.desktop
%{_mandir}/man7/qtsmbstatusl.7.gz

%files client
%defattr(755,root,root)
%{_bindir}/qtsmbstatus

%defattr(644,root,root)
%attr(-,root,root) %doc README README-* INSTALL COPYING changelog
%{_datadir}/pixmaps/qtsmbstatus.xpm
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

* Thu Dec 6 2012 Daniel Rocher <daniel.rocher@adella.org> 2.3.0-1
- New upstream version

* Sun Feb 19 2012 Daniel Rocher <daniel.rocher@adella.org> 2.2.1-1
- New upstream version

* Tue Jun 28 2011 Daniel Rocher <daniel.rocher@adella.org> 2.2.0-1
- New upstream version

* Thu Apr 22 2010 Daniel Rocher <daniel.rocher@adella.org> 2.1.3-1
- New upstream version

* Sun Jun 21 2009 Daniel Rocher <daniel.rocher@adella.org> 2.1.2-1
- New upstream version

* Sun Apr 12 2009 Daniel Rocher <daniel.rocher@adella.org> 2.1.1-1
- New upstream version

* Wed Nov 19 2008 Daniel Rocher <daniel.rocher@adella.org> 2.1-1
- New upstream version

* Sun Aug 31 2008 Daniel Rocher <daniel.rocher@adella.org> 2.0.5-1
- Initial release

