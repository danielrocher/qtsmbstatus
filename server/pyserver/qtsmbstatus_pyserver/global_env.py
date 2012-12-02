#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Written by Daniel Rocher <daniel.rocher@adella.org>
# Copyright (C) 2012

import sys

class Global(object):
    #variables environments
    version_qtsmbstatus="2.3.0"
    int_qtsmbstatus_version=230
    date_qtsmbstatus="2012-12-01"
    author_qtsmbstatus="Daniel Rocher"
    mail_qtsmbstatus="<a href='mailto:daniel.rocher@adella.org'>daniel.rocher@adella.org</a>"
    web_qtsmbstatus="<a href='http://qtsmbstatus.free.fr'>http://qtsmbstatus.free.fr</a>"

    # default value
    debug_qtsmbstatus=False # view debug messages
    port_server=4443 # TCP port
    certificate="/etc/qtsmbstatusd/server.pem"
    privatekey="/etc/qtsmbstatusd/privkey.pem"
    ssl_password=""
    daemonize=False
    # Allow client to disconnect user
    AllowUserDisconnect=[]
    # Allow client to send message
    AllowUserSendMsg=[]

    @staticmethod
    def debug(msg):
        """print a debug message"""
        if Global.debug_qtsmbstatus:
            print (msg)

    @staticmethod
    def error(msg):
        """print an error message"""
        sys.stderr.write("\n{}\n".format(msg))



if __name__ == "__main__":
    print ("Version of pyQtSmbstatusd: {0}\nDate: {1}".format(Global.version_qtsmbstatus, Global.date_qtsmbstatus))
    pass
