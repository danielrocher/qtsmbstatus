#!/usr/bin/env python3
# -*- coding: utf-8 -*-


from distutils.core import setup


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
      scripts=['qtsmbstatusd']
      )

