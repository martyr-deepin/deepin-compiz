/*
 * showdesktop.cpp
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

#include "showdesktop.h"

COMPIZ_PLUGIN_20090315 (showdesktop, ShowdesktopPluginVTable);

const unsigned short SD_STATE_OFF          = 0;
const unsigned short SD_STATE_ACTIVATING   = 1;
const unsigned short SD_STATE_ON           = 2;
const unsigned short SD_STATE_DEACTIVATING = 3;

/* non interfacing code, aka the logic of the plugin */
bool
ShowdesktopWindow::is ()
{
    SD_SCREEN (screen);

    if (window->grabbed ())
	return false;

    if (!window->focus ())
	return false;

    if (!ss->optionGetWindowMatch ().evaluate (window))
	return false;

    if (window->wmType () & (CompWindowTypeDesktopMask |
			     CompWindowTypeDockMask))
	return false;

    if (window->state () & CompWindowStateSkipPagerMask)
	return false;

    return true;
}

void
ShowdesktopWindow::setHints (bool enterSDMode)
{
    unsigned int state = window->state ();

    showdesktoped = enterSDMode;

    if (enterSDMode)
    {
	stateMask = state & CompWindowStateSkipPagerMask;
	state |= CompWindowStateSkipPagerMask;
	notAllowedMask = (CompWindowActionMoveMask |
			  CompWindowActionResizeMask);

	window->changeState (state);

	//#warning need to make window->managed wrappable
	//window->setManaged (false)
    }
    else
    {
	//window->setManaged (wasManaged);

	state &= ~CompWindowStateSkipPagerMask;
	state |= (stateMask & CompWindowStateSkipPagerMask);
	notAllowedMask = 0;
	stateMask = 0;

	window->changeState (state);
    }
}

void
ShowdesktopWindow::repositionPlacer (int oldState)
{
    if (!placer)
	return;

    SD_SCREEN (screen);

    if (oldState == SD_STATE_OFF)
    {
	placer->onScreenX = window->x ();
	placer->onScreenY = window->y ();
	placer->origViewportX = screen->vp ().x ();
	placer->origViewportY = screen->vp ().y ();
    }

    switch (ss->optionGetDirection ())
    {
    case ShowdesktopOptions::DirectionUp:
	placer->offScreenX = window->x ();
	placer->offScreenY = screen->workArea ().y () - OFF_TOP (window) +
	                         ss->optionGetWindowPartSize ();
	break;
    case ShowdesktopOptions::DirectionDown:
	placer->offScreenX = window->x ();
	placer->offScreenY = screen->workArea ().y () +
	                         screen->workArea ().height ()
							+ OFF_BOTTOM (window) -
				 ss->optionGetWindowPartSize ();
	break;
    case ShowdesktopOptions::DirectionLeft:
	placer->offScreenX = screen->workArea ().x () - OFF_LEFT (window) +
	                         ss->optionGetWindowPartSize ();
	placer->offScreenY = window->y ();
	break;
    case ShowdesktopOptions::DirectionRight:
	placer->offScreenX = screen->workArea ().x () +
	                         screen->workArea ().width ()
							+ OFF_RIGHT (window) -
				 ss->optionGetWindowPartSize ();
	placer->offScreenY = window->y ();
	break;
    case ShowdesktopOptions::DirectionUpDown:
	placer->offScreenX = window->x ();
	if (MOVE_UP (window))
    	    placer->offScreenY = screen->workArea ().y () - OFF_TOP (window) +
		                     ss->optionGetWindowPartSize ();
	else
	    placer->offScreenY = screen->workArea ().y () +
		                     screen->workArea ().height ()
							+ OFF_BOTTOM (window) -
				     ss->optionGetWindowPartSize ();
	break;
    case ShowdesktopOptions::DirectionLeftRight:
	placer->offScreenY = window->y ();
	if (MOVE_LEFT (window))
    	    placer->offScreenX = screen->workArea ().x () - OFF_LEFT (window) +
		                     ss->optionGetWindowPartSize ();
	else
	    placer->offScreenX = screen->workArea ().x () +
		                     screen->workArea ().width ()
							+ OFF_RIGHT (window) -
				     ss->optionGetWindowPartSize ();
	break;
    case ShowdesktopOptions::DirectionToCorners:
	if (MOVE_LEFT (window))
	    placer->offScreenX = screen->workArea ().x () - OFF_LEFT (window) +
		                     ss->optionGetWindowPartSize ();
	else
	    placer->offScreenX = screen->workArea ().x () +
		                     screen->workArea ().width ()
							+ OFF_RIGHT (window) -
				     ss->optionGetWindowPartSize ();
	if (MOVE_UP(window))
	    placer->offScreenY = screen->workArea ().y () - OFF_TOP (window) +
		                     ss->optionGetWindowPartSize ();
	else
	    placer->offScreenY = screen->workArea ().y () +
		                     screen->workArea ().height ()
							+ OFF_BOTTOM (window) -
		     		     ss->optionGetWindowPartSize ();
	break;
    default:
	break;
    }
}

int
ShowdesktopScreen::prepareWindows (int oldState)
{
    int count = 0;

    foreach (CompWindow *w, screen->windows ())
    {
	SD_WINDOW (w);

	if (!sw->is ())
	    continue;

	if (!sw->placer)
	    sw->placer = new ShowdesktopPlacer ();

	if (!sw->placer)
	    continue;

	sw->repositionPlacer (oldState);

	sw->placer->placed   = true;
	sw->adjust           = true;
	w->setShowDesktopMode  (true);

	sw->setHints (true);

	if (sw->tx)
	    sw->tx -= (sw->placer->onScreenX - sw->placer->offScreenX);
	if (sw->ty)
	    sw->ty -= (sw->placer->onScreenY - sw->placer->offScreenY);

	w->move (sw->placer->offScreenX - w->x (),
		 sw->placer->offScreenY - w->y (),
		 true);

	w->syncPosition ();

	count++;
    }

    return count;
}


int
ShowdesktopWindow::adjustVelocity ()
{
    float dx, dy, adjust, amount;
    float x1, y1;
    float baseX, baseY;

    SD_SCREEN (screen);

    x1 = y1 = 0.0;

    if (ss->state == SD_STATE_ACTIVATING)
    {
	x1 = placer->offScreenX;
	y1 = placer->offScreenY;
	baseX = placer->onScreenX;
	baseY = placer->onScreenY;
    }
    else
    {
	x1 = placer->onScreenX;
	y1 = placer->onScreenY;
	baseX = placer->offScreenX;
	baseY = placer->offScreenY;
    }

    dx = x1 - (baseX + tx);

    adjust = dx * 0.15f;
    amount = fabs (dx) * 1.5f;
    if (amount < 0.5f)
	amount = 0.5f;
    else if (amount > 5.0f)
	amount = 5.0f;

    xVelocity = (amount * xVelocity + adjust) / (amount + 1.0f);

    dy = y1 - (baseY + ty);

    adjust = dy * 0.15f;
    amount = fabs (dy) * 1.5f;
    if (amount < 0.5f)
	amount = 0.5f;
    else if (amount > 5.0f)
	amount = 5.0f;

    yVelocity = (amount * yVelocity + adjust) / (amount + 1.0f);

    if (fabs (dx) < 0.1f && fabs (xVelocity) < 0.2f &&
	fabs (dy) < 0.1f && fabs (yVelocity) < 0.2f)
    {
	xVelocity = yVelocity = 0.0f;
	tx = x1 - baseX;
	ty = y1 - baseY;

	return 0;
    }
    return 1;
}

/* this function gets called periodically (about every 15ms on this machine),
 * animation takes place here */
void
ShowdesktopScreen::preparePaint (int msSinceLastPaint)
{
    cScreen->preparePaint (msSinceLastPaint);

    if ((state == SD_STATE_ACTIVATING) ||
	(state == SD_STATE_DEACTIVATING))
    {
	int steps;
	float amount, chunk;

	amount = msSinceLastPaint * 0.05f * optionGetSpeed ();
	steps = amount / (0.5f * optionGetTimestep ());
	if (!steps)
    	    steps = 1;
	chunk = amount / (float)steps;

	while (steps--)
	{
	    moreAdjust = 0;

	    foreach (CompWindow *w, screen->windows ())
	    {
		SD_WINDOW (w);
		if (sw->adjust)
		{
		    sw->adjust = sw->adjustVelocity ();

		    moreAdjust |= sw->adjust;

    		    sw->tx += sw->xVelocity * chunk;
		    sw->ty += sw->yVelocity * chunk;
		}
	    }
	    if (!moreAdjust)
		break;
	}
    }
}
bool
ShowdesktopScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
				  const GLMatrix	    &transform,
				  const CompRegion          &region,
				  CompOutput *output,
				  unsigned int mask)
{
    if ((state == SD_STATE_ACTIVATING) ||
	(state == SD_STATE_DEACTIVATING))
	mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK;

    return gScreen->glPaintOutput (attrib, transform, region, output, mask);
}

void
ShowdesktopScreen::donePaint ()
{
    if (moreAdjust)
	cScreen->damageScreen ();
    else
    {
	if ((state == SD_STATE_ACTIVATING) ||
	    (state == SD_STATE_DEACTIVATING))
	{
	    if (state == SD_STATE_ACTIVATING)
	    {
		state = SD_STATE_ON;
	    }
	    else
    	    {
		bool inSDMode = false;

		foreach (CompWindow *w, screen->windows ())
		{
		    if (w->inShowDesktopMode ())
			inSDMode = true;
		    else
		    {
			SD_WINDOW (w);
			if (sw->placer)
			{
			    delete sw->placer;
			    sw->placer = NULL;
			    sw->tx     = 0;
			    sw->ty     = 0;
			}
		    }
		}

		if (inSDMode)
		    state = SD_STATE_ON;
		else
		    state = SD_STATE_OFF;
	    }

	    cScreen->damageScreen ();
	}
    }

    cScreen->donePaint ();
}

bool
ShowdesktopWindow::glPaint (const GLWindowPaintAttrib &attrib,
			    const GLMatrix	      &transform,
			    const CompRegion	      &region,
			    unsigned int	      mask)
{
    SD_SCREEN (screen);

    if ((ss->state == SD_STATE_ACTIVATING) ||
	(ss->state == SD_STATE_DEACTIVATING))
    {
	GLMatrix wTransform = transform;
	GLWindowPaintAttrib wAttrib    = attrib;

	if (adjust)
	{
	    float offsetX, offsetY;

	    offsetX = (ss->state == SD_STATE_DEACTIVATING) ?
		      (placer->offScreenX - placer->onScreenX) :
		      (placer->onScreenX - placer->offScreenX);
	    offsetY = (ss->state == SD_STATE_DEACTIVATING) ?
		      (placer->offScreenY - placer->onScreenY) :
		      (placer->onScreenY - placer->offScreenY);

	    mask |= PAINT_WINDOW_TRANSFORMED_MASK;

	    wTransform.translate (window->x (), window->y (), 0.0f);
	    wTransform.scale (1.0f, 1.0f, 1.0f);
	    wTransform.translate(tx + offsetX - window->x (),
			     	 ty + offsetY - window->y (), 0.0f);
	}

	return gWindow->glPaint (wAttrib, wTransform, region, mask);
    }
    else if (ss->state == SD_STATE_ON)
    {
	GLWindowPaintAttrib wAttrib = attrib;

	if (window->inShowDesktopMode ())
	    wAttrib.opacity = wAttrib.opacity *
		              ss->optionGetWindowOpacity ();

	return gWindow->glPaint (wAttrib, transform, region, mask);
    }
    else
    {
	return gWindow->glPaint (attrib, transform, region, mask);
    }
}

void
ShowdesktopScreen::handleEvent (XEvent *event)
{
    switch (event->type)
    {
    case PropertyNotify:
	if (event->xproperty.atom == Atoms::desktopViewport)
	{
	    SD_SCREEN (screen);

	    if ((ss->state == SD_STATE_ON) ||
		(ss->state == SD_STATE_ACTIVATING))
	    {
		screen->leaveShowDesktopMode (NULL);
	    }
	}
	break;
    }

    screen->handleEvent (event);
}

void
ShowdesktopWindow::getAllowedActions (unsigned int &setActions,
				      unsigned int &clearActions)
{
    window->getAllowedActions (setActions, clearActions);

    clearActions |= notAllowedMask;
}


void
ShowdesktopScreen::enterShowDesktopMode ()
{
    if (state == SD_STATE_OFF || state == SD_STATE_DEACTIVATING)
    {
	int count = prepareWindows (state);
	if (count > 0)
	{
	    XSetInputFocus (screen->dpy (), screen->root (),
			    RevertToPointerRoot, CurrentTime);
	    state = SD_STATE_ACTIVATING;
	    cScreen->damageScreen ();
	}
    }

    screen->enterShowDesktopMode ();
}

void
ShowdesktopScreen::leaveShowDesktopMode (CompWindow *w)
{
    if (state != SD_STATE_OFF)
    {
	foreach (CompWindow *cw, screen->windows ())
	{
	    SD_WINDOW (cw);

    	    if (w && (w->id () != cw->id ()))
		continue;

	    if (sw->placer && sw->placer->placed)
	    {
		sw->adjust         = true;
		sw->placer->placed = false;

		/* adjust onscreen position to handle viewport changes  */
		sw->tx += (sw->placer->onScreenX - sw->placer->offScreenX);
		sw->ty += (sw->placer->onScreenY - sw->placer->offScreenY);

		sw->placer->onScreenX += (sw->placer->origViewportX -
					  screen->vp (). x ())
							* screen->width ();
		sw->placer->onScreenY += (sw->placer->origViewportY -
					  screen->vp ().y ())
							* screen->height ();

		cw->move   (sw->placer->onScreenX - cw->x (),
			    sw->placer->onScreenY - cw->y (),
			    true);
		cw->syncPosition ();

		sw->setHints (false);
		cw->setShowDesktopMode (false);
	    }
	}
	state = SD_STATE_DEACTIVATING;
	cScreen->damageScreen ();
    }

    screen->leaveShowDesktopMode (w);
}

bool
ShowdesktopWindow::focus ()
{
    bool ret;

/*    if (sw->showdesktoped)
	w->managed = sw->wasManaged;*/

    ret = window->focus ();

/*    if (sw->showdesktoped)
	w->managed = false; */

    return ret;
}

ShowdesktopPlacer::ShowdesktopPlacer () :
    placed (0),
    onScreenX (0),
    onScreenY (0),
    offScreenX (0),
    offScreenY (0),
    origViewportX (0),
    origViewportY (0)
{
}

/* screen initialization */
ShowdesktopScreen::ShowdesktopScreen (CompScreen *screen) :
    PluginClassHandler <ShowdesktopScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    state (SD_STATE_OFF),
    moreAdjust (0)
{
    ScreenInterface::setHandler (screen);
    CompositeScreenInterface::setHandler (cScreen);
    GLScreenInterface::setHandler (gScreen);
}

/* window initialization */
ShowdesktopWindow::ShowdesktopWindow (CompWindow *window) :
    PluginClassHandler <ShowdesktopWindow, CompWindow> (window),
    window (window),
    gWindow (GLWindow::get (window)),
    sid (0),
    distance (0),
    placer (NULL),
    xVelocity (0.0f),
    yVelocity (0.0f),
    tx (0.0f),
    ty (0.0f),
    notAllowedMask (0),
    stateMask (0),
    showdesktoped (false),
    wasManaged (window->managed ()),
    delta (1.0f),
    adjust (false),
    state (0),
    moreAdjust (false)
{
    WindowInterface::setHandler (window);
    GLWindowInterface::setHandler (gWindow);
}

ShowdesktopWindow::~ShowdesktopWindow ()
{
    if (placer)
	delete placer;
}

bool
ShowdesktopPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;
    if (!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI))
	return false;
    if (!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

    return true;
}
