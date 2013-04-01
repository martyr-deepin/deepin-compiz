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

#ifndef _ROTATE_H
#define _ROTATE_H

#include <core/screen.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>
#include <cube/cube.h>

#include "rotate_options.h"

#define ROTATE_SCREEN(s) RotateScreen *rs = RotateScreen::get(s)
#define ROTATE_WINDOW(w) RotateWindow *rw = RotateWindow::get(w)


class RotateScreen :
    public PluginClassHandler<RotateScreen,CompScreen>,
    public GLScreenInterface,
    public CompositeScreenInterface,
    public CubeScreenInterface,
    public ScreenInterface,
    public RotateOptions
{
    public:
	RotateScreen (CompScreen *s);
	~RotateScreen () {};
	
	bool setOption (const CompString &name, CompOption::Value &value);

	void handleEvent (XEvent *event);
	
	void preparePaint (int);
	void donePaint ();

	bool glPaintOutput (const GLScreenPaintAttrib &,
			    const GLMatrix &, const CompRegion &, CompOutput *,
			    unsigned int);

	void cubeGetRotation (float &x, float &v, float &progress);
	
	bool adjustVelocity (int size, int invert);
	void releaseMoveWindow ();
	
	bool initiate (CompAction         *action,
		       CompAction::State  state,
		       CompOption::Vector &options);

	bool terminate (CompAction         *action,
		        CompAction::State  state,
		        CompOption::Vector &options);
			
	bool rotate (CompAction         *action,
		     CompAction::State  state,
		     CompOption::Vector &options,
		     int                direction);

	bool rotateWithWindow (CompAction         *action,
			       CompAction::State  state,
			       CompOption::Vector &options,
			       int                direction);

	bool rotateFlip (int direction);
	
	bool rotateEdgeFlip (CompAction         *action,
			     CompAction::State  state,
			     CompOption::Vector &options,
			     int                direction);

	bool flipTerminate (CompAction         *action,
			    CompAction::State  state,
			    CompOption::Vector &options);

	int rotateToDirection (int face);
	
	bool rotateTo (CompAction         *action,
		       CompAction::State  state,
		       CompOption::Vector &options,
		       int                face,
		       bool               withWindow);


    public:
	
	GLScreen        *gScreen;
	CompositeScreen *cScreen;
	CubeScreen      *cubeScreen;

	float                  mPointerSensitivity;
	
	bool                   mSnapTop;
	bool                   mSnapBottom;

	CompScreen::GrabHandle mGrabIndex;

	GLfloat                mXrot, mXVelocity;
	GLfloat                mYrot, mYVelocity;

	GLfloat                mBaseXrot;

	bool                   mMoving;
	GLfloat                mMoveTo;

	Window                 mMoveWindow;
	int                    mMoveWindowX;

	CompPoint              mSavedPointer;
	bool                   mGrabbed;

	CompTimer              mRotateTimer;
	bool                   mSlow;
	unsigned int           mGrabMask;
	CompWindow             *mGrabWindow;

	float                  mProgress;
	float                  mProgressVelocity;

	GLfloat                mZoomTranslate;
};


class RotateWindow :
    public PluginClassHandler<RotateWindow,CompWindow>,
    public WindowInterface
{
    public:
	RotateWindow (CompWindow *w);
	~RotateWindow () {};
	
	void grabNotify (int x, int y, unsigned int state, unsigned int mask);
	void ungrabNotify ();
	
	void activate ();
	
    public:
	CompWindow *window;
	RotateScreen *rScreen;
};

class RotatePluginVTable :
    public CompPlugin::VTableForScreenAndWindow<RotateScreen, RotateWindow>
{
    public:
	bool init ();
};


#endif
