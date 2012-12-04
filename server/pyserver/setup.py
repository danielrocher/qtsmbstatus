#!/usr/bin/env python3
# -*- coding: utf-8 -*-


from distutils.core import setup
from distutils.command import install
import os, sys


class custom_install(install.install):
    def run(self):
        install.install.run(self)
        print("Run postinstall ...")
        pathname = format(os.path.dirname(sys.argv[0]))
        if os.system("{}/../postinst.sh".format(os.path.abspath(pathname)))!=0:
            print("postinstall failed !")

setup(name='qtsmbstatus_pyserver',
      version='2.3.0',
      description='Provides Qtsmbstatus Python Server',
      long_description='This package provides the server-side application for qtsmbstatus',
      license='GPL Version 2',
      author='Daniel Rocher',
      author_email='daniel.rocher@adella.org',
      url='http://qtsmbstatus.free.fr',
      download_url='http://qtsmbstatus.free.fr/index.php?page=downloads',
      packages=['qtsmbstatus_pyserver'],
      scripts=['qtsmbstatusd'],
      data_files=[('/etc/pam.d', ['../pam.d/qtsmbstatusd']),
                  ('/etc/qtsmbstatusd', ['../etc/privkey.pem', '../etc/qtsmbstatusd.conf', '../etc/qtsmbstatusd.users', '../etc/server.pem']),
                  ('/etc/init.d', ['../etc/qtsmbstatusd'])],
      cmdclass={ 'install': custom_install }
      )

