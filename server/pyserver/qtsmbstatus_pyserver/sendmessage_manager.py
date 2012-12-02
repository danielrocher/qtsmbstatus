#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Written by Daniel Rocher <daniel.rocher@adella.org>
# Copyright (C) 2012

import threading, subprocess, time, sys


class SendMessageManager(threading.Thread):
    """Send message popup (smbclient required)"""
    def __init__(self, machine, message, debug_mode=False, callback=None):
        threading.Thread.__init__(self)
        self.machine=machine.strip()
        self.message=message.strip()
        self.message=self.message.replace( "\"", " ").replace( "\\", " ")
        self.stdout=""
        self.stderr=""
        self.debug_mode=debug_mode
        self.eventTerminated = threading.Event()
        self.eventError = threading.Event()
        self.proc_timeout=15
        self.callback=callback

    def debug(self, msg):
        if self.debug_mode:
            print (msg)


    def process(self):
        """Run 'smbclient' process"""
        self.debug ("===== SendMessageManager - Run process {} =====".format(self.getName()))

        try:
            proc = subprocess.Popen(["sh", "-c", "echo {0} | smbclient -M  {1} -N".format(self.message, self.machine)], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        except OSError as e:
            self.error("process smbclient error : {}".format(e))
            return
        except:
            self.error("process smbclient error")
            return


        if sys.hexversion >= 0x030300F0:
            proc.wait(self.proc_timeout) # Changed in version 3.3: timeout was added.
        else:
            poll_seconds = .300
            deadline = time.time()+self.proc_timeout
            while time.time() < deadline and proc.poll() == None:
                time.sleep(poll_seconds)
            if proc.poll() == None:
                proc.terminate()

        stdout, stderr = proc.communicate()
        self.stdout=stdout.decode("utf-8")
        self.stderr=stderr.decode("utf-8")

        if "Cannot resolve" in self.stdout or " failed" in self.stdout or " ERRSRV" in self.stdout:
            self.error("Could not send message to {}".format(self.machine))

        if proc.returncode > 0:
            self.error("process smbclient error (exit code !=0): {}".format(self.stderr))


    def error(self, msg):
        """an error occurs with the process"""
        print (msg)
        self.stderr=msg
        self.eventError.set()

    def run(self):
        self.process()
        if self.callback:
            self.callback((self.stdout, self.stderr, self.machine, self.eventError.is_set()))
        self.eventTerminated.set()



if __name__ == "__main__":
    def callback(a):
        print ("sendmsgmanager - callback {}".format(a))

    # use callback
    sendmsgmanager=SendMessageManager("localhost", "hello", debug_mode=True, callback=callback)
    sendmsgmanager.start()

