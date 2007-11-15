# SPECS - create RPM
#    by Daniel Rocher
# Thanks:
#    Mike A. Harris <mharris@redhat.com>
#    Stephane URBANOVSKI


%define instaldir_bin  /usr/bin
%define instaldir_int  /usr/share/qtsmbstatus
%define instaldir_icons /usr/share/pixmaps/
%define instaldir_man /usr/local/share/man/man7/
%define instaldir_menu /usr/share/applications/

# file .pro
%define qmake_pro      client/client.pro

%define is_mandrake %(test -e /etc/mandrake-release && echo 1 || echo 0)
%define is_suse %(test -e /etc/SuSE-release && echo 1 || echo 0)
%define is_fedora %(test -e /etc/fedora-release && echo 1 || echo 0)

%if %is_mandrake
%define distribution Mandrake
%endif

%if %is_suse
%define distribution Suse
%endif

%if %is_fedora
%define distribution Fedora
%endif

Summary:		A GUI for smbstatus with Qt
Name:			qtsmbstatus-client
Version:		2.0.2
Release:		1
License:		GPL
Vendor:			ADELLA
Packager:		Daniel ROCHER <daniel.rocher@adella.org>
Group:			Networking/Other
URL:			http://qtsmbstatus.free.fr
Source0:		%{name}-%{version}.tar.gz
BuildRoot:		%{_tmppath}/%{name}-buildroot
BuildArchitectures:	i586

%if "%{distribution}" == "Mandriva" || "%{distribution}" == "Mandrake"
# Mandrake / Mandriva requirements
BuildRequires:		gcc-c++, XFree86-devel
BuildRequires:		libqt4-dev >= 4.2
BuildRequires:		libopenssl-devel
Requires:		openssl
Requires:		libqt4-core >= 4.2, libqt4-gui >= 4.2, libqt4-sql >= 4.2, libqt4-qt3support >= 4.2
%else
%if  "%{distribution}" == "Suse"
# Suse requirements
BuildRequires:		gcc-c++, XFree86-devel
BuildRequires:		libqt4-devel >= 4.2
BuildRequires:		openssl-devel
Requires:		openssl
Requires:		libqt4 >= 4.2, libqt4-x11 >= 4.2, libqt4-sql >= 4.2, libqt4-qt3support >= 4.2
%else
%if  "%{distribution}" == "Fedora"
# Fedora requirements
BuildRequires:          gcc-c++,libX11-devel
BuildRequires:          qt4 >= 4.2, qt4-devel >= 4.2
BuildRequires:          openssl-devel
Requires:		openssl
Requires:		qt4 >= 4.2, qt4-x11 >= 4.2
%else
#Unknown Distribution !
%endif
%endif
%endif

%description
QtSmbstatus is a smbstatus graphical interface (GUI). It is meant to provide
the possibility of administering remote machines. QtSmbstatus was designed
as a client/server system secured with SSL. A login and password is required
to log on to server.


%prep
# delete temporary files
rm %{_builddir}/* -r -f
rm %{_tmppath}/* -r -f

%setup -q

%build
# create Makefile
qmake-qt4 %{qmake_pro}

lrelease-qt4 %{qmake_pro}

# build
make

%install

mkdir -p $RPM_BUILD_ROOT/%{instaldir_bin}
mkdir -p $RPM_BUILD_ROOT/%{instaldir_int}
mkdir -p $RPM_BUILD_ROOT/%{instaldir_icons}
mkdir -p $RPM_BUILD_ROOT/%{instaldir_menu}
mkdir -p $RPM_BUILD_ROOT/%{instaldir_man}

# bin
cp -a bin/qtsmbstatus $RPM_BUILD_ROOT/%{instaldir_bin}
# translations
cp -a client/tr/*.qm $RPM_BUILD_ROOT/%{instaldir_int}
#icons
cp -a client/qtsmbstatus.xpm $RPM_BUILD_ROOT/%{instaldir_icons}
#menu
cp -a client/qtsmbstatus.desktop $RPM_BUILD_ROOT/%{instaldir_menu}
#manpage
cp -a client/qtsmbstatus.7.gz $RPM_BUILD_ROOT/%{instaldir_man}

%clean

%preun

%postun
if [ $1 = 0 ]; then
# If uninstall, remove directory
test -d $RPM_BUILD_ROOT/%{instaldir_int} && rmdir $RPM_BUILD_ROOT/%{instaldir_int} 2>/dev/null
fi

%files
# defaults attr
%defattr(755,root,root)
%{instaldir_bin}/qtsmbstatus

# defaults attr
%defattr(644,root,root)
%{instaldir_int}/*.qm
%attr(-,root,root) %doc README README-* INSTALL COPYING changelog
%{instaldir_icons}/*.xpm
%{instaldir_menu}/*.desktop
%{instaldir_man}/qtsmbstatus.7.gz

%changelog
* Wed Nov 14 2007 - daniel.rocher (at) adella.org
- Initial release
