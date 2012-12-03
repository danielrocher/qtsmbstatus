#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Written by Daniel Rocher <daniel.rocher@adella.org>
# Copyright (C) 2012

import threading, socket


class ClientSocketSSL(threading.Thread):
    """This class dialogs with client. (One object/client)"""
    counter=0
    lock=threading.Lock()
    client_collection={}

    def __init__(self, conn, debug_mode=False):
        threading.Thread.__init__(self)
        ClientSocketSSL.lock.acquire()
        ClientSocketSSL.counter+=1
        ClientSocketSSL.lock.release()
        self.connection=conn
        self.eventTerminated = threading.Event()
        self.lockStoppingSocket=threading.Lock()
        self.debug_mode=debug_mode
        self.debug ("number of ClientSocketSSL: {}".format(ClientSocketSSL.counter))

    def debug(self, msg):
        if self.debug_mode:
            print (msg)


    def readyReadHandler(self, data):
        """Reimplement in order to read data."""
        print ("'readyReadHandler' is not implemented yet. Reimplement in order to read data.\nData: {}".format(data))
        self.sendDatas("you sent: {}".format(data))

    def sendDatas(self, data):
        """Send datas to server"""
        if self.eventTerminated.is_set(): return
        self.debug("ClientSocketSSL.sendDatas()")
        try:
            self.connection.send("{}".format(data).encode("utf-8"))
        except socket.error:
             self.debug('socket connection broken')

    def connectedHandler(self):
        """Reimplement me"""
        pass # nothing to do here
        

    def run(self):
        self.debug("Create new Thread: {}".format(self.getName()))
        ClientSocketSSL.client_collection[self.getName()]=self
        self.connectedHandler()

        while 1:
            if self.eventTerminated.is_set(): break
            try:
                data= self.connection.recv(1024)
            except socket.error:
                self.debug('socket connection broken')
                break
            if not data: break
            string=data.decode("utf-8")
            self.readyReadHandler(string)

        self.stop() # close socket properly


    def stop(self):
        """stop connection with client"""
        self.lockStoppingSocket.acquire()
        if self.eventTerminated.is_set():
            self.lockStoppingSocket.release()
            return # already stopped
        self.eventTerminated.set()
        self.lockStoppingSocket.release()
        self.debug ("stopping connection with client {0} - {1}".format(self.connection.getpeername(),self.getName()))
        self.connection.shutdown(socket.SHUT_RDWR)
        self.connection.close()
        ClientSocketSSL.lock.acquire()
        ClientSocketSSL.counter-=1
        ClientSocketSSL.lock.release()
        self.debug ("number of ClientSocketSSL: {}".format(ClientSocketSSL.counter))
        del (ClientSocketSSL.client_collection[self.getName()])


if __name__ == "__main__":
    pass

