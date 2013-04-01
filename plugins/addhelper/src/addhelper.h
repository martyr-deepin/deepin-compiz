/**
 * Compiz ADD Helper. Makes it easier to concentrate.
 *
 * addhelper.h
 *
 * Copyright (c) 2007 Kristian Lyngstøl <kristian@beryl-project.org>
 * Ported and highly modified by Patrick Niklaus <marex@beryl-project.org>
 * Ported to compiz 0.9 by Sam Spilsbury <smspillaz@gmail.com>
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
 * This plugin provides a toggle-feature that dims all but the active
 * window. This makes it easier for people with lousy concentration
 * to focus. Like me.
 * 
 * Please note any major changes to the code in this header with who you
 * are and what you did. 
 *
 */

#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>


#include "addhelper_options.h"

class AddScreen :
    public PluginClassHandler <AddScreen, CompScreen>,
    public ScreenInterface,
    public AddhelperOptions
{
    public:
	AddScreen (CompScreen *screen);

	CompositeScreen *cScreen;

	GLushort	    opacity;
	GLushort	    brightness;
	GLushort	    saturation;

	bool	    	    isToggle;

	void
	handleEvent (XEvent *event);

	void
	walkWindows ();

	bool
	toggle (CompAction         *action,
		CompAction::State  state,
		CompOption::Vector options);

	void
	optionChanged (CompOption                *options,
		       AddhelperOptions::Options num);


};

class AddWindow :
    public PluginClassHandler <AddWindow, CompWindow>,
    public GLWindowInterface
{
    public:
	AddWindow (CompWindow *window);
	~AddWindow ();

	CompWindow *window;
	CompositeWindow *cWindow;
	GLWindow   *gWindow;

	bool dim;

	bool
	glPaint (const GLWindowPaintAttrib &,
		 const GLMatrix		   &,
		 const CompRegion	   &,
		 unsigned int		   );
};

class AddPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <AddScreen, AddWindow>
{
    public:
	bool init ();
};

#define ADD_SCREEN(s)							       \
    AddScreen *as = AddScreen::get (s)

#define ADD_WINDOW(w)							       \
    AddWindow *aw = AddWindow::get (w)
