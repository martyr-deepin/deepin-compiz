/*
 *
 * Compiz widget handling plugin
 *
 * widget.c
 *
 * Copyright : (C) 2007 by Danny Baumann
 * E-mail    : maniac@opencompositing.org
 *
 * Ported to Compiz 0.9 by:
 * Copyright : (C) 2008 Sam Spilsbury
 * E-mail    : smspillaz@gmail.com
 *
 * Idea based on widget.c:
 * Copyright : (C) 2006 Quinn Storm
 * E-mail    : livinglatexkali@gmail.com
 *
 * Copyright : (C) 2007 Mike Dransfield
 * E-mail    : mike@blueroot.co.uk
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

#include "widget.h"

class WidgetExp :
    public CompMatch::Expression
{
    public:
	WidgetExp (const CompString &str);

	bool evaluate (CompWindow *w);

	bool value;
};

COMPIZ_PLUGIN_20090315 (widget, WidgetPluginVTable);

static void enableFunctions (WidgetScreen *ws, bool enabled)
{
    ws->cScreen->preparePaintSetEnabled (ws, enabled);
    ws->cScreen->donePaintSetEnabled (ws, enabled);

    foreach (CompWindow *w, screen->windows ())
    {
	WIDGET_WINDOW (w);

	ww->window->focusSetEnabled (ww, enabled);
	ww->gWindow->glPaintSetEnabled (ww, enabled);
    }
}

void
WidgetWindow::updateTreeStatus ()
{
    /* first clear out every reference to our window */
    foreach (CompWindow *win, screen->windows ())
    {
	WIDGET_WINDOW (win);
	if (ww->mParentWidget == win)
	    ww->mParentWidget = NULL;
    }

    if (window->destroyed ())
	return;

    if (!mIsWidget)
	return;

    foreach (CompWindow *win, screen->windows ())
    {
	Window clientLeader = win->clientLeader (true);

	if ((clientLeader == window->clientLeader ()) &&
	    (window->id () != win->id ()))
	{
	    WIDGET_WINDOW (win);
	    ww->mParentWidget = window;
	}
    }
}

bool
WidgetWindow::updateWidgetStatus ()
{
    bool isWidget, retval;

    WIDGET_SCREEN (screen);

    switch (mPropertyState) {
    case PropertyWidget:
	isWidget = true;
	break;
    case PropertyNoWidget:
	isWidget = false;
	break;
    default:
	if (!window->managed () ||
	    (window->wmType () & CompWindowTypeDesktopMask))
	{
	    isWidget = false;
	}
	else
	{
	    isWidget = ws->optionGetMatch ().evaluate (window);
	}
	break;
    }

    retval = (!isWidget && mIsWidget) || (isWidget && !mIsWidget);
    mIsWidget = isWidget;

    return retval;
}

bool
WidgetWindow::updateWidgetPropertyState ()
{
    Atom          retType;
    int           format, result;
    unsigned long nitems, remain;
    unsigned char *data = NULL;

    WIDGET_SCREEN (screen);

    result = XGetWindowProperty (screen->dpy (), window->id (),
				 ws->mCompizWidgetAtom, 0, 1L, false,
				 AnyPropertyType, &retType,
				 &format, &nitems, &remain, &data);

    if (result == Success && data)
    {
	if (nitems && format == 32)
	{
	    unsigned long int *retData = (unsigned long int *) data;
	    if (*retData)
		mPropertyState = PropertyWidget;
	    else
		mPropertyState = PropertyNoWidget;
	}

	XFree (data);
    }
    else
	mPropertyState = PropertyNotSet;

    return updateWidgetStatus ();
}

void
WidgetWindow::updateWidgetMapState (bool map)
{
    if (map && mWasHidden)
    {
	window->show ();
	window->raise ();
	mWasHidden = false;
	window->managedSetEnabled (this, false);
    }
    else if (!map && !mWasHidden)
    {
	/* never set ww->mHidden on previously unmapped windows -
	   it might happen that we map windows when entering the
	   widget mode which aren't supposed to be unmapped */
	if (window->isViewable ())
	{
	    window->hide ();
	    mWasHidden = true;
	    window->managedSetEnabled (this, true);
	}
    }
}

bool
WidgetWindow::managed ()
{
    return false;
}

void
WidgetScreen::setWidgetLayerMapState (bool map)
{
    CompWindow   *highest = NULL;
    unsigned int highestActiveNum = 0;
    CompWindowList copyWindows = screen->windows ();

    /* We have to operate on a copy of the list, since it's possible
     * for the screen->windows () to be re-ordered by
     * WidgetWindow::updateWidgetMapState, (-> CompWindow::raise ->
     * CompScreen::unhookWindow)
     */
    foreach (CompWindow *window, copyWindows)
    {
	WIDGET_WINDOW (window);

	if (!ww->mIsWidget)
	    continue;

	if (window->activeNum () > highestActiveNum)
	{
	    highest = window;
	    highestActiveNum = window->activeNum ();
	}

	ww->updateWidgetMapState (map);
    }

    if (map && highest)
    {
	if (!mLastActiveWindow)
	    mLastActiveWindow = screen->activeWindow ();
	highest->moveInputFocusTo ();
    }
    else if (!map)
    {
	CompWindow *w = screen->findWindow (mLastActiveWindow);
	mLastActiveWindow = None;
	if (w)
	    w->moveInputFocusTo ();
    }
}

bool
WidgetScreen::registerExpHandler ()
{
    screen->matchExpHandlerChanged ();

    return false;
}

WidgetExp::WidgetExp (const CompString &str) :
    value (strtol (str.c_str (), NULL, 0))
{
}

bool
WidgetExp::evaluate (CompWindow *w)
{
    WIDGET_WINDOW (w);

    return ((value && ww->mIsWidget) || (!value && !ww->mIsWidget));
}

CompMatch::Expression *
WidgetScreen::matchInitExp (const CompString &str)
{
    /* Create a new match object */

    if (str.find ("widget=") == 0)
	return new WidgetExp (str.substr (7));

    return screen->matchInitExp (str);
}

void
WidgetScreen::matchExpHandlerChanged ()
{
    screen->matchExpHandlerChanged ();

    /* match options are up to date after the call to matchExpHandlerChanged */

    foreach (CompWindow *w, screen->windows ())
    {
	WIDGET_WINDOW (w);
	if (ww->updateWidgetStatus ())
	{
	    bool map;

	    map = !ww->mIsWidget || (mState != StateOff);
	    ww->updateWidgetMapState (map);

	    ww->updateTreeStatus ();

	    screen->matchPropertyChanged (w);
	}
    }
}

bool
WidgetScreen::toggle (CompAction         *action,
		      CompAction::State  state,
		      CompOption::Vector &options)
{
    switch (mState) {
	case StateOn:
	case StateFadeIn:
	    setWidgetLayerMapState (false);
	    mFadeTime = 1000.0f * optionGetFadeTime ();
	    mState = StateFadeOut;
	    break;
	case StateOff:
	case StateFadeOut:
	    setWidgetLayerMapState (true);
	    mFadeTime = 1000.0f * optionGetFadeTime ();
	    mState = StateFadeIn;
	    break;
	default:
	    break;
    }

    if (!mGrabIndex)
	mGrabIndex = screen->pushGrab (mCursor, "widget");

    enableFunctions (this, true);

    cScreen->damageScreen ();

    return true;
}

void
WidgetScreen::endWidgetMode (CompWindow *closedWidget)
{
    CompOption::Vector options;

    if (mState != StateOn && mState != StateFadeIn)
	return;

    if (closedWidget)
    {
	/* end widget mode if the closed widget was the last one */

	WIDGET_WINDOW (closedWidget);
	if (ww->mIsWidget)
	{
	    foreach (CompWindow *w, screen->windows ())
	    {
		WIDGET_WINDOW (w);
		if (w == closedWidget)
		    continue;

		if (ww->mIsWidget)
		    return;
	    }
	}
	else
	    return;
    }

    options.push_back (CompOption ("root", CompOption::TypeInt));
    options[0].value ().set ((int) screen->root ());

    toggle (NULL, 0, options);
}

void
WidgetScreen::handleEvent (XEvent *event)
{
    CompWindow *w;

    screen->handleEvent (event);

    switch (event->type)
    {
    case PropertyNotify:
	if (event->xproperty.atom == mCompizWidgetAtom)
	{
	    w = screen->findWindow (event->xproperty.window);
	    if (w)
	    {
		WIDGET_WINDOW (w);
		if (ww->updateWidgetPropertyState ())
		{
		    bool map;

		    map = !ww->mIsWidget || mState != StateOff;
		    ww->updateWidgetMapState (map);
		    ww->updateTreeStatus ();
		    screen->matchPropertyChanged (w);
		}
	    }
	}
	else if (event->xproperty.atom == Atoms::wmClientLeader)
	{
	    w = screen->findWindow (event->xproperty.window);
	    if (w)
	    {
		WIDGET_WINDOW (w);

		if (ww->mIsWidget)
		    ww->updateTreeStatus ();
		else if (ww->mParentWidget)
		{
		    WidgetWindow *pww = WidgetWindow::get (ww->mParentWidget);
		    pww->updateTreeStatus ();
		}
	    }
	}
	break;
    case ButtonPress:
	/* terminate widget mode if a non-widget window was clicked */
	if (optionGetEndOnClick () &&
	    event->xbutton.button == Button1)
	{
	    if (mState == StateOn)
	    {
		w = screen->findWindow (event->xbutton.window);
		if (w && w->managed ())
		{
		    WIDGET_WINDOW (w);

		    if (!ww->mIsWidget && !ww->mParentWidget)
			endWidgetMode (NULL);
		}
	    }
	}
	break;
    case MapNotify:
	w = screen->findWindow (event->xmap.window);
	if (w)
	{
	    WIDGET_WINDOW (w);

	    ww->updateWidgetStatus ();
	    if (ww->mIsWidget)
		ww->updateWidgetMapState (mState != StateOff);
	}
	break;
    case UnmapNotify:
	w = screen->findWindow (event->xunmap.window);
	if (w)
	{
	    WIDGET_WINDOW (w);
	    ww->updateTreeStatus ();
	    endWidgetMode (w);
	}
	break;
    case DestroyNotify:
	w = screen->findWindow (event->xdestroywindow.window);
	if (w)
	{
	    WIDGET_WINDOW (w);
	    ww->updateTreeStatus ();
	    endWidgetMode (w);
	}
	break;
    }
}

bool
WidgetWindow::updateMatch ()
{
    if (updateWidgetStatus ())
    {
	WIDGET_SCREEN (screen);

	updateTreeStatus ();
	updateWidgetMapState (ws->mState != WidgetScreen::StateOff);
	screen->matchPropertyChanged (window);
    }

    return false;
}

bool
WidgetScreen::updateStatus (CompWindow *w)
{
    Window clientLeader;

    WIDGET_WINDOW (w);

    if (ww->updateWidgetPropertyState ())
	ww->updateWidgetMapState (mState != StateOff);

    clientLeader = w->clientLeader (true);

    if (ww->mIsWidget)
    {
	ww->updateTreeStatus ();
    }
    else if (clientLeader)
    {
	CompWindow *lw;

	lw = screen->findWindow (clientLeader);
	if (lw)
	{
	    WidgetWindow *lww;
	    lww = WidgetWindow::get (lw);

	    if (lww->mIsWidget)
		ww->mParentWidget = lw;
	    else if (lww->mParentWidget)
		ww->mParentWidget = lww->mParentWidget;
	}
    }

    return false;
}

void
WidgetScreen::matchPropertyChanged (CompWindow  *w)
{
    WIDGET_WINDOW (w);

    /* one shot timeout which will update the widget status (timer
       is needed because we don't want to call wrapped functions
       recursively) */
    if (!ww->mMatchUpdate.active ())
	ww->mMatchUpdate.start (boost::bind (&WidgetWindow::updateMatch, ww),
				0, 0);

    screen->matchPropertyChanged (w);
}

bool
WidgetWindow::glPaint (const GLWindowPaintAttrib &attrib,
		       const GLMatrix            &transform,
		       const CompRegion          &region,
		       unsigned int              mask)
{
    bool       status;

    WIDGET_SCREEN (screen);

    if (ws->mState != WidgetScreen::StateOff)
    {
	GLWindowPaintAttrib wAttrib = attrib;
	float               fadeProgress;

	if (ws->mState == WidgetScreen::StateOn)
	    fadeProgress = 1.0f;
	else
	{
	    fadeProgress = ws->optionGetFadeTime ();
	    if (fadeProgress)
		fadeProgress = (float) ws->mFadeTime / (1000.0f * fadeProgress);
	    fadeProgress = 1.0f - fadeProgress;
	}

	if (!mIsWidget && !mParentWidget)
	{
	    float progress;

	    if (ws->mState == WidgetScreen::StateFadeIn ||
		ws->mState == WidgetScreen::StateOn)
	    {
		fadeProgress = 1.0f - fadeProgress;
	    }

	    progress = ws->optionGetBgSaturation () / 100.0f;
	    progress += (1.0f - progress) * fadeProgress;
	    wAttrib.saturation = (float) wAttrib.saturation * progress;

	    progress = ws->optionGetBgBrightness () / 100.0f;
	    progress += (1.0f - progress) * fadeProgress;

	    wAttrib.brightness = (float) wAttrib.brightness * progress;
	}

	status = gWindow->glPaint (wAttrib, transform, region, mask);
    }
    else
    {
	status = gWindow->glPaint (attrib, transform, region, mask);
    }

    return status;
}

bool
WidgetWindow::focus ()
{
    WIDGET_SCREEN (screen);

    /* Don't focus non-widget windows while widget mode is enabled */
    if (ws->mState != WidgetScreen::StateOff && !mIsWidget && !mParentWidget)
	return false;

    return window->focus ();
}

void
WidgetScreen::preparePaint (int msSinceLastPaint)
{
    if ((mState == StateFadeIn) || (mState == StateFadeOut))
    {
	mFadeTime -= msSinceLastPaint;
	mFadeTime = MAX (mFadeTime, 0);
    }

    cScreen->preparePaint (msSinceLastPaint);
}

void
WidgetScreen::donePaint ()
{
    if (mState == StateFadeIn || mState == StateFadeOut)
    {
	if (mFadeTime)
	    cScreen->damageScreen ();
	else
	{
	    if (mGrabIndex)
	    {
		screen->removeGrab (mGrabIndex, NULL);
		mGrabIndex = 0;
	    }

	    if (mState == StateFadeIn)
		mState = StateOn;
	    else
		mState = StateOff;
	}
    }

    if (mState == StateOff)
    {
	cScreen->damageScreen ();
	enableFunctions (this, false);
    }

    cScreen->donePaint ();
}

void
WidgetScreen::optionChanged (CompOption              *option,
			     WidgetOptions::Options  num)
{
    switch (num)
    {
        case WidgetOptions::Match:
	{
	    foreach (CompWindow *w, screen->windows ())
	    {
		WIDGET_WINDOW (w);

		if (ww->updateWidgetStatus ())
		{
		    bool map;

		    map = !ww->mIsWidget || mState != StateOff;
		    ww->updateWidgetMapState (map);

		    ww->updateTreeStatus ();
		    screen->matchPropertyChanged (w);
		}
	    }
	}
	break;
    default:
	break;
    }
}

/* ------ */

WidgetScreen::WidgetScreen (CompScreen *screen) :
    PluginClassHandler <WidgetScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    mLastActiveWindow (None),
    mCompizWidgetAtom (XInternAtom (screen->dpy (), "_COMPIZ_WIDGET", false)),
    mFadeTime (0),
    mGrabIndex (0),
    mCursor (XCreateFontCursor (screen->dpy (), XC_watch))
{
    CompAction::CallBack cb;
    ChangeNotify         notify;
    CompTimer            registerTimer;

    ScreenInterface::setHandler (screen, false);
    CompositeScreenInterface::setHandler (cScreen, false);

    screen->handleEventSetEnabled (this, true);
    screen->matchInitExpSetEnabled (this, true);
    screen->matchExpHandlerChangedSetEnabled (this, true);

    cb = boost::bind (&WidgetScreen::toggle, this, _1, _2, _3);
    optionSetToggleKeyInitiate (cb);
    optionSetToggleButtonInitiate (cb);
    optionSetToggleEdgeInitiate (cb);

    notify = boost::bind (&WidgetScreen::optionChanged, this, _1, _2);
    optionSetMatchNotify (notify);

    /* one shot timeout to which will register the expression handler
       after all screens and windows have been initialized */
    registerTimer.start (boost::bind(&WidgetScreen::registerExpHandler, this),
			 0, 0);

    mState = StateOff;
}

WidgetScreen::~WidgetScreen ()
{
    screen->matchExpHandlerChangedSetEnabled (this, false);
    screen->matchExpHandlerChanged ();

    if (mCursor)
	XFreeCursor (screen->dpy (), mCursor);
}

WidgetWindow::WidgetWindow (CompWindow *window) :
    PluginClassHandler <WidgetWindow, CompWindow> (window),
    window (window),
    gWindow (GLWindow::get (window)),
    mIsWidget (false),
    mWasHidden (false),
    mParentWidget (NULL),
    mPropertyState (PropertyNotSet)
{
    WindowInterface::setHandler (window);
    GLWindowInterface::setHandler (gWindow, false);

    window->managedSetEnabled (this, false);

    mWidgetStatusUpdate.start (boost::bind (&WidgetScreen::updateStatus,
					    WidgetScreen::get (screen), window),
			       0, 0);
}

WidgetWindow::~WidgetWindow ()
{
    if (mMatchUpdate.active ())
	mMatchUpdate.stop ();

    if (mWidgetStatusUpdate.active ())
	mWidgetStatusUpdate.stop ();
}

bool
WidgetPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;
    if (!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI))
	return false;
    if (!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

    return true;
}
