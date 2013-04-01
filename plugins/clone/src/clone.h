#ifndef COMPIZ_CLONE_H
#define COMPIZ_CLONE_H
/*
 * Copyright © 2006 Novell, Inc.
 *
 * clone.h
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
 * Ported to Compiz 0.9 by:
 * Copyright (c) 2009 Sam Spilsbury <smspillaz@gmail.com>
 *
 * Author: David Reveman <davidr@novell.com>
 */


#include "clone_options.h"

#include <core/pluginclasshandler.h>
#include <composite/composite.h>

#include <opengl/opengl.h>

class Clone
{
    public:
	int src;
	int dst;
	CompRegion region;
	Window	   input;
};

class CloneScreen :
    public PluginClassHandler <CloneScreen, CompScreen>,
    public CloneOptions,
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface
{
    public:
	CloneScreen (CompScreen *);
	~CloneScreen ();

	CompositeScreen *cScreen;
	GLScreen	*gScreen;

	CompScreen::GrabHandle grabHandle;
	bool		       grab;

	float offset;

	bool transformed;

	std::list <Clone *> clones;
	int	   x, y;
	int	   grabbedOutput;
	int	   src, dst;

	void
	handleEvent (XEvent *);

	void
	preparePaint (int);

	bool
	glPaintOutput (const GLScreenPaintAttrib &,
		       const GLMatrix		   &,
		       const CompRegion	   &,
		       CompOutput	   *,
		       unsigned int);

	void
	donePaint ();

	void
	outputChangeNotify ();

	/* Internal class functions */

	void
	finish ();

	bool
	initiate (CompAction         *action,
	          CompAction::State  state,
	          CompOption::Vector options);

	bool
	terminate (CompAction         *action,
		   CompAction::State  state,
		   CompOption::Vector options);

	void
	setStrutsForCloneWindow (Clone *clone);

	void
	handleMotionEvent (CompPoint &p);

};

class CloneWindow :
    public PluginClassHandler <CloneWindow, CompWindow>,
    public GLWindowInterface
{
    public:
	CloneWindow (CompWindow *window);

	CompositeWindow *cWindow;
	GLWindow *gWindow;

	bool
	glPaint (const GLWindowPaintAttrib &attrib,
		 const GLMatrix &transform,
		 const CompRegion &region,
		 unsigned int mask);
};

#define CLONE_SCREEN(s)							       \
    CloneScreen *cs = CloneScreen::get (s)

#define CLONE_WINDOW(w)							       \
    CloneWindow *cw = CloneWindow::get (w)

class ClonePluginVTable :
    public CompPlugin::VTableForScreenAndWindow <CloneScreen, CloneWindow>
{
    public:

	bool init ();
};

#endif
