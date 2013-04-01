#!/usr/bin/python
# -*- coding: utf-8 -*-
# Copyright (C) 2012 Canonical
#
# Authors:
#  Didier Roche <didrocks@ubuntu.com>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; version 3.
#
# This program is distributed in the hope that it will be useful, but WITHOUTa
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

from gi.repository import Gio
import os

gsettings=Gio.Settings(schema="org.compiz.core", path="/org/compiz/profiles/Default/plugins/core/")
plugins_list = gsettings.get_strv("active-plugins")

# Remove unityshell the default profile if present (case of new install and first session startup)
try:
    plugins_list.remove('unityshell')
except ValueError:
    pass
try:
    plugins_list.remove('unitymtgrabhandles')
except ValueError:
    pass
# gsettings doesn't work directly, the key is somewhat reverted. Work one level under then: dconf!
#gsettings.set_strv("active-plugins", plugins_list)
from subprocess import Popen, PIPE, STDOUT
p = Popen(['dconf', 'load', '/org/compiz/profiles/Default/plugins/core/'], stdout=PIPE, stdin=PIPE, stderr=STDOUT)
p.communicate(input="[/]\nactive-plugins=%s" % plugins_list)

# remove the eventually existing local config file which can prevent the default profile to use gsettings for quantal users before this fix
try:
    os.remove(os.path.join(os.path.expanduser('~'), '.config', 'compiz-1', 'compizconfig', 'config'))
except OSError:
    pass

