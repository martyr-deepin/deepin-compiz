/*
 * Compiz vpswitch plugin
 *
 * vpswitch.cpp
 *
 * Copyright (c) 2007 Dennis Kasprzyk <onestone@opencompositing.org>
 *
 * Go-to-numbered-viewport functionality by
 * 
 * Copyright (c) 2007 Robert Carr <racarr@opencompositing.org>
 *           (c) 2007 Danny Baumann <maniac@opencompositing.org>
 *
 * Go-to-specific-viewport functionality by
 *
 * Copyright (c) 2007 Michael Vogt <mvo@ubuntu.com>
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

#include "vpswitch_options.h"

/* Helper macro to obtain necesary action data */

/* ??? xid = screen->grabWindow () else xid = screen->below () */

#define GET_DATA \
    CompWindow *w;\
    Window     xid; \
    if (screen->otherGrabExist ("rotate", "wall", "plane", 0)) \
	return false; \
    xid = CompOption::getIntOptionNamed (options, "window"); \
    w = screen->findWindow (xid); \
    if ((!w || (w->type () & CompWindowTypeDesktopMask) == 0) && \
	xid != screen->root ()) \
	return false;

/* number-to-keysym mapping */
static const KeySym numberKeySyms[3][10] = {
    /* number key row */
    { XK_0, XK_1, XK_2, XK_3, XK_4, XK_5, XK_6, XK_7, XK_8, XK_9 },
    /* number keypad with activated NumLock */
    { XK_KP_0, XK_KP_1, XK_KP_2, XK_KP_3, XK_KP_4,
      XK_KP_5, XK_KP_6, XK_KP_7, XK_KP_8, XK_KP_9 },
    /* number keypad without NumLock */
    { XK_KP_Insert, XK_KP_End, XK_KP_Down, XK_KP_Next, XK_KP_Left,
      XK_KP_Begin, XK_KP_Right, XK_KP_Home, XK_KP_Up, XK_KP_Prior }
};

class VPSwitchScreen :
    public PluginClassHandler <VPSwitchScreen, CompScreen>,
    public VpswitchOptions,
    public ScreenInterface
{
    public:

	VPSwitchScreen (CompScreen *);

	int destination;
	bool numberedActive;

	/* Wrappable functions */
	void
	handleEvent (XEvent *);

	/* Actions */
	bool
	movevp (CompAction	   *action,
		CompAction::State  state,
		CompOption::Vector &options,
		unsigned int dx, unsigned int dy);

	bool
	switchto (CompAction         *action,
		  CompAction::State  state,
		  CompOption::Vector &options,
		  int num);

	bool
	initiateNumbered (CompAction         *action,
			  CompAction::State  state,
			  CompOption::Vector &options);

	bool
	terminateNumbered (CompAction	      *action,
			   CompAction::State  state,
			   CompOption::Vector &options);

	bool
	prev (CompAction         *action,
	      CompAction::State  state,
	      CompOption::Vector &options);

	bool
	next (CompAction         *action,
	      CompAction::State  state,
	      CompOption::Vector &options);

	bool
	initPluginAction (CompAction         *action,
			  CompAction::State  state,
			  CompOption::Vector &options);

	bool
	termPluginAction (CompAction         *action,
			  CompAction::State  state,
			  CompOption::Vector &options);

	/* General Functions */

	void
	gotovp (int x, int y);
};
	
#define VPSWITCH_SCREEN(s)						       \
    VPSwitchScreen *vs = VPSwitchScreen::get (s);

class VPSwitchPluginVTable :
    public CompPlugin::VTableForScreen <VPSwitchScreen>
{
    public:

	bool init ();
};	
