/*
 * Compiz Shelf plugin
 *
 * shelf.h
 *
 * Copyright (C) 2007  Canonical Ltd.
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
 * Author(s): 
 * Kristian Lyngstøl <kristian@bohemians.org>
 * Danny Baumann <maniac@opencompositing.org>
 * Sam Spilsbury <smspillaz@gmail.com>
 *
 * Description:
 *
 * This plugin visually resizes a window to allow otherwise obtrusive
 * windows to be visible in a monitor-fashion. Use case: Anything with
 * progress bars, notification programs, etc.
 *
 * Todo: 
 *  - Check for XShape events
 *  - Handle input in a sane way
 *  - Mouse-over?
 */

#include <X11/extensions/shape.h>
#include <X11/cursorfont.h>

#include <cmath>

#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>

#include "shelf_options.h"

class ShelfedWindowInfo {
    public:

	ShelfedWindowInfo (CompWindow *);
	~ShelfedWindowInfo ();

    public:
	CompWindow                *w;

	Window     ipw;

	XRectangle *inputRects;
	int        nInputRects;
	int        inputRectOrdering;

	XRectangle *frameInputRects;
	int        frameNInputRects;
	int        frameInputRectOrdering;
};

class ShelfWindow :
    public PluginClassHandler <ShelfWindow, CompWindow>,
    public WindowInterface,
    public CompositeWindowInterface,
    public GLWindowInterface
{
    public:
	ShelfWindow (CompWindow *);
	~ShelfWindow ();

	CompWindow *window;
	CompositeWindow *cWindow;
	GLWindow	*gWindow;

	float mScale;
	float targetScale;
	float steps;

	ShelfedWindowInfo *info;

	void
	moveNotify (int, int, bool);

	bool
	damageRect (bool, const CompRect &);

	bool
	glPaint (const GLWindowPaintAttrib &,
		 const GLMatrix		   &,
		 const CompRegion	   &,
		 unsigned int);

	void
	saveInputShape (XRectangle **rectRects,
			int        *retCount,
			int        *retOrdering);

	CompWindow *
	getRealWindow ();

	void
	shapeInput ();

	void
	unshapeInput ();

	void
	adjustIPW ();

	void
	createIPW ();

	bool
	handleShelfInfo ();

	void
	scale (float fScale);

	void
	handleButtonPress (unsigned int x,
			   unsigned int y);

	void
	handleButtonRelease ();

	void
	handleEnter (XEvent *event);
};

#define SHELF_WINDOW(w)							       \
	ShelfWindow *sw = ShelfWindow::get (w)

class ShelfScreen :
    public PluginClassHandler <ShelfScreen, CompScreen>,
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public ShelfOptions
{
    public:

	ShelfScreen (CompScreen *);
	~ShelfScreen ();

	CompositeScreen *cScreen;
	GLScreen	*gScreen;

	CompScreen::GrabHandle grabIndex;
	Window		       grabbedWindow;

	Cursor		       moveCursor;

	unsigned int	       lastPointerX;
	unsigned int	       lastPointerY;

	std::list <ShelfedWindowInfo *> shelfedWindows;

	void
	handleEvent (XEvent *);

	void
	preparePaint (int);

	bool
	glPaintOutput (const GLScreenPaintAttrib &,
		       const GLMatrix		 &,
		       const CompRegion		 &,
		       CompOutput		 *,
		       unsigned int);

	void
	donePaint ();

	void
	addWindowToList (ShelfedWindowInfo *info);

	void
	removeWindowFromList (ShelfedWindowInfo *info);

	void
	adjustIPWStacking ();

	void
	handleMotionEvent (unsigned int x,
			   unsigned int y);

	bool
	trigger (CompAction         *action,
		 CompAction::State  state,
		 CompOption::Vector options);

	bool
	reset (CompAction         *action,
	       CompAction::State  state,
	       CompOption::Vector options);

	bool
	triggerScreen (CompAction         *action,
		       CompAction::State  state,
		       CompOption::Vector options);

	bool
	inc (CompAction         *action,
	     CompAction::State  state,
	     CompOption::Vector options);

	bool
	dec (CompAction         *action,
	     CompAction::State  state,
	     CompOption::Vector options);

	CompWindow *
	findRealWindowID (Window wid);
};

#define SHELF_SCREEN(w)							       \
	ShelfScreen *ss = ShelfScreen::get (w)

extern const float SHELF_MIN_SIZE; // Minimum pixelsize a window can be scaled to


class ShelfPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <ShelfScreen, ShelfWindow>
{
    public:
	bool init ();
};
