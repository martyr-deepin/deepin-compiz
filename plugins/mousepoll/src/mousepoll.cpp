/*
 *
 * Compiz mouse position polling plugin
 *
 * mousepoll.c
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

#include "private.h"

COMPIZ_PLUGIN_20090315 (mousepoll, MousepollPluginVTable);

const unsigned short MP_OPTION_MOUSE_POLL_INTERVAL = 0;
const unsigned short MP_OPTION_NUM = 1;

bool
MousepollScreen::getMousePosition ()
{
    Window       root, child;
    int          rootX, rootY;
    int          winX, winY;
    int          w = screen->width (), h = screen->height ();
    unsigned int maskReturn;
    bool         status;

    status = XQueryPointer (screen->dpy (), screen->root (),
			    &root, &child, &rootX, &rootY,
			    &winX, &winY, &maskReturn);

    if (!status || rootX > w || rootY > h || screen->root () != root)
	return false;

    if (rootX != pos.x () || rootY != pos.y ())
    {
	pos.set (rootX, rootY);
	return true;
    }

    return false;
}

bool
MousepollScreen::updatePosition ()
{

    if (getMousePosition ())
    {
        std::list<MousePoller *>::iterator it;
        for (it = pollers.begin (); it != pollers.end (); )
        {
            MousePoller *poller = *it;

            ++it;
            poller->mPoint = pos;
            poller->mCallback (pos);
        }
    }


    return true;
}

bool
MousepollScreen::addTimer (MousePoller *poller)
{
    bool                               start = pollers.empty ();
    std::list<MousePoller *>::iterator it;

    it = std::find (pollers.begin (), pollers.end (), poller);
    if (it != pollers.end ())
	return false;

    pollers.insert (it, poller);

    if (start)
    {
	getMousePosition ();
	timer.start ();
    }

    return true;
}

void
MousepollScreen::removeTimer (MousePoller *poller)
{
    std::list<MousePoller *>::iterator it;

    it = std::find (pollers.begin(), pollers.end (), poller);
    if (it == pollers.end ())
	return;

    pollers.erase (it);

    if (pollers.empty ())
	timer.stop ();
}

void
MousePoller::setCallback (MousePoller::CallBack callback)
{
    bool wasActive = mActive;

    if (mActive)
	stop ();

    mCallback = callback;

    if (wasActive)
	start ();
}

void
MousePoller::start ()
{
    MOUSEPOLL_SCREEN (screen);

    if (!ms)
    {
	compLogMessage ("mousepoll", CompLogLevelWarn,
			"Plugin version mismatch, can't start mouse poller.");

	return;
    }

    if (mCallback.empty ())
    {
	compLogMessage ("mousepoll", CompLogLevelWarn,
			"Can't start mouse poller without callback.");
	return;
    }

    ms->addTimer (this);

    mActive = true;
}

void
MousePoller::stop ()
{
    MOUSEPOLL_SCREEN (screen);

    /* Prevent broken plugins from calling stop () twice */

    if (!mActive)
	return;

    if (!ms)
    {
	compLogMessage ("mousepoll",
			CompLogLevelWarn,
			"Plugin version mismatch, can't stop mouse poller.");

	return;
    }

    mActive = false;

    ms->removeTimer (this);
}

bool
MousePoller::active ()
{
    return mActive;
}

CompPoint
MousePoller::getCurrentPosition ()
{
    CompPoint p;

    MOUSEPOLL_SCREEN (screen);

    if (!ms)
    {
	compLogMessage ("mousepoll", CompLogLevelWarn,
			"Plugin version mismatch, can't get mouse position.");
    }
    else
    {
	ms->getMousePosition ();
	p = ms->pos;
    }

    return p;
}

CompPoint
MousePoller::getPosition ()
{
    return mPoint;
}

MousePoller::MousePoller () :
    mActive (false),
    mPoint (0, 0),
    mCallback (NULL)
{
}

MousePoller::~MousePoller ()
{
    if (mActive)
	stop ();
}

void
MousepollScreen::updateTimer ()
{
    float timeout = optionGetMousePollInterval ();
    timer.setTimes (timeout, timeout * 1.5);

}

template class PluginClassHandler <MousepollScreen, CompScreen, COMPIZ_MOUSEPOLL_ABI>;

MousepollScreen::MousepollScreen (CompScreen *screen) :
    PluginClassHandler <MousepollScreen, CompScreen, COMPIZ_MOUSEPOLL_ABI> (screen)
{
    updateTimer ();
    timer.setCallback (boost::bind (&MousepollScreen::updatePosition, this));

    optionSetMousePollIntervalNotify (boost::bind (&MousepollScreen::updateTimer, this));
}

bool
MousepollPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    CompPrivate p;
    p.uval = COMPIZ_MOUSEPOLL_ABI;
    screen->storeValue ("mousepoll_ABI", p);

    return true;
}

void
MousepollPluginVTable::fini ()
{
    screen->eraseValue ("mousepoll_ABI");
}
