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

#include "rotate.h"
#include <core/atoms.h>

#include <math.h>

static const float ROTATE_POINTER_SENSITIVITY_FACTOR = 0.05f;

COMPIZ_PLUGIN_20090315 (rotate, RotatePluginVTable)

bool
RotateScreen::setOption (const CompString &name, CompOption::Value &value)
{
    unsigned int index;

    bool rv = RotateOptions::setOption (name, value);

    if (!rv || !CompOption::findOption (getOptions (), name, &index))
	return false;

    switch (index) {
	case RotateOptions::Sensitivity:
	    mPointerSensitivity = optionGetSensitivity () *
		    ROTATE_POINTER_SENSITIVITY_FACTOR;
	    break;
	default:
	    break;
    }

    return rv;
}

bool
RotateScreen::adjustVelocity (int size, int invert)
{
    float xrot, yrot, adjust, amount;

    if (mMoving)
    {
	xrot = mMoveTo + (mXrot + mBaseXrot);
    }
    else
    {
	xrot = mXrot;
	if (mXrot < -180.0f / size)
	    xrot = 360.0f / size + mXrot;
	else if (mXrot > 180.0f / size)
	    xrot = mXrot - 360.0f / size;
    }

    adjust = -xrot * 0.05f * optionGetAcceleration ();
    amount = fabs (xrot);
    if (amount < 10.0f)
	amount = 10.0f;
    else if (amount > 30.0f)
	amount = 30.0f;

    if (mSlow)
	adjust *= 0.05f;

    mXVelocity = (amount * mXVelocity + adjust) / (amount + 2.0f);

    yrot = mYrot;
    /* Only snap if more than 2 viewports */
    if (size > 2)
    {
	if (mYrot > 50.0f && ((mSnapTop && invert == 1) ||
	    (mSnapBottom && invert != 1)))
	    yrot -= 90.f;
	else if (mYrot < -50.0f && ((mSnapTop && invert != 1) ||
	         (mSnapBottom && invert == 1)))
	    yrot += 90.f;
    }

    adjust = -yrot * 0.05f * optionGetAcceleration ();
    amount = fabs (mYrot);
    if (amount < 10.0f)
	amount = 10.0f;
    else if (amount > 30.0f)
	amount = 30.0f;

    mYVelocity = (amount * mYVelocity + adjust) / (amount + 2.0f);

    return (fabs (xrot) < 0.1f && fabs (mXVelocity) < 0.2f &&
	    fabs (yrot) < 0.1f && fabs (mYVelocity) < 0.2f);
}

void
RotateScreen::releaseMoveWindow ()
{
    CompWindow *w = screen->findWindow (mMoveWindow);
    if (w)
	w->syncPosition ();

    mMoveWindow = None;
}

void 
RotateScreen::preparePaint (int msSinceLastPaint)
{
    float oldXrot = mXrot + mBaseXrot;

    if (mGrabIndex || mMoving)
    {
	int   steps;
	float amount, chunk;

	amount = msSinceLastPaint * 0.05f * optionGetSpeed ();
	steps  = amount / (0.5f * optionGetTimestep ());
	if (!steps) steps = 1;
	chunk  = amount / (float) steps;

	while (steps--)
	{
	    mXrot += mXVelocity * chunk;
	    mYrot += mYVelocity * chunk;

	    if (mXrot > 360.0f / screen->vpSize ().width ())
	    {
		mBaseXrot += 360.0f / screen->vpSize ().width ();
		mXrot -= 360.0f / screen->vpSize ().width ();
	    }
	    else if (mXrot < 0.0f)
	    {
		mBaseXrot -= 360.0f / screen->vpSize ().width ();
		mXrot += 360.0f / screen->vpSize ().width ();
	    }

	    if (cubeScreen->invert () == -1)
	    {
		if (mYrot > 45.0f)
		{
		    mYVelocity = 0.0f;
		    mYrot = 45.0f;
		}
		else if (mYrot < -45.0f)
		{
		    mYVelocity = 0.0f;
		    mYrot = -45.0f;
		}
	    }
	    else
	    {
		if (mYrot > 100.0f)
		{
		    mYVelocity = 0.0f;
		    mYrot = 100.0f;
		}
		else if (mYrot < -100.0f)
		{
		    mYVelocity = 0.0f;
		    mYrot = -100.0f;
		}
	    }

	    if (mGrabbed)
	    {
		mXVelocity /= 1.25f;
		mYVelocity /= 1.25f;

		if (fabs (mXVelocity) < 0.01f)
		    mXVelocity = 0.0f;
		if (fabs (mYVelocity) < 0.01f)
		    mYVelocity = 0.0f;
	    }
	    else if (adjustVelocity (screen->vpSize ().width (), cubeScreen->invert ()))
	    {
		mXVelocity = 0.0f;
		mYVelocity = 0.0f;

		if (fabs (mYrot) < 0.1f)
		{
		    CompOption::Vector o (0);
		    float xrot;
		    int   tx;

		    xrot = mBaseXrot + mXrot;
		    if (xrot < 0.0f)
			tx = (screen->vpSize ().width () * xrot / 360.0f) - 0.5f;
		    else
			tx = (screen->vpSize ().width () * xrot / 360.0f) + 0.5f;

		    /* flag end of rotation */
		    cubeScreen->rotationState (CubeScreen::RotationNone);

		    screen->moveViewport (tx, 0, true);

		    mXrot = 0.0f;
		    mYrot = 0.0f;
		    mBaseXrot = mMoveTo = 0.0f;
		    mMoving = false;

		    if (mGrabIndex)
		    {
			screen->removeGrab (mGrabIndex, &mSavedPointer);
			mGrabIndex = 0;
		    }

		    if (mMoveWindow)
		    {
			CompWindow *w;

			w = screen->findWindow (mMoveWindow);
			if (w)
			{
			    w->move (mMoveWindowX - w->x (), 0);
			    w->syncPosition ();
			}
		    }
		    /* only focus default window if switcher isn't active */
		    else if (!screen->grabExist ("switcher"))
			screen->focusDefaultWindow ();

		    mMoveWindow = 0;

		    screen->handleCompizEvent ("rotate", "end_viewport_switch", o);
		}
		break;
	    }
	}

	if (mMoveWindow)
	{
	    CompWindow *w;

	    w = screen->findWindow (mMoveWindow);
	    if (w)
	    {
		float xrot = (screen->vpSize ().width () * (mBaseXrot + mXrot)) / 360.0f;
		w->moveToViewportPosition (mMoveWindowX - xrot * screen->width (),
					   w->y (), false);
	    }
	}
    }

    if (mMoving)
    {
	if (fabs (mXrot + mBaseXrot + mMoveTo) <=
	    (360.0 / (screen->vpSize ().width () * 2.0)))
	{
	    mProgress = fabs (mXrot + mBaseXrot + mMoveTo) /
			 (360.0 / (screen->vpSize ().width () * 2.0));
	}
	else if (fabs (mXrot + mBaseXrot) <= (360.0 / (screen->vpSize ().width () * 2.0)))
	{
	    mProgress = fabs (mXrot + mBaseXrot) /
			(360.0 / (screen->vpSize ().width () * 2.0));
	}
	else
	{
	    mProgress += fabs (mXrot + mBaseXrot - oldXrot) /
			 (360.0 / (screen->vpSize ().width () * 2.0));
	    mProgress = MIN (mProgress, 1.0);
	}
    }
    else if (mProgress != 0.0f || mGrabbed)
    {
	int   steps;
	float amount, chunk;

	amount = msSinceLastPaint * 0.05f * optionGetSpeed ();
	steps = amount / (0.5f * optionGetTimestep ());
	if (!steps)
	    steps = 1;

	chunk = amount / (float) steps;

	while (steps--)
	{
	    float dt, adjust, tamount;

	    if (mGrabbed)
		dt = 1.0 - mProgress;
	    else
		dt = 0.0f - mProgress;

	    adjust = dt * 0.15f;
	    tamount = fabs (dt) * 1.5f;
	    if (tamount < 0.2f)
		tamount = 0.2f;
	    else if (tamount > 2.0f)
		tamount = 2.0f;

	    mProgressVelocity = (tamount * mProgressVelocity + adjust) /
				   (tamount + 1.0f);

	    mProgress += mProgressVelocity * chunk;

	    if (fabs (dt) < 0.01f && fabs (mProgressVelocity) < 0.0001f)
	    {
		if (mGrabbed)
		    mProgress = 1.0f;
		else
		    mProgress = 0.0f;

		break;
	    }
	}
    }

    if (cubeScreen->invert () == 1 && !cubeScreen->unfolded ())
    {
	mZoomTranslate = optionGetZoom () * mProgress;
    }
    else
    {
	mZoomTranslate = 0.0;
    }

    cScreen->preparePaint (msSinceLastPaint);
}

void
RotateScreen::donePaint ()
{
    if (mGrabIndex || mMoving ||
	(mProgress != 0.0 && mProgress != 1.0))
    {
	if ((!mGrabbed && !mSnapTop && !mSnapBottom) ||
	    mXVelocity || mYVelocity || mProgressVelocity)
	{
	    cScreen->damageScreen ();
	}
    }

    cScreen->donePaint ();
}

void 
RotateScreen::cubeGetRotation (float &x, float &v, float &progress)
{
    cubeScreen->cubeGetRotation (x, v, progress);

    x += mBaseXrot + mXrot;
    v += mYrot;
    progress = MAX (progress, mProgress);
}

bool 
RotateScreen::glPaintOutput (const GLScreenPaintAttrib &sAttrib,
			     const GLMatrix            &transform, 
			     const CompRegion          &region, 
			     CompOutput                *output,
			     unsigned int              mask)
{
    if (mGrabIndex || mMoving || mProgress != 0.0f)
    {
	GLMatrix sTransform = transform;

	sTransform.translate (0.0f, 0.0f, -mZoomTranslate);

	mask &= ~PAINT_SCREEN_REGION_MASK;
	mask |= PAINT_SCREEN_TRANSFORMED_MASK;
	
	return gScreen->glPaintOutput (sAttrib, sTransform, region, output, mask);
    }

    return gScreen->glPaintOutput (sAttrib, transform, region, output, mask);
}

bool 
RotateScreen::initiate (CompAction         *action,
			CompAction::State  state,
			CompOption::Vector &options)
{
    CompOption::Vector o (0);

    if (screen->vpSize ().width () < 2)
	return false;

    if (mRotateTimer.active () && mGrabWindow)
    {
	if (screen->otherGrabExist ("rotate", "move", NULL))
	    return false;
    }
    else
    {
	if (screen->otherGrabExist ("rotate", "switcher", "cube", NULL))
	    return false;
    }

    mMoving = false;
    mSlow   = false;

    /* Set the rotation state for cube - if action is non-NULL,
	we set it to manual (as we were called from the 'Initiate
	Rotation' binding. Otherwise, we set it to Change. */
    if (action)
	cubeScreen->rotationState (CubeScreen::RotationManual);
    else
	cubeScreen->rotationState (CubeScreen::RotationChange);

    screen->handleCompizEvent ("rotate", "start_viewport_switch", o);

    if (!mGrabIndex)
    {
	mGrabIndex = screen->pushGrab (screen->invisibleCursor (), "rotate");
	if (mGrabIndex)
	{
	    int x, y;

	    x = CompOption::getIntOptionNamed (options, "x");
	    y = CompOption::getIntOptionNamed (options, "y");

	    mSavedPointer.set (x, y);
	}
    }

    if (mGrabIndex)
    {
	mMoveTo = 0.0f;

	mGrabbed = true;
	mSnapTop = optionGetSnapTop ();
	mSnapBottom = optionGetSnapBottom ();

	if (state & CompAction::StateInitButton)
	    action->setState (action->state () | CompAction::StateTermButton);

	if (state & CompAction::StateInitKey)
	    action->setState (action->state () | CompAction::StateTermKey);
    }

    return true;
}

bool 
RotateScreen::terminate (CompAction         *action,
			 CompAction::State  state,
			 CompOption::Vector &options)
{
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "root" );

    if (!xid || screen->root () == xid)
    {
	if (mGrabIndex)
	{
	    if (!xid)
	    {
		mSnapTop = false;
		mSnapBottom = false;
	    }

	    mGrabbed = false;
	    cScreen->damageScreen ();
	}
    }

    action->setState (action->state () & ~(CompAction::StateTermButton | CompAction::StateTermKey));

    return false;
}

bool 
RotateScreen::rotate (CompAction         *action,
		      CompAction::State  state,
		      CompOption::Vector &options,
		      int                direction)
{
    if (screen->vpSize ().width () < 2)
	return false;

    if (screen->otherGrabExist ("rotate", "move", "switcher",
				"group-drag", "cube", NULL))
	return false;

    if (!direction)
	return false;

    if (mMoveWindow)
	releaseMoveWindow ();

    /* we allow the grab to fail here so that we can rotate on
	drag-and-drop */
    if (!mGrabIndex)
    {
	CompOption::Vector o (0);

	o.push_back (CompOption ("root", CompOption::TypeInt));
	o.push_back (CompOption ("x", CompOption::TypeInt));
	o.push_back (CompOption ("y", CompOption::TypeInt));
	
	o[0].value ().set ((int) screen->root ());
	o[1].value ().set (CompOption::getIntOptionNamed (options, "x", 0));
	o[2].value ().set (CompOption::getIntOptionNamed (options, "y", 0));
	
	initiate (NULL, 0, o);
    }

    mMoving  = true;
    mMoveTo += (360.0f / screen->vpSize ().width ()) * direction;
    mGrabbed = false;

    cScreen->damageScreen ();

    return false;
}

bool 
RotateScreen::rotateWithWindow (CompAction         *action,
				CompAction::State  state,
				CompOption::Vector &options,
				int                direction)
{

    Window xid;

    if (screen->vpSize ().width () < 2)
	return false;

    if (!direction)
	return false;

    xid = (Window) CompOption::getIntOptionNamed (options, "window");

    if (mMoveWindow != xid)
    {
	releaseMoveWindow ();

	if (!mGrabIndex && !mMoving)
	{
	    CompWindow *w = screen->findWindow (xid);
	    if (w)
	    {
		if (!(w->type () & (CompWindowTypeDesktopMask |
				    CompWindowTypeDockMask)))
		{
		    if (!(w->state () & CompWindowStateStickyMask))
		    {
			mMoveWindow  = w->id ();
			mMoveWindowX = w->x ();

			if (optionGetRaiseOnRotate ())
			    w->raise ();
		    }
		}
	    }
	}
    }

    if (!mGrabIndex)
    {
	CompOption::Vector o (0);

	o.push_back (CompOption ("root", CompOption::TypeInt));
	o.push_back (CompOption ("x", CompOption::TypeInt));
	o.push_back (CompOption ("y", CompOption::TypeInt));
	
	o[0].value ().set ((int) screen->root ());
	o[1].value ().set (CompOption::getIntOptionNamed (options, "x", 0));
	o[2].value ().set (CompOption::getIntOptionNamed (options, "y", 0));
	
	initiate (NULL, 0, o);
    }

    if (mGrabIndex)
    {
	mMoving  = true;
	mMoveTo += (360.0f / screen->vpSize ().width ()) * direction;
	mGrabbed = false;

	cScreen->damageScreen ();
    }

    return false;
}

bool 
RotateScreen::rotateFlip (int direction)
{

    int                warpX;
    CompOption::Vector o (0);

    mMoveTo = 0.0f;
    mSlow = false;

    if (screen->otherGrabExist ("rotate", "move", "group-drag", NULL))
	return false;

    warpX = pointerX - (screen->width () * direction);
    if (direction == -1)
	screen->warpPointer (screen->width () - 10, 0);
    else
	screen->warpPointer (10 - screen->width (), 0);
    lastPointerX = warpX;

    o.push_back (CompOption ("root", CompOption::TypeInt));
    o.push_back (CompOption ("x", CompOption::TypeInt));
    o.push_back (CompOption ("y", CompOption::TypeInt));

    o[0].value ().set ((int) screen->root ());
    o[1].value ().set (0);
    o[2].value ().set (pointerY);

    rotate (NULL, 0, o, direction);

    XWarpPointer (screen->dpy (), None, None, 0, 0, 0, 0, direction, 0);
    mSavedPointer.setX (lastPointerX + (9 * direction));

    return false;
}



bool
RotateScreen::rotateEdgeFlip (CompAction         *action,
			      CompAction::State  state,
			      CompOption::Vector &options,
			      int                direction)
{
    CompOption::Vector o (0);

    if (screen->vpSize ().width () < 2)
	return false;

    if (screen->otherGrabExist ("rotate", "move", "group-drag", NULL))
	return false;

    if (state & CompAction::StateInitEdgeDnd)
    {
	if (!optionGetEdgeFlipDnd ())
	    return false;

	if (screen->otherGrabExist ("rotate", NULL))
	    return false;
    }
    else if (screen->otherGrabExist ("rotate", "group-drag", NULL))
    {
	if (!optionGetEdgeFlipWindow ())
	    return false;

	if (!mGrabWindow)
	    return false;

	/* bail out if window is horizontally maximized or fullscreen,
	 * or sticky  */
	if (mGrabWindow->state () & (CompWindowStateMaximizedHorzMask |
				     CompWindowStateFullscreenMask |
				     CompWindowStateStickyMask))
	    return false;
    }
    else if (screen->otherGrabExist ("rotate", NULL))
    {
	/* in that case, 'group-drag' must be the active screen grab */
	if (!optionGetEdgeFlipWindow ())
	    return false;
    }
    else
    {
	if (!optionGetEdgeFlipPointer ())
	    return false;
    }

    o.push_back (CompOption ("root", CompOption::TypeInt));
    o.push_back (CompOption ("x", CompOption::TypeInt));
    o.push_back (CompOption ("y", CompOption::TypeInt));

    o[0].value ().set ((int) screen->root ());
    o[1].value ().set (CompOption::getIntOptionNamed (options, "x", 0));
    o[2].value ().set (CompOption::getIntOptionNamed (options, "y", 0));

    if (optionGetFlipTime () == 0 || (mMoving && !mSlow))
    {
	int pointerDx = pointerX - lastPointerX;
	int warpX;
	
	if (direction == -1)
	{
	    warpX = pointerX + screen->width ();
	    screen->warpPointer (screen->width () - 10, 0);
	    lastPointerX = warpX - pointerDx;
	    rotate (NULL, 0, o, direction);

	    XWarpPointer (screen->dpy (), None, None, 0, 0, 0, 0, -1, 0);
	    mSavedPointer.setX (lastPointerX - 9);
	}
	else
	{
	    warpX = pointerX - screen->width ();
	    screen->warpPointer (10 - screen->width (), 0);
	    lastPointerX = warpX - pointerDx;
	    rotate (NULL, 0, o, direction);

	    XWarpPointer (screen->dpy (), None, None, 0, 0, 0, 0, 1, 0);
	    mSavedPointer.setX (lastPointerX + 9);
	}

    }
    else
    {
	if (!mRotateTimer.active ())
	{
	    mRotateTimer.start (boost::bind (&RotateScreen::rotateFlip, this, direction),
				optionGetFlipTime (), (float) optionGetFlipTime () * 1.2);
	}

	mMoving  = true;
	mMoveTo  += 360.0f / screen->vpSize ().width () * direction;
	mSlow    = true;

	if (state & CompAction::StateInitEdge)
	    action->setState (action->state () | CompAction::StateTermEdge);

	if (state & CompAction::StateInitEdgeDnd)
	    action->setState (action->state () | CompAction::StateTermEdgeDnd);

	cScreen->damageScreen ();
    }

    return false;
}

bool 
RotateScreen::flipTerminate (CompAction         *action,
			     CompAction::State  state,
			     CompOption::Vector &options)

{
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "root", 0);

 
    if (xid && screen->root () != xid)
	return false;

    if (mRotateTimer.active ())
    {
	mRotateTimer.stop ();
	
	if (mSlow)
	{
	    mMoveTo = 0.0f;
	    mSlow = false;
	}

	cScreen->damageScreen ();
    }

    action->setState (action->state () & ~(CompAction::StateTermEdge |
					   CompAction::StateTermEdgeDnd));

    return false;
}


int 
RotateScreen::rotateToDirection (int face)
{
    int delta;

    delta = face - screen->vp ().x () - (mMoveTo / (360.0f / screen->vpSize ().width ()));
    if (delta > screen->vpSize ().width () / 2)
	delta -= screen->vpSize ().width ();
    else if (delta < -(screen->vpSize ().width () / 2))
	delta += screen->vpSize ().width ();

    return delta;
}

bool
RotateScreen::rotateTo (CompAction         *action,
			CompAction::State  state,
			CompOption::Vector &options,
			int                face,
			bool               withWindow)
{
    CompOption::Vector o (0);

    if (face < 0)
	face = CompOption::getIntOptionNamed (options, "face", screen->vp ().x ());

    if (face > screen->vpSize ().width ())
	return false;

    o.push_back (CompOption ("root", CompOption::TypeInt));
    o.push_back (CompOption ("x", CompOption::TypeInt));
    o.push_back (CompOption ("y", CompOption::TypeInt));

    o[0].value ().set ((int) screen->root ());
    o[1].value ().set (CompOption::getIntOptionNamed (options, "x", pointerX));
    o[2].value ().set (CompOption::getIntOptionNamed (options, "y", pointerY));

    if (withWindow)
    {
	o.push_back (CompOption ("window", CompOption::TypeInt));
	o[3].value ().set (CompOption::getIntOptionNamed (options, "window", 0));
	rotateWithWindow (NULL, 0, o, rotateToDirection (face));
    }
    else
      rotate (NULL, 0, o, rotateToDirection (face));

    return false;
}

void 
RotateScreen::handleEvent (XEvent *event)
{
    switch (event->type) {
	case MotionNotify:
	    if (screen->root () == event->xmotion.root)
	    {
		if (mGrabIndex)
		{
		    if (mGrabbed)
		    {
			GLfloat pointerDx, pointerDy;

			pointerDx = pointerX - lastPointerX;
			pointerDy = pointerY - lastPointerY;

			if (event->xmotion.x_root < 50	       ||
			    event->xmotion.y_root < 50	       ||
			    event->xmotion.x_root > screen->width () - 50 ||
			    event->xmotion.y_root > screen->height () - 50)
			{
			    screen->warpPointer ((screen->width () / 2) - pointerX,
						 (screen->height () / 2) - pointerY);
			}

			if (optionGetInvertY ())
			    pointerDy = -pointerDy;

			mXVelocity += pointerDx * mPointerSensitivity *
			    cubeScreen->invert ();
			mYVelocity += pointerDy * mPointerSensitivity;

			cScreen->damageScreen ();
		    }
		    else
		    {
			mSavedPointer.setX (mSavedPointer.x () + pointerX - lastPointerX);
			mSavedPointer.setY (mSavedPointer.y () + pointerY - lastPointerY);
		    }
		}
	    }
	    break;
	case ClientMessage:
	    if (event->xclient.message_type == Atoms::desktopViewport)
	    {
		if (screen->root () == event->xclient.window)
		{
		    int dx;

		    if (screen->otherGrabExist ("rotate", "switcher", "cube", NULL))
			break;

		    /* reset movement */
		    mMoveTo = 0.0f;

		    dx = (event->xclient.data.l[0] / screen->width ()) - screen->vp ().x ();
		    if (dx)
		    {
			Window             win;
			int                i, x, y;
			unsigned int       ui;
			CompOption::Vector o (0);

			XQueryPointer (screen->dpy (), screen->root (),
				       &win, &win, &x, &y, &i, &i, &ui);

			if (dx * 2 > screen->vpSize ().width ())
			    dx -= screen->vpSize ().width ();
			else if (dx * 2 < -screen->vpSize ().width ())
			    dx += screen->vpSize ().width ();
			
			o.push_back (CompOption ("root", CompOption::TypeInt));
			o.push_back (CompOption ("x", CompOption::TypeInt));
			o.push_back (CompOption ("y", CompOption::TypeInt));

			o[0].value ().set ((int) screen->root ());
			o[1].value ().set (x);
			o[2].value ().set (y);

			rotate (NULL, 0, o, dx);
		    }
		}
	    }
	default:
	    break;
    }

    screen->handleEvent (event);
}

void
RotateWindow::activate ()
{
    if (window->placed () &&
	!screen->otherGrabExist ("rotate", "switcher", "cube", NULL))
    {
	int dx;

	/* reset movement */
	rScreen->mMoveTo = 0.0f;

	dx = window->defaultViewport ().x ();
	dx -= screen->vp ().x ();
	if (dx)
	{
	    Window             win;
	    int                i, x, y;
	    unsigned int       ui;
	    CompOption::Vector o (0);

	    XQueryPointer (screen->dpy (), screen->root (),
			   &win, &win, &x, &y, &i, &i, &ui);

	    if (dx * 2 > screen->vpSize ().width ())
		dx -= screen->vpSize ().width ();
	    else if (dx * 2 < -screen->vpSize ().width ())
		dx += screen->vpSize ().width ();

	    o.push_back (CompOption ("root", CompOption::TypeInt));
	    o.push_back (CompOption ("x", CompOption::TypeInt));
	    o.push_back (CompOption ("y", CompOption::TypeInt));

	    o[0].value ().set ((int) screen->root ());
	    o[1].value ().set (x);
	    o[2].value ().set (y);

	    rScreen->rotate (NULL, 0, o, dx);
	}
    }

    window->activate ();
}

void 
RotateWindow::grabNotify (int x, int y, unsigned int state, unsigned int mask)
{
    if (!rScreen->mGrabWindow)
    {
	rScreen->mGrabMask   = mask;
	rScreen->mGrabWindow = window;
    }

    window->grabNotify (x, y, state, mask);
}

void 
RotateWindow::ungrabNotify ()
{
    if (window == rScreen->mGrabWindow)
    {
	rScreen->mGrabMask   = 0;
	rScreen->mGrabWindow = NULL;
    }

    window->ungrabNotify ();
}

RotateScreen::RotateScreen (CompScreen *s) :
    PluginClassHandler<RotateScreen,CompScreen> (s),
    gScreen (GLScreen::get (s)),
    cScreen (CompositeScreen::get (s)),
    cubeScreen (CubeScreen::get (s)),
    mSnapTop (false),
    mSnapBottom (false),
    mGrabIndex (0),
    mXrot (0.0f),
    mXVelocity (0.0f),
    mYrot (0.0f),
    mYVelocity (0.0f),
    mBaseXrot (0.0f),
    mMoving (false),
    mMoveTo (0.0f),
    mMoveWindow (0),
    mMoveWindowX (0),
    mSavedPointer (0,0),
    mGrabbed (false),
    mSlow (false),
    mGrabMask (0),
    mGrabWindow (0),
    mProgress (0.0f),
    mProgressVelocity (0.0f),
    mZoomTranslate (0.0f)
{
    mPointerSensitivity = optionGetSensitivity () *
	ROTATE_POINTER_SENSITIVITY_FACTOR;

#define ROTATEBIND(name) boost::bind (&RotateScreen::name, this, _1, _2, _3)
#define ROTATEBINDOPT(name, ...) boost::bind (&RotateScreen::name, this, _1, _2, _3, __VA_ARGS__)

    optionSetInitiateButtonInitiate (ROTATEBIND (initiate));
    optionSetInitiateButtonTerminate (ROTATEBIND (terminate));
    optionSetRotateLeftKeyInitiate (ROTATEBINDOPT (rotate, -1));
    optionSetRotateLeftButtonInitiate (ROTATEBINDOPT (rotate, -1));
    optionSetRotateRightKeyInitiate (ROTATEBINDOPT (rotate, 1));
    optionSetRotateRightButtonInitiate (ROTATEBINDOPT (rotate, 1));
    optionSetRotateLeftWindowKeyInitiate (ROTATEBINDOPT (rotateWithWindow, -1));
    optionSetRotateLeftWindowButtonInitiate (ROTATEBINDOPT (rotateWithWindow, -1));
    optionSetRotateRightWindowKeyInitiate (ROTATEBINDOPT (rotateWithWindow, 1));
    optionSetRotateRightWindowButtonInitiate (ROTATEBINDOPT (rotateWithWindow, 1));

    optionSetRotateTo1KeyInitiate (ROTATEBINDOPT(rotateTo, 0, false));
    optionSetRotateTo2KeyInitiate (ROTATEBINDOPT(rotateTo, 1, false));
    optionSetRotateTo3KeyInitiate (ROTATEBINDOPT(rotateTo, 2, false));
    optionSetRotateTo4KeyInitiate (ROTATEBINDOPT(rotateTo, 3, false));
    optionSetRotateTo5KeyInitiate (ROTATEBINDOPT(rotateTo, 4, false));
    optionSetRotateTo6KeyInitiate (ROTATEBINDOPT(rotateTo, 5, false));
    optionSetRotateTo7KeyInitiate (ROTATEBINDOPT(rotateTo, 6, false));
    optionSetRotateTo8KeyInitiate (ROTATEBINDOPT(rotateTo, 7, false));
    optionSetRotateTo9KeyInitiate (ROTATEBINDOPT(rotateTo, 8, false));
    optionSetRotateTo10KeyInitiate (ROTATEBINDOPT(rotateTo, 9, false));
    optionSetRotateTo11KeyInitiate (ROTATEBINDOPT(rotateTo, 10, false));
    optionSetRotateTo12KeyInitiate (ROTATEBINDOPT(rotateTo, 11, false));
    optionSetRotateTo1WindowKeyInitiate (ROTATEBINDOPT(rotateTo, 0, true));
    optionSetRotateTo2WindowKeyInitiate (ROTATEBINDOPT(rotateTo, 1, true));
    optionSetRotateTo3WindowKeyInitiate (ROTATEBINDOPT(rotateTo, 2, true));
    optionSetRotateTo4WindowKeyInitiate (ROTATEBINDOPT(rotateTo, 3, true));
    optionSetRotateTo5WindowKeyInitiate (ROTATEBINDOPT(rotateTo, 4, true));
    optionSetRotateTo6WindowKeyInitiate (ROTATEBINDOPT(rotateTo, 5, true));
    optionSetRotateTo7WindowKeyInitiate (ROTATEBINDOPT(rotateTo, 6, true));
    optionSetRotateTo8WindowKeyInitiate (ROTATEBINDOPT(rotateTo, 7, true));
    optionSetRotateTo9WindowKeyInitiate (ROTATEBINDOPT(rotateTo, 8, true));
    optionSetRotateTo10WindowKeyInitiate (ROTATEBINDOPT(rotateTo, 9, true));
    optionSetRotateTo11WindowKeyInitiate (ROTATEBINDOPT(rotateTo, 10, true));
    optionSetRotateTo12WindowKeyInitiate (ROTATEBINDOPT(rotateTo, 11, true));

    optionSetRotateToKeyInitiate (ROTATEBINDOPT(rotateTo, -1, false));
    optionSetRotateWindowKeyInitiate (ROTATEBINDOPT(rotateTo, -1, true));

    optionSetRotateFlipLeftEdgeInitiate (ROTATEBINDOPT(rotateEdgeFlip, -1));
    optionSetRotateFlipLeftEdgeTerminate (ROTATEBIND (flipTerminate));
    optionSetRotateFlipRightEdgeInitiate (ROTATEBINDOPT(rotateEdgeFlip, 1));
    optionSetRotateFlipRightEdgeTerminate (ROTATEBIND (flipTerminate));

#undef ROTATEBIND
#undef ROTATEBINDOPT

    ScreenInterface::setHandler (screen);
    CompositeScreenInterface::setHandler (cScreen);
    GLScreenInterface::setHandler (gScreen);
    CubeScreenInterface::setHandler (cubeScreen);
}

RotateWindow::RotateWindow (CompWindow *w) :
    PluginClassHandler<RotateWindow,CompWindow> (w),
    window (w),
    rScreen (RotateScreen::get (screen))
{
    WindowInterface::setHandler (window);
}

bool
RotatePluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI) ||
	!CompPlugin::checkPluginABI ("cube", COMPIZ_CUBE_ABI))
	 return false;

    return true;
}

