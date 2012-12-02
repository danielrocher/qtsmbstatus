#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Written by Daniel Rocher <daniel.rocher@adella.org>
# Copyright (C) 2012

import threading, subprocess, time, sys

class SmbManager(threading.Thread):
    """start smbstatus process"""
    processlock=threading.Lock()
    
    class __Cache:
        date=None
        stdout=None
        stderr=None
        lock=threading.Lock()

    def __init__(self, debug_mode=False, cache_timeout=60, callback=None):
        threading.Thread.__init__(self)
        self.debug_mode=debug_mode
        self.cache_timeout=cache_timeout
        self.eventTerminated = threading.Event()
        self.eventError = threading.Event()
        self.proc_timeout=15
        self.callback=callback

    def debug(self, msg):
        if self.debug_mode:
            print (msg)


    def cacheExpired(self):
        """Return True if Cache has expired"""
        
        SmbManager.__Cache.lock.acquire(timeout=self.proc_timeout)
        cache_expired=not (SmbManager.__Cache.date and time.time() < SmbManager.__Cache.date)
        SmbManager.__Cache.lock.release()
        return cache_expired


    def getStdOut(self):
        """Return a tuple with Std Out and Std Err"""
        
        if self.cacheExpired():
            self.process()
        else:
            self.debug ("SmbManager - Use cache - {}".format(self.getName()))

        return SmbManager.__Cache.stdout, SmbManager.__Cache.stderr

    def process(self):
        """Run smbstatus process"""
        SmbManager.processlock.acquire(timeout=self.proc_timeout)
        if not self.cacheExpired():
            SmbManager.processlock.release()
            return

        self.debug ("===== SmbManager - Run process {} =====".format(self.getName()))
        # reset flags
        self.eventTerminated.clear()
        self.eventError.clear()
        SmbManager.__Cache.lock.acquire(timeout=self.cache_timeout)
        SmbManager.__Cache.stdout=""
        SmbManager.__Cache.stderr=""
        SmbManager.__Cache.lock.release()

        try:
            proc = subprocess.Popen("smbstatus", stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        except OSError as e:
            self.error("process smbstatus error : {}".format(e))
            SmbManager.processlock.release()
            return
        except:
            self.error("process smbstatus error")
            SmbManager.processlock.release()
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

        # Update Cache
        SmbManager.__Cache.lock.acquire(timeout=self.cache_timeout)
        SmbManager.__Cache.date=time.time()+self.cache_timeout
        SmbManager.__Cache.stdout=stdout.decode("utf-8")
        SmbManager.__Cache.stderr=stderr.decode("utf-8")
        SmbManager.__Cache.lock.release()
        SmbManager.processlock.release()

        if proc.returncode > 0:
            self.error("process smbstatus error (exit code !=0): {}".format(SmbManager.__Cache.stderr))


    def error(self, msg):
        """an error occurs with the process"""
        print (msg)
        SmbManager.__Cache.lock.acquire(timeout=self.cache_timeout)
        SmbManager.__Cache.stderr=msg
        SmbManager.__Cache.lock.release()
        self.eventError.set()

    def run(self):
        if self.cacheExpired():
            self.process()
        if self.callback:
            self.callback(self.getStdOut())
        self.eventTerminated.set()



if __name__ == "__main__":
    def callback(a):
        print ("smbmanager1 - callback {}".format(a))

    # use callback
    smbmanager1=SmbManager(debug_mode=True, cache_timeout=30, callback=callback)
    
    # without callback
    smbmanager2=SmbManager(debug_mode=True, cache_timeout=30)
    smbmanager1.start()

    # Now, use cache    
    smbmanager2.start()
    print("smbmanager2 - use cache {}".format(smbmanager2.getStdOut()))
    time.sleep(30)
    # cache expired
    print("smbmanager2 - cache expired: {}".format(smbmanager2.getStdOut()))
    print("smbmanager2 - use cache {}".format(smbmanager2.getStdOut()))

