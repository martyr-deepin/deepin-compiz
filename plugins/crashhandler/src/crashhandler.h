/*
 *
 * Compiz crash handler plugin
 *
 * crashhandler.h
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

#include <core/core.h>
#include <core/pluginclasshandler.h>
#include "crashhandler_options.h"

#include <sys/prctl.h>

class CrashScreen :
    public PluginClassHandler<CrashScreen,CompScreen>,
    public CrashhandlerOptions
{
    public:

	CrashScreen (CompScreen *screen);
	~CrashScreen ();

	void optionChanged (CompOption                   *opt,
			    CrashhandlerOptions::Options num);
};

class CrashPluginVTable :
    public CompPlugin::VTableForScreen<CrashScreen>
{
    public:

	bool init ();
};
