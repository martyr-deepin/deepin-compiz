/*
 * Copyright © 2007 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#ifndef _SCALE_PRIVATES_H
#define _SCALE_PRIVATES_H

#include <scale/scale.h>
#include "scale_options.h"

class SlotArea {
    public:
	int      nWindows;
	CompRect workArea;

	typedef std::vector<SlotArea> vector;
};

class PrivateScaleScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public ScaleOptions
{
    public:
	PrivateScaleScreen (CompScreen *);
	~PrivateScaleScreen ();

	void handleEvent (XEvent *event);

	void preparePaint (int);
	void donePaint ();

	bool glPaintOutput (const GLScreenPaintAttrib &,
			    const GLMatrix &, const CompRegion &,
			    CompOutput *, unsigned int);

	void activateEvent (bool activating);

	void layoutSlotsForArea (const CompRect&, int);
	void layoutSlots ();
	void findBestSlots ();
	bool fillInWindows ();
	bool layoutThumbs ();
	bool layoutThumbsAll ();
	bool layoutThumbsSingle ();

	SlotArea::vector getSlotAreas ();

	ScaleWindow * checkForWindowAt (int x, int y);

	void sendDndStatusMessage (Window);

	bool
	actionShouldToggle (CompAction        *action,
			    CompAction::State state);

	static bool scaleTerminate (CompAction         *action,
				    CompAction::State  state,
				    CompOption::Vector &options);
	static bool scaleInitiate (CompAction         *action,
				   CompAction::State  state,
				   CompOption::Vector &options,
				   ScaleType          type);

	bool scaleInitiateCommon (CompAction         *action,
				  CompAction::State  state,
				  CompOption::Vector &options);

	bool ensureDndRedirectWindow ();

	bool selectWindowAt (int x, int y, bool moveInputFocus);
	bool selectWindowAt (int x, int y);

	void moveFocusWindow (int dx, int dy);

	void windowRemove (CompWindow *);

	bool hoverTimeout ();

	void updateOpacity ();

	int getMultioutputMode ();

    public:

	CompositeScreen *cScreen;
	GLScreen        *gScreen;

	unsigned int lastActiveNum;
	Window       lastActiveWindow;

	Window       selectedWindow;
	Window       hoveredWindow;
	Window       previousActiveWindow;

	KeyCode	 leftKeyCode, rightKeyCode, upKeyCode, downKeyCode;

	bool grab;
	CompScreen::GrabHandle grabIndex;

	Window dndTarget;

	CompTimer hover;

	ScaleScreen::State state;
	int                moreAdjust;

	Cursor cursor;

	std::vector<ScaleSlot> slots;
	int                  nSlots;

	ScaleScreen::WindowList windows;

	GLushort opacity;

	ScaleType type;

	Window clientLeader;

	CompMatch match;
	CompMatch currentMatch;
};

class PrivateScaleWindow :
    public CompositeWindowInterface,
    public GLWindowInterface
{
    public:
	PrivateScaleWindow (CompWindow *);
	~PrivateScaleWindow ();

	bool damageRect (bool, const CompRect &);

	bool glPaint (const GLWindowPaintAttrib &, const GLMatrix &,
		      const CompRegion &, unsigned int);

	bool isNeverScaleWin () const;
	bool isScaleWin () const;

	bool adjustScaleVelocity ();

	static bool compareWindowsDistance (ScaleWindow *, ScaleWindow *);

    public:
	CompWindow         *window;
	CompositeWindow    *cWindow;
	GLWindow           *gWindow;
	ScaleWindow        *sWindow;
	ScaleScreen        *sScreen;
	PrivateScaleScreen *spScreen;

	ScaleSlot *slot;

	int sid;
	int distance;

	GLfloat xVelocity, yVelocity, scaleVelocity;
	GLfloat scale;
	GLfloat lastTargetScale, lastTargetX, lastTargetY;
	GLfloat tx, ty;
	float   delta;
	bool    adjust;

	float lastThumbOpacity;
};


#endif
