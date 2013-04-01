/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <libgen.h>

#include "privatescreen.h"
#include "privatestackdebugger.h"

void
CompManager::usage ()
{
    printf ("Usage: %s [OPTIONS] [PLUGINS ...]\n"
            "Options:\n"
            "  --replace             Replace any existing window managers\n"
            "  --display DISPLAY     Connect to X display DISPLAY (instead of $DISPLAY)\n"
            "  --sm-disable          Disable session management\n"
            "  --sm-client-id ID     Session management client ID\n"
            "  --keep-desktop-hints  Retain existing desktop hints\n"
            "  --sync                Make all X calls synchronous\n"
            "  --debug               Enable debug mode\n"
            "  --version             Show the program version\n"
            "  --help                Show this summary\n"
            , programName);
}

static void
chldSignalHandler (int sig)
{
    int status;

    switch (sig) {
    case SIGCHLD:
	waitpid (-1, &status, WNOHANG | WUNTRACED);
	break;
    }
}

bool
CompManager::parseArguments (int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
	if (!strcmp (argv[i], "--help"))
	{
	    usage ();
	    return false;
	}
	else if (!strcmp (argv[i], "--version"))
	{
	    printf (PACKAGE_STRING "\n");
	    return false;
	}
	else if (!strcmp (argv[i], "--debug"))
	{
	    debugOutput = true;
	}
	else if (!strcmp (argv[i], "--sync"))
	{
	    synchronousX = true;
	}
	else if (!strcmp (argv[i], "--display"))
	{
	    if (i + 1 < argc)
		displayName = argv[++i];
	}
	else if (!strcmp (argv[i], "--keep-desktop-hints"))
	{
	    useDesktopHints = true;
	}
	else if (!strcmp (argv[i], "--replace"))
	{
	    replaceCurrentWm = true;
	}
	else if (!strcmp (argv[i], "--sm-disable"))
	{
	    disableSm = true;
	}
	else if (!strcmp (argv[i], "--sm-client-id"))
	{
	    if (i + 1 < argc)
		clientId = argv[++i];
	}
	else if (*argv[i] == '-')
	{
	    compLogMessage ("core", CompLogLevelWarn,
			    "Unknown option '%s'\n", argv[i]);
	}
	else
	{
	    initialPlugins.push_back (argv[i]);
	}
    }

    return true;
}

CompManager::CompManager () :
    disableSm (false),
    clientId (NULL),
    displayName (NULL)
{
}

bool
CompManager::init ()
{
    std::auto_ptr<CompScreenImpl> screen(new CompScreenImpl ());

    if (screen->createFailed ())
    {
	return false;
    }

    ::screen = screen.get();

    modHandler = new ModifierHandler ();

    if (!initialPlugins.empty ())
    {
	CompOption::Value::Vector list;
        CompOption::Value         value;
	CompOption                *o = screen->getOption ("active_plugins");

	foreach (CompString &str, initialPlugins)
	{
	    value.set (str);
	    list.push_back (value);
	}

	value.set (CompOption::TypeString, list);

	if (o)
	    o->set (value);
    }

    if (!screen->init (displayName))
	return false;

     if (!disableSm)
     {
	if (clientId == NULL)
	{
	    char *desktop_autostart_id = getenv ("DESKTOP_AUTOSTART_ID");
	    if (desktop_autostart_id != NULL)
		clientId = strdup (desktop_autostart_id);
	    unsetenv ("DESKTOP_AUTOSTART_ID");
 	}
 	CompSession::init (clientId);
     }

    screen.release ();

    return true;
}

void
CompManager::run ()
{
    screen->eventLoop ();
}

void
CompManager::fini ()
{
    if (!disableSm)
	CompSession::close ();

    StackDebugger::SetDefault (NULL);

    delete screen;
    delete modHandler;
}

/*
 * Try to detect the true bin directory compiz was run from and store it
 * in environment variable COMPIZ_BIN_PATH. If all else fails, don't define it.
 */
static void
detectCompizBinPath (char **argv)
{
    const char *bin = argv[0];
#ifdef __linux__
    char exe[PATH_MAX];
    ssize_t len = readlink ("/proc/self/exe", exe, sizeof(exe)-1);
    if (len > 0)
    {
	exe[len] = '\0';
	bin = exe;
    }
#endif
    if (strchr (bin, '/'))   // dirname needs a '/' to work reliably
    {
	// We need a private copy for dirname() to modify
	char *tmpBin = strdup (bin);
	if (tmpBin)
	{
	    const char *binDir = dirname (tmpBin);
	    if (binDir)
	    {
		char env[PATH_MAX];
		snprintf (env, sizeof(env)-1, "COMPIZ_BIN_PATH=%s/", binDir);
		putenv (strdup (env));  // parameter needs to be leaked!
	    }
	    free (tmpBin);
	}
    }
}

int
main (int argc, char **argv)
{
    CompManager		    manager;

    programName = argv[0];
    programArgc = argc;
    programArgv = argv;

    detectCompizBinPath (argv);

    signal (SIGCHLD, chldSignalHandler);

    if (!manager.parseArguments (argc, argv))
	return 0;

    if (!manager.init ())
	return 1;

    manager.run ();

    manager.fini ();

    if (restartSignal)
    {
	execvp (programName, programArgv);
	return 1;
    }

    return 0;
}
