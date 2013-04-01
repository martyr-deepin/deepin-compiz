/*
 *
 * Compiz crash handler plugin
 *
 * crashhandler.cpp
 *
 * Copyright : (C) 2008 by Dennis Kasprzyk
 * E-mail    : onestone@compiz-fusion.org
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include <core/core.h>

#include "crashhandler.h"


COMPIZ_PLUGIN_20090315 (crashhandler, CrashPluginVTable)

static void
crash_handler (int sig)
{

#ifndef PR_SET_PTRACER
#define PR_SET_PTRACER 0x59616d61
#endif

    prctl (PR_SET_PTRACER, getpid (), 0, 0, 0);

    if (sig == SIGSEGV || sig == SIGFPE || sig == SIGILL || sig == SIGABRT)
    {
	CrashScreen *cs = CrashScreen::get (screen);
	static int  count = 0;

	if (++count > 1 || !cs)
	    exit (1);

	// backtrace
	char cmd[1024];

	snprintf (cmd, 1024,
		  "echo -e \"set prompt\nthread apply all bt full\n"
	      	  "echo \\\\\\n\necho \\\\\\n\nbt\nquit\" > /tmp/gdb.tmp;"
		  "gdb -q %s %i < /tmp/gdb.tmp | "
		  "grep -v \"No symbol table\" | "
		  "tee %s/compiz_crash-%i.out; rm -f /tmp/gdb.tmp; "
		  "echo \"\n[CRASH_HANDLER]: "
		  "\\\"%s/compiz_crash-%i.out\\\" created!\n\"",
		 programName, getpid (), cs->optionGetDirectory ().c_str (),
		 getpid (), cs->optionGetDirectory ().c_str (), getpid () );

	int ret = system (cmd);

	if (cs->optionGetStartWm ())
	{
	    if (fork () == 0)
	    {
		setsid ();
		putenv (const_cast <char *> (screen->displayString ()));
		execl ("/bin/sh", "/bin/sh", "-c",
		       cs->optionGetWmCmd ().c_str (), NULL);
		exit (0);
	    }
	}
	exit (ret ? ret : 1);
    }
}


void
CrashScreen::optionChanged (CompOption                   *opt,
			    CrashhandlerOptions::Options num)
{
    switch (num)
    {
	case CrashhandlerOptions::Enabled:
	    if (optionGetEnabled ())
	    {
		// enable crash handler
		signal (SIGSEGV, crash_handler);
		signal (SIGFPE, crash_handler);
		signal (SIGILL, crash_handler);
		signal (SIGABRT, crash_handler);
	    }
	    else
	    {
		// disable crash handler
		signal (SIGSEGV, SIG_DFL);
		signal (SIGFPE, SIG_DFL);
		signal (SIGILL, SIG_DFL);
		signal (SIGABRT, SIG_DFL);
	    }
	    break;
	default:
	    break;
    }
}


CrashScreen::CrashScreen (CompScreen *screen) :
    PluginClassHandler<CrashScreen,CompScreen> (screen),
    CrashhandlerOptions ()
{
    if (optionGetEnabled ())
    {
	// segmentation fault
	signal (SIGSEGV, crash_handler);
	// floating point exception
	signal (SIGFPE, crash_handler);
	// illegal instruction
	signal (SIGILL, crash_handler);
	// abort
	signal (SIGABRT, crash_handler);
    }

    optionSetEnabledNotify (
	boost::bind (&CrashScreen::optionChanged, this, _1, _2));
}

CrashScreen::~CrashScreen ()
{
    signal (SIGSEGV, SIG_DFL);
    signal (SIGFPE, SIG_DFL);
    signal (SIGILL, SIG_DFL);
    signal (SIGABRT, SIG_DFL);
}

bool
CrashPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	 return false;

    return true;
}
