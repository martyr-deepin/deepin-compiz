/*
 * winrules plugin for compiz
 *
 * Copyright (C) 2007 Bellegarde Cedric (gnumdk (at) gmail.com)
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
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include "winrules.h"

COMPIZ_PLUGIN_20090315 (winrules, WinrulesPluginVTable);

void
WinrulesScreen::setProtocols (unsigned int protocols,
		      	      Window       id)
{
    Atom protocol[4];
    int  count = 0;

    if (protocols & CompWindowProtocolDeleteMask)
	protocol[count++] = Atoms::wmDeleteWindow;
    if (protocols & CompWindowProtocolTakeFocusMask)
	protocol[count++] = Atoms::wmTakeFocus;
    if (protocols & CompWindowProtocolPingMask)
	protocol[count++] = Atoms::wmPing;
    if (protocols & CompWindowProtocolSyncRequestMask)
	protocol[count++] = Atoms::wmSyncRequest;

    XSetWMProtocols (screen->dpy (), id, protocol, count);
}

bool
WinrulesWindow::is ()
{
    if (window->overrideRedirect ())
	return false;

    if (window->wmType () & CompWindowTypeDesktopMask)
	return false;

    return true;
}

bool
WinrulesWindow::isFocussable ()
{
    window->isFocussable ();

    return false; // We only want to return false else where we are not wrapped
}

bool
WinrulesWindow::focus ()
{
    window->focus ();

    return false; // We only want to return false for the window we are wrapped
}

bool
WinrulesWindow::alpha ()
{
    window->alpha ();

    return false; // We only want to return false else where we are not wrapped
}

void
WinrulesWindow::setNoFocus (int        optNum)
{
    unsigned int newProtocol = window->protocols ();

    WINRULES_SCREEN (screen);

    if (!is ())
	return;

    if (ws->getOptions ().at (optNum). value ().match ().evaluate (window))
    {
	if (window->protocols () & CompWindowProtocolTakeFocusMask)
	{
	    protocolSetMask |= (window->protocols () &
				CompWindowProtocolTakeFocusMask);
	    newProtocol = window->protocols () & ~CompWindowProtocolTakeFocusMask;
	}
	window->isFocussableSetEnabled (this, true);// causes w->isFocussable ()
						    // to return false
	window->focusSetEnabled (this, true); // causes w->focus () to return
					      // false for this window
    }
    else if ((protocolSetMask & CompWindowProtocolTakeFocusMask))
    {
	newProtocol = window->protocols () |
	              (protocolSetMask & CompWindowProtocolTakeFocusMask);
	protocolSetMask &= ~CompWindowProtocolTakeFocusMask;
	window->isFocussableSetEnabled (this, false);
	window->focusSetEnabled (this, false);
    }

    if (newProtocol != window->protocols ())
    {
	ws->setProtocols (newProtocol, window->id ());
    }
}

void
WinrulesWindow::setNoAlpha (int        optNum)
{
    WINRULES_SCREEN (screen);

    if (!is ())
	return;

    if (ws->getOptions ().at (optNum). value ().match ().evaluate (window))
    {
	window->alphaSetEnabled (this, true); // Causes w->alpha () to return
					      // false
    }
    else
    {
	window->alphaSetEnabled (this, false);
    }
}

void
WinrulesWindow::updateState (int        optNum,
		             int        mask)
{
    unsigned int newState = window->state ();

    WINRULES_SCREEN (screen);

    if (!is ())
	return;

    if (ws->getOptions ().at (optNum). value ().match ().evaluate (window))
    {
	newState |= mask;
	newState = window->constrainWindowState (newState, window->actions ());
	stateSetMask |= (newState & mask);
    }
    else if (stateSetMask & mask)
    {
	newState &= ~mask;
	stateSetMask &= ~mask;
    }

    if (newState != window->state ())
    {
	window->changeState (newState);

	if (mask & (CompWindowStateFullscreenMask |
		    CompWindowStateAboveMask      |
		    CompWindowStateBelowMask       ))
	    window->updateAttributes (CompStackingUpdateModeNormal);
	else
	    window->updateAttributes (CompStackingUpdateModeNone);
    }
}

void
WinrulesWindow::setAllowedActions (int        optNum,
			   	  int        action)
{
    WINRULES_SCREEN (screen);

    if (!is ())
	return;

    if (ws->getOptions ().at (optNum). value ().match ().evaluate (window))
	allowedActions &= ~action;
    else if (!(allowedActions & action))
	allowedActions |= action;

    window->recalcActions ();
}

bool
WinrulesWindow::matchSizeValue (CompOption::Value::Vector matches,
				CompOption::Value::Vector widthValues,
				CompOption::Value::Vector heightValues,
				int	   *width,
				int	   *height)
{
    int min;

    if (!is ())
	return false;

    if (window->type () & CompWindowTypeDesktopMask)
	return false;

    min = MIN (matches.size (), widthValues.size ());
    min = MIN ((unsigned int) min, heightValues.size ());
    min = MIN ((unsigned int) min, matches.size ());

    for (int i = 0; i < min; i++)
    {
	if ((matches.at (i).match ().evaluate (window)))
	{
	    *width = widthValues.at (i).i ();
	    *height = heightValues.at (i).i ();
	
	    return true;
	}
    }

    return false;
}

bool
WinrulesWindow::matchSize (int	      *width,
			   int	      *height)
{
    WINRULES_SCREEN (screen);

    return matchSizeValue (ws->optionGetSizeMatches (),
			   ws->optionGetSizeWidthValues (),
			   ws->optionGetSizeHeightValues (),
			   width, height);
}

void
WinrulesWindow::updateWindowSize (int        width,
				  int        height)
{
    XWindowChanges xwc;
    unsigned int   xwcm = 0;

    if (width != window->serverWidth ())
	xwcm |= CWWidth;
    if (height != window->serverHeight ())
	xwcm |= CWHeight;

    xwc.width = width;
    xwc.height = height;

    if (window->mapNum () && xwcm)
	window->sendSyncRequest ();

    window->configureXWindow (xwcm, &xwc);
}

void
WinrulesScreen::optionChanged (CompOption	       *option,
			       WinrulesOptions::Options num)
{

    unsigned int updateStateMask = 0, updateActionsMask = 0;

    switch (num)
    {
	case WinrulesOptions::SkiptaskbarMatch:
	    updateStateMask = CompWindowStateSkipTaskbarMask;
	    break;
	case WinrulesOptions::SkippagerMatch:
	    updateStateMask = CompWindowStateSkipPagerMask;
	    break;
	case WinrulesOptions::AboveMatch:
	    updateStateMask = CompWindowStateAboveMask;
	break;
	case WinrulesOptions::BelowMatch:
	    updateStateMask = CompWindowStateBelowMask;
	break;
	case WinrulesOptions::StickyMatch:
	    updateStateMask = CompWindowStateStickyMask;
	break;
	case WinrulesOptions::FullscreenMatch:
	    updateStateMask = CompWindowStateFullscreenMask;
	break;
	case WinrulesOptions::MaximizeMatch:
	    updateStateMask = CompWindowStateMaximizedHorzMask |
			      CompWindowStateMaximizedVertMask;
	break;
	case WinrulesOptions::NoMoveMatch:
	    updateActionsMask = CompWindowActionMoveMask;
	break;
	case WinrulesOptions::NoResizeMatch:
	    updateActionsMask = CompWindowActionResizeMask;
	break;
	case WinrulesOptions::NoMinimizeMatch:
	    updateActionsMask = CompWindowActionMinimizeMask;
	break;
	case WinrulesOptions::NoMaximizeMatch:
	    updateActionsMask = CompWindowActionMaximizeVertMask |
		                CompWindowActionMaximizeHorzMask;
	break;
	case WinrulesOptions::NoCloseMatch:
	    updateActionsMask = CompWindowActionCloseMask;
	break;
	case WinrulesOptions::NoArgbMatch:
	    foreach (CompWindow *w, screen->windows ())
	    {
		WINRULES_WINDOW (w);
		ww->setNoAlpha (num);
	    }

	    return;
	break;
	case WinrulesOptions::SizeMatches:
	    foreach (CompOption::Value &v, option->value ().list ())
	    {
	        CompMatch &m = v.match ();
		m.update ();
	    }
	    return;
	break;
	default:
	    return;
	break;
    }

    if (updateStateMask)
    {
	/* We traverse a copy of the list here because windows can be unhooked
	 * on state change rather than the delayed unhook that happens in <0.8.x
	 */

	CompWindowList windows = screen->windows ();

	foreach (CompWindow *w, windows)
	{
	    WINRULES_WINDOW (w);
	    ww->updateState (num, updateStateMask);
	}

	return;
    }

    if (updateActionsMask)
    {
	foreach (CompWindow *w, screen->windows ())
	{
	    WINRULES_WINDOW (w);
	    ww->setAllowedActions (num, updateActionsMask);
	}

	return;
    }

    return;
}


bool
WinrulesWindow::applyRules ()
{
    int        width, height;

    updateState (WinrulesOptions::SkiptaskbarMatch,
		 CompWindowStateSkipTaskbarMask);

    updateState (WinrulesOptions::SkippagerMatch,
		 CompWindowStateSkipPagerMask);

    updateState (WinrulesOptions::AboveMatch,
		 CompWindowStateAboveMask);

    updateState (WinrulesOptions::BelowMatch,
		 CompWindowStateBelowMask);

    updateState (WinrulesOptions::StickyMatch,
		 CompWindowStateStickyMask);

    updateState (WinrulesOptions::FullscreenMatch,
		 CompWindowStateFullscreenMask);

    updateState (WinrulesOptions::MaximizeMatch,
		 CompWindowStateMaximizedHorzMask |
		 CompWindowStateMaximizedVertMask);

    setAllowedActions (WinrulesOptions::NoMoveMatch,
		       CompWindowActionMoveMask);

    setAllowedActions (WinrulesOptions::NoResizeMatch,
		       CompWindowActionResizeMask);

    setAllowedActions (WinrulesOptions::NoMinimizeMatch,
		       CompWindowActionMinimizeMask);

    setAllowedActions (WinrulesOptions::NoMaximizeMatch,
		       CompWindowActionMaximizeVertMask |
		       CompWindowActionMaximizeHorzMask);

    setAllowedActions (WinrulesOptions::NoCloseMatch,
		       CompWindowActionCloseMask);

    setNoAlpha (WinrulesOptions::NoArgbMatch);

    if (matchSize (&width, &height))
	updateWindowSize (width, height);

    return false;
}


void
WinrulesScreen::handleEvent (XEvent *event)
{
    if (event->type == MapRequest)
    {
	CompWindow *w = screen->findWindow (event->xmap.window);
	if (w)
	{
	    WINRULES_WINDOW (w);
	    ww->setNoFocus (WinrulesOptions::NoFocusMatch);
	    ww->applyRules ();
	}
    }

    screen->handleEvent (event);

}

void
WinrulesWindow::getAllowedActions (unsigned int &setActions,
				   unsigned int &clearActions)
{
    window->getAllowedActions (setActions, clearActions);

    clearActions |= ~allowedActions;
}

void
WinrulesScreen::matchExpHandlerChanged ()
{
    screen->matchExpHandlerChanged ();

    /* match options are up to date after the call to matchExpHandlerChanged */
    foreach (CompWindow *w, screen->windows ())
    {
	WINRULES_WINDOW (w);
	ww->applyRules ();
    }
}

void
WinrulesScreen::matchPropertyChanged (CompWindow *w)
{
    WINRULES_WINDOW (w);

    /* Re-apply rules on match property change */
    ww->applyRules ();

    screen->matchPropertyChanged (w);
}

WinrulesScreen::WinrulesScreen (CompScreen *screen) :
    PluginClassHandler <WinrulesScreen, CompScreen> (screen)
{
    ScreenInterface::setHandler (screen);

    optionSetSkiptaskbarMatchNotify (boost::bind
					(&WinrulesScreen::optionChanged, this,
					 _1, _2));

    optionSetSkippagerMatchNotify (boost::bind
				   (&WinrulesScreen::optionChanged, this,
				    _1, _2));

    optionSetAboveMatchNotify (boost::bind
				(&WinrulesScreen::optionChanged, this,
				 _1, _2));

    optionSetBelowMatchNotify (boost::bind
				(&WinrulesScreen::optionChanged, this,
				 _1, _2));

    optionSetFullscreenMatchNotify (boost::bind
				    (&WinrulesScreen::optionChanged, this,
				     _1, _2));

    optionSetStickyMatchNotify (boost::bind
				(&WinrulesScreen::optionChanged, this,
				_1, _2));

    optionSetMaximizeMatchNotify (boost::bind
				  (&WinrulesScreen::optionChanged, this,
				   _1, _2));

    optionSetNoArgbMatchNotify (boost::bind
				(&WinrulesScreen::optionChanged, this,
				 _1, _2));

    optionSetNoMoveMatchNotify (boost::bind
				(&WinrulesScreen::optionChanged, this,
				 _1, _2));

    optionSetNoResizeMatchNotify (boost::bind
				  (&WinrulesScreen::optionChanged, this,
				   _1, _2));

    optionSetNoMinimizeMatchNotify (boost::bind
				    (&WinrulesScreen::optionChanged, this,
				     _1, _2));

    optionSetNoMaximizeMatchNotify (boost::bind
				    (&WinrulesScreen::optionChanged, this,
				     _1, _2));

    optionSetNoCloseMatchNotify (boost::bind
				 (&WinrulesScreen::optionChanged, this,
				  _1, _2));

    optionSetNoFocusMatchNotify (boost::bind
				 (&WinrulesScreen::optionChanged, this,
				  _1, _2));

}

WinrulesWindow::WinrulesWindow (CompWindow *window) :
    PluginClassHandler <WinrulesWindow, CompWindow> (window),
    window (window),
    allowedActions (~0),
    stateSetMask (0),
    protocolSetMask (0)
{
    CompTimer timer;

    WindowInterface::setHandler (window);

    window->isFocussableSetEnabled (this, false);
    window->alphaSetEnabled (this, false);
    window->focusSetEnabled (this, false);

    timer.setCallback (boost::bind(&WinrulesWindow::applyRules, this));
    timer.setTimes (0, 0);

    timer.start ();

}

bool
WinrulesPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    return true;
}

