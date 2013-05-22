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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>

#include <compiztoolbox/compiztoolbox.h>

#include <core/pluginclasshandler.h>
#include <core/propertywriter.h>

#include "switcher_options.h"

#define ZOOMED_WINDOW_MASK (1 << 0)
#define NORMAL_WINDOW_MASK (1 << 1)

class SwitchScreen :
    public BaseSwitchScreen,
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public PluginClassHandler<SwitchScreen,CompScreen>,
    public SwitcherOptions
{
    public:
	SwitchScreen (CompScreen *screen);
	~SwitchScreen ();

	void setZoom ();

	void preparePaint (int);
	void donePaint ();

	bool glPaintOutput (const GLScreenPaintAttrib &,
			    const GLMatrix &, const CompRegion &,
			    CompOutput *, unsigned int);

	void updateWindowList (int count);
	void createWindowList (int count);

	void getMinimizedAndMatch (bool &minimizedOption,
				   CompMatch *&match);
	bool getMipmap ();
	void switchToWindow (bool toNext);
	void handleSelectionChange (bool toNext, int nextIdx);
	int countWindows ();
	void handleEvent (XEvent *event);
	void initiate (SwitchWindowSelection selection,
		       bool                  showPopup);
	void windowRemove (CompWindow *w);

	bool adjustVelocity ();

	Window	   lastActiveWindow;

	CompWindow *zoomedWindow;

	float zoom;

	bool switching;
	bool zooming;
	int  zoomMask;

	GLfloat mVelocity;
	GLfloat tVelocity;
	GLfloat sVelocity;

	int pos;
	int move;

	float translate;
	float sTranslate;
};

class SwitchWindow :
    public BaseSwitchWindow,
    public WindowInterface,
    public CompositeWindowInterface,
    public GLWindowInterface,
    public PluginClassHandler<SwitchWindow,CompWindow>
{
    public:
	SwitchWindow (CompWindow *window);

	bool managed () const;

	bool damageRect (bool initial, const CompRect &rect);

	bool glPaint (const GLWindowPaintAttrib &, const GLMatrix &,
		      const CompRegion &, unsigned int);

	void paintThumb (const GLWindowPaintAttrib &attrib,
			 const GLMatrix            &transform,
			 unsigned int              mask,
			 int                       x,
			 int                       y);
	void updateIconTexturedWindow (GLWindowPaintAttrib  &sAttrib,
				       int                  &wx,
				       int                  &wy,
				       int                  x,
				       int                  y,
				       GLTexture            *icon);
	void updateIconNontexturedWindow (GLWindowPaintAttrib  &sAttrib,
					  int                  &wx,
					  int                  &wy,
					  float                &width,
					  float                &height,
					  int                  x,
					  int                  y,
					  GLTexture            *icon);
	void updateIconPos (int   &wx,
			    int   &wy,
			    int   x,
			    int   y,
			    float width,
			    float height);

	IconMode getIconMode ();

	SwitchScreen    *sScreen;
};

#define MwmHintsDecorations (1L << 1)

typedef struct {
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
} MwmHints;

extern const unsigned short WIDTH;
extern const unsigned short HEIGHT;
extern const unsigned short SPACE;

extern const unsigned short BOX_WIDTH;

#define WINDOW_WIDTH(count) (WIDTH * (count) + (SPACE << 1))
#define WINDOW_HEIGHT (HEIGHT + (SPACE << 1))

#define SWITCH_SCREEN(s) \
    SwitchScreen *ss = SwitchScreen::get (s)

#define SWITCH_WINDOW(w) \
    SwitchWindow *sw = SwitchWindow::get (w)

class SwitchPluginVTable :
    public CompPlugin::VTableForScreenAndWindow<SwitchScreen, SwitchWindow>
{
    public:

	bool init ();
};


