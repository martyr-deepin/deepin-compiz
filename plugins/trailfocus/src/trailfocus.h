/**
 * Beryl Trailfocus - take three
 *
 * Copyright (c) 2006 Kristian Lyngstøl <kristian@beryl-project.org>
 * Ported to Compiz and BCOP usage by Danny Baumann <maniac@beryl-project.org>
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
 * This version is completly rewritten from scratch with opacify as a 
 * basic template. The original trailfocus was written by: 
 * François Ingelrest <Athropos@gmail.com> and rewritten by:
 * Dennis Kasprzyk <onestone@beryl-project.org>
 * 
 *
 * Trailfocus modifies the opacity, brightness and saturation on a window 
 * based on when it last had focus. 
 *
 */

#include <core/core.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>

#include "trailfocus_options.h"

typedef struct
{
    GLushort opacity;
    GLushort brightness;
    GLushort saturation;
} TfAttribs;

class TrailfocusWindow :
    public GLWindowInterface,
    public PluginClassHandler<TrailfocusWindow, CompWindow>
{
    public:
	TrailfocusWindow (CompWindow *);
	~TrailfocusWindow ();

	bool glPaint (const GLWindowPaintAttrib&, const GLMatrix&,
		      const CompRegion&, unsigned int);

	bool      isTfWindow;
	TfAttribs attribs;

	CompWindow      *window;
	CompositeWindow *cWindow;
	GLWindow        *gWindow;
};

class TrailfocusScreen :
    public ScreenInterface,
    public PluginClassHandler<TrailfocusScreen, CompScreen>,
    public TrailfocusOptions
{
    public:
	TrailfocusScreen (CompScreen *);

	void handleEvent (XEvent *);

	bool pushWindow (Window);
	void popWindow (TrailfocusWindow *);

    private:
	bool isTrailfocusWindow (CompWindow *);
	void setWindows (TrailfocusWindow *);
	void refillList ();
	void recalculateAttributes ();
	void optionChanged (CompOption *, Options);

	static bool setupTimerCb ();

	typedef std::vector<TrailfocusWindow *> TfWindowList;

	TfWindowList           windows;
	std::vector<TfAttribs> attribs;
	CompTimer              setupTimer;
};

class TrailfocusPluginVTable :
    public CompPlugin::VTableForScreenAndWindow<TrailfocusScreen,
						TrailfocusWindow>
{
    public:
	bool init ();
};
