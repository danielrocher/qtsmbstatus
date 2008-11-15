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
	update-rc.d qtsmbstatusd start 01 2 3 5 . stop 20 0 6 .
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