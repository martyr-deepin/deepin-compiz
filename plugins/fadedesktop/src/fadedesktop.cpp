/**
 *
 * Compiz fade to desktop plugin
 *
 * fadedesktop.cpp
 *
 * Copyright (c) 2006 Robert Carr <racarr@beryl-project.org>
 * 		 2007 Danny Baumann <maniac@beryl-project.org>
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
 **/

#include "fadedesktop.h"

COMPIZ_PLUGIN_20090315 (fadedesktop, FadedesktopPluginVTable);

void
FadedesktopScreen::activateEvent (bool activating)
{
    CompOption::Vector o;

    o.push_back (CompOption ("root", CompOption::TypeInt));
    o.push_back (CompOption ("active", CompOption::TypeBool));

    o[0].value (). set ((int) screen->root ());
    o[1].value (). set (activating);

    screen->handleCompizEvent ("fadedesktop", "activate", o);
}

bool
FadedesktopWindow::isFadedesktopWindow ()
{
    FD_SCREEN (screen);

    if (!window->managed ())
	return false;

    if (window->grabbed ())
	return false;

    if (window->wmType () & (CompWindowTypeDesktopMask |
			     CompWindowTypeDockMask))
	return false;

    if (window->state () & CompWindowStateSkipPagerMask)
	return false;

    if (!fs->optionGetWindowMatch ().evaluate (window))
	return false;

    return true;
}

void
FadedesktopScreen::preparePaint (int msSinceLastPaint)
{
    fadeTime -= msSinceLastPaint;
    if (fadeTime < 0)
	fadeTime = 0;

    if (state == Out || state == In)
    {
	foreach (CompWindow *w, screen->windows ())
	{
	    bool doFade;

	    FD_WINDOW (w);

	    if (state == Out)
		doFade = fw->fading && w->inShowDesktopMode ();
	    else
		doFade = fw->fading && !w->inShowDesktopMode ();

	    if (doFade)
	    {
		float windowFadeTime;

		if (state == Out)
		    windowFadeTime = fadeTime;
		else
		    windowFadeTime = optionGetFadetime () - fadeTime;

		fw->opacity = fw->cWindow->opacity () *
		              (windowFadeTime / optionGetFadetime ());
	    }
	}
    }

    cScreen->preparePaint (msSinceLastPaint);
}

void
FadedesktopScreen::donePaint ()
{
    if (state == Out || state == In)
    {
	if (fadeTime <= 0)
	{
	    bool isStillSD = false;

	    foreach (CompWindow *w, screen->windows ())
	    {
		FD_WINDOW (w);

		if (fw->fading)
		{
		    if (state == Out)
		    {
			w->hide ();
			fw->isHidden = true;
		    }
		    fw->fading = false;
		}
		if (w->inShowDesktopMode ())
		    isStillSD = true;
	    }

	    if (state == Out || isStillSD)
		state = On;
	    else
		state = Off;

	    activateEvent (false);
	}
	else
	{
	    cScreen->damageScreen ();
	}
    }

    cScreen->donePaint ();
}

bool
FadedesktopWindow::glPaint (const GLWindowPaintAttrib &attrib,
			    const GLMatrix            &transform,
			    const CompRegion          &region,
			    unsigned int              mask)
{
    if (fading || isHidden)
    {
	GLWindowPaintAttrib wAttrib = attrib;
	wAttrib.opacity = opacity;

	return gWindow->glPaint (wAttrib, transform, region, mask);
    }

    return gWindow->glPaint (attrib, transform, region, mask);
}

void
FadedesktopScreen::enterShowDesktopMode ()
{
    if (state == Off || state == In)
    {
	if (state == Off)
	    activateEvent (true);

	state = Out;
	fadeTime = optionGetFadetime() - fadeTime;

	foreach (CompWindow *w, screen->windows ())
	{
	    FD_WINDOW (w);
	    if (fw->isFadedesktopWindow ())
	    {

		fw->fading = true;
		w->setShowDesktopMode (true);
		fw->opacity = fw->cWindow->opacity ();
	    }
	}

	cScreen->damageScreen ();
    }

    screen->enterShowDesktopMode ();
}

void
FadedesktopScreen::leaveShowDesktopMode (CompWindow *w)
{
    if (state != Off)
    {
	if (state != In)
	{
	    if (state == On)
		activateEvent (true);

	    state = In;
	    fadeTime = optionGetFadetime() - fadeTime;
	}

	foreach (CompWindow *cw, screen->windows ())
	{
	    if (w && (w->id () != cw->id ()))
		continue;

	    FD_WINDOW (cw);

	    if (fw->isHidden)
	    {
		cw->setShowDesktopMode (false);
		cw->show ();
		fw->isHidden = false;
		fw->fading = true;
	    }
	    else if (fw->fading)
	    {
		cw->setShowDesktopMode (false);
	    }
	}

	cScreen->damageScreen ();
    }

    screen->leaveShowDesktopMode (w);
}

FadedesktopScreen::FadedesktopScreen (CompScreen *screen) :
    PluginClassHandler <FadedesktopScreen, CompScreen> (screen),
    FadedesktopOptions (),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    state (Off),
    fadeTime (0)
{
    ScreenInterface::setHandler (screen);
    CompositeScreenInterface::setHandler (cScreen);
    GLScreenInterface::setHandler (gScreen);
}

FadedesktopWindow::FadedesktopWindow (CompWindow *window) :
    PluginClassHandler <FadedesktopWindow, CompWindow> (window),
    window (window),
    cWindow (CompositeWindow::get (window)),
    gWindow (GLWindow::get (window)),
    fading (false),
    isHidden (false),
    opacity (OPAQUE)
{
    WindowInterface::setHandler (window);
    GLWindowInterface::setHandler (gWindow);
}

bool
FadedesktopPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
	!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

    return true;
}
