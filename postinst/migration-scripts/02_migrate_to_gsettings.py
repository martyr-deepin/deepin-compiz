#!/usr/bin/python
# -*- coding: utf-8 -*-
# Copyright (C) 2012 Canonical
#
# Authors:
#  Łukasz 'sil2100' Zemczak <lukasz.zemczak@canonical.com>
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

import gconf
import glib
import subprocess
import os.path

# this should point to the directory where all the .convert files are stored
CONVERT_PATH = "/usr/lib/compiz/migration/"

def migrate_file(convert_file):
    subprocess.Popen(["gsettings-data-convert", "--file={}{}".format(CONVERT_PATH, convert_file)]).communicate()

def migrate_gconf_to_gsettings():
    client = gconf.client_get_default()

    if not client:
        print "WARNING: no gconf client found. No transitionning will be done"
        return

    # get current compiz profile to know if we need to switch or not
    # as compiz is setting that as a default key schema each time you
    # change the profile, the key isn't straightforward to get and set
    # as compiz set a new schema instead of a value..
    try:
        current_profile_schema = client.get_schema("/apps/compizconfig-1/current_profile")
    except (glib.GError, AttributeError):
        print "No current profile set, no migration needed"
        return

    if current_profile_schema:
        current_profile_gconfvalue = current_profile_schema.get_default_value()
        current_profile_str = current_profile_gconfvalue.get_string()
    else:
        print "No current profile set, no migration needed"
        return

    # depending what current profile is set, migrate using specific .convert files
    if current_profile_str == 'unity':
        print "Will migrate 'unity' as the active profile"
        migrate_file('compiz-profile-active-unity.convert')
        migrate_file('compiz-profile-Default.convert')
    else:
        print "Will migrate 'Default' as the active profile"
        migrate_file('compiz-profile-active-Default.convert')
        if os.path.exists(CONVERT_PATH + 'compiz-profile-unity.convert'):
            migrate_file('compiz-profile-unity.convert')

if __name__ == '__main__':
    migrate_gconf_to_gsettings ()
