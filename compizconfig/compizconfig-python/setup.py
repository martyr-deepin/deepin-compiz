# -*- coding: utf-8 -*-
from distutils.core import setup, Command
from distutils.command.build import build as _build
from distutils.command.install import install as _install
from distutils.command.install_data import install_data as _install_data
from distutils.command.sdist import sdist as _sdist
from distutils.extension import Extension
import os
import subprocess
import sys
import unittest
pkg_config_environ = os.environ
pkg_config_environ["PKG_CONFIG_PATH"] = os.getcwd () + "/../libcompizconfig:" + os.environ.get ("PKG_CONFIG_PATH", '')

from distutils.command.build_ext import build_ext
ext_module_src = os.getcwd () + "/compizconfig.c"

def pkgconfig(*packages, **kw):
    flag_map = {'-I': 'include_dirs', '-L': 'library_dirs', '-l': 'libraries', '-R': 'runtime_library_dirs'}
    cmd = ['pkg-config', '--libs', '--cflags']

    tokens = subprocess.Popen (cmd + list(packages), stdout=subprocess.PIPE, env=pkg_config_environ).communicate()[0].split ()

    for t in tokens:
        if '-L' in t[:2]:
            kw.setdefault (flag_map.get ("-L"), []).append (t[2:])
        elif '-I' in t[:2]:
            kw.setdefault (flag_map.get ("-I"), []).append (t[2:])
        elif '-l' in t[:2]:
            kw.setdefault (flag_map.get ("-l"), []).append (t[2:])
    
    return kw

for arg in sys.argv:
    if "--version" in arg:
        VERSION = arg.split ("=")[1]

pkgconfig_libs = subprocess.Popen (["pkg-config", "--libs", "libcompizconfig_internal"], stdout=subprocess.PIPE, env=pkg_config_environ, stderr=open(os.devnull, 'w')).communicate ()[0]

if len (pkgconfig_libs) is 0:
  print ("CompizConfig Python [ERROR]: No libcompizconfig_internal.pc found in the pkg-config search path")
  print ("Ensure that libcompizonfig is installed or libcompizconfig.pc is in your $PKG_CONFIG_PATH")
  exit (1);
libs = pkgconfig_libs[2:].split (" ")[0]

INSTALLED_FILES = "installed_files"

class build (_build):

    user_options = _build.user_options[:]
    user_options.extend ([('version=', None, "Version of the package")])

    def initialize_options(self):
        self.version = None
        _build.initialize_options (self)

    def finalize_options(self):
        _build.finalize_options (self)

class install (_install):

    user_options = _install.user_options[:]
    user_options.extend ([('version=', None, "Version of the package")])

    def initialize_options(self):
        self.version = None
        _install.initialize_options (self)

    def finalize_options(self):
        _install.finalize_options (self)

    def run (self):
        _install.run (self)
        outputs = self.get_outputs ()
        length = 0
        if self.root:
            length += len (self.root)
        if self.prefix:
            length += len (self.prefix)
        if length:
            for counter in xrange (len (outputs)):
                outputs[counter] = outputs[counter][length:]
        data = "\n".join (outputs)
        try:
            file = open (INSTALLED_FILES, "w")
        except:
            self.warn ("Could not write installed files list %s" % \
                       INSTALLED_FILES)
            return 
        file.write (data)
        file.close ()

class install_data (_install_data):

    def run (self):
        def chmod_data_file (file):
            try:
                os.chmod (file, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
            except:
                self.warn ("Could not chmod data file %s" % file)
        _install_data.run (self)
        map (chmod_data_file, self.get_outputs ())

class uninstall (_install):

    def run (self):
        try:
            file = open (INSTALLED_FILES, "r")
        except:
            self.warn ("Could not read installed files list %s" % \
                       INSTALLED_FILES)
            return 
        files = file.readlines ()
        file.close ()
        prepend = ""
        if self.root:
            prepend += self.root
        if self.prefix:
            prepend += self.prefix
        if len (prepend):
            for counter in xrange (len (files)):
                files[counter] = prepend + files[counter].rstrip ()
        for file in files:
            print ("Uninstalling %s" % file)
            try:
                os.unlink (file)
            except:
                self.warn ("Could not remove file %s" % file)

class test (Command):
    description = "run tests"
    user_options = []

    def initialize_options (self):
	self.cwd = None

    def finalize_options (self):
	self.cwd = os.getcwd ()

    def run (self):
	assert os.getcwd () == self.cwd, 'Must be in package root: %s' % self.cwd
	loader = unittest.TestLoader ()

	tests = loader.discover (os.getcwd (), pattern='test*')
	result = unittest.TestResult ()

	unittest.TextTestRunner (verbosity=2).run (tests)


setup (
  name = "compizconfig-python",
  version = VERSION,
  description      = "CompizConfig Python",
  url              = "https://launchpad.net/compiz",
  license          = "GPL",
  maintainer	   = "Guillaume Seguin",
  maintainer_email = "guillaume@segu.in",
  cmdclass         = {"uninstall" : uninstall,
                      "install" : install,
                      "install_data" : install_data,
                      "build"     : build,
                      "build_ext" : build_ext,
		      "test"  : test},
  ext_modules=[ 
    Extension ("compizconfig", [ext_module_src],
	       **pkgconfig("libcompizconfig_internal"))
    ]
)

