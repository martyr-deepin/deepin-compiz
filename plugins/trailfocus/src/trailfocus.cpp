/**
 * Beryl Trailfocus - take three
 *
 * Copyright (c) 2006 Kristian Lyngstøl <kristian@beryl-project.org>
 * Ported to Compiz and BCOP usage by Danny Baumann <maniac@beryl-project.org>
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
 * This version is completly rewritten from scratch with opacify as a 
 * basic template. The original trailfocus was written by: 
 * François Ingelrest <Athropos@gmail.com> and rewritten by:
 * Dennis Kasprzyk <onestone@beryl-project.org>
 * 
 *
 * Trailfocus modifies the opacity, brightness and saturation on a window 
 * based on when it last had focus. 
 *
 */

#include "trailfocus.h"
#include <core/atoms.h>

COMPIZ_PLUGIN_20090315 (trailfocus, TrailfocusPluginVTable);

/* Determines if a window should be handled by trailfocus or not */
bool
TrailfocusScreen::isTrailfocusWindow (CompWindow *w)
{
    CompRect input (w->inputRect ());

    if (input.left () >= (int) screen->width ()  || input.right () <= 0 ||
	input.top ()  >= (int) screen->height () || input.bottom () <= 0)
    {
	return false;
    }

    if (w->overrideRedirect ())
	return false;

    if (w->destroyed () || !w->mapNum () || w->minimized () || w->shaded ())
	return false;

    if (!optionGetWindowMatch ().evaluate (w))
	return false;

    return true;
}

/* Walks through the window-list and sets the opacity-levels for
 * all windows. The inner loop will result in ts->win[i] either
 * representing a recently focused window, or the least
 * focused window.
 */
void
TrailfocusScreen::setWindows (TrailfocusWindow *removedWindow)
{
    bool wasTfWindow;

    foreach (CompWindow *w, screen->windows ())
    {
	TrailfocusWindow *tw = TrailfocusWindow::get (w);
	bool             needDamage;
	
	if (removedWindow == tw)
	    continue;

	wasTfWindow    = tw->isTfWindow;
	tw->isTfWindow = isTrailfocusWindow (w);

	needDamage = wasTfWindow != tw->isTfWindow;

	if (tw->isTfWindow)
	{
	    unsigned int i;

	    for (i = 0; i < windows.size (); i++)
		if (windows[i] == tw)
		    break;

	    if (memcmp (&tw->attribs, &attribs[i], sizeof (TfAttribs)) != 0)
		needDamage = true;

	    if (!wasTfWindow && tw->gWindow)
		tw->gWindow->glPaintSetEnabled (tw, true);

	    tw->attribs = attribs[i];
	}
	else if (wasTfWindow && tw->gWindow)
	{
	    tw->gWindow->glPaintSetEnabled (tw, false);
	}

	if (needDamage && tw->cWindow)
	    tw->cWindow->addDamage ();
    }
}

/* Push a new window-id on the trailfocus window-stack (not to be
 * confused with the real window stack).  Only keep one copy of a
 * window on the stack. If the window allready exist on the stack,
 * move it to the top.
 */
bool
TrailfocusScreen::pushWindow (Window id)
{
    CompWindow             *w;
    TfWindowList::iterator iter;

    w = screen->findWindow (id);
    if (!w)
	return false;

    if (!isTrailfocusWindow (w))
	return false;

    for (iter = windows.begin (); iter != windows.end (); ++iter)
	if ((*iter)->window->id () == id)
	    break;

    if (iter == windows.begin ())
	return false;

    if (iter != windows.end ())
	windows.erase (iter);

    windows.insert (windows.begin (), TrailfocusWindow::get (w));

    if ((int) windows.size () > optionGetWindowsCount ())
	windows.pop_back ();

    return true;
}

/* Pop a window-id from the trailfocus window-stack (not to be
 * confused with the real window stack).  Only keep one copy of a
 * window on the stack. Also fill the empty space with the next
 * window on the real window stack.
 */
void
TrailfocusScreen::popWindow (TrailfocusWindow *tw)
{
    CompWindow             *best = NULL;
    TfWindowList::iterator iter;
    int                    distance, bestDist = 1000000;
    unsigned int           i;

    for (iter = windows.begin (); iter != windows.end (); ++iter)
	if (*iter == tw)
	    break;

    if (iter == windows.end ())
	return;

    windows.erase (iter);

    /* find window that was activated right before the destroyed one
       to fill the empty space */
    foreach (CompWindow *cur, screen->windows ())
    {
	bool present = false;

	if (!isTrailfocusWindow (cur))
	    continue;

	if (cur == tw->window)
	    continue;

	/* we only want windows that were activated before the popped one */
	if (cur->activeNum () > tw->window->activeNum ())
	    continue;

	/* we do not want any windows already present in the list */
	for (i = 0; i < windows.size (); i++)
	{
	    if (windows[i]->window == cur)
	    {
		present = true;
		break;
	    }
	}

	if (present)
	    continue;

	if (!best)
	{
	    best = cur;
	}
	else
	{
	    distance = abs (cur->activeNum () - best->activeNum ());
	    if (distance < bestDist)
	    {
		best     = cur;
		bestDist = distance;
	    }
	}
    }

    if (best)
	windows.push_back (TrailfocusWindow::get (best));

    setWindows (tw);
}

static bool
compareActiveness (CompWindow *w1,
		   CompWindow *w2)
{
    return w1->activeNum () >= w2->activeNum ();
}

/* Walks through the existing stack and removes windows that should
 * (no longer) be there. Used for option-change.
 */
void
TrailfocusScreen::refillList ()
{
    CompWindowList         activeList = screen->windows ();
    TfWindowList::iterator iter;
    unsigned int           winMax = optionGetWindowsCount ();

    activeList.sort (compareActiveness);
    windows.clear ();

    foreach (CompWindow *w, activeList)
    {
	if (!isTrailfocusWindow (w))
	    continue;

	windows.push_back (TrailfocusWindow::get (w));

	if (windows.size () == winMax)
	    break;
    }
}

/* Handles the event if it was a FocusIn event.  */
void
TrailfocusScreen::handleEvent (XEvent *event)
{
    switch (event->type) {
    case FocusIn:
	if (pushWindow (event->xfocus.window))
	    setWindows (NULL);
	break;
    case PropertyNotify:
	if (event->xproperty.atom == Atoms::desktopViewport)
	{
	    refillList ();
	    setWindows (NULL);
	}
	break;
    default:
	break;
    }

    screen->handleEvent (event);
}

/* Settings changed. Reallocate rs->inc and re-populate it and the
 * rest of the TrailfocusScreen (-wMask).
 */
void
TrailfocusScreen::recalculateAttributes ()
{
    TfAttribs tmp, min, max;
    int       i, start, winMax;

    start  = optionGetWindowsStart () - 1;
    winMax = optionGetWindowsCount ();

    if (start >= winMax)
    {
	compLogMessage ("trailfocus", CompLogLevelWarn,
			"Attempting to define start higher than max windows.");
	start = winMax - 1;
    }

    min.opacity    = optionGetMinOpacity () * OPAQUE / 100;
    min.brightness = optionGetMinBrightness () * BRIGHT / 100;
    min.saturation = optionGetMinSaturation () * COLOR / 100;

    max.opacity    = optionGetMaxOpacity () * OPAQUE / 100;
    max.brightness = optionGetMaxBrightness () * BRIGHT / 100;
    max.saturation = optionGetMaxSaturation () * COLOR / 100;

    attribs.resize (winMax + 1);

    tmp.opacity    = (max.opacity - min.opacity) / (winMax - start);
    tmp.brightness = (max.brightness - min.brightness) / (winMax - start);
    tmp.saturation = (max.saturation - min.saturation) / (winMax - start);

    for (i = 0; i < start; i++)
	attribs[i] = max;

    for (i = 0; i + start <= winMax; i++)
    {
	attribs[i + start].opacity    = max.opacity - (tmp.opacity * i);
	attribs[i + start].brightness = max.brightness - (tmp.brightness * i);
	attribs[i + start].saturation = max.saturation - (tmp.saturation * i);
    }
}

bool
TrailfocusWindow::glPaint (const GLWindowPaintAttrib& attrib,
			   const GLMatrix&            transform,
			   const CompRegion&          region,
			   unsigned int               mask)
{
    if (isTfWindow)
    {
	GLWindowPaintAttrib wAttrib (attrib);

	wAttrib.opacity    = MIN (attrib.opacity, attribs.opacity);
	wAttrib.brightness = MIN (attrib.brightness, attribs.brightness);
	wAttrib.saturation = MIN (attrib.saturation, attribs.saturation);

	return gWindow->glPaint (wAttrib, transform, region, mask);
    }

    return gWindow->glPaint (attrib, transform, region, mask);
}

void
TrailfocusScreen::optionChanged (CompOption *opt,
				 Options    num)
{
    switch (num) {
    case MinOpacity:
    case MaxOpacity:
    case MinSaturation:
    case MaxSaturation:
    case MinBrightness:
    case MaxBrightness:
    case WindowsStart:
    case WindowsCount:
	recalculateAttributes ();
	break;
    default:
	break;
    }

    refillList ();
    setWindows (NULL);
}

bool
TrailfocusScreen::setupTimerCb ()
{
    TrailfocusScreen *ts = TrailfocusScreen::get (screen);

    ts->refillList ();
    ts->setWindows (NULL);

    return false;
}

TrailfocusScreen::TrailfocusScreen (CompScreen *s) :
    PluginClassHandler<TrailfocusScreen, CompScreen> (s),
    TrailfocusOptions ()
{
    ChangeNotify optionCb = boost::bind (&TrailfocusScreen::optionChanged,
					 this, _1, _2);

    optionSetWindowMatchNotify (optionCb);
    optionSetWindowsCountNotify (optionCb);
    optionSetWindowsStartNotify (optionCb);
    optionSetMinOpacityNotify (optionCb);
    optionSetMaxOpacityNotify (optionCb);
    optionSetMinSaturationNotify (optionCb);
    optionSetMaxSaturationNotify (optionCb);
    optionSetMinBrightnessNotify (optionCb);
    optionSetMaxBrightnessNotify (optionCb);

    ScreenInterface::setHandler (screen);

    recalculateAttributes ();

    setupTimer.start (setupTimerCb, 0, 0);
}

TrailfocusWindow::TrailfocusWindow (CompWindow *w) :
    PluginClassHandler<TrailfocusWindow, CompWindow> (w),
    isTfWindow (false),
    window (w),
    cWindow (CompositeWindow::get (w)),
    gWindow (GLWindow::get (w))
{
    attribs.opacity = OPAQUE;
    attribs.brightness = BRIGHT;
    attribs.saturation = COLOR;

    GLWindowInterface::setHandler (gWindow, false);
}

TrailfocusWindow::~TrailfocusWindow ()
{
    /* Since we are popping the window from the stack
     * gWindow and cWindow are invalidated. Set them to
     * NULL so that their functions cannot be called
     */
    gWindow = NULL;
    cWindow = NULL;
    TrailfocusScreen::get (screen)->popWindow (this);
}

bool
TrailfocusPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
	!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
    {
	return false;
    }

    return true;
}
