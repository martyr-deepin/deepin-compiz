/*
 * Copyright (c) 2009 Dennis Kasprzyk <onestone@compiz.org>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "kde.h"

#include <KDE/KCmdLineArgs>

COMPIZ_PLUGIN_20090315 (kde, KdePluginVTable);

typedef int (*X11ErrorHandlerProc)(Display *, XErrorEvent *);

KdeScreen::KdeScreen (CompScreen *screen) :
    PluginClassHandler <KdeScreen, CompScreen> (screen)
{
    mEventDispatcher = new EventDispatcherCompiz ();
    argv[0] = strdup ("compiz");
    argc = 1;
    KCmdLineArgs::init (argc, argv, "compiz", "compiz",
                        ki18n ("Compiz KDE event loop plugin"), "0.0.1");

    // Save the compiz error handler
    X11ErrorHandlerProc er = XSetErrorHandler(NULL);

    mApp = new KApplication ();

    // Restore our error handler
    XSetErrorHandler(er);
}

KdeScreen::~KdeScreen ()
{
    // Save the compiz error handler
    X11ErrorHandlerProc er = XSetErrorHandler(NULL);

    delete mApp;
    delete mEventDispatcher;

    // Restore our error handler
    XSetErrorHandler(er);

    free (argv[0]);
}

bool
KdePluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    return true;
}
