/*
 *
 * Compiz shift switcher plugin
 *
 * shift.h
 *
 * Copyright : (C) 2007 by Dennis Kasprzyk
 * E-mail    : onestone@opencompositing.org
 *
 *
 * Based on ring.c:
 * Copyright : (C) 2007 by Danny Baumann
 * E-mail    : maniac@opencompositing.org
 *
 * Based on scale.c and switcher.c:
 * Copyright : (C) 2007 David Reveman
 * E-mail    : davidr@novell.com
 *
 * Rounded corner drawing taken from wall.c:
 * Copyright : (C) 2007 Robert Carr
 * E-mail    : racarr@beryl-project.org
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
#include <text/text.h>

#include <cmath>

#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include "shift_options.h"

typedef enum {
    ShiftStateNone = 0,
    ShiftStateOut,
    ShiftStateSwitching,
    ShiftStateFinish,
    ShiftStateIn
} ShiftState;

typedef enum {
    ShiftTypeNormal = 0,
    ShiftTypeGroup,
    ShiftTypeAll
} ShiftType;

typedef struct _ShiftSlot {
    int   x, y;            /* thumb center coordinates */
    float z;
    float scale;           /* size scale (fit to maximal thumb size */
    float opacity;
    float rotation;

    GLfloat tx;
    GLfloat ty;

    bool    primary;

} ShiftSlot;

typedef struct _ShiftDrawSlot {
    CompWindow *w;
    ShiftSlot  *slot;
    float      distance;
} ShiftDrawSlot;

class ShiftScreen :
    public PluginClassHandler <ShiftScreen, CompScreen>,
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public ShiftOptions
{
    public:
	
	ShiftScreen (CompScreen *);
	~ShiftScreen ();
	
	CompositeScreen *cScreen;
	GLScreen	*gScreen;
	CompText	text;
    
    public:

	void
	handleEvent (XEvent *);
	
	void
	preparePaint (int);
	
	void
	paint (CompOutput::ptrList &,
	       unsigned int);
	
	bool glPaintOutput (const GLScreenPaintAttrib &,
			    const GLMatrix &, const CompRegion &,
			    CompOutput *, unsigned int);
	
	void
	donePaint ();

    public:
	
	KeyCode mLeftKey;
	KeyCode mRightKey;
	KeyCode mUpKey;
	KeyCode mDownKey;
    
	CompScreen::GrabHandle mGrabIndex;
	
	ShiftState	mState;
	ShiftType	mType;
	
	bool		mMoreAdjust;
	bool		mMoveAdjust;
	
	float		mMvTarget;
	float		mMvAdjust;
	float		mMvVelocity;
	bool		mInvert;
	
	Cursor		mCursor;
	
	/* only used for sorting */
	CompWindow	**mWindows;
	int		mNWindows;
	int		mWindowsSize;

	
	ShiftDrawSlot	*mDrawSlots;
	int		mNSlots;
	int		mSlotsSize;
	ShiftDrawSlot	*mActiveSlot;
	
	Window		mClientLeader;
	Window		mSelectedWindow;
	
	CompMatch	mMatch;
	CompMatch	*mCurrentMatch;
	
	CompOutput	*mOutput;
	int		mUsedOutput;
	
	float		mReflectBrightness;
	bool		mReflectActive;
	
	float		mAnim;
	float		mAnimVelocity;
	
	int		mButtonPressTime;
	bool		mButtonPressed;
	int		mStartX;
	int		mStartY;
	float		mStartTarget;
	float		mLastTitle;
	
	bool		mPaintingAbove;
	
	bool		mCancelled;

    public:

	void
	activateEvent (bool       activating);

	void
	freeWindowTitle ();

	void
	renderWindowTitle ();

	void
	drawWindowTitle (const GLMatrix &transform);

	bool
	layoutThumbsCover ();

	bool
	layoutThumbsFlip ();

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
	adjustShiftMovement (float chunk);

	bool
	adjustShiftAnimationAttribs (float chunk);

	void
	term (bool cancel);

	bool
	terminate (CompAction         *action,
		    CompAction::State  aState,
		    CompOption::Vector &options);

	bool
	initiateScreen (CompAction         *action,
			CompAction::State  aState,
			CompOption::Vector &options);

	bool
	doSwitch (CompAction         *action,
		    CompAction::State  aState,
		    CompOption::Vector &options,
		    bool            nextWindow,
		    ShiftType       type);

	bool
	initiate (CompAction         *action,
		  CompAction::State    state,
		  CompOption::Vector &options);

	bool
	initiateAll (CompAction         *action,
			CompAction::State  aState,
			CompOption::Vector &options);

	void
	windowRemove (Window id);

};

class ShiftWindow :
    public PluginClassHandler <ShiftWindow, CompWindow>,
    public WindowInterface,
    public CompositeWindowInterface,
    public GLWindowInterface
{
    public:

	ShiftWindow (CompWindow *);
	~ShiftWindow ();
	
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
	damageRect (bool	initial,
		    const CompRect &rect);

    public:
	
	ShiftSlot	mSlots[2];
	float		mOpacity;
	float		mBrightness;
	float		mOpacityVelocity;
	float		mBrightnessVelocity;
	
	bool		mActive;
    
    public:

	bool
        adjustShiftAttribs (float chunk);

	bool
	canStackRelativeTo ();

	bool
	isShiftable ();

};

extern const double PI;

#define SHIFT_WINDOW(w)							      \
    ShiftWindow *sw = ShiftWindow::get (w)

#define SHIFT_SCREEN(s)							      \
    ShiftScreen *ss = ShiftScreen::get (s)


class ShiftPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <ShiftScreen, ShiftWindow>
{
    public:

	bool init ();
};
