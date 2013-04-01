/*
 * Compiz login/logout effect plugin
 *
 * loginout.cpp
 *
 * Copyright : (C) 2008 by Dennis Kasprzyk
 * E-mail    : onestone@opencompositing.org
 *
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
 */

#include "loginout.h"

COMPIZ_PLUGIN_20090315 (loginout, LoginoutPluginVTable)

void
LoginoutScreen::updateWindowMatch (CompWindow *w)
{
    bool curr;

    LOGINOUT_WINDOW (w);

    curr = optionGetInMatch ().evaluate (w);
    if (curr != lw->login)
    {
	lw->login = curr;
	if (curr)
	{
	    lw->gWindow->glPaintSetEnabled (lw, true);
	    lw->gWindow->glDrawSetEnabled (lw, true);
	    numLoginWin++;
	}
	else
	{
	    lw->gWindow->glPaintSetEnabled (lw, false);
	    lw->gWindow->glDrawSetEnabled (lw, false);
	    numLoginWin--;
	}
	cScreen->damageScreen ();
    }
    curr = optionGetOutMatch ().evaluate (w);
    if (curr != lw->logout)
    {
	lw->logout = curr;
	if (curr)
	{
	    lw->gWindow->glPaintSetEnabled (lw, true);
	    lw->gWindow->glDrawSetEnabled (lw, true);
	    numLogoutWin++;
	}
	else
	{
	    lw->gWindow->glPaintSetEnabled (lw, false);
	    lw->gWindow->glDrawSetEnabled (lw, false);
	    numLogoutWin--;
	}
	cScreen->damageScreen ();
    }

    if (numLoginWin || numLogoutWin)
    {
	cScreen->preparePaintSetEnabled (this, true);
	cScreen->donePaintSetEnabled (this, true);
    }
    else
    {
	cScreen->preparePaintSetEnabled (this, false);
	cScreen->donePaintSetEnabled (this, false);
    }
}

void
LoginoutScreen::optionChanged (CompOption               *opt,
			       LoginoutOptions::Options num)
{
    switch (num)
    {
    case LoginoutOptions::InMatch:
    case LoginoutOptions::OutMatch:
	foreach (CompWindow *w, screen->windows ())
	    updateWindowMatch (w);

	cScreen->damageScreen ();
	break;

    default:
	cScreen->damageScreen ();
	break;
    }
}

void
LoginoutScreen::matchExpHandlerChanged ()
{
    screen->matchExpHandlerChanged ();

    /* match options are up to date after the call to matchExpHandlerChanged */
    foreach (CompWindow *w, screen->windows ())
    {
	updateWindowMatch (w);
    }
}

void
LoginoutScreen::matchPropertyChanged (CompWindow  *w)
{
    updateWindowMatch (w);

    screen->matchPropertyChanged (w);
}

bool
LoginoutWindow::glPaint (const GLWindowPaintAttrib &attrib,
		         const GLMatrix &transform,
		         const CompRegion &region,
		         unsigned int mask)
{
    bool status;

    LOGINOUT_SCREEN (screen);

    if ((ls->in > 0.0 || ls->out > 0.0) && !login && !logout &&
	!(window->wmType () & CompWindowTypeDesktopMask) && ls->opacity < 1.0)
	mask |= PAINT_WINDOW_TRANSLUCENT_MASK;

    status = gWindow->glPaint (attrib, transform, region, mask);

    return status;
}

bool
LoginoutWindow::glDraw (const GLMatrix &transform,
		        GLFragment::Attrib &fragment,
		        const CompRegion &region,
		        unsigned int mask)
{
    bool       status;

    LOGINOUT_SCREEN (screen);

    if ((ls->in > 0.0 || ls->out > 0.0) && !login && !logout)
    {
	GLFragment::Attrib fA = fragment;

	if (!(window->wmType () & CompWindowTypeDesktopMask))
	    fA.setOpacity (fragment.getOpacity () * ls->opacity);
	
	fA.setBrightness (fragment.getBrightness () * ls->brightness);
	fA.setSaturation (fragment.getSaturation () * ls->saturation);
	
	status = gWindow->glDraw (transform, fA, region, mask);
    }
    else
    {
	status = gWindow->glDraw (transform, fragment, region, mask);
    }

    return status;
}

void
LoginoutScreen::preparePaint (int        ms)
{
    float val;

    val = ((float)ms / 1000.0) / optionGetInTime ();

    if (numLoginWin)
	in = MIN (1.0, in + val);
    else 
	in = MAX (0.0, in - val);

    val = ((float)ms / 1000.0) / optionGetOutTime ();

    if (numLogoutWin)
	out = MIN (1.0, out + val);
    else 
	out = MAX (0.0, out - val);

    if (in > 0.0 || out > 0.0)
    {
	val  = (in * optionGetInOpacity () / 100.0) + (1.0 - in);
	float val2 = (out * optionGetOutOpacity () / 100.0) + (1.0 - out);
	opacity = MIN (val, val2);

	val  = (in * optionGetInSaturation () / 100.0) + (1.0 - in);
	val2 = (out * optionGetOutSaturation () / 100.0) +
	       (1.0 - out);
	saturation = MIN (val, val2);

	val  = (in * optionGetInBrightness () / 100.0) +
	       (1.0 - in);
	val2 = (out * optionGetOutBrightness () / 100.0) +
	       (1.0 - out);
	brightness = MIN (val, val2);
    }

    cScreen->preparePaint (ms);
}

void
LoginoutScreen::donePaint ()
{
    if ((in > 0.0 && in < 1.0) || (out > 0.0 && out < 1.0))
	cScreen->damageScreen ();

    cScreen->donePaint ();
}

LoginoutScreen::LoginoutScreen (CompScreen *screen) :
    PluginClassHandler <LoginoutScreen, CompScreen> (screen),
    LoginoutOptions (),
    cScreen (CompositeScreen::get (screen)),
    kdeLogoutInfoAtom (XInternAtom (screen->dpy (), "_KWIN_LOGOUT_EFFECT", 0)),
    numLoginWin (0),
    numLogoutWin (0),
    brightness (1.0),
    saturation (1.0),
    opacity (1.0),
    in      (0.0),
    out     (0.0)
{
    char buf[128];
    int  scr = DefaultScreen (screen->dpy ());

    ScreenInterface::setHandler (screen);
    CompositeScreenInterface::setHandler (cScreen);

    optionSetInMatchNotify (boost::bind (&LoginoutScreen::optionChanged,
					 this, _1, _2));
    optionSetOutMatchNotify (boost::bind (&LoginoutScreen::optionChanged,
					  this, _1, _2));

    /* wmSnSelectionWindow is not available, so we have to retrieve it
     * ourselves.
     */

    sprintf (buf, "WM_S%d", scr);

    wmSnSelectionWindow = XInternAtom (screen->dpy (), buf, 0);

    /* This is a temporary solution until an official spec will be released */
    XChangeProperty (screen->dpy (), wmSnSelectionWindow,
		     kdeLogoutInfoAtom, kdeLogoutInfoAtom, 8,
		     PropModeReplace,
		     (unsigned char*)&kdeLogoutInfoAtom, 1);

    /* Disable paint functions until we need them */

    cScreen->preparePaintSetEnabled (this, false);
    cScreen->donePaintSetEnabled (this, false);

}

LoginoutScreen::~LoginoutScreen ()
{
    char buf[128];
    int  scr = DefaultScreen (screen->dpy ());

    sprintf (buf, "WM_S%d", scr);

    XDeleteProperty (screen->dpy (), wmSnSelectionWindow,
		     kdeLogoutInfoAtom);

}

LoginoutWindow::LoginoutWindow (CompWindow *window) :
    PluginClassHandler <LoginoutWindow, CompWindow> (window),
    window (window),
    gWindow (GLWindow::get (window)),
    login (false),
    logout (false)
{
    LOGINOUT_SCREEN (screen);

    WindowInterface::setHandler (window);
    GLWindowInterface::setHandler (gWindow);

    gWindow->glPaintSetEnabled (this, false);
    gWindow->glDrawSetEnabled (this, false);

    ls->updateWindowMatch (window);
}

LoginoutWindow::~LoginoutWindow ()
{
    LOGINOUT_SCREEN (screen);

    if (login)
    {
	ls->numLoginWin--;
	ls->cScreen->damageScreen ();
    }
    if (logout)
    {
	ls->numLogoutWin--;
	ls->cScreen->damageScreen ();
    }
}

bool
LoginoutPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
	!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

    return true;
}
