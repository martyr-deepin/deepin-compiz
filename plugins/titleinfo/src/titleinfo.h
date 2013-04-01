/*
 *
 * Compiz title bar information extension plugin
 *
 * titleinfo.h
 *
 * Copyright : (C) 2009 by Danny Baumann
 * E-mail    : maniac@compiz.org
 *
 * Ported to Compiz 0.9 by:
 * Copyright : (C) 2009 Sam Spilsbury
 * E-mail    : smspillaz@gmail.com
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
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <cmath>
#include <unistd.h>
#include <cstring>
#include <X11/Xatom.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <core/core.h>
#include <core/pluginclasshandler.h>
#include <core/atoms.h>

#include "titleinfo_options.h"

class TitleinfoScreen :
    public PluginClassHandler <TitleinfoScreen, CompScreen>,
    public ScreenInterface,
    public TitleinfoOptions
{
    public:

	TitleinfoScreen (CompScreen *);
	~TitleinfoScreen ();

	Atom visibleNameAtom;
	Atom wmPidAtom;

	void
	handleEvent (XEvent *);


	void
	addSupportedAtoms (std::vector<Atom> &atoms);

	CompString
	getUtf8Property (Window      id,
			 Atom        atom);

	CompString
	getTextProperty (Window      id,
			 Atom        atom);


};

#define TITLEINFO_SCREEN(s)						       \
    TitleinfoScreen *ts = TitleinfoScreen::get (screen)

class TitleinfoWindow :
    public PluginClassHandler <TitleinfoWindow, CompWindow>
{
    public:
	TitleinfoWindow (CompWindow *);

	CompWindow *window;

	CompString title;
	CompString remoteMachine;
	int	   owner;

	void
	updateMachine ();

	void
	updateTitle ();

	void
	updatePid ();

	void
	updateVisibleName ();


};

#define TITLEINFO_WINDOW(w)						       \
     TitleinfoWindow *tw = TitleinfoWindow::get (w);

class TitleinfoPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <TitleinfoScreen, TitleinfoWindow>
{
    public:

	bool init ();
};
