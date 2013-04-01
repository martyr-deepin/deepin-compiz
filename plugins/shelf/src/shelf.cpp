/*
 * Compiz Shelf plugin
 *
 * shelf.h
 *
 * Copyright (C) 2007  Canonical Ltd.
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
 * Author(s): 
 * Kristian Lyngstøl <kristian@bohemians.org>
 * Danny Baumann <maniac@opencompositing.org>
 * Sam Spilsbury <smspillaz@gmail.com>
 *
 * Description:
 *
 * This plugin visually resizes a window to allow otherwise obtrusive
 * windows to be visible in a monitor-fashion. Use case: Anything with
 * progress bars, notification programs, etc.
 *
 * Todo: 
 *  - Check for XShape events
 *  - Handle input in a sane way
 *  - Mouse-over?
 */

#include "shelf.h"

COMPIZ_PLUGIN_20090315 (shelf, ShelfPluginVTable);

const float SHELF_MIN_SIZE = 50.0f; // Minimum pixelsize a window can be scaled to

/* Enables / Disables screen paint functions */
static void
toggleScreenFunctions (bool enabled)
{
    SHELF_SCREEN (screen);

    screen->handleEventSetEnabled (ss, enabled);
    ss->cScreen->preparePaintSetEnabled (ss, enabled);
    ss->gScreen->glPaintOutputSetEnabled (ss, enabled);
    ss->cScreen->donePaintSetEnabled (ss, enabled);
}

static void
toggleWindowFunctions (CompWindow *w, bool enabled)
{
    SHELF_WINDOW (w);

    sw->window->moveNotifySetEnabled (sw, enabled);
    sw->cWindow->damageRectSetEnabled (sw, enabled);
    sw->gWindow->glPaintSetEnabled (sw, enabled);
}

/* Checks if w is a ipw and returns the real window */
CompWindow *
ShelfWindow::getRealWindow ()
{
    ShelfedWindowInfo *run;

    SHELF_SCREEN (screen);

    foreach (run, ss->shelfedWindows)
    {
	if (window->id () == run->ipw)
	    return run->w;
    }

    return NULL;
}

void
ShelfWindow::saveInputShape (XRectangle **retRects,
			     int       *retCount,
			     int       *retOrdering)
{
    XRectangle *rects;
    int        count = 0, ordering;
    Display    *dpy = screen->dpy ();

    rects = XShapeGetRectangles (dpy, window->id (), ShapeInput, &count, &ordering);

    /* check if the returned shape exactly matches the window shape -
       if that is true, the window currently has no set input shape */
    if ((count == 1) &&
	(rects[0].x == -window->geometry ().border ()) &&
	(rects[0].y == -window->geometry ().border ()) &&
	(rects[0].width == (window->serverWidth () +
			    window->serverGeometry ().border ())) &&
	(rects[0].height == (window->serverHeight () +
			     window->serverGeometry (). border ())))
    {
	count = 0;
    }
    
    *retRects    = rects;
    *retCount    = count;
    *retOrdering = ordering;
}

/* Shape the input of the window when scaled.
 * Since the IPW will be dealing with the input, removing input
 * from the window entirely is a perfectly good solution. */
void
ShelfWindow::shapeInput ()
{
    Window     frame;
    Display    *dpy = screen->dpy();

    saveInputShape (&info->inputRects,
		    &info->nInputRects,
		    &info->inputRectOrdering);

    frame = window->frame();
    if (frame)
    {
	saveInputShape (&info->frameInputRects, &info->frameNInputRects,
			&info->frameInputRectOrdering);
    } 
    else
    {
	info->frameInputRects        = NULL;
	info->frameNInputRects       = -1;
	info->frameInputRectOrdering = 0;
    }

    /* clear shape */
    XShapeSelectInput (dpy, window->id(), NoEventMask);
    XShapeCombineRectangles  (dpy, window->id(), ShapeInput, 0, 0,
			      NULL, 0, ShapeSet, 0);
    
    if (frame)
	XShapeCombineRectangles  (dpy, window->frame(), ShapeInput, 0, 0,
				  NULL, 0, ShapeSet, 0);

    XShapeSelectInput (dpy, window->id(), ShapeNotify);
}

/* Restores the shape of the window:
 * If the window had a custom shape defined by inputRects then we restore
 * that in order with XShapeCombineRectangles.
 * Most windows have no specific defined shape so we can restore it with
 * setting the shape to a 0x0 mask
 */
void
ShelfWindow::unshapeInput ()
{
    Display *dpy = screen->dpy ();

    if (info->nInputRects)
    {
	XShapeCombineRectangles (dpy, window->id(), ShapeInput, 0, 0,
				 info->inputRects, info->nInputRects,
				 ShapeSet, info->inputRectOrdering);
    }
    else
    {
	XShapeCombineMask (dpy, window->id(), ShapeInput, 0, 0, None, ShapeSet);
    }

    if (info->frameNInputRects >= 0)
    {
	if (info->frameNInputRects)
	{
	    XShapeCombineRectangles (dpy, window->frame(), ShapeInput, 0, 0,
				     info->frameInputRects,
				     info->frameNInputRects,
				     ShapeSet,
				     info->frameInputRectOrdering);
	}
	else
	{
	    XShapeCombineMask (dpy, window->frame(), ShapeInput, 0, 0, None, ShapeSet);
	}
    }
}

void
ShelfScreen::preparePaint (int msSinceLastPaint)
{
    float      steps;

    steps =  (float) msSinceLastPaint / (float) optionGetAnimtime ();

    if (steps < 0.005)
	steps = 0.005;

    /* FIXME: should only loop over all windows if at least one animation
       is running */
    foreach (CompWindow *w, screen->windows ())
	ShelfWindow::get (w)->steps = steps;

    cScreen->preparePaint (msSinceLastPaint);
}

void
ShelfScreen::addWindowToList (ShelfedWindowInfo *info)
{
    shelfedWindows.push_back (info);
}

void
ShelfScreen::removeWindowFromList (ShelfedWindowInfo *info)
{
    shelfedWindows.remove (info);
}

/* Adjust size and location of the input prevention window
 */
void
ShelfWindow::adjustIPW ()
{
    XWindowChanges xwc;
    Display        *dpy = screen->dpy ();
    float          f_width, f_height;

    if (!info || !info->ipw)
	return;

    f_width  = window->width () + 2 * window->geometry ().border () +
	     window->border ().left + window->border ().right + 2.0f;
    f_width  *= targetScale;
    f_height = window->height () + 2 * window->geometry ().border () +
	     window->border ().top + window->border ().bottom + 2.0f;
    f_height *= targetScale;

    xwc.x          = window->x () - window->border ().left;
    xwc.y          = window->y () - window->border ().top;
    xwc.width      = (int) f_width;
    xwc.height     = (int) f_height;
    xwc.stack_mode = Below;
    /* XXX: This causes XConfigureWindow to break */
    //xwc.sibling    = window->id ();

    XMapWindow (dpy, info->ipw);

    XConfigureWindow (dpy, info->ipw,
		      CWStackMode | CWX | CWY | CWWidth | CWHeight,
		      &xwc);

}

void
ShelfScreen::adjustIPWStacking ()
{

    foreach (ShelfedWindowInfo *run, shelfedWindows)
    {
	if (!run->w->prev || run->w->prev->id () != run->ipw)
	    ShelfWindow::get (run->w)->adjustIPW ();
    }
}

/* Create an input prevention window */
void
ShelfWindow::createIPW ()
{
    Window               ipw;
    XSetWindowAttributes attrib;
    XWindowChanges       xwc;

    if (!info || info->ipw)
	return;

    attrib.override_redirect = true;
    //attrib.event_mask        = 0;
    
    ipw = XCreateWindow (screen->dpy (),
    			 screen->root (),
    			 0, 0, -100, -100, 0, CopyFromParent, InputOnly,
    			 CopyFromParent, CWOverrideRedirect, &attrib);
    			 
    xwc.x = window->serverGeometry ().x () - window->border ().left;
    xwc.y = window->serverGeometry ().y () - window->border ().top;
    xwc.width = window->serverGeometry ().width () +
			window->border ().left + window->border ().right;
    xwc.height = window->serverGeometry ().height () +
			window->border ().top  + window->border ().bottom;

    XMapWindow (screen->dpy (), ipw);

    XConfigureWindow (screen->dpy (), ipw, CWStackMode | CWX | CWY | CWWidth | CWHeight, &xwc);
 
    info->ipw = ipw;
}

ShelfedWindowInfo::ShelfedWindowInfo (CompWindow *window) :
    w (window),
    ipw (None),
    inputRects (NULL),
    nInputRects (0),
    inputRectOrdering (0),
    frameInputRects (NULL),
    frameNInputRects (0),
    frameInputRectOrdering (0)
{
}

ShelfedWindowInfo::~ShelfedWindowInfo ()
{
}


bool
ShelfWindow::handleShelfInfo ()
{
    SHELF_SCREEN (screen);

    if (targetScale == 1.0f && info)
    {
	if (info->ipw)
	    XDestroyWindow (screen->dpy (), info->ipw);

	unshapeInput ();
	ss->removeWindowFromList (info);

	delete info;
	info = NULL;

	return false;
    }
    else if (targetScale != 1.0f && !info)
    {
	info = new ShelfedWindowInfo (window);
	if (!info)
	    return false;

	shapeInput ();
	createIPW ();
	ss->addWindowToList (info);
    }

    return true;
}

/* Sets the scale level and adjust the shape */
void
ShelfWindow::scale (float fScale)
{
    if (window->wmType () & (CompWindowTypeDesktopMask | CompWindowTypeDockMask))
	return;

    targetScale = MIN (fScale, 1.0f);

    if ((float) window->width () * targetScale < SHELF_MIN_SIZE)
	targetScale = SHELF_MIN_SIZE / (float) window->width ();

    if (handleShelfInfo ())
	adjustIPW ();

    cWindow->addDamage ();
}

/* Binding for toggle mode. 
 * Toggles through three preset scale levels, 
 * currently hard coded to 1.0f (no scale), 0.5f and 0.25f.
 */
bool
ShelfScreen::trigger (CompAction         *action,
		      CompAction::State  state,
		      CompOption::Vector options)
{
    CompWindow *w = screen->findWindow (screen->activeWindow ());
    if (!w)
	return true;

    SHELF_WINDOW (w);

    if (sw->targetScale > 0.5f)
	sw->scale (0.5f);
    else if (sw->targetScale <= 0.5f && sw->targetScale > 0.25)
	sw->scale (0.25f);
    else 
	sw->scale (1.0f);

    toggleWindowFunctions (w, true);
    toggleScreenFunctions (true);

    return true;
}

/* Reset window to 1.0f scale */
bool
ShelfScreen::reset (CompAction         *action,
		    CompAction::State  state,
		    CompOption::Vector options)
{
    CompWindow *w = screen->findWindow (screen->activeWindow ());
    if (!w)
	return true;

    SHELF_WINDOW (w);

    sw->scale (1.0f);

    toggleWindowFunctions (w, true);
    toggleScreenFunctions (true);

    return true;
}

/* Returns the ratio to multiply by to get a window that's 1/ration the
 * size of the screen.
 */
static inline float
shelfRat (CompWindow *w,
	  float      ratio)
{
    float winHeight    = (float) w->height ();
    float winWidth     = (float) w->width ();
    float screenHeight = (float) screen->height ();
    float screenWidth  = (float) screen->width ();
    float ret;

    if (winHeight / screenHeight < winWidth / screenWidth)
	ret = screenWidth / winWidth;
    else
	ret = screenHeight / winHeight;

    return ret / ratio;
}

bool
ShelfScreen::triggerScreen (CompAction         *action,
			    CompAction::State  state,
			    CompOption::Vector options)
{
    CompWindow *w = screen->findWindow (screen->activeWindow ());
    if (!w)
	return true;

    SHELF_WINDOW (w);

    /* FIXME: better should save calculated ratio and reuse it */
    if (sw->targetScale > shelfRat (w, 2.0f))
	sw->scale (shelfRat (w, 2.0f));
    else if (sw->targetScale <= shelfRat (w, 2.0f) && 
	     sw->targetScale > shelfRat (w, 3.0f))
	sw->scale (shelfRat (w, 3.0f));
    else if (sw->targetScale <= shelfRat (w, 3.0f) && 
	     sw->targetScale > shelfRat (w, 6.0f))
	sw->scale (shelfRat (w, 6.0f));
    else 
	sw->scale (1.0f);

    toggleWindowFunctions (w, true);
    toggleScreenFunctions (true);

    return true;
}

/* shelfInc and shelfDec are matcing functions and bindings;
 * They increase and decrease the scale factor by 'interval'.
 */

bool
ShelfScreen::inc (CompAction         *action,
		  CompAction::State  state,
		  CompOption::Vector options)
{
    CompWindow *w = screen->findWindow (screen->activeWindow ());
    if (!w)
	return true;

    SHELF_WINDOW (w);

    sw->scale (sw->targetScale / optionGetInterval ());

    toggleWindowFunctions (w, true);
    toggleScreenFunctions (true);

    return true;
}

bool
ShelfScreen::dec (CompAction         *action,
		  CompAction::State  state,
		  CompOption::Vector options)
{
    CompWindow *w = screen->findWindow (screen->activeWindow ());
    if (!w)
	return true;

    SHELF_WINDOW (w);

    sw->scale (sw->targetScale * optionGetInterval ());

    toggleWindowFunctions (w, true);
    toggleScreenFunctions (true);

    return true;
}

void
ShelfWindow::handleButtonPress (unsigned int x,
				unsigned int y)
{
    SHELF_SCREEN (screen);

    if (!screen->otherGrabExist ("shelf", 0))
    {
	window->activate ();
	ss->grabbedWindow = window->id ();
	ss->grabIndex = screen->pushGrab (ss->moveCursor, "shelf");

	ss->lastPointerX = x;
	ss->lastPointerY = y;
    }
}

void
ShelfScreen::handleMotionEvent (unsigned int x,
				unsigned int y)
{
    CompWindow   *w;
    int dx, dy;

    if (!grabIndex)
	return;

    w = screen->findWindow (grabbedWindow);
    if (!w)
	return;

    dx = x - lastPointerX;
    dy = y - lastPointerY;

    w->move (dx, dy, true);
    w->syncPosition ();

    lastPointerX += dx;
    lastPointerY += dy;
}

void
ShelfWindow::handleButtonRelease ()
{
    SHELF_SCREEN (screen);

    ss->grabbedWindow = None;
    if (ss->grabIndex)
    {
	window->moveInputFocusTo ();
	screen->removeGrab (ss->grabIndex, NULL);
	ss->grabIndex = 0;
    }
}

void
ShelfWindow::handleEnter (XEvent *event)
{
    XEvent enterEvent;

    memcpy (&enterEvent.xcrossing, &event->xcrossing,
	    sizeof (XCrossingEvent));
    enterEvent.xcrossing.window = window->id ();

    XSendEvent (screen->dpy (), window->id (),
		false, EnterWindowMask, &enterEvent);
}

CompWindow *
ShelfScreen::findRealWindowID (Window wid)
{
    CompWindow *orig;

    orig = screen->findWindow (wid);
    if (!orig)
	return NULL;

    return ShelfWindow::get (orig)->getRealWindow ();
}

void
ShelfScreen::handleEvent (XEvent *event)
{
    CompWindow *w = NULL, *oldPrev = NULL, *oldNext = NULL;

    switch (event->type)
    {
	case EnterNotify:
	    w = findRealWindowID (event->xcrossing.window);
	    if (w)
		ShelfWindow::get (w)->handleEnter (event);
	    break;
	case ButtonPress:
	    w = findRealWindowID (event->xbutton.window);
	    if (w)
		ShelfWindow::get (w)->handleButtonPress (event->xbutton.x_root,
				   			 event->xbutton.y_root);
	    break;
	case ButtonRelease:
	    w = screen->findWindow (grabbedWindow);
	    if (w)
		ShelfWindow::get (w)->handleButtonRelease ();
	    break;
	case MotionNotify:
	    handleMotionEvent (event->xmotion.x_root,
			       event->xmotion.y_root);
	    break;
	case ConfigureNotify:
	    w = screen->findWindow (event->xconfigure.window);
	    if (w)
	    {
		oldPrev = w->prev;
		oldNext = w->next;
	    }
	    break;
    }

    screen->handleEvent (event);

    switch (event->type)
    {
	case ConfigureNotify:
	    if (w) /* already assigned above */
	    {
		if (w->prev != oldPrev || w->next != oldNext)
		{
		    /* restacking occured, ensure ipw stacking */
		    adjustIPWStacking ();
		}
	    }
	    break;
    }
}

/* The window was damaged, adjust the damage to fit the actual area we
 * care about.
 */

bool
ShelfWindow::damageRect (bool     initial,
			 const CompRect &rect)
{
    bool status = false;

    if (mScale != 1.0f)
    {
	float xTranslate, yTranslate;

	xTranslate = window->border ().left * (mScale - 1.0f);
	yTranslate = window->border ().top * (mScale - 1.0f);

	cWindow->damageTransformedRect (mScale, mScale,
				        xTranslate, yTranslate, rect);
	status = true;
    }

    status |= cWindow->damageRect (initial, rect);

    return status;
}

/* Scale the window if it is supposed to be scaled.
 * Translate into place.
 *
 * FIXME: Merge the two translations.
 */
bool
ShelfWindow::glPaint (const GLWindowPaintAttrib &attrib,
		      const GLMatrix		&transform,
		      const CompRegion		&region,
		      unsigned int		mask)
{
    if (targetScale != mScale && steps)
    {
	mScale += (float) steps * (targetScale - mScale);
	if (fabsf (targetScale - mScale) < 0.005)
	    mScale = targetScale;
    }

    if (mScale != 1.0f)
    {
	GLMatrix      mTransform = transform;
	float         xTranslate, yTranslate;

	xTranslate = window->border ().left * (mScale - 1.0f);
	yTranslate = window->border ().top * (mScale - 1.0f);

	mTransform.translate (window->x (), window->y (), 0);
	mTransform.scale (mScale, mScale, 0);
	mTransform.translate (xTranslate / mScale - window->x (),
			      yTranslate / mScale - window->y (),
			      0.0f);
	
	mask |= PAINT_WINDOW_TRANSFORMED_MASK;

	return gWindow->glPaint (attrib, mTransform, region, mask);  
    }
    else
    {
	return gWindow->glPaint (attrib, transform, region, mask);
    }
}

bool
ShelfScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
			    const GLMatrix	      &transform,
			    const CompRegion	      &region,
			    CompOutput		      *output,
			    unsigned int	      mask)
{
    if (!shelfedWindows.empty ())
	mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK;

    return gScreen->glPaintOutput (attrib, transform, region, output, mask);
}

/* Checks to see if we need to adjust the window further and hence
 * damages it's area. Also checks if we still need to paint the area of the
 * window
 */
void
ShelfScreen::donePaint ()
{
    bool stillPainting = false;
    /* Fixme: should create internal window list */
    foreach (CompWindow *w, screen->windows ())
    {
	SHELF_WINDOW (w);

	if (sw->mScale != sw->targetScale)
        {
	    sw->cWindow->addDamage ();
	}

	if (sw->mScale == 1.0f && sw->targetScale == 1.0f)
	    toggleWindowFunctions (w, false);
	else
	    stillPainting = true;
    }

    if (!stillPainting)
	toggleScreenFunctions (false);

    cScreen->donePaint ();
}

void
ShelfWindow::moveNotify (int dx, int dy, bool immediate)
{
    if (targetScale != 1.00f)
	adjustIPW ();

    window->moveNotify (dx, dy, immediate);
}

/* Configuration, initialization, boring stuff. --------------------- */
ShelfScreen::ShelfScreen (CompScreen *screen) :
    PluginClassHandler <ShelfScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    grabIndex (0),
    grabbedWindow (None),
    moveCursor (XCreateFontCursor (screen->dpy (), XC_fleur)),
    lastPointerX (0),
    lastPointerY (0)
{
    ScreenInterface::setHandler (screen, false);
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);

    optionSetTriggerKeyInitiate (boost::bind (&ShelfScreen::trigger, this, _1,
						_2, _3));
    optionSetResetKeyInitiate (boost::bind (&ShelfScreen::reset, this, _1 , _2,
					     _3));
    optionSetTriggerscreenKeyInitiate (boost::bind (&ShelfScreen::triggerScreen,
						    this, _1, _2, _3));
    optionSetIncButtonInitiate (boost::bind (&ShelfScreen::inc, this, _1, _2,
								_3));
    optionSetDecButtonInitiate (boost::bind (&ShelfScreen::dec, this, _1, _2,
								_3));
}

ShelfScreen::~ShelfScreen ()
{
    if (moveCursor)
	XFreeCursor (screen->dpy (), moveCursor);
}

ShelfWindow::ShelfWindow (CompWindow *window) :
    PluginClassHandler <ShelfWindow, CompWindow> (window),
    window (window),
    cWindow (CompositeWindow::get (window)),
    gWindow (GLWindow::get (window)),
    mScale (1.0f),
    targetScale (1.0f),
    steps (0),
    info (NULL)
{
    WindowInterface::setHandler (window, false);
    CompositeWindowInterface::setHandler (cWindow, false);
    GLWindowInterface::setHandler (gWindow, false);
}

ShelfWindow::~ShelfWindow ()
{
    if (info)
    {
	targetScale = 1.0f;
	/* implicitly frees sw->info */
	handleShelfInfo ();
    }
}

/* Check for necessary plugin dependencies and for Xorg shape extension.
 * If we don't have either, bail out */
bool
ShelfPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;
    if (!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI))
	return false;
    if (!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

    if (!screen->XShape ())
    {
	compLogMessage ("shelf", CompLogLevelError,
			"No Shape extension found. Shelfing not possible \n");
	return false;
    }

    return true;
}
