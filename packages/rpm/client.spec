# SPECS - create RPM
#    by Daniel Rocher
# Thanks:
#    Mike A. Harris <mharris@redhat.com>
#    Stephane URBANOVSKI


%define instaldir_bin  /usr/bin
%define instaldir_int  /usr/local/share/qtsmbstatus

# file .pro
%define qmake_pro      client/client.pro


Summary:		A GUI for smbstatus with Qt
Name:			qtsmbstatus-client
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
Requires:		openssl
Requires:		libqt4-core >=4.2, libqt4-gui >=4.2, libqt4-sql >=4.2, libqt4-qt3support >=4.2
%else
%if  "%{_distribution}" == "Suse"
# Suse requirements
BuildRequires:		gcc-c++, XFree86-devel
BuildRequires:		qt4-dev >= 4.2
BuildRequires:		openssl-devel
Requires:		openssl
Requires:		libqt4-core >=4.2, libqt4-gui >=4.2, libqt4-sql >=4.2, libqt4-qt3support >=4.2
%else
%if  "%{_distribution}" == "Fedora"
# Fedora requirements
BuildRequires:          gcc-c++,libX11-devel
BuildRequires:          qt4-devel >= 4.2
BuildRequires:          openssl-devel
Requires:		openssl
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

cp -a bin/qtsmbstatus $RPM_BUILD_ROOT/%{instaldir_bin}
cp -a client/tr/*.qm $RPM_BUILD_ROOT/%{instaldir_int}

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
%attr(-,root,root) %doc README README-FR INSTALL COPYING CHANGELOG

%changelog

