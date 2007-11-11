#!/bin/bash
#
# Stop service
# Remove PAM files
# Remove System-V style init script links
# 

ROOT_DIR=$1
PATH_PAM=$ROOT_DIR/etc/pam.d

echo $ROOT_DIR

# stop service
echo "stop qtsmbstatusd service"
$ROOT_DIR/etc/init.d/qtsmbstatusd stop

if test -r $PATH_PAM/qtsmbstatusd; then
      rm $PATH_PAM/qtsmbstatusd -f 2>/dev/null
fi

# service

echo "remove System-V style init script links"

if test -s $ROOT_DIR/etc/debian_version; then
	# Debian systems
	update-rc.d -f qtsmbstatusd remove 2>/dev/null
	echo "Debian systems"
elif test -s $ROOT_DIR/etc/SuSE-release; then
	# SuSE systems
	insserv -r qtsmbstatusd 2>/dev/null
	echo "SuSE systems"
else
	# RedHat systems
	/sbin/chkconfig --del qtsmbstatusd 2>/dev/null
	echo "RedHat systems"
fi


echo "Done." 
