#!/bin/bash

# Create PAM files for different Linux PAM kinds
# Install System-V style init script links
# Start service

ROOT_DIR=$1
PATH_PAM=$ROOT_DIR/etc/pam.d

echo $ROOT_DIR

if test -r $PATH_PAM/common-auth; then
      if test -s $ROOT_DIR/etc/debian_version; then
	# Debian systems
	cat <<-eof > $PATH_PAM/qtsmbstatusd
		#%PAM-1.0
		@include common-auth
		auth     required    pam_env.so
		auth     required    pam_listfile.so file=/etc/qtsmbstatusd/qtsmbstatusd.users onerr=fail sense=allow item=user
		@include common-account
		@include common-session
eof
	echo "Created Debian-style $PATH_PAM/qtsmbstatusd"
      else
	# SuSE systems
	cat <<-eof > $PATH_PAM/qtsmbstatusd
		#%PAM-1.0
		auth     include  common-auth
		auth     required pam_listfile.so file=/etc/qtsmbstatusd/qtsmbstatusd.users onerr=fail sense=allow item=user
		account  include  common-account
		password include  common-password
		session  include  common-session
eof
	echo "Created SuSE-style $PATH_PAM/qtsmbstatusd"
      fi
elif test -r $PATH_PAM/system-auth; then
	# Red Hat systems
	cat <<-eof > $PATH_PAM/qtsmbstatusd
		#%PAM-1.0
		auth      include     system-auth
		auth      required    pam_listfile.so file=/etc/qtsmbstatusd/qtsmbstatusd.users onerr=fail sense=allow item=user
		account   include     system-auth
		password  include     system-auth
		session   include     system-auth
eof
	echo "Created RedHat-style $PATH_PAM/qtsmbstatusd"
else
	# Old systems
	cat <<-eof > $PATH_PAM/qtsmbstatusd
		#
		# The PAM configuration file for the qtsmbstatusd daemon
		#
		
		auth 		required	pam_unix.so nullok
		auth		required	pam_listfile.so file=/etc/qtsmbstatusd/qtsmbstatusd.users onerr=fail sense=allow item=user
		account		required	pam_unix.so
		session		required	pam_unix.so
		password	required	pam_unix.so
eof
	echo "Created Default-Style (old systems) $PATH_PAM/qtsmbstatusd"
fi

chmod 640 $ROOT_DIR/etc/qtsmbstatusd/*
chmod 750 $ROOT_DIR/etc/init.d/qtsmbstatusd

# service

echo "install System-V style init script links"

if test -s $ROOT_DIR/etc/debian_version; then
	# Debian systems
	update-rc.d qtsmbstatusd start 20 2 3 5 . stop 20 0 6 .
	echo "Debian systems"
elif test -s $ROOT_DIR/etc/SuSE-release; then
	# SuSE systems
	insserv qtsmbstatusd
	echo "SuSE systems"
else
	# RedHat systems
	/sbin/chkconfig --add qtsmbstatusd
	echo "RedHat systems"
fi

# start service
echo "start qtsmbstatusd service"
$ROOT_DIR/etc/init.d/qtsmbstatusd start

echo "Done."