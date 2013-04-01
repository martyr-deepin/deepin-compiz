/*
 * Compiz extra WM actions plugins
 * extrawm.cpp
 *
 * Copyright: (C) 2007 Danny Baumann <maniac@beryl-project.org>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "extrawm.h"

COMPIZ_PLUGIN_20090315 (extrawm, ExtraWMPluginVTable);

bool compositeAvailable;

void
ExtraWMScreen::addAttentionWindow (CompWindow *w)
{
    std::list <CompWindow *>::iterator it;

    /* check if the window is already there */
    for (it = attentionWindows.begin (); it != attentionWindows.end (); ++it)
    {
	if (*it == w)
	    return;
    }

    attentionWindows.push_back (w);
}

void
ExtraWMScreen::removeAttentionWindow (CompWindow *w)
{
    attentionWindows.remove (w);
}

void
ExtraWMScreen::updateAttentionWindow (CompWindow *w)
{
    XWMHints *hints;
    bool     urgent = false;

    if (w->overrideRedirect ())
	return;

    if (w->wmType () & (CompWindowTypeDockMask | CompWindowTypeDesktopMask))
	return;

    hints = XGetWMHints (screen->dpy (), w->id ());
    if (hints)
    {
	if (hints->flags & XUrgencyHint)
	    urgent = true;

	XFree (hints);
    }

    if (urgent || (w->state () & CompWindowStateDemandsAttentionMask))
	addAttentionWindow (w);
    else
	removeAttentionWindow (w);
}

bool
ExtraWMScreen::activateDemandsAttention (CompAction         *action,
					 CompAction::State  state,
					 CompOption::Vector &options)
{
    EXTRAWM_SCREEN (screen);

    if (!es->attentionWindows.empty ())
    {
	CompWindowList::iterator it = es->attentionWindows.begin ();

	/* We want to keep these windows in the list and skip over them
	 * if they are currently unmapped (since they could be mapped
	 * again, and that would mean that we'd want to handle them
	 * if they became wanting attention again) */

	for (; it != es->attentionWindows.end (); ++it)
	{
	    CompWindow *w = *it;

	    if (!w->mapNum () || !w->isViewable ())
	    {
		if (!w->minimized () &&
		    !w->inShowDesktopMode () &&
		    !w->shaded ())
		{
		    continue;
		}
	    }

	    w->activate ();
	    break;
	}
    }

    return false;
}

bool
ExtraWMScreen::activateWin (CompAction         *action,
			    CompAction::State  state,
			    CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");
    w   = screen->findWindow (xid);

    if (w)
	screen->sendWindowActivationRequest (w->id ());

    return true;
}

void
ExtraWMScreen::fullscreenWindow (CompWindow   *w,
		  		 unsigned int state)
{
    unsigned int newState = w->state ();

    if (w->overrideRedirect ())
	return;

    /* It would be a bug, to put a shaded window to fullscreen. */
    if (w->shaded ())
	return;

    state = w->constrainWindowState (state, w->actions ());
    state &= CompWindowStateFullscreenMask;

    if (state == (w->state () & CompWindowStateFullscreenMask))
	return;

    newState &= ~CompWindowStateFullscreenMask;
    newState |= state;

    w->changeState (newState);
    w->updateAttributes (CompStackingUpdateModeNormal);
}

bool
ExtraWMScreen::toggleFullscreen (CompAction         *action,
			         CompAction::State  state,
			         CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");
    w   = screen->findWindow (xid);

    if (w && (w->actions () & CompWindowActionFullscreenMask))
    {
	EXTRAWM_SCREEN (screen);

	es->fullscreenWindow (w, w->state () ^ CompWindowStateFullscreenMask);
    }

    return true;
}

bool
ExtraWMScreen::toggleRedirect (CompAction         *action,
			       CompAction::State  state,
			       CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");
    w   = screen->findTopLevelWindow (xid);

    if (!compositeAvailable)
    {
	compLogMessage ("extrawm", CompLogLevelWarn, "composite plugin "\
			"not loaded, cannot redirect/unredirect window");
	return true;
    }

    if (w)
    {
	CompositeWindow *cWindow = CompositeWindow::get (w);

	if (cWindow)
	{
	    if (cWindow->redirected ())
		cWindow->unredirect ();
	    else
		cWindow->redirect ();
	}
    }

    return true;
}

bool
ExtraWMScreen::toggleAlwaysOnTop (CompAction         *action,
			          CompAction::State  state,
			          CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");
    w   = screen->findTopLevelWindow (xid);

    if (w)
    {
	unsigned int newState;

	newState = w->state () ^ CompWindowStateAboveMask;
	w->changeState (newState);
	w->updateAttributes (CompStackingUpdateModeNormal);
    }

    return true;
}

bool
ExtraWMScreen::toggleSticky (CompAction         *action,
			     CompAction::State  state,
			     CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");
    w   = screen->findTopLevelWindow (xid);

    if (w && (w->actions () & CompWindowActionStickMask))
    {
	unsigned int newState;
	newState = w->state () ^ CompWindowStateStickyMask;
	w->changeState (newState);
    }

    return true;
}

void
ExtraWMScreen::handleEvent (XEvent *event)
{
    screen->handleEvent (event);

    switch (event->type) {
    case PropertyNotify:
	if (event->xproperty.atom == XA_WM_HINTS)
	{
	    CompWindow *w;

	    w = screen->findWindow (event->xproperty.window);
	    if (w)
		updateAttentionWindow (w);
	}
	break;
    default:
	break;
    }
}

void
ExtraWMWindow::stateChangeNotify (unsigned int lastState)
{
    EXTRAWM_SCREEN (screen);

    window->stateChangeNotify (lastState);

    if ((window->state () ^ lastState) & CompWindowStateDemandsAttentionMask)
	es->updateAttentionWindow (window);
}

ExtraWMScreen::ExtraWMScreen (CompScreen *screen) :
    PluginClassHandler <ExtraWMScreen, CompScreen> (screen),
    ExtrawmOptions ()
{
    ScreenInterface::setHandler (screen);

    optionSetToggleRedirectKeyInitiate (toggleRedirect);
    optionSetToggleAlwaysOnTopKeyInitiate (toggleAlwaysOnTop);
    optionSetToggleStickyKeyInitiate (toggleSticky);
    optionSetToggleFullscreenKeyInitiate (toggleFullscreen);
    optionSetActivateInitiate (activateWin);
    optionSetActivateDemandsAttentionKeyInitiate (activateDemandsAttention);
}

ExtraWMWindow::ExtraWMWindow (CompWindow *window) :
    PluginClassHandler <ExtraWMWindow, CompWindow> (window),
    window (window)
{
    WindowInterface::setHandler (window);
}

ExtraWMWindow::~ExtraWMWindow ()
{
    ExtraWMScreen::get (screen)->removeAttentionWindow (window);
}

bool
ExtraWMPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    if (!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI))
	compositeAvailable = false;
    else
	compositeAvailable = true;

    return true;
}
