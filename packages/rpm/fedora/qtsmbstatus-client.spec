# SPECS - create RPM
#    by Daniel Rocher
# Thanks:
#    Mike A. Harris <mharris@redhat.com>
#    Stephane URBANOVSKI
#    Johan Cwiklinski

Summary:                A GUI for smbstatus with Qt
Name:                   qtsmbstatus-client
Version:                2.0.5
Release:                1%{?dist}
License:                GPLv2
Group:                  Applications/System
Vendor:                 ADELLA
Packager:               Daniel Rocher <daniel.rocher@adella.org>
URL:                    http://qtsmbstatus.free.fr
Source0:                http://qtsmbstatus.free.fr/files/qtsmbstatus-%{version}.tar.gz
BuildRoot:              %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:          gcc-c++ libX11-devel openssl-devel
BuildRequires:          qt4 >= 4.2 qt4-devel >= 4.2 desktop-file-utils
Requires:               openssl qt4 >= 4.2 qt4-x11 >= 4.2


%description
QtSmbstatus is a smbstatus graphical interface (GUI). It is meant to provide
the possibility of administering remote machines. QtSmbstatus was designed
as a client/server system secured with SSL. A login and password is required
to log on to server.


%prep
%setup -q -n qtsmbstatus-%{version}

%build
# create Makefile
qmake-qt4 client/client.pro -o client/Makefile

# Internationalization
lrelease-qt4 client/tr/*.ts

# build
make --directory=client/

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/%{_bindir}
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/qtsmbstatus
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/pixmaps
mkdir -p $RPM_BUILD_ROOT/%{_datadir}/applications
mkdir -p $RPM_BUILD_ROOT/%{_mandir}/man7

# bin
cp -a client/bin/qtsmbstatus $RPM_BUILD_ROOT/%{_bindir}
# translations
cp -a client/tr/*.qm $RPM_BUILD_ROOT/%{_datadir}/qtsmbstatus/
#icons
cp -a client/qtsmbstatus.xpm $RPM_BUILD_ROOT/%{_datadir}/pixmaps/
#menu
cp -a client/qtsmbstatus.desktop $RPM_BUILD_ROOT/%{_datadir}/applications/
desktop-file-install --vendor=""  \
       --dir=$RPM_BUILD_ROOT/%{_datadir}/applications qtsmbstatus.desktop
#manpage
cp -a client/qtsmbstatus.7.gz $RPM_BUILD_ROOT/%{_mandir}/man7/

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(755,root,root)
%{_bindir}/qtsmbstatus
%dir %{_datadir}/qtsmbstatus

%defattr(644,root,root)
%{_datadir}/qtsmbstatus/*.qm
%attr(-,root,root) %doc README README-* INSTALL COPYING changelog
%{_datadir}/pixmaps/*.xpm
%{_datadir}/applications/qtsmbstatus.desktop
%{_mandir}/man7/qtsmbstatus.7.gz

%changelog
* Sun Aug 31 2008 Daniel Rocher <daniel.rocher@adella.org> 2.0.5-1
- Initial release

