#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Written by Daniel Rocher <daniel.rocher@adella.org>
# Copyright (C) 2012

import threading, subprocess, time, sys
from qtsmbstatus_pyserver.process_smbd_exist import ProcessSmbdExist

class DisconnectManager(threading.Thread):
    """Kill a smb process (disconnect an user)"""
    def __init__(self, pid, user, debug_mode=False, callback=None):
        threading.Thread.__init__(self)
        self.pid=pid.strip()
        self.user=user.strip()
        self.stdout=""
        self.stderr=""
        self.debug_mode=debug_mode
        self.eventTerminated = threading.Event()
        self.eventError = threading.Event()
        self.proc_timeout=15
        self.callback=callback
        self.processSmbdExist=ProcessSmbdExist(pid, debug_mode, self.processSmbdExistCallback)

    def debug(self, msg):
        if self.debug_mode:
            print (msg)


    def process(self):
        """Run kill process"""
        self.debug ("===== DisconnectManager - Run process {} =====".format(self.getName()))

        try:
            int(self.pid)
        except:
            self.error("process kill error (pid must be an integer)")
            return
        try:
            proc = subprocess.Popen(["kill", "-15", self.pid], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        except OSError as e:
            self.error("process kill error : {}".format(e))
            return
        except:
            self.error("process kill error")
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


        if proc.returncode > 0:
            self.error("process kill error (exit code !=0): {}".format(self.stderr))


    def error(self, msg):
        """an error occurs with the process"""
        print (msg)
        self.stderr=msg
        self.eventError.set()

    def processSmbdExistCallback(self, datas):
        stdout, stderr, isSmbdProcess = datas

        if isSmbdProcess:
            self.process()

        if self.processSmbdExist.eventError.is_set():
            self.eventError.set()
            if self.stderr=="":
                self.stderr=stderr

        if self.callback:
            self.callback((self.stdout, self.stderr, self.user, self.eventError.is_set()))


    def run(self):
        self.processSmbdExist.start()
        self.processSmbdExist.join(5)
        self.eventTerminated.set()




if __name__ == "__main__":
    def callback(a):
        print ("disconnectmanager - callback {}".format(a))

    # use callback
    disconnectmanager=DisconnectManager(debug_mode=True, pid="8777", user="daniel", callback=callback)
    disconnectmanager.start()

