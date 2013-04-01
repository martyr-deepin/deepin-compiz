/*
 * Copyright © 2005 Novell, Inc.
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

#include "move_options.h"

#include <core/screen.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>


#define NUM_KEYS (sizeof (mKeys) / sizeof (mKeys[0]))

extern const unsigned short KEY_MOVE_INC;

extern const unsigned short SNAP_BACK;
extern const unsigned short SNAP_OFF;

struct _MoveKeys {
    const char *name;
    int        dx;
    int        dy;
} mKeys[] = {
    { "Left",  -1,  0 },
    { "Right",  1,  0 },
    { "Up",     0, -1 },
    { "Down",   0,  1 }
};

class MoveScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public PluginClassHandler<MoveScreen,CompScreen>,
    public MoveOptions
{
    public:
	MoveScreen (CompScreen *screen);
	~MoveScreen ();

	CompositeScreen *cScreen;

	void updateOpacity ();

	void handleEvent (XEvent *);

	bool registerPaintHandler (compiz::composite::PaintHandler *pHnd);
	void unregisterPaintHandler ();

	CompWindow *w;
	int        savedX;
	int        savedY;
	int        x;
	int        y;
	Region     region;
	int        status;
	KeyCode    key[NUM_KEYS];

	int releaseButton;

	GLushort moveOpacity;

	CompScreen::GrabHandle grab;

	Cursor moveCursor;

	unsigned int origState;

	int	snapOffY;
	int	snapBackY;

	bool hasCompositing;

	bool yConstrained;
};

class MoveWindow :
    public GLWindowInterface,
    public PluginClassHandler<MoveWindow,CompWindow>
{
    public:
	MoveWindow (CompWindow *window) :
	    PluginClassHandler<MoveWindow,CompWindow> (window),
	    window (window),
	    gWindow (GLWindow::get (window)),
	    cWindow (CompositeWindow::get (window))
	{
	    if (gWindow)
		GLWindowInterface::setHandler (gWindow, false);
	};

	bool glPaint (const GLWindowPaintAttrib &, const GLMatrix &,
		      const CompRegion &, unsigned int);

	CompWindow      *window;
	GLWindow        *gWindow;
	CompositeWindow *cWindow;
};

#define MOVE_SCREEN(s) \
    MoveScreen *ms = MoveScreen::get (s)

#define MOVE_WINDOW(w) \
    MoveWindow *mw = MoveWindow::get (w)


class MovePluginVTable :
    public CompPlugin::VTableForScreenAndWindow<MoveScreen, MoveWindow>
{
    public:

	bool init ();
};

