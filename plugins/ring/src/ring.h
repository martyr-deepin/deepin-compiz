/*
 *
 * Compiz ring switcher plugin
 *
 * ring.h
 *
 * Copyright : (C) 2007 by Danny Baumann
 * E-mail    : maniac@opencompositing.org
 *
 * Based on scale.c and switcher.c:
 * Copyright : (C) 2007 David Reveman
 * E-mail    : davidr@novell.com
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

#include <cmath>

#include <core/atoms.h>
#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <text/text.h>

#include <X11/Xatom.h>

#include "ring_options.h"

extern bool textAvailable;

class RingScreen :
    public PluginClassHandler <RingScreen, CompScreen>,
    public RingOptions,
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface
{
    public:

	RingScreen (CompScreen *screen);
	~RingScreen ();

    public:

	typedef enum {
	    RingStateNone = 0,
	    RingStateOut = 1,
	    RingStateSwitching = 2,
	    RingStateIn = 3
	} RingState;

	typedef enum {
	    RingTypeNormal = 0,
	    RingTypeGroup = 1,
	    RingTypeAll = 2
	} RingType;

	class RingSlot {
	    public:
	        int   x, y;            /* thumb center coordinates */
	        float scale;           /* size scale (fit to max thumb size) */
	        float depthScale;      /* scale for depth impression */
	        float depthBrightness; /* brightness for depth impression */
	};

	class RingDrawSlot {
	    public:
		CompWindow *w;
		RingSlot   **slot;
	};

    public:

	CompositeScreen *cScreen;
	GLScreen        *gScreen;

	CompText	mText;

	CompScreen::GrabHandle mGrabIndex;

	RingState mState;
	RingType  mType;
	bool      mMoreAdjust;
	bool	  mRotateAdjust;

	int     mRotTarget;
	int     mRotAdjust;
	GLfloat mRVelocity;

	/* only used for sorting */
	std::vector <CompWindow *>   mWindows;
	std::vector <RingDrawSlot>   mDrawSlots;
	int			     mWindowsSize;
	int			     mNWindows;

	Window mClientLeader;

	CompWindow *mSelectedWindow;

	CompMatch mMatch;
	CompMatch mCurrentMatch;

    public:

	/* Functions that we hook */

	void
	handleEvent (XEvent *);

	void
	preparePaint (int);

	bool
	glPaintOutput (const GLScreenPaintAttrib &,
		       const GLMatrix		 &,
		       const CompRegion		 &,
		       CompOutput		 *,
		       unsigned int		   );

	void
	donePaint ();

    public:

	/* Internal Functions */

	void
	freeWindowTitle ();

	void
	renderWindowTitle ();

	void
	drawWindowTitle (const GLMatrix &transform);

	bool
	layoutThumbs ();

	void
	addWindowToList (CompWindow *w);

	bool
	updateWindowList ();

	bool
	createWindowList ();

	void
	switchToWindow (bool	   toNext);

	int
	countWindows ();

	int
	adjustRingRotation (float      chunk);

	bool
	terminate (CompAction         *action,
		   CompAction::State  state,
		   CompOption::Vector options);

	bool
	initiate (CompAction         *action,
		  CompAction::State  state,
		  CompOption::Vector options);

	bool
	doSwitch (CompAction         *action,
		  CompAction::State  state,
		  CompOption::Vector options,
		  bool		 nextWindow,
		  RingType		 f_type);

	void
	windowSelectAt (int  x,
			int  y,
			bool f_terminate);

	void
	windowRemove (CompWindow *w);

	void
	switchActivateEvent (bool);

};

class RingWindow :
    public PluginClassHandler <RingWindow, CompWindow>,
    public GLWindowInterface,
    public CompositeWindowInterface
{
    public:

	RingWindow (CompWindow *);
	~RingWindow ();

	CompWindow	*window;
	CompositeWindow *cWindow;
	GLWindow	*gWindow;

	RingScreen::RingSlot *mSlot;

	GLfloat mXVelocity;
	GLfloat mYVelocity;
	GLfloat mScaleVelocity;

	GLfloat mTx;
	GLfloat mTy;
	GLfloat mScale;
	bool    mAdjust;

	bool
	glPaint (const GLWindowPaintAttrib &,
		 const GLMatrix		   &,
		 const CompRegion	   &,
		 unsigned int		     );

	bool
	damageRect (bool, const CompRect &);

	bool
	is (bool removing = false);

	static bool
	compareWindows (CompWindow *w1,
			CompWindow *w2);

	static bool 
	compareRingWindowDepth (RingScreen::RingDrawSlot e1,
				RingScreen::RingDrawSlot e2);

	int
	adjustVelocity ();
};

extern const double PI;
#define DIST_ROT (3600 / mWindows.size ())
#define DIST_ROT_w (3600 / rs->mWindows.size ())
extern const unsigned short ICON_SIZE;

#define RING_SCREEN(s)							       \
    RingScreen *rs = RingScreen::get (s)

#define RING_WINDOW(w)							       \
    RingWindow *rw = RingWindow::get (w)

class RingPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <RingScreen, RingWindow>
{
    public:

	bool init ();
};
