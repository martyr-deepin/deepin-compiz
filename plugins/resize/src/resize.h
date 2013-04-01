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

#ifndef _RESIZE_H
#define _RESIZE_H

#include <core/screen.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>

#include "resize_options.h"
#include "resize-logic.h"
#include "resize-defs.h"

class ResizeScreen :
    public PluginClassHandler<ResizeScreen,CompScreen>,
    public GLScreenInterface,
    public ScreenInterface,
    public ResizeOptions
{
    public:
	ResizeScreen (CompScreen *s);
	~ResizeScreen ();

	void handleEvent (XEvent *event);

	bool glPaintOutput (const GLScreenPaintAttrib &,
			    const GLMatrix &, const CompRegion &, CompOutput *,
			    unsigned int);

	void optionChanged (CompOption *o, Options);
	void resizeMaskValueToKeyMask (int valueMask,
				       int *mask);

	void glPaintRectangle (const GLScreenPaintAttrib &sAttrib,
			       const GLMatrix            &transform,
			       CompOutput                *output,
			       unsigned short            *borderColor,
			       unsigned short            *fillColor);

    public:
	ResizeLogic logic;

	GLScreen        *gScreen;
};

class ResizeWindow :
    public WindowInterface,
    public CompositeWindowInterface,
    public GLWindowInterface,
    public PluginClassHandler<ResizeWindow,CompWindow>
{
    public:
	ResizeWindow (CompWindow *w);
	~ResizeWindow ();

	bool damageRect (bool, const CompRect &);

	bool glPaint (const GLWindowPaintAttrib &, const GLMatrix &,
		      const CompRegion &, unsigned int);

	void getStretchScale (BoxPtr pBox, float *xScale, float *yScale);

    public:
	CompWindow      *window;
	GLWindow        *gWindow;
	CompositeWindow *cWindow;
	ResizeScreen    *rScreen;
};

class ResizePluginVTable :
    public CompPlugin::VTableForScreenAndWindow<ResizeScreen, ResizeWindow>
{
    public:
	bool init ();
};

#endif
