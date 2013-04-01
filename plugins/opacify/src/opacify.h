/**
 * Compiz Opacify 
 *
 * opacify.h
 *
 * Copyright (c) 2006 Kristian Lyngstøl <kristian@beryl-project.org>
 * Ported to Compiz and BCOP usage by Danny Baumann <maniac@beryl-project.org>
 * Ported to Compiz 0.9 by Sam Spilsbury <smspillaz@gmail.com>
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
 *
 * Opacify increases opacity on targeted windows and reduces it on
 * blocking windows, making whatever window you are targeting easily
 * visible. 
 *
 */

#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>

#include "opacify_options.h"

/* Size of the Window array storing passive windows. */
extern const unsigned short MAX_WINDOWS;

class OpacifyScreen :
    public PluginClassHandler <OpacifyScreen, CompScreen>,
    public OpacifyOptions,
    public ScreenInterface
{
    public:
	OpacifyScreen (CompScreen *);

	CompositeScreen *cScreen;
	GLScreen	*gScreen;

	bool isToggle;

	CompTimer timeoutHandle;

	CompWindow *newActive;

	Window active;
	std::vector<Window> passive;
	CompRegion intersect;
	unsigned short int passiveNum;

	bool justMoved;

	void
	handleEvent (XEvent *);

	void
	resetWindowOpacity (Window  id);

	void
	resetScreenOpacity ();

	void
	clearPassive ();

	int
	passiveWindows (CompRegion     fRegion);

	bool
	handleTimeout ();

	bool
	checkDelay ();

	bool
	toggle (CompAction         *action,
		CompAction::State  state,
		CompOption::Vector options);

	void
	optionChanged (CompOption              *option,
		       OpacifyOptions::Options num);
		       
	bool
	checkStateTimeout ();
};

class OpacifyWindow :
    public PluginClassHandler <OpacifyWindow, CompWindow>,
    public GLWindowInterface
{
    public:

	OpacifyWindow (CompWindow *);

	CompWindow      *window;
	CompositeWindow *cWindow;
	GLWindow	*gWindow;

	bool opacified;
	int opacity;

	bool
	glPaint (const GLWindowPaintAttrib &,
		 const GLMatrix &,
		 const CompRegion &,
		 unsigned int);

	void
	setOpacity (int fOpacity);

	void
	dim ();

	void
	handleEnter ();



};

#define OPACIFY_SCREEN(s)						       \
    OpacifyScreen *os = OpacifyScreen::get (s);

#define OPACIFY_WINDOW(w)						       \
    OpacifyWindow *ow = OpacifyWindow::get (w);

class OpacifyPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <OpacifyScreen, OpacifyWindow>
{
    public:

	bool init ();
}; 
