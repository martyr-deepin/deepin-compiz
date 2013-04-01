/**
 *
 * Compiz group plugin
 *
 * group.h
 *
 * Copyright : (C) 2006-2010 by Patrick Niklaus, Roi Cohen,
 * 				Danny Baumann, Sam Spilsbury
 * Authors: Patrick Niklaus <patrick.niklaus@googlemail.com>
 *          Roi Cohen       <roico.beryl@gmail.com>
 *          Danny Baumann   <maniac@opencompositing.org>
 * 	    Sam Spilsbury   <smspillaz@gmail.com>
 *
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
 * Some terminology used in this plugin:
 *
 * 1. The "top tab" means the currently selected visible tab in the
 * group. All other windows in the tabbed group are invisible.
 *
 * 2. There are two animations, "Tabbing/Untabbing" and the "Change"
 * animation. The "Tabbing/Untabbing" one is probably the most complex.
 * This is where we center windows to some central top tab and then
 * animate them to appear to be morphing into this window. We also
 * do a similar animation when animating out (which is why we need
 * to save their relevant distance from the main window initially).
 * There is also the "change" animation, which is where we switch
 * between a number of top tabs
 *
 * 3. The glow around windows is actually in fact a small texture which
 * is strectched according to some gloq quads and then painted.
 *
 * 4. Each "layer" here is a GUI object which is painted on screen
 * (such as the tab bar, the text, the selection highlight, etc)
 *
 **/

#ifndef _GROUP_H
#define _GROUP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>
#include <cairo/cairo-xlib-xrender.h>
#include <core/core.h>
#include <core/atoms.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <mousepoll/mousepoll.h>
#include <text/text.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>

#include <math.h>
#include <limits.h>

class GroupSelection;
class GroupWindow;
class GroupScreen;

#include "layers.h"
#include "tabbar.h"
#include "glow.h"
#include "group_options.h"

/*
 * Used to check if we can use the text plugin
 *
 */
extern bool gTextAvailable;


/*
 * Constants
 *
 */
extern const double PI;

/*
 * Helpers
 *
 */

#define WIN_X(w) (w->x ())
#define WIN_Y(w) (w->y ())
#define WIN_WIDTH(w) (w->width ())
#define WIN_HEIGHT(w) (w->height ())

#define WIN_CENTER_X(w) (WIN_X (w) + (WIN_WIDTH (w) / 2))
#define WIN_CENTER_Y(w) (WIN_Y (w) + (WIN_HEIGHT (w) / 2))

/* definitions used for glow painting */
#define WIN_REAL_X(w) (w->x () - w->border ().left)
#define WIN_REAL_Y(w) (w->y () - w->border ().top)
#define WIN_REAL_WIDTH(w) (w->width () + 2 * w->geometry ().border () + \
			   w->border ().left + w->border ().right)
#define WIN_REAL_HEIGHT(w) (w->height () + 2 * w->geometry ().border () + \
			    w->border ().top + w->border ().bottom)

#define TOP_TAB(g) ((g)->mTabBar->mTopTab->mWindow)
#define PREV_TOP_TAB(g) ((g)->mTabBar->mPrevTopTab->mWindow)
#define NEXT_TOP_TAB(g) ((g)->mTabBar->mNextTopTab->mWindow)

#define HAS_TOP_WIN(group) (((group)->mTabBar && (group)->mTabBar->mTopTab) && ((group)->mTabBar->mTopTab->mWindow))
#define HAS_PREV_TOP_WIN(group) (((group)->mTabBar->mPrevTopTab) && \
				 ((group)->mTabBar->mPrevTopTab->mWindow))

#define IS_TOP_TAB(w, group) (HAS_TOP_WIN (group) && \
			      ((TOP_TAB (group)->id ()) == (w)->id ()))
#define IS_PREV_TOP_TAB(w, group) (HAS_PREV_TOP_WIN (group) && \
				   ((PREV_TOP_TAB (group)->id ()) == (w)->id ()))

/*
 * Selection
 */

class Selection :
    public CompWindowList
{
public:
    Selection () :
	mPainted (false),
	mVpX (0),
	mVpY (0),
	mX1 (0),
	mY1 (0),
	mX2 (0),
	mY2 (0) {};

    void checkWindow (CompWindow *w);
    void deselect (CompWindow *w);
    void deselect (GroupSelection *group);
    void select (CompWindow *w);
    void select (GroupSelection *g);
    void selectRegion ();
    GroupSelection * toGroup ();
    void damage (int, int);
    void paint (const GLScreenPaintAttrib sa,
		const GLMatrix		  transform,
		CompOutput		  *output,
		bool			  transformed);

    /* For selection */
    bool mPainted;
    int  mVpX, mVpY;
    int  mX1, mY1, mX2, mY2;
};

/*
 * GroupSelection
 */
class GroupSelection
{
    public:

	class ResizeInfo
	{
	    public:
		CompWindow *mResizedWindow;
		CompRect    mOrigGeometry;
	};

    public:
	/*
	 * Ungrouping states
	 */
	typedef enum {
	    UngroupNone = 0,
	    UngroupAll,
	    UngroupSingle
	} UngroupState;

	typedef enum {
	    NoTabbing = 0,
	    Tabbing,
	    Untabbing
	} TabbingState;

    public:

	typedef std::list <GroupSelection *> List;

	GroupSelection ();
	~GroupSelection ();

    public:

	void tabGroup (CompWindow *main);
	void untabGroup ();

	void raiseWindows (CompWindow *top);
	void minimizeWindows (CompWindow *top,
			  bool	     minimize);
	void shadeWindows (CompWindow  *top,
		       bool	   shade);
	void moveWindows (CompWindow *top,
			  int 	 dx,
			  int 	 dy,
			  bool 	 immediate,
			  bool	 viewportChange = false);
	void prepareResizeWindows (CompRect &resizeRect);
	void resizeWindows (CompWindow *top);
	void maximizeWindows (CompWindow *top);
	void changeColor ();

	void
	applyConstraining (CompRegion  constrainRegion,
			   Window	   constrainedWindow,
			   int	   dx,
			   int	   dy);

	bool tabBarTimeout ();
	bool showDelayTimeout ();

	void
	tabSetVisibility (bool           visible,
			  unsigned int   mask);

	void
	handleHoverDetection (const CompPoint &);

	bool
	handleAnimation ();

	void
	finishTabbing ();

	bool
	drawTabAnimation (int	      msSinceLastPaint);

	void
	startTabbingAnimation (bool           tab);

	void fini ();

public:
    CompScreen *mScreen;
    CompWindowList mWindows;

    MousePoller	mPoller;

    GroupTabBar *mTabBar;

    GroupSelection::TabbingState mTabbingState;

    UngroupState mUngroupState;

    Window       mGrabWindow;
    unsigned int mGrabMask;

    GLushort mColor[4];

    ResizeInfo *mResizeInfo;

    /* It's easier to keep track of these things, serialize them
     * and rebuild what the group would have looked like later
     */

    std::list <Window> mWindowIds;
    Window	       mTopId;
};

/*
 * GroupWindow structure
 */
class GroupWindow :
    public PluginClassHandler <GroupWindow, CompWindow>,
    public WindowInterface,
    public CompositeWindowInterface,
    public GLWindowInterface
{
    public:

	/*
	 * Window states
	 */
	typedef enum {
	    WindowNormal = 0,
	    WindowMinimized,
	    WindowShaded
	} State;

	class HideInfo {
	    public:
		Window mShapeWindow;

		unsigned long mSkipState;
		unsigned long mShapeMask;

		XRectangle *mInputRects;
		int        mNInputRects;
		int        mInputRectOrdering;
	};

	/*
	 * Structs for pending callbacks
	 */
	class PendingMoves
	{
	    public:
		CompWindow        *w;
		int               dx;
		int               dy;
		bool              immediate;
		bool              sync;
		GroupWindow::PendingMoves *next;
	};

	class PendingGrabs
	{
	    public:
		CompWindow        *w;
		int               x;
		int               y;
		unsigned int      state;
		unsigned int      mask;
		PendingGrabs *next;
	};

	class PendingUngrabs
	{
	    public:
		CompWindow          *w;
		PendingUngrabs *next;
	};

	class PendingSyncs
	{
	    public:
		CompWindow        *w;
		PendingSyncs *next;
	};

    public:

	GroupWindow (CompWindow *);
	~GroupWindow ();

    public:

	CompWindow *window;
	CompositeWindow *cWindow;
	GLWindow   *gWindow;

    public:

	void
	moveNotify (int, int, bool);

	void
	resizeNotify (int, int, int, int);

	void
	grabNotify (int, int,
		    unsigned int,
		    unsigned int);

	void
	ungrabNotify ();

	void
	windowNotify (CompWindowNotify n);

	void
	stateChangeNotify (unsigned int);


	void
	activate ();

	void
	getOutputExtents (CompWindowExtents &);

	bool
	glDraw (const GLMatrix &,
		GLFragment::Attrib &,
		const CompRegion	&,
		unsigned int);

	bool
	glPaint (const GLWindowPaintAttrib &,
		 const GLMatrix		   &,
		 const CompRegion	   &,
		 unsigned int		     );

	bool
	damageRect (bool,
		    const CompRect &);

    public:

	/* glow.cpp */

	bool checkTabbing ();
	bool checkRotating ();
	bool checkShowTabBar ();

	void
	paintGlow (GLFragment::Attrib        &attrib,
		   const CompRegion	     &paintRegion,
		   unsigned int		     mask);

	void
	computeGlowQuads (GLTexture::Matrix *matrix);

	/* paint.c */

	void
	getStretchRectangle (CompRect &box,
			     float  &xScaleRet,
			     float  &yScaleRet);

	/* queues.c */

	void
	enqueueMoveNotify (int  dx,
			   int  dy,
			   bool immediate,
			   bool sync);

	void
	enqueueGrabNotify (int          x,
			   int          y,
			   unsigned int state,
			   unsigned int mask);

	void
	enqueueUngrabNotify ();

	/* selection.c */

	bool
	windowInRegion (CompRegion src,
			  float  precision);

	/* group.c */

	bool
	isGroupWindow ();

	bool
	dragHoverTimeout ();

	unsigned int
	updateResizeRectangle (CompRect	    masterGeometry,
				    bool	    damage);

	void
	deleteGroupWindow ();

	void
	removeWindowFromGroup ();

	void
	addWindowToGroup (GroupSelection *group);

	/* tab.cpp */

	CompRegion
	getClippingRegion ();

	void
	clearWindowInputShape (GroupWindow::HideInfo *hideInfo);

	void
	setWindowVisibility (bool visible);

	int
	adjustTabVelocity ();

	bool
	constrainMovement (CompRegion constrainRegion,
			   int        dx,
			   int        dy,
			   int        &new_dx,
			   int        &new_dy);

	/* init.cpp */

	void checkFunctions ();


    public:

	GroupSelection *mGroup;
	bool mInSelection;

	/* For the tab bar */
	GroupTabBarSlot *mSlot;

	bool mNeedsPosSync;

	GlowQuad *mGlowQuads;

	GroupWindow::State    mWindowState;
	GroupWindow::HideInfo   *mWindowHideInfo;

	CompRect	    mResizeGeometry;

	/* For tab animation */
	int       mAnimateState;
	CompPoint mMainTabOffset;
	CompPoint mDestination;
	CompPoint mOrgPos;

	float mTx,mTy;
	float mXVelocity, mYVelocity;
};

/*
 * GroupScreen structure
 */
class GroupScreen :
    public PluginClassHandler <GroupScreen, CompScreen>,
    public GroupOptions,
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface
{
    public:

	/*
	 * Screengrab states
	 */
	typedef enum {
	    ScreenGrabNone = 0,
	    ScreenGrabSelect,
	    ScreenGrabTabDrag
	} GrabState;

    public:

	GroupScreen (CompScreen *);
	~GroupScreen ();

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
	glPaintOutput (const GLScreenPaintAttrib &attrib,
		       const GLMatrix	      &transform,
		       const CompRegion	      &region,
		       CompOutput	      *output,
		       unsigned int	      mask);

	void
	glPaintTransformedOutput (const GLScreenPaintAttrib &,
				  const GLMatrix	    &,
				  const CompRegion	    &,
				  CompOutput		    *,
				  unsigned int		      );

	CompMatch::Expression *
	matchInitExp (const CompString &str);

	void
	matchExpHandlerChanged ();


    public:

        void
        checkFunctions ();

	void
	optionChanged (CompOption *opt,
		       Options    num);

	bool
	applyInitialActions ();

	/* cairo.c */

	void
	damagePaintRectangle (const CompRect &box);

	/* queues.c */

	void
	dequeueSyncs (GroupWindow::PendingSyncs *);

	void
	dequeueMoveNotifies ();

	void
	dequeueGrabNotifies ();

	void
	dequeueUngrabNotifies ();

	bool
	dequeueTimer ();

	/* selection.c */

	bool
	selectSingle (CompAction         *action,
			CompAction::State  state,
			CompOption::Vector options);
	bool
	select (CompAction         *action,
		  CompAction::State  state,
		  CompOption::Vector options);

	bool
	selectTerminate (CompAction         *action,
			   CompAction::State  state,
			   CompOption::Vector options);

	/* group.c */

	void
	grabScreen (GroupScreen::GrabState newState);

	bool
	groupWindows (CompAction         *action,
			CompAction::State  state,
			CompOption::Vector options);

	bool
	ungroupWindows (CompAction          *action,
			  CompAction::State   state,
			  CompOption::Vector  options);

	bool
	removeWindow (CompAction         *action,
			CompAction::State  state,
			CompOption::Vector options);

	bool
	closeWindows (CompAction           *action,
			CompAction::State    state,
			CompOption::Vector   options);

	bool
	changeColor (CompAction           *action,
		       CompAction::State    state,
		       CompOption::Vector   options);

	bool
	setIgnore (CompAction         *action,
		     CompAction::State  state,
		     CompOption::Vector options);

	bool
	unsetIgnore (CompAction          *action,
		       CompAction::State   state,
		       CompOption::Vector  options);

	void
	handleButtonPressEvent (XEvent *event);

	void
	handleButtonReleaseEvent (XEvent *event);

	void
	handleMotionEvent (int xRoot,
			     int yRoot);

	/* tab.c */

	bool
	getCurrentMousePosition (int &x, int &y);

	void
	tabChangeActivateEvent (bool activating);

	void
	updateTabBars (Window enteredWin);

	CompRegion
	getConstrainRegion ();

	bool
	changeTab (GroupTabBarSlot             *topTab,
		        GroupTabBar::ChangeAnimationDirection direction);

	void
	recalcSlotPos (GroupTabBarSlot *slot,
		       int		 slotPos);

	bool
	initTab (CompAction         *aciton,
		 CompAction::State  state,
		 CompOption::Vector options);

	bool
	changeTabLeft (CompAction          *action,
		       CompAction::State   state,
		       CompOption::Vector  options);
	bool
	changeTabRight (CompAction         *action,
			CompAction::State  state,
			CompOption::Vector options);

	void
	switchTopTabInput (GroupSelection *group,
			   bool		  enable);

    public:

	bool		mIgnoreMode;
	GlowTextureProperties *mGlowTextureProperties;
	GroupSelection	*mLastRestackedGroup;
	Atom		mResizeNotifyAtom;

	CompText	mText;

	GroupWindow::PendingMoves   *mPendingMoves;
	GroupWindow::PendingGrabs   *mPendingGrabs;
	GroupWindow::PendingUngrabs *mPendingUngrabs;
	CompTimer	    mDequeueTimeoutHandle;

	GroupSelection::List mGroups;
	Selection	     mTmpSel;

	bool mQueued;

	GroupScreen::GrabState   mGrabState;
	CompScreen::GrabHandle mGrabIndex;

	GroupSelection *mLastHoveredGroup;

	CompTimer       mShowDelayTimeoutHandle;

	/* For d&d */
	GroupTabBarSlot   *mDraggedSlot;
	CompTimer	  mDragHoverTimeoutHandle;
	bool              mDragged;
	int               mPrevX, mPrevY; /* Buffer for mouse coordinates */

	CompTimer	  mInitialActionsTimeoutHandle;

	GLTexture::List   mGlowTexture;

	Window		  mLastGrabbedWindow;
};

#define GROUP_SCREEN(s)							       \
    GroupScreen *gs = GroupScreen::get (s);

#define GROUP_WINDOW(w)							       \
    GroupWindow *gw = GroupWindow::get (w);

class GroupPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <GroupScreen, GroupWindow>
{
    public:

	bool init ();
};

/*
 * Pre-Definitions
 *
 */
#endif
