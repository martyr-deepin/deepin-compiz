/*
 *
 * Compiz scale plugin addon plugin
 *
 * scaleaddon.h
 *
 * Copyright : (C) 2007 by Danny Baumann
 * E-mail    : maniac@opencompositing.org
 *
 * Organic scale mode taken from Beryl's scale.c, written by
 * Copyright : (C) 2006 Diogo Ferreira
 * E-mail    : diogo@underdev.org
 *
 * Ported to Compiz 0.9 by:
 * Copyright : (C) 2009 by Sam Spilsbury
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
 *
 */

#include <cmath>
#include <cstring>
#include <X11/Xatom.h>

#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <text/text.h>
#include <scale/scale.h>

#include "scaleaddon_options.h"

class ScaleAddonScreen :
    public PluginClassHandler <ScaleAddonScreen, CompScreen>,
    public ScreenInterface,
    public CompositeScreenInterface,
    public ScaleScreenInterface,
    public ScaleaddonOptions
{
    public:
	ScaleAddonScreen (CompScreen *);

	CompositeScreen *cScreen;
	ScaleScreen	*sScreen;

	Window		highlightedWindow;
	Window		lastHighlightedWindow;

	int		lastState;

	float		scale;
	
	std::vector <ScaleSlot> paintSlots;

	void
	handleEvent (XEvent *);

	bool
	layoutSlotsAndAssignWindows ();
	
	bool
	layoutNaturalThumbs ();

	bool
	isOverlappingAny (ScaleWindow *w,
			  const std::map <ScaleWindow *, CompRegion>,
			  const CompRegion &border);

	void
	donePaint ();

	void
	checkWindowHighlight ();

	bool
	closeWindow (CompAction         *action,
		     CompAction::State  state,
		     CompOption::Vector options);

	bool
	pullWindow (CompAction         *action,
		    CompAction::State  state,
		    CompOption::Vector options);

	bool
	zoomWindow (CompAction         *action,
		    CompAction::State  state,
		    CompOption::Vector options);

	void
	handleCompizEvent (const char         *pluginName,
			   const char         *eventName,
			   CompOption::Vector &options);

	void
	optionChanged (CompOption                 *opt,
		       ScaleaddonOptions::Options num);
};

#define ADDON_SCREEN(s)				     \
     ScaleAddonScreen *as = ScaleAddonScreen::get (s)

class ScaleAddonWindow :
    public PluginClassHandler <ScaleAddonWindow, CompWindow>,
    public ScaleWindowInterface
{
    public:
	ScaleAddonWindow (CompWindow *);

	CompWindow *window;
	ScaleWindow *sWindow;
	CompositeWindow *cWindow;

	ScaleSlot    origSlot;
	CompText     text;

	bool	     rescaled;

	CompWindow   *oldAbove;

	void
	scalePaintDecoration (const GLWindowPaintAttrib &,
			      const GLMatrix &,
			      const CompRegion &,
			      unsigned int);

	void
	scaleSelectWindow ();

	void
	renderTitle ();

	void
	drawTitle (const GLMatrix &transform);

	void
	drawHighlight (const GLMatrix &transform);
};

#define ADDON_WINDOW(w)				     \
     ScaleAddonWindow *aw = ScaleAddonWindow::get (w)


class ScaleAddonPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <ScaleAddonScreen,
						 ScaleAddonWindow>
{
    public:
	bool init ();
};
