/*
 *
 * Compiz stackswitch switcher plugin
 *
 * stackswitch.h
 *
 * Copyright : (C) 2007 by Danny Baumann
 * E-mail    : maniac@opencompositing.org
 *
 * Based on scale.c and switcher.c:
 * Copyright : (C) 2007 David Reveman
 * E-mail    : davidr@novell.com
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

#include <cmath>

#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <text/text.h>

#include "stackswitch_options.h"

typedef enum {
    StackswitchStateNone = 0,
    StackswitchStateOut,
    StackswitchStateSwitching,
    StackswitchStateIn
} StackswitchState;

typedef enum {
    StackswitchTypeNormal = 0,
    StackswitchTypeGroup,
    StackswitchTypeAll
} StackswitchType;

typedef struct _StackswitchSlot {
    int   x, y;            /* thumb center coordinates */
    float scale;           /* size scale (fit to maximal thumb size) */
} StackswitchSlot;

typedef struct _StackswitchDrawSlot {
    CompWindow      *w;
    StackswitchSlot **slot;
} StackswitchDrawSlot;

class StackswitchScreen:
    public PluginClassHandler <StackswitchScreen, CompScreen>,
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public StackswitchOptions
{
    public:

	StackswitchScreen (CompScreen *);
	~StackswitchScreen ();

    public:

	CompositeScreen *cScreen;
	GLScreen	*gScreen;

    public:

	void
	handleEvent (XEvent *);

	void
	preparePaint (int);

	void
	donePaint ();

	bool
	glPaintOutput (const GLScreenPaintAttrib &,
		       const GLMatrix		 &,
		       const CompRegion		 &,
		       CompOutput		 *,
		       unsigned int		   );

    public:

	void
	renderWindowTitle ();

	void
	drawWindowTitle (GLMatrix &transform, CompWindow *w);
					    
	bool
	layoutThumbs ();

	void
	addWindowToList (CompWindow *w);

	bool
	updateWindowList ();

	bool
	createWindowList ();

	void
	switchToWindow (bool toNext);

	int
	countWindows ();

	int
	adjustStackswitchRotation (float chunk);

	bool
	terminate (CompAction *action,
		   CompAction::State state,
		   CompOption::Vector options);
				      
	bool
	initiate (CompAction         *action,
		  CompAction::State  state,
		   CompOption::Vector options);
				     
	bool
	doSwitch (CompAction         *action,
		  CompAction::State  state,
		  CompOption::Vector options,
		  bool		nextWindow,
		  StackswitchType type);

	void
	windowRemove (Window id);

    public:

	CompText mText;

	CompScreen::GrabHandle mGrabIndex;

	StackswitchState mState;
	StackswitchType  mType;

	bool	mMoreAdjust;
	bool	mRotateAdjust;

	bool	mPaintingSwitcher;

	GLfloat mRVelocity;
	GLfloat mRotation;

	/* only used for sorting */
	CompWindow          **mWindows;
	StackswitchDrawSlot *mDrawSlots;
	int                 mWindowsSize;
	int                 mNWindows;

	Window mClientLeader;
	Window mSelectedWindow;

	CompMatch mMatch;
	CompMatch mCurrentMatch;
};

class StackswitchWindow:
    public PluginClassHandler <StackswitchWindow, CompWindow>,
    public CompositeWindowInterface,
    public GLWindowInterface
{
    public:

	StackswitchWindow (CompWindow *);
	~StackswitchWindow ();

    public:

	CompWindow *window;
	CompositeWindow *cWindow;
	GLWindow	*gWindow;

    public:

	bool
	glPaint (const GLWindowPaintAttrib &,
		 const GLMatrix		   &,
		 const CompRegion	   &,
		 unsigned int		     );

	bool
	damageRect (bool,
		    const CompRect &);

    public:

	int
	adjustVelocity ();

	bool
	isStackswitchable ();

    public:

	StackswitchSlot *mSlot;

	GLfloat mXVelocity;
	GLfloat mYVelocity;
	GLfloat mScaleVelocity;
	GLfloat mRotVelocity;

	GLfloat mTx;
	GLfloat mTy;
	GLfloat mScale;
	GLfloat mRotation;
	bool    mAdjust;
};

#define STACKSWITCH_WINDOW(w)						      \
    StackswitchWindow *sw = StackswitchWindow::get (w);

#define STACKSWITCH_SCREEN(s)						      \
    StackswitchScreen *ss = StackswitchScreen::get (s);

class StackswitchPluginVTable:
    public CompPlugin::VTableForScreenAndWindow <StackswitchScreen, StackswitchWindow>
{
    public:

	bool init ();
};
