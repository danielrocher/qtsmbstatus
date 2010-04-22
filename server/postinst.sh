#!/bin/bash

# Install System-V style init script links
# Start service

ROOT_DIR=$1

chmod 640 $ROOT_DIR/etc/qtsmbstatusd/*
chmod 750 $ROOT_DIR/etc/init.d/qtsmbstatusd

# service

echo "install System-V style init script links"

if test -s $ROOT_DIR/etc/debian_version; then
	# Debian systems
	echo "Debian systems"
	update-rc.d -f qtsmbstatusd remove 2>/dev/null
	update-rc.d qtsmbstatusd defaults
elif test -s $ROOT_DIR/etc/SuSE-release; then
	# SuSE systems
	echo "SuSE systems"
	insserv -r qtsmbstatusd 2>/dev/null
	insserv qtsmbstatusd
else
	# RedHat systems
	echo "RedHat systems"
	/sbin/chkconfig --del qtsmbstatusd 2>/dev/null
	/sbin/chkconfig --add qtsmbstatusd
fi

# start service
echo "start qtsmbstatusd service"
$ROOT_DIR/etc/init.d/qtsmbstatusd start

echo "Done."
echo
echo "WARNING:"
echo "    PAM configuration file (/etc/pam.d/qtsmbstatusd) is distribution dependent."
echo "    Please read the README file."
echo
