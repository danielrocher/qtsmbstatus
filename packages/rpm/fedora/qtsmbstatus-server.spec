# SPECS - create RPM
#    by Daniel Rocher
# Thanks:
#    Mike A. Harris <mharris@redhat.com>
#    Stephane URBANOVSKI
#    Johan Cwiklinski

Summary:                A GUI for smbstatus with Qt
Name:                   qtsmbstatus-server
Version:                2.0.5
Release:                1%{?dist}
License:                GPLv2
Group:                  Applications/System
Vendor:                 ADELLA
Packager:               Daniel Rocher <daniel.rocher@adella.org>
URL:                    http://qtsmbstatus.free.fr
Source0:                http://qtsmbstatus.free.fr/files/qtsmbstatus-%{version}.tar.gz
BuildRoot:              %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:          gcc-c++ libX11-devel
BuildRequires:          qt4 >= 4.2 qt4-devel >= 4.2
BuildRequires:          openssl-devel pam-devel
Requires:               openssl pam samba samba-client
Requires:               qt4 >= 4.2 qt4-x11 >= 4.2


%description
QtSmbstatus is a smbstatus graphical interface (GUI). It is meant to provide
the possibility of administering remote machines. QtSmbstatus was designed
as a client/server system secured with SSL. A login and password is required
to log on to server. The server's application ought to be installed on host
where SAMBA executes.


%prep
%setup -q -n qtsmbstatus-%{version}

%build
# create Makefile
qmake-qt4 server/server.pro -o server/Makefile

# build
make --directory=server/

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{_bindir}
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/pam.d
mkdir -p $RPM_BUILD_ROOT/%{_sysconfdir}/qtsmbstatusd
mkdir -p $RPM_BUILD_ROOT/%{_initrddir}
mkdir -p $RPM_BUILD_ROOT/%{_mandir}/man7

cp -a server/bin/qtsmbstatusd $RPM_BUILD_ROOT/%{_bindir}
cp -a server/pam.d/qtsmbstatusd $RPM_BUILD_ROOT/%{_sysconfdir}/pam.d/
cp -a server/etc/qtsmbstatusd $RPM_BUILD_ROOT/%{_initrddir}
cp -a server/etc/privkey.pem server/etc/qtsmbstatusd.conf \
        server/etc/server.pem server/etc/qtsmbstatusd.users \
        $RPM_BUILD_ROOT/%{_sysconfdir}/qtsmbstatusd/
#manpage
cp -a server/qtsmbstatusd.7.gz $RPM_BUILD_ROOT/%{_mandir}/man7/

#====[ postinstall :
%post

# service
chkconfig --add qtsmbstatusd
%{_initrddir}/qtsmbstatusd start

%clean
rm -rf $RPM_BUILD_ROOT

#====[ pre uninstall :
%preun
# If uninstall
if [ $1 = 0 ]; then
# stop qtsmbstatus server and remove service
%{_initrddir}/qtsmbstatusd stop
# service
chkconfig --del qtsmbstatusd 2>/dev/null
fi

%files
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

