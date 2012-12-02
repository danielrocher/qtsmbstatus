#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Written by Daniel Rocher <daniel.rocher@adella.org>
# Copyright (C) 2012

import sys, socket
from qtsmbstatus_pyserver._clientsocketssl import ClientSocketSSL

# requirement
try:
        import ssl
except ImportError:
        sys.stderr.write ("SSL/TLS is required. Please, install it before !\n")
        sys.exit(1)

if sys.hexversion < 0x030200F0:
        sys.stderr.write ("Python >= 3.2 is required !\n")
        sys.exit(1)


class ServerSSL(object):
    def __init__(self,certificate, privatekey, ssl_password="", port=8000, debug_mode=False):
        self.certificate=certificate
        self.privatekey=privatekey
        self.ssl_password=ssl_password
        self.port=port
        self.started=False
        self.bindsocket=0
        self.debug_mode=debug_mode

    def debug(self, msg):
        if self.debug_mode:
            print (msg)


    def isStarted(self):
        """return True if serveur's running""" 
        return self.started


    def createSocketHandler(self, ssl_sock):
        """Reimplement to customize. Return a ClientSocketSSL object"""
        return ClientSocketSSL(ssl_sock, self.debug_mode)


    def start(self):
        """start server"""
        
        if self.started:
            print ("Server's already started !")
            return # already started

        try:
            context = ssl.SSLContext(ssl.PROTOCOL_TLSv1)
        except:
            sys.stderr.write ("Impossible to create a new TLS/SSL context.\n")
            sys.exit(1)

        try:
            if sys.hexversion >= 0x030300F0: # Changed in version 3.3: New optional argument password
                context.load_cert_chain (certfile=self.certificate,keyfile=self.privatekey, password=self.ssl_password)
            else:
                context.load_cert_chain (certfile=self.certificate,keyfile=self.privatekey)
        except IOError as file:
            sys.stderr.write ("Impossible to load certificate and/or private key: '{}'\n".format(file))
            sys.exit(1)

        self.bindsocket = socket.socket()
        try:
            self.bindsocket.bind(('', self.port))
        except socket.error:
            sys.stderr.write ("Failed to bind to port {}\n".format(self.port))
            sys.exit(1)
        
        self.bindsocket.listen(1)
        self.debug  ("Server is started.")
        self.started=True

        while 1:
            if self.started==False:
                break
            newsocket, fromaddr = self.bindsocket.accept()
            self.debug ("incomingConnection: {0}, port {1}<->{2}".format(fromaddr[0], fromaddr[1], self.port))
            try:
                ssl_sock = context.wrap_socket(newsocket, server_side=True)
            except ssl.SSLError as err:
                sys.stderr.write ("SSL Error: {}\n".format(err))
                continue

            try:
                client=self.createSocketHandler(ssl_sock)
                client.start()
            except:
                sys.stderr.write ("Client socket error: {0}, port {1}<->{2}\n".format(fromaddr[0], fromaddr[1], self.port))
                continue


    def stop(self):
        """stop server"""
        if not self.started:
            return # already stopped
        self.started=False

        for key in list(ClientSocketSSL.client_collection):
            try:
                ClientSocketSSL.client_collection[key].stop()
            except:
                pass

        try:
            self.bindsocket.close()
        except:
            pass
        self.bindsocket=0
        self.debug  ("Server is stopped.")


if __name__ == "__main__":
    server=ServerSSL("cert.pem", "cert.pem", debug_mode=True)
    server.start()
