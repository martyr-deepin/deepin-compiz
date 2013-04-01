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

#include "screenshot_options.h"

#include <core/screen.h>
#include <core/propertywriter.h>

#include <compiztoolbox/compiztoolbox.h>
#include <composite/composite.h>
#include <opengl/opengl.h>

class ShotScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public PluginClassHandler<ShotScreen, CompScreen>,
    public ScreenshotOptions
{
    public:
	ShotScreen (CompScreen *screen);

	bool initiate (CompAction            *action,
		       CompAction::State     state,
		       CompOption::Vector    &options);
	bool terminate (CompAction            *action,
			CompAction::State     state,
			CompOption::Vector    &options);
	void handleMotionEvent (int xRoot,
				int yRoot);

	void handleEvent (XEvent *event);
	bool glPaintOutput (const GLScreenPaintAttrib &attrib,
			    const GLMatrix            &matrix,
			    const CompRegion          &region,
			    CompOutput                *output,
			    unsigned int               mask);
	void paint (CompOutput::ptrList &outputs,
		    unsigned int        mask);

	CompositeScreen *cScreen;
	GLScreen        *gScreen;

	CompScreen::GrabHandle mGrabIndex;
	Bool                   mGrab;

	int  mX1, mY1, mX2, mY2;
};

class ShotPluginVTable :
    public CompPlugin::VTableForScreen<ShotScreen>
{
    public:

	bool init ();
};

