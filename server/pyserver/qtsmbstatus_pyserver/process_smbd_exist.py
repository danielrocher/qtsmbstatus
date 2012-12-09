#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Written by Daniel Rocher <daniel.rocher@adella.org>
# Copyright (C) 2012

import threading, subprocess, time, sys


class ProcessSmbdExist(threading.Thread):
    """test if PID it's a correct smbd process"""
    def __init__(self, pid, debug_mode=False, callback=None):
        threading.Thread.__init__(self)
        self.pid=pid.strip()
        self.stdout=""
        self.stderr=""
        self.isSmbdProcess=False
        self.debug_mode=debug_mode
        self.eventTerminated = threading.Event()
        self.eventError = threading.Event()
        self.proc_timeout=15
        self.callback=callback

    def debug(self, msg):
        if self.debug_mode:
            print (msg)


    def process(self):
        """Run 'ps' process"""
        self.debug ("===== ProcessSmbdExist - Run process {} =====".format(self.getName()))

        try:
            int(self.pid)
        except:
            self.error("process ps error (pid must be an integer)")
            return
        try:
            proc = subprocess.Popen(["ps", "-f", self.pid], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        except OSError as e:
            self.error("process ps error : {}".format(e))
            return
        except:
            self.error("process ps error")
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

        if self.pid in self.stdout and "smbd" in self.stdout:
            # It's a correct process
            self.debug("ProcessSmbdExist: process is smbd")
            self.isSmbdProcess=True

        if proc.returncode > 0:
            self.error("process ps error (exit code !=0): {}".format(self.stderr))


    def error(self, msg):
        """an error occurs with the process"""
        print (msg)
        self.stderr=msg
        self.eventError.set()

    def run(self):
        self.process()
        if self.callback:
            self.callback((self.stdout, self.stderr, self.isSmbdProcess))
        self.eventTerminated.set()



if __name__ == "__main__":
    def callback(a):
        print ("processsmbdexist - callback {}".format(a))

    # use callback
    processsmbdexist=ProcessSmbdExist(debug_mode=True, pid="7624", callback=callback)
    processsmbdexist.start()

