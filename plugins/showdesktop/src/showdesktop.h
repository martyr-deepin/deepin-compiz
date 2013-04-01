/*
 * showdesktop.h
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * Give credit where credit is due, keep the authors message below.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *   - Diogo Ferreira (playerX) <diogo@beryl-project.org>
 *   - Danny Baumann <maniac@beryl-project.org>
 *   - Sam Spilsbury <smspillaz@gmail.com>
 *
 *
 * Copyright (c) 2007 Diogo "playerX" Ferreira
 *
 * This wouldn't have been possible without:
 *   - Ideas from the compiz community (mainly throughnothing's)
 *   - David Reveman's work
 *
 * */

#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <core/atoms.h>
#include <cmath>

#include "showdesktop_options.h"

#define WIN_X(w) ((w)->x () - (w)->border ().left)
#define WIN_Y(w) ((w)->y () - (w)->border ().top)
#define WIN_W(w) ((w)->width () + (w)->border ().left + (w)->border ().right)
#define WIN_H(w) ((w)->height () + (w)->border ().top + (w)->border ().bottom)

#define OFF_LEFT(w) ((w)->width () + (w)->border ().right)
#define OFF_RIGHT(w) ((w)->border ().left)
#define OFF_TOP(w) ((w)->height () + (w)->border ().bottom)
#define OFF_BOTTOM(w) ((w)->border ().top)

#define MOVE_LEFT(w) ((WIN_X (w) + (WIN_W (w) / 2)) < (screen->width () / 2))
#define MOVE_UP(w) ((WIN_Y (w) + (WIN_H (w) / 2)) < (screen->height () / 2))

extern const unsigned short SD_STATE_OFF;
extern const unsigned short SD_STATE_ACTIVATING;
extern const unsigned short SD_STATE_ON;
extern const unsigned short SD_STATE_DEACTIVATING;

class ShowdesktopPlacer
{
    public:
	ShowdesktopPlacer ();

	int placed;
	int onScreenX, onScreenY;
	int offScreenX, offScreenY;
	int origViewportX;
	int origViewportY;
};

class ShowdesktopScreen :
    public PluginClassHandler <ShowdesktopScreen, CompScreen>,
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public ShowdesktopOptions
{
    public:

	ShowdesktopScreen (CompScreen *);

	CompositeScreen *cScreen;
	GLScreen	*gScreen;

	int state;
	int moreAdjust;

	void
	handleEvent (XEvent *event);

	void
	preparePaint (int);

	bool
	glPaintOutput (const GLScreenPaintAttrib &,
		       const GLMatrix &,
		       const CompRegion &,
		       CompOutput *,
		       unsigned int);

	void
	donePaint ();

	void
	enterShowDesktopMode ();

	void
	leaveShowDesktopMode (CompWindow *);

	int
	prepareWindows (int oldState);



};

class ShowdesktopWindow:
    public PluginClassHandler <ShowdesktopWindow, CompWindow>,
    public WindowInterface,
    public GLWindowInterface
{
    public:
	ShowdesktopWindow (CompWindow *);
	~ShowdesktopWindow ();

	CompWindow *window;
	GLWindow *gWindow;

	int sid;
	int distance;

	ShowdesktopPlacer *placer;

	GLfloat xVelocity, yVelocity;
	GLfloat tx, ty;

	unsigned int notAllowedMask;
	unsigned int stateMask;

	bool showdesktoped;
	bool wasManaged;

	float delta;
	bool adjust;

	bool
	glPaint (const GLWindowPaintAttrib &,
	         const GLMatrix &,
	         const CompRegion &,
	         unsigned int);

	void
	getAllowedActions (unsigned int &,
		           unsigned int &);

	bool
	focus ();

	bool
	is ();

	void
	setHints (bool enterSDMode);

	void
	repositionPlacer (int oldState);

	int
	adjustVelocity ();

	int state;
	int moreAdjust;
};

/* shortcut macros, usually named X_SCREEN and X_WINDOW
 * these might seem overly complicated but they are shortcuts so 
 * we don't have to access the privates arrays all the time
 * */
#define SD_SCREEN(s)							       \
    ShowdesktopScreen *ss = ShowdesktopScreen::get(s)

#define SD_WINDOW(w)							       \
    ShowdesktopWindow *sw = ShowdesktopWindow::get(w)

/* class vtable definition */
class ShowdesktopPluginVTable:
    public CompPlugin::VTableForScreenAndWindow <ShowdesktopScreen, ShowdesktopWindow>
{
    public:

	bool init ();
};
