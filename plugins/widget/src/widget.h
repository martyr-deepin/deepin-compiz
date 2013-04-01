/*
 *
 * Compiz widget handling plugin
 *
 * widget.c
 *
 * Copyright : (C) 2007 by Danny Baumann
 * E-mail    : maniac@opencompositing.org
 *
 * Ported to Compiz 0.9 by:
 * Copyright : (C) 2008 Sam Spilsbury
 * E-mail    : smspillaz@gmail.com
 *
 * Idea based on widget.c:
 * Copyright : (C) 2006 Quinn Storm
 * E-mail    : livinglatexkali@gmail.com
 *
 * Copyright : (C) 2007 Mike Dransfield
 * E-mail    : mike@blueroot.co.uk
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
#include <core/atoms.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <X11/cursorfont.h>
#include "widget_options.h"

class WidgetScreen :
    public PluginClassHandler <WidgetScreen, CompScreen>,
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public WidgetOptions
{
    public:

	enum WidgetState
	{
	    StateOff = 0,
	    StateFadeIn = 1,
	    StateOn = 2,
	    StateFadeOut = 3
	};

    public:
	WidgetScreen (CompScreen *screen);
	~WidgetScreen ();

	CompositeScreen *cScreen;

	void
	handleEvent (XEvent *event);

	void
	matchPropertyChanged (CompWindow *w);

	void
	matchExpHandlerChanged ();

	CompMatch::Expression *
	matchInitExp (const CompString &value);

	void
	preparePaint (int);

	void
	donePaint ();

	void
	setWidgetLayerMapState (bool map);

	bool
	registerExpHandler ();

	bool
	toggle (CompAction         *action,
		CompAction::State  state,
		CompOption::Vector &options);

	void
	endWidgetMode (CompWindow *closedWidget);

	bool
	updateStatus (CompWindow *w);

	void
	optionChanged (CompOption *,
		       WidgetOptions::Options  num);

	Window mLastActiveWindow;
	Atom   mCompizWidgetAtom;

	WidgetState mState;
	int	    mFadeTime;
	CompScreen::GrabHandle  mGrabIndex;
	Cursor	    mCursor;
};

#define WIDGET_SCREEN(screen)						       \
    WidgetScreen *ws = WidgetScreen::get (screen)

class WidgetWindow :
    public PluginClassHandler <WidgetWindow, CompWindow>,
    public WindowInterface,
    public GLWindowInterface
{
    public:

	enum WidgetPropertyState
	{
	    PropertyNotSet = 0,
	    PropertyWidget,
	    PropertyNoWidget
	};

    public:

	WidgetWindow (CompWindow *w);
	~WidgetWindow ();

	CompWindow *window;
	GLWindow *gWindow;

	bool
	glPaint (const GLWindowPaintAttrib &, const GLMatrix &,
		 const CompRegion &, unsigned int);

	bool
	focus ();

	void
	updateTreeStatus ();

	bool
	updateWidgetStatus ();

	bool
	updateWidgetPropertyState ();

	void
	updateWidgetMapState (bool map);

	bool
	updateMatch ();

	bool
	managed ();

	bool mIsWidget;
	bool mWasHidden;
	CompWindow *mParentWidget;
	CompTimer  mMatchUpdate;
	CompTimer  mWidgetStatusUpdate;
	WidgetPropertyState mPropertyState;
};

#define WIDGET_WINDOW(window)						       \
    WidgetWindow *ww = WidgetWindow::get (window)

class WidgetPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <WidgetScreen, WidgetWindow>
{
    public:

	bool init ();
};
