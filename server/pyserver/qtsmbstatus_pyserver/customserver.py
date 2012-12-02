#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Written by Daniel Rocher <daniel.rocher@adella.org>
# Copyright (C) 2012

from qtsmbstatus_pyserver.serverssl import ServerSSL
from qtsmbstatus_pyserver.customclientsocket import CustomClientSocket


class CustomServer(ServerSSL):
    def __init__(self,certificate, privatekey, ssl_password="", port=4443, debug_mode=False):
        ServerSSL.__init__(self, certificate, privatekey, ssl_password, port, debug_mode)

    def createSocketHandler(self, ssl_sock):
        """create a new client socket"""
        return CustomClientSocket(ssl_sock, self.debug_mode)


if __name__ == "__main__":
    server=CustomServer("cert.pem", "cert.pem", port=4443, debug_mode=True)
    server.start()
