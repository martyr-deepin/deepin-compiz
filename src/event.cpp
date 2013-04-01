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

#include <stdlib.h>
#include <string.h>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/Xfixes.h>

#include <core/atoms.h>
#include <core/servergrab.h>
#include "privatescreen.h"
#include "privatewindow.h"
#include "privatestackdebugger.h"

namespace cps = compiz::private_screen;

namespace
{
Window CompWindowToWindow (CompWindow *window)
{
    return window ? window->id () : None;
}
}


bool
PrivateWindow::handleSyncAlarm ()
{
    if (priv->syncWait)
    {
	priv->syncWait = false;

	if (window->resize (priv->syncGeometry))
	{
	    window->windowNotify (CompWindowNotifySyncAlarm);
	}
	else
	{
	    /* resizeWindow failing means that there is another pending
	       resize and we must send a new sync request to the client */
	    window->sendSyncRequest ();
	}
    }

    return false;
}


static bool
autoRaiseTimeout (CompScreen *screen)
{
    CompWindow  *w = screen->findWindow (screen->activeWindow ());

    if (screen->autoRaiseWindow () == screen->activeWindow () ||
	(w && (screen->autoRaiseWindow () == w->transientFor ())))
    {
	w = screen->findWindow (screen->autoRaiseWindow ());
	if (w)
	    w->updateAttributes (CompStackingUpdateModeNormal);
    }

    return false;
}

#define REAL_MOD_MASK (ShiftMask | ControlMask | Mod1Mask | Mod2Mask | \
		       Mod3Mask | Mod4Mask | Mod5Mask | CompNoMask)

static bool
isCallBackBinding (CompOption	           &option,
		   CompAction::BindingType type,
		   CompAction::State       state)
{
    if (!option.isAction ())
	return false;

    if (!(option.value ().action ().type () & type))
	return false;

    if (!(option.value ().action ().state () & state))
	return false;

    return true;
}

static bool
isInitiateBinding (CompOption	           &option,
		   CompAction::BindingType type,
		   CompAction::State       state,
		   CompAction	           **action)
{
    if (!isCallBackBinding (option, type, state))
	return false;

    if (option.value ().action ().initiate ().empty ())
	return false;

    *action = &option.value ().action ();

    return true;
}

static bool
isBound (CompOption             &option,
         CompAction::BindingType type,
         CompAction::State       state,
         CompAction            **action)
{
    if (!isCallBackBinding (option, type, state))
	return false;

    *action = &option.value ().action ();

    return true;
}

bool
cps::EventManager::triggerPress (CompAction         *action,
                             CompAction::State   state,
                             CompOption::Vector &arguments)
{
    bool actionEventHandled = false;

    if (state == CompAction::StateInitKey && grabsEmpty ())
    {
        possibleTap = action;
    }

    if (!action->initiate ().empty ())
    {
	if (action->initiate () (action, state, arguments))
	    actionEventHandled = true;
    }
    else if (!action->terminate ().empty ())
    {
	/* Default Initiate implementation for plugins that only
	   provide a Terminate callback */
	if (state & CompAction::StateInitKey)
	    action->setState (action->state () | CompAction::StateTermKey);
    }

    return actionEventHandled;
}

bool
cps::EventManager::triggerRelease (CompAction         *action,
                               CompAction::State   state,
                               CompOption::Vector &arguments)
{
    if (action == possibleTap)
    {
        state |= CompAction::StateTermTapped;
        possibleTap = NULL;
    }

    if (!action->terminate ().empty () &&
        action->terminate () (action, state, arguments))
        return true;

    return false;
}

bool
PrivateScreen::triggerButtonPressBindings (CompOption::Vector &options,
					   XButtonEvent       *event,
					   CompOption::Vector &arguments)
{
    CompAction::State state = CompAction::StateInitButton;
    CompAction        *action;
    unsigned int      ignored = modHandler->ignoredModMask ();
    unsigned int      modMask = REAL_MOD_MASK & ~ignored;
    unsigned int      bindMods;
    unsigned int      edge = 0;

    if (edgeWindow)
    {
	unsigned int i;

	if (event->root != screen->root())
	    return false;

	if (event->window != edgeWindow)
	{
	    if (eventManager.grabsEmpty () || event->window != screen->root())
		return false;
	}

	for (i = 0; i < SCREEN_EDGE_NUM; i++)
	{
	    if (edgeWindow == screenEdge[i].id)
	    {
		edge = 1 << i;
		arguments[1].value ().set ((int) orphanData.activeWindow);
		break;
	    }
	}
    }

    foreach (CompOption &option, options)
    {
	if (isBound (option, CompAction::BindingTypeButton, state, &action))
	{
	    if (action->button ().button () == (int) event->button)
	    {
		bindMods = modHandler->virtualToRealModMask (
		    action->button ().modifiers ());

		if ((bindMods & modMask) == (event->state & modMask))
		{
		    if (eventManager.triggerPress (action, state, arguments))
			return true;
		}
	    }
	}

	if (edge)
	{
	    if (isInitiateBinding (option, CompAction::BindingTypeEdgeButton,
				   state | CompAction::StateInitEdge, &action))
	    {
		if ((action->button ().button () == (int) event->button) &&
		    (action->edgeMask () & edge))
		{
		    bindMods = modHandler->virtualToRealModMask (
			action->button ().modifiers ());

		    if ((bindMods & modMask) == (event->state & modMask))
			if (action->initiate () (action, state |
						 CompAction::StateInitEdge,
						 arguments))
			    return true;
		}
	    }
	}
    }

    return false;
}

bool
PrivateScreen::triggerButtonReleaseBindings (CompOption::Vector &options,
					     XButtonEvent       *event,
					     CompOption::Vector &arguments)
{
    CompAction::State       state = CompAction::StateTermButton;
    CompAction::BindingType type  = CompAction::BindingTypeButton |
				    CompAction::BindingTypeEdgeButton;
    CompAction	            *action;

    foreach (CompOption &option, options)
    {
	if (isBound (option, type, state, &action))
	{
	    if (action->button ().button () == (int) event->button)
	    {
	        if (eventManager.triggerRelease (action, state, arguments))
		    return true;
	    }
	}
    }

    return false;
}

bool
PrivateScreen::triggerKeyPressBindings (CompOption::Vector &options,
					XKeyEvent          *event,
					CompOption::Vector &arguments)
{
    CompAction::State state = 0;
    CompAction	      *action;
    unsigned int      modMask = REAL_MOD_MASK & ~modHandler->ignoredModMask ();
    unsigned int      bindMods;

    if (event->keycode == escapeKeyCode)
	state = CompAction::StateCancel;
    else if (event->keycode == returnKeyCode)
	state = CompAction::StateCommit;

    if (state)
    {
	foreach (CompOption &o, options)
	{
	    if (o.isAction ())
	    {
		if (!o.value ().action ().terminate ().empty ())
		    o.value ().action ().terminate () (&o.value ().action (),
						       state, noOptions ());
	    }
	}

	if (state == CompAction::StateCancel)
	    return false;
    }

    state = CompAction::StateInitKey;
    foreach (CompOption &option, options)
    {
	if (isBound (option, CompAction::BindingTypeKey, state, &action))
	{
	    bindMods = modHandler->virtualToRealModMask (
		action->key ().modifiers ());

	    bool match = false;
	    if (action->key ().keycode () == (int) event->keycode)
		match = ((bindMods & modMask) == (event->state & modMask));
	    else if (!xkbEvent.get() && action->key ().keycode () == 0)
		match = (bindMods == (event->state & modMask));

	    if (match && eventManager.triggerPress (action, state, arguments))
		return true;
	}
    }

    return false;
}

bool
PrivateScreen::triggerKeyReleaseBindings (CompOption::Vector &options,
					  XKeyEvent          *event,
					  CompOption::Vector &arguments)
{
    CompAction::State state = CompAction::StateTermKey;
    CompAction        *action;
    unsigned int      ignored = modHandler->ignoredModMask ();
    unsigned int      modMask = REAL_MOD_MASK & ~ignored;
    unsigned int      bindMods;
    unsigned int      mods;

    mods = modHandler->keycodeToModifiers (event->keycode);
    if (!xkbEvent.get() && !mods)
	return false;

    bool handled = false;

    foreach (CompOption &option, options)
    {
	if (isBound (option, CompAction::BindingTypeKey, state, &action))
	{
	    bindMods = modHandler->virtualToRealModMask (action->key ().modifiers ());

	    bool match = false;
	    if ((bindMods & modMask) == 0)
		match = ((unsigned int) action->key ().keycode () ==
		         (unsigned int) event->keycode);
	    else if (!xkbEvent.get() && ((mods & modMask & bindMods) != bindMods))
	        match = true;

	    handled |= match && eventManager.triggerRelease (action, state, arguments);
	}
    }

    return handled;
}

bool
PrivateScreen::triggerStateNotifyBindings (CompOption::Vector  &options,
					   XkbStateNotifyEvent *event,
					   CompOption::Vector  &arguments)
{
    CompAction::State state;
    CompAction        *action;
    unsigned int      ignored = modHandler->ignoredModMask ();
    unsigned int      modMask = REAL_MOD_MASK & ~ignored;
    unsigned int      bindMods;

    if (event->event_type == KeyPress)
    {
	state = CompAction::StateInitKey;

	foreach (CompOption &option, options)
	{
	    if (isBound (option, CompAction::BindingTypeKey, state, &action))
	    {
		if (action->key ().keycode () == 0)
		{
		    bindMods =
			modHandler->virtualToRealModMask (action->key ().modifiers ());

		    if ((event->mods & modMask) == bindMods)
		    {
		        if (eventManager.triggerPress (action, state, arguments))
			    return true;
		    }
		}
	    }
	}
    }
    else if (event->event_type == KeyRelease)
    {
	state = CompAction::StateTermKey;
	bool handled = false;

	foreach (CompOption &option, options)
	{
	    if (isBound (option, CompAction::BindingTypeKey, state, &action))
	    {
		bindMods = modHandler->virtualToRealModMask (
		    action->key ().modifiers ());
		unsigned int modKey =
		    modHandler->keycodeToModifiers (event->keycode);

		if ((event->mods && ((event->mods & modMask) != bindMods)) ||
		    (!event->mods && (modKey == bindMods)))
		{
		    handled |= eventManager.triggerRelease (action, state, arguments);
		}
	    }
	}

	if (handled)
	    return true;
    }

    return false;
}

static bool
isBellAction (CompOption        &option,
	      CompAction::State state,
	      CompAction        **action)
{
    if (option.type () != CompOption::TypeAction &&
	option.type () != CompOption::TypeBell)
	return false;

    if (!option.value ().action ().bell ())
	return false;

    if (!(option.value ().action ().state () & state))
	return false;

    if (option.value ().action ().initiate ().empty ())
	return false;

    *action = &option.value ().action ();

    return true;
}

static bool
triggerBellNotifyBindings (CompOption::Vector &options,
			   CompOption::Vector &arguments)
{
    CompAction::State state = CompAction::StateInitBell;
    CompAction        *action;

    foreach (CompOption &option, options)
    {
	if (isBellAction (option, state, &action))
	{
	    if (action->initiate () (action, state, arguments))
		return true;
	}
    }

    return false;
}

static bool
isEdgeAction (CompOption        &option,
	      CompAction::State state,
	      unsigned int      edge)
{
    if (option.type () != CompOption::TypeAction &&
	option.type () != CompOption::TypeButton &&
	option.type () != CompOption::TypeEdge)
	return false;

    if (!(option.value ().action ().edgeMask () & edge))
	return false;

    if (!(option.value ().action ().state () & state))
	return false;

    return true;
}

static bool
isEdgeEnterAction (CompOption        &option,
		   CompAction::State state,
		   CompAction::State delayState,
		   unsigned int      edge,
		   CompAction        **action)
{
    if (!isEdgeAction (option, state, edge))
	return false;

    if (option.value ().action ().type () & CompAction::BindingTypeEdgeButton)
	return false;

    if (option.value ().action ().initiate ().empty ())
	return false;

    if (delayState)
    {
	if ((option.value ().action ().state () &
	     CompAction::StateNoEdgeDelay) !=
	    (delayState & CompAction::StateNoEdgeDelay))
	{
	    /* ignore edge actions which shouldn't be delayed when invoking
	       undelayed edges (or vice versa) */
	    return false;
	}
    }


    *action = &option.value ().action ();

    return true;
}

static bool
isEdgeLeaveAction (CompOption        &option,
		   CompAction::State state,
		   unsigned int      edge,
		   CompAction        **action)
{
    if (!isEdgeAction (option, state, edge))
	return false;

    if (option.value ().action ().terminate ().empty ())
	return false;

    *action = &option.value ().action ();

    return true;
}

static bool
triggerEdgeEnterBindings (CompOption::Vector &options,
			  CompAction::State  state,
			  CompAction::State  delayState,
			  unsigned int	     edge,
			  CompOption::Vector &arguments)
{
    CompAction *action;

    foreach (CompOption &option, options)
    {
	if (isEdgeEnterAction (option, state, delayState, edge, &action))
	{
	    if (action->initiate () (action, state, arguments))
		return true;
	}
    }

    return false;
}

static bool
triggerEdgeLeaveBindings (CompOption::Vector &options,
			  CompAction::State  state,
			  unsigned int	     edge,
			  CompOption::Vector &arguments)
{
    CompAction *action;

    foreach (CompOption &option, options)
    {
	if (isEdgeLeaveAction (option, state, edge, &action))
	{
	    if (action->terminate () (action, state, arguments))
		return true;
	}
    }

    return false;
}

static bool
triggerAllEdgeEnterBindings (CompAction::State  state,
			     CompAction::State  delayState,
			     unsigned int       edge,
			     CompOption::Vector &arguments)
{
    foreach (CompPlugin *p, CompPlugin::getPlugins ())
    {
	CompOption::Vector &options = p->vTable->getOptions ();
	if (triggerEdgeEnterBindings (options, state, delayState, edge,
				      arguments))
	{
	    return true;
	}
    }
    return false;
}

static bool
delayedEdgeTimeout (CompDelayedEdgeSettings *settings)
{
    triggerAllEdgeEnterBindings (settings->state,
				 ~CompAction::StateNoEdgeDelay,
				 settings->edge,
				 settings->options);

    return false;
}

bool
PrivateScreen::triggerEdgeEnter (unsigned int       edge,
				 CompAction::State  state,
				 CompOption::Vector &arguments)
{
    int                     delay;

    delay = optionGetEdgeDelay ();

    if (delay > 0)
    {
	CompAction::State delayState;
	edgeDelaySettings.edge    = edge;
	edgeDelaySettings.state   = state;
	edgeDelaySettings.options = arguments;

	edgeDelayTimer.start  (
	    boost::bind (delayedEdgeTimeout, &edgeDelaySettings),
			 delay, (unsigned int) ((float) delay * 1.2));

	delayState = CompAction::StateNoEdgeDelay;
	if (triggerAllEdgeEnterBindings (state, delayState, edge, arguments))
	    return true;
    }
    else
    {
	if (triggerAllEdgeEnterBindings (state, 0, edge, arguments))
	    return true;
    }

    return false;
}

bool
PrivateScreen::handleActionEvent (XEvent *event)
{
    static CompOption::Vector o;
    Window xid;

    if (o.empty ())
    {
	o.resize (8);
	o[0].setName ("event_window", CompOption::TypeInt);
	o[1].setName ("window", CompOption::TypeInt);
	o[2].setName ("modifiers", CompOption::TypeInt);
	o[3].setName ("x", CompOption::TypeInt);
	o[4].setName ("y", CompOption::TypeInt);
	o[5].setName ("root", CompOption::TypeInt);
    }
    else
    {
	o[6].reset ();
	o[7].reset ();
    }

    switch (event->type) {
    case ButtonPress:
	/* We need to determine if we clicked on a parent frame
	 * window, if so, pass the appropriate child window as
	 * "window" and the frame as "event_window"
	 */

	xid = event->xbutton.window;

	foreach (CompWindow *w, screen->windows ())
	{
	    if (w->priv->frame == xid)
		xid = w->id ();
	}

	o[0].value ().set ((int) event->xbutton.window);
	o[1].value ().set ((int) xid);
	o[2].value ().set ((int) event->xbutton.state);
	o[3].value ().set ((int) event->xbutton.x_root);
	o[4].value ().set ((int) event->xbutton.y_root);
	o[5].value ().set ((int) event->xbutton.root);

	o[6].setName ("button", CompOption::TypeInt);
	o[7].setName ("time", CompOption::TypeInt);

	o[6].value ().set ((int) event->xbutton.button);
	o[7].value ().set ((int) event->xbutton.time);

	eventManager.resetPossibleTap();
	foreach (CompPlugin *p, CompPlugin::getPlugins ())
	{
	    CompOption::Vector &options = p->vTable->getOptions ();
	    if (triggerButtonPressBindings (options, &event->xbutton, o))
		return true;
	}
	break;
    case ButtonRelease:
	o[0].value ().set ((int) event->xbutton.window);
	o[1].value ().set ((int) event->xbutton.window);
	o[2].value ().set ((int) event->xbutton.state);
	o[3].value ().set ((int) event->xbutton.x_root);
	o[4].value ().set ((int) event->xbutton.y_root);
	o[5].value ().set ((int) event->xbutton.root);

	o[6].setName ("button", CompOption::TypeInt);
	o[7].setName ("time", CompOption::TypeInt);

	o[6].value ().set ((int) event->xbutton.button);
	o[7].value ().set ((int) event->xbutton.time);

	foreach (CompPlugin *p, CompPlugin::getPlugins ())
	{
	    CompOption::Vector &options = p->vTable->getOptions ();
	    if (triggerButtonReleaseBindings (options, &event->xbutton, o))
		return true;
	}
	break;
    case KeyPress:
	o[0].value ().set ((int) event->xkey.window);
	o[1].value ().set ((int) orphanData.activeWindow);
	o[2].value ().set ((int) event->xkey.state);
	o[3].value ().set ((int) event->xkey.x_root);
	o[4].value ().set ((int) event->xkey.y_root);
	o[5].value ().set ((int) event->xkey.root);

	o[6].setName ("keycode", CompOption::TypeInt);
	o[7].setName ("time", CompOption::TypeInt);

	o[6].value ().set ((int) event->xkey.keycode);
	o[7].value ().set ((int) event->xkey.time);

	eventManager.resetPossibleTap();
	foreach (CompPlugin *p, CompPlugin::getPlugins ())
	{
	    CompOption::Vector &options = p->vTable->getOptions ();
	    if (triggerKeyPressBindings (options, &event->xkey, o))
		return true;
	}
	break;
    case KeyRelease:
    {
	o[0].value ().set ((int) event->xkey.window);
	o[1].value ().set ((int) orphanData.activeWindow);
	o[2].value ().set ((int) event->xkey.state);
	o[3].value ().set ((int) event->xkey.x_root);
	o[4].value ().set ((int) event->xkey.y_root);
	o[5].value ().set ((int) event->xkey.root);

	o[6].setName ("keycode", CompOption::TypeInt);
	o[7].setName ("time", CompOption::TypeInt);

	o[6].value ().set ((int) event->xkey.keycode);
	o[7].value ().set ((int) event->xkey.time);

	bool handled = false;

	foreach (CompPlugin *p, CompPlugin::getPlugins ())
	{
	    CompOption::Vector &options = p->vTable->getOptions ();
	    handled |= triggerKeyReleaseBindings (options, &event->xkey, o);
	}

        if (handled)
	    return true;

	break;
    }
    case EnterNotify:
	if (event->xcrossing.mode   != NotifyGrab   &&
	    event->xcrossing.mode   != NotifyUngrab &&
	    event->xcrossing.detail != NotifyInferior)
	{
	    unsigned int      edge, i;
	    CompAction::State state;

	    if (event->xcrossing.root != rootWindow())
		return false;

	    if (edgeDelayTimer.active ())
		edgeDelayTimer.stop ();

	    if (edgeWindow && edgeWindow != event->xcrossing.window)
	    {
		state = CompAction::StateTermEdge;
		edge  = 0;

		for (i = 0; i < SCREEN_EDGE_NUM; i++)
		{
		    if (edgeWindow == screenEdge[i].id)
		    {
			edge = 1 << i;
			break;
		    }
		}

		edgeWindow = None;

		o[0].value ().set ((int) event->xcrossing.window);
		o[1].value ().set ((int) orphanData.activeWindow);
		o[2].value ().set ((int) event->xcrossing.state);
		o[3].value ().set ((int) event->xcrossing.x_root);
		o[4].value ().set ((int) event->xcrossing.y_root);
		o[5].value ().set ((int) event->xcrossing.root);

		o[6].setName ("time", CompOption::TypeInt);
		o[6].value ().set ((int) event->xcrossing.time);

		foreach (CompPlugin *p, CompPlugin::getPlugins ())
		{
		    CompOption::Vector &options = p->vTable->getOptions ();
		    if (triggerEdgeLeaveBindings (options, state, edge, o))
			return true;
		}
	    }

	    edge = 0;

	    for (i = 0; i < SCREEN_EDGE_NUM; i++)
	    {
		if (event->xcrossing.window == screenEdge[i].id)
		{
		    edge = 1 << i;
		    break;
		}
	    }

	    if (edge)
	    {
		state = CompAction::StateInitEdge;

		edgeWindow = event->xcrossing.window;

		o[0].value ().set ((int) event->xcrossing.window);
		o[1].value ().set ((int) orphanData.activeWindow);
		o[2].value ().set ((int) event->xcrossing.state);
		o[3].value ().set ((int) event->xcrossing.x_root);
		o[4].value ().set ((int) event->xcrossing.y_root);
		o[5].value ().set ((int) event->xcrossing.root);

		o[6].setName ("time", CompOption::TypeInt);
		o[6].value ().set ((int) event->xcrossing.time);

		if (triggerEdgeEnter (edge, state, o))
		    return true;
	    }
	}
	break;
    case ClientMessage:
	if (event->xclient.message_type == Atoms::xdndEnter)
	{
	    xdndWindow = event->xclient.window;
	}
	else if (event->xclient.message_type == Atoms::xdndLeave)
	{
	    unsigned int      edge = 0;
	    CompAction::State state;

	    if (!xdndWindow)
	    {
		CompWindow *w;

		w = screen->findWindow (event->xclient.window);
		if (w)
		{
		    unsigned int i;

		    for (i = 0; i < SCREEN_EDGE_NUM; i++)
		    {
			if (event->xclient.window == screenEdge[i].id)
			{
			    edge = 1 << i;
			    break;
			}
		    }
		}
	    }

	    if (edge)
	    {
		state = CompAction::StateTermEdgeDnd;

		o[0].value ().set ((int) event->xclient.window);
		o[1].value ().set ((int) orphanData.activeWindow);
		o[2].value ().set ((int) 0); /* fixme */
		o[3].value ().set ((int) 0); /* fixme */
		o[4].value ().set ((int) 0); /* fixme */
		o[5].value ().set ((int) rootWindow());

		foreach (CompPlugin *p, CompPlugin::getPlugins ())
		{
		    CompOption::Vector &options = p->vTable->getOptions ();
		    if (triggerEdgeLeaveBindings (options, state, edge, o))
			return true;
		}
	    }
	}
	else if (event->xclient.message_type == Atoms::xdndPosition)
	{
	    unsigned int      edge = 0;
	    CompAction::State state;

	    if (xdndWindow == event->xclient.window)
	    {
		CompWindow *w;

		w = screen->findWindow (event->xclient.window);
		if (w)
		{
		    unsigned int i;

		    for (i = 0; i < SCREEN_EDGE_NUM; i++)
		    {
			if (xdndWindow == screenEdge[i].id)
			{
			    edge = 1 << i;
			    break;
			}
		    }
		}
	    }

	    if (edge)
	    {
		state = CompAction::StateInitEdgeDnd;

		o[0].value ().set ((int) event->xclient.window);
		o[1].value ().set ((int) orphanData.activeWindow);
		o[2].value ().set ((int) 0); /* fixme */
		o[3].value ().set ((int) event->xclient.data.l[2] >> 16);
		o[4].value ().set ((int) event->xclient.data.l[2] & 0xffff);
		o[5].value ().set ((int) rootWindow());

		if (triggerEdgeEnter (edge, state, o))
		    return true;
	    }

	    xdndWindow = None;
	}
	break;
    default:
	if (event->type == xkbEvent.get())
	{
	    XkbAnyEvent *xkbEvent = (XkbAnyEvent *) event;
	    static CompOption::Vector arg;

	    if (arg.empty ())
	    {
		arg.resize (8);
		arg[0].setName ("event_window", CompOption::TypeInt);
		arg[1].setName ("window", CompOption::TypeInt);
	    }

	    if (xkbEvent->xkb_type == XkbStateNotify)
	    {
		XkbStateNotifyEvent *stateEvent = (XkbStateNotifyEvent *) event;

		arg[0].value ().set ((int) orphanData.activeWindow);
		arg[1].value ().set ((int) orphanData.activeWindow);
		arg[2].setName ("modifiers", CompOption::TypeInt);
		arg[2].value ().set ((int) stateEvent->mods);
		arg[3].setName ("time", CompOption::TypeInt);
		arg[3].value ().set ((int) xkbEvent->time);
		arg[7].value ().set ((int) xkbEvent->time);

		if (stateEvent->event_type == KeyPress)
		    eventManager.resetPossibleTap();

		bool handled = false;

		foreach (CompPlugin *p, CompPlugin::getPlugins ())
		{
		    CompOption::Vector &options = p->vTable->getOptions ();
		    handled |= triggerStateNotifyBindings (options, stateEvent, arg);
		}

		if (handled)
		    return true;
	    }
	    else if (xkbEvent->xkb_type == XkbBellNotify)
	    {
		arg[0].value ().set ((int) orphanData.activeWindow);
		arg[1].value ().set ((int) orphanData.activeWindow);
		arg[2].setName ("time", CompOption::TypeInt);
		arg[2].value ().set ((int) xkbEvent->time);
		arg[3].reset ();
		arg[7].reset ();

		foreach (CompPlugin *p, CompPlugin::getPlugins ())
		{
		    CompOption::Vector &options = p->vTable->getOptions ();
		    if (triggerBellNotifyBindings (options, arg))
			return true;
		}
	    }
	}
	break;
    }

    return false;
}

void
PrivateScreen::setDefaultWindowAttributes (XWindowAttributes *wa)
{
    wa->x		      = 0;
    wa->y		      = 0;
    wa->width		      = 1;
    wa->height		      = 1;
    wa->border_width	      = 0;
    wa->depth		      = 0;
    wa->visual		      = NULL;
    wa->root		      = rootWindow();
    wa->c_class		      = InputOnly;
    wa->bit_gravity	      = NorthWestGravity;
    wa->win_gravity	      = NorthWestGravity;
    wa->backing_store	      = NotUseful;
    wa->backing_planes	      = 0;
    wa->backing_pixel	      = 0;
    wa->save_under	      = false;
    wa->colormap	      = None;
    wa->map_installed	      = false;
    wa->map_state	      = IsUnviewable;
    wa->all_event_masks	      = 0;
    wa->your_event_mask	      = 0;
    wa->do_not_propagate_mask = 0;
    wa->override_redirect     = true;
    wa->screen		      = ScreenOfDisplay (dpy, screenNum);
}

void
CompScreen::handleCompizEvent (const char         *plugin,
			       const char         *event,
			       CompOption::Vector &options)
{
    WRAPABLE_HND_FUNCTN (handleCompizEvent, plugin, event, options);
    _handleCompizEvent (plugin, event, options);
}

void
CompScreenImpl::_handleCompizEvent (const char         *plugin,
			       const char         *event,
			       CompOption::Vector &options)
{
}

void
CompScreen::handleEvent (XEvent *event)
{
    WRAPABLE_HND_FUNCTN (handleEvent, event)
    _handleEvent (event);
}

void
CompScreenImpl::alwaysHandleEvent (XEvent *event)
{
    /*
     * Critical event handling that cannot be overridden by plugins
     */
    
    if (event->type == ButtonPress || event->type == KeyPress)
	privateScreen.eventManager.resetPossibleTap();

    eventHandled = true;  // if we return inside WRAPABLE_HND_FUNCTN

    handleEvent (event);

    bool keyEvent = (event->type == KeyPress || event->type == KeyRelease);

    /* Always either replay the keyboard or consume the key
     * event on keypresses */
    if (keyEvent)
    {
	int mode = eventHandled ? AsyncKeyboard : ReplayKeyboard;
	XAllowEvents (privateScreen.dpy, mode, event->xkey.time);
    }

    if (privateScreen.eventManager.grabsEmpty () && event->type == KeyPress)
    {
	XUngrabKeyboard (privateScreen.dpy, event->xkey.time);
    }
}

ServerGrabInterface *
CompScreenImpl::serverGrabInterface ()
{
    return static_cast <ServerGrabInterface *> (this);
}

void
CompScreenImpl::grabServer ()
{
    XGrabServer (privateScreen.dpy);
}

void
CompScreenImpl::syncServer ()
{
    XSync (privateScreen.dpy, false);
}

void
CompScreenImpl::ungrabServer ()
{
    XUngrabServer (privateScreen.dpy);
}

void
CompScreenImpl::_handleEvent (XEvent *event)
{
    /*
     * Non-critical event handling that might be overridden by plugins
     */

    CompWindow *w = NULL;
    XWindowAttributes wa;

    switch (event->type) {
    case ButtonPress:
	if (event->xbutton.root == privateScreen.rootWindow())
	    privateScreen.outputDevices.setCurrentOutput (
		outputDeviceForPoint (event->xbutton.x_root,
						 event->xbutton.y_root));
	break;
    case MotionNotify:
	if (event->xmotion.root == privateScreen.rootWindow())
	    privateScreen.outputDevices.setCurrentOutput (
		outputDeviceForPoint (event->xmotion.x_root,
				      event->xmotion.y_root));
	break;
    case KeyPress:
	w = findWindow (privateScreen.orphanData.activeWindow);
	if (w)
	    privateScreen.outputDevices.setCurrentOutput (w->outputDevice ());
	break;
    default:
	break;
    }

    eventHandled = privateScreen.handleActionEvent (event);
    if (eventHandled)
    {
	if (privateScreen.eventManager.grabsEmpty ())
	    XAllowEvents (privateScreen.dpy, AsyncPointer, event->xbutton.time);
	return;
    }

    switch (event->type) {
    case SelectionRequest:
	privateScreen.handleSelectionRequest (event);
	break;
    case SelectionClear:
	privateScreen.handleSelectionClear (event);
	break;
    case ConfigureNotify:
	w = findWindow (event->xconfigure.window);

	if (w && !w->priv->frame)
	{
	    w->priv->configure (&event->xconfigure);
	}
	else
	{
	    w = findTopLevelWindow (event->xconfigure.window);

	    if (w && w->priv->frame == event->xconfigure.window)
		w->priv->configureFrame (&event->xconfigure);
	    else
	    {
		if (event->xconfigure.window == privateScreen.rootWindow())
		    privateScreen.configure (&event->xconfigure);
	    }
	}
	break;
    case CreateNotify:
    {
	bool create = true;

	/* Failure means that window has been destroyed. We still have to add 
	 * the window to the window list as we might get configure requests
	 * which require us to stack other windows relative to it. Setting
	 * some default values if this is the case. */
	if (!XGetWindowAttributes (privateScreen.dpy, event->xcreatewindow.window, &wa))
	    privateScreen.setDefaultWindowAttributes (&wa);

	foreach (CompWindow *w, screen->windows ())
	{
	    if (w->priv->serverFrame == event->xcreatewindow.window)
	    {
		w->priv->frame = event->xcreatewindow.window;
		w->priv->updatePassiveButtonGrabs ();
		create = false;
	    }
	}

	foreach (CompWindow *w, destroyedWindows())
	{
	    if (w->priv->serverId == event->xcreatewindow.window)
	    {
		/* Previously destroyed window
		 * plugins were keeping around
		 * in order to avoid an xid conflict,
		 * destroy it right away and manage
		 * the new window */

		StackDebugger *dbg = StackDebugger::Default ();

		while (w->priv->destroyRefCnt)
		    w->destroy ();

		if (dbg)
		    dbg->removeDestroyedFrame (event->xcreatewindow.window);
	    }

	}

	if (wa.root != event->xcreatewindow.parent)
	    create = false;

	if (create)
	{
	    /* Track the window if it was created on this
	     * screen, otherwise we still need to register
	     * for FocusChangeMask. Also, we don't want to
	     * manage it straight away - in reality we want
	     * that to wait until the map request */
	    if (wa.root == privateScreen.rootWindow())
	    {
		PrivateWindow::createCompWindow (CompWindowToWindow (getTopWindow ()), CompWindowToWindow (getTopServerWindow ()), wa, event->xcreatewindow.window);
            }
	    else
		XSelectInput (privateScreen.dpy, event->xcreatewindow.window,
			      FocusChangeMask);
	}
	else
	    compLogMessage ("core", CompLogLevelDebug, "refusing to manage window 0x%x", (unsigned int) event->xcreatewindow.window);

	break;
    }
    case DestroyNotify:
	w = findWindow (event->xdestroywindow.window);

	/* It is possible that some plugin might call
	 * w->destroy () before the window actually receives
	 * its first DestroyNotify event which would mean
	 * that it is already in the list of destroyed
	 * windows, so check that list too */

	if (!w)
	{
	    foreach (CompWindow *dw, destroyedWindows())
	    {
		if (dw->priv->serverId == event->xdestroywindow.window)
		{
		    w = dw;
		    break;
		}
	    }
	}

	if (w)
	{
	    w->moveInputFocusToOtherWindow ();
	    w->destroy ();
	}
	break;
    case MapNotify:

	/* Search in already-created windows for this window */
	if (!w)
	    w = findWindow (event->xmap.window);

	if (w)
	{
	    if (w->priv->pendingMaps)
	    {
		/* The only case where this happens
		 * is where the window unmaps itself
		 * but doesn't get destroyed so when
		 * it re-maps we need to reparent it */

		if (!w->priv->serverFrame)
		    w->priv->reparent ();

		w->priv->managed = true;
	    }

	    /* been shaded */
	    if (w->shaded ())
	    {
		if (w->id () == privateScreen.orphanData.activeWindow)
		    w->moveInputFocusTo ();
	    }

	    w->map ();
	}

	break;
    case UnmapNotify:
	w = findWindow (event->xunmap.window);
	if (w)
	{
	    /* Normal -> Iconic */
	    if (w->pendingUnmaps ())
	    {
		setWmState (IconicState, w->id ());
		w->priv->pendingUnmaps--;
	    }
	    else /* X -> Withdrawn */
	    {
		/* Iconic -> Withdrawn:
		 *
		 * The window is already unmapped so we need to check the
		 * synthetic UnmapNotify that comes and withdraw the window here */
		if (w->state () & CompWindowStateHiddenMask)
		{
		    w->priv->minimized = false;
		    w->changeState (w->state () & ~CompWindowStateHiddenMask);

		    privateScreen.updateClientList ();
		    w->priv->withdraw ();
		}
		/* Closing:
		 *
		 * ICCCM Section 4.1.4 says that clients need to send
		 * a synthetic UnmapNotify for every real unmap
		 * in order to reflect the change in state, but
		 * since we already withdraw the window on the real
		 * UnmapNotify, no need to do it again on the synthetic
		 * one. */
		else if (!event->xunmap.send_event)
		{
		    w->windowNotify (CompWindowNotifyClose);
		    w->priv->withdraw ();
		}
	    }

	    if (!event->xunmap.send_event)
	    {
		w->unmap ();

		if (!w->shaded () && !w->priv->pendingMaps)
		    w->moveInputFocusToOtherWindow ();
	    }
	}
	break;
    case ReparentNotify:

	w = findWindow (event->xreparent.window);

	/* If this window isn't part of our tracked window
	 * list and was reparented into the root window then
	 * we need to track it */
	if (!w)
	{
	    if (event->xreparent.parent == privateScreen.rootWindow())
	    {
		/* Failure means that window has been destroyed. We still have to add 
		 * the window to the window list as we might get configure requests
		 * which require us to stack other windows relative to it. Setting
		 * some default values if this is the case. */
		if (!XGetWindowAttributes (privateScreen.dpy, event->xcreatewindow.window, &wa))
		    privateScreen.setDefaultWindowAttributes (&wa);

		PrivateWindow::createCompWindow (getTopWindow ()->id (), getTopServerWindow ()->id (), wa, event->xcreatewindow.window);
		break;
	    }
	    else
	    {
		/* It is possible that some plugin might call
		 * w->destroy () before the window actually receives
		 * its first ReparentNotify event which would mean
		 * that it is already in the list of destroyed
		 * windows, so check that list too */

		foreach (CompWindow *dw, destroyedWindows())
		{
		    if (dw->priv->serverId == event->xreparent.window)
		    {
			w = dw;
			break;
		    }
		}
	    }
	}

	/* This is the only case where a window is removed but not
	   destroyed. We must remove our event mask and all passive
	   grabs. */

        if (w)
	{
	    if ((w->priv->wrapper && event->xreparent.parent != w->priv->wrapper) ||
		(!w->priv->wrapper && event->xreparent.parent != privateScreen.rootWindow ()))
	    {
		w->moveInputFocusToOtherWindow ();
		w->destroy ();

		XSelectInput (privateScreen.dpy, w->id (), NoEventMask);
		XShapeSelectInput (privateScreen.dpy, w->id (), NoEventMask);
		XUngrabButton (privateScreen.dpy, AnyButton, AnyModifier, w->id ());
	    }
        }

	break;
    case CirculateNotify:
	w = findWindow (event->xcirculate.window);
	if (w)
	    w->priv->circulate (&event->xcirculate);
	break;
    case ButtonPress:
	if (event->xbutton.button == Button1 ||
	    event->xbutton.button == Button2 ||
	    event->xbutton.button == Button3)
	{
	    w = findTopLevelWindow (event->xbutton.window);
	    if (w)
	    {
		if (privateScreen.optionGetRaiseOnClick ())
		{
		    w->updateAttributes (CompStackingUpdateModeAboveFullscreen);
		}

	        if (w->id () != privateScreen.orphanData.activeWindow)
		    if (!(w->type () & CompWindowTypeDockMask))
			if (w->focus ())
			    w->moveInputFocusTo ();
	    }
	}

	if (privateScreen.eventManager.grabsEmpty ())
	    XAllowEvents (privateScreen.dpy, ReplayPointer, event->xbutton.time);

	break;
    case PropertyNotify:
	if (event->xproperty.atom == Atoms::winType)
	{
	    w = findWindow (event->xproperty.window);
	    if (w)
	    {
		unsigned int type;

		type = getWindowType (w->id ());

		if (type != w->wmType ())
		{
		    if (w->isViewable ())
		    {
			if (w->type () == CompWindowTypeDesktopMask)
			    decrementDesktopWindowCount();
			else if (type == CompWindowTypeDesktopMask)
			    incrementDesktopWindowCount();
		    }

		    w->wmType () = type;

		    w->recalcType ();
		    w->recalcActions ();

		    if (type & (CompWindowTypeDockMask |
				CompWindowTypeDesktopMask))
			w->setDesktop (0xffffffff);

		    privateScreen.updateClientList ();

		    matchPropertyChanged (w);
		}
	    }
	}
	else if (event->xproperty.atom == Atoms::winState)
	{
	    w = findWindow (event->xproperty.window);
	    if (w && !w->managed ())
	    {
		unsigned int state;

		state = getWindowState (w->id ());
		state = CompWindow::constrainWindowState (state, w->actions ());

		/* EWMH suggests that we ignore changes
		   to _NET_WM_STATE_HIDDEN */
		if (w->state () & CompWindowStateHiddenMask)
		    state |= CompWindowStateHiddenMask;
		else
		    state &= ~CompWindowStateHiddenMask;

		w->changeState (state);
	    }
	}
	else if (event->xproperty.atom == XA_WM_NORMAL_HINTS)
	{
	    w = findWindow (event->xproperty.window);
	    if (w)
	    {
		w->priv->updateNormalHints ();
		w->recalcActions ();
	    }
	}
	else if (event->xproperty.atom == XA_WM_HINTS)
	{
	    w = findWindow (event->xproperty.window);
	    if (w)
		w->priv->updateWmHints ();
	}
	else if (event->xproperty.atom == XA_WM_TRANSIENT_FOR)
	{
	    w = findWindow (event->xproperty.window);
	    if (w)
	    {
		w->priv->updateTransientHint ();
		w->recalcActions ();
	    }
	}
	else if (event->xproperty.atom == Atoms::wmClientLeader)
	{
	    w = findWindow (event->xproperty.window);
	    if (w)
		w->priv->clientLeader = w->priv->getClientLeader ();
	}
	else if (event->xproperty.atom == Atoms::wmIconGeometry)
	{
	    w = findWindow (event->xproperty.window);
	    if (w)
		w->priv->updateIconGeometry ();
	}
	else if (event->xproperty.atom == Atoms::wmStrut ||
		 event->xproperty.atom == Atoms::wmStrutPartial)
	{
	    w = findWindow (event->xproperty.window);
	    if (w)
	    {
		if (w->updateStruts ())
		    updateWorkarea ();
	    }
	}
	else if (event->xproperty.atom == Atoms::mwmHints)
	{
	    w = findWindow (event->xproperty.window);
	    if (w)
		w->priv->updateMwmHints ();
	}
	else if (event->xproperty.atom == Atoms::wmProtocols)
	{
	    w = findWindow (event->xproperty.window);
	    if (w)
		w->priv->protocols = getProtocols (w->id ());
	}
	else if (event->xproperty.atom == Atoms::wmIcon)
	{
	    w = findWindow (event->xproperty.window);
	    if (w)
		w->priv->freeIcons ();
	}
	else if (event->xproperty.atom == Atoms::startupId)
	{
	    w = findWindow (event->xproperty.window);
	    if (w)
		w->priv->updateStartupId ();
	}
	else if (event->xproperty.atom == XA_WM_CLASS)
	{
	    w = findWindow (event->xproperty.window);
	    if (w)
		w->priv->updateClassHints ();
	}
	break;
    case MotionNotify:
	break;
    case ClientMessage:
	if (event->xclient.message_type == Atoms::winActive)
	{
	    w = findTopLevelWindow (event->xclient.window);
	    if (w)
	    {
		/* use focus stealing prevention if request came
		   from an application  */
		if (event->xclient.data.l[0] != ClientTypeApplication ||
		    w->priv->allowWindowFocus (0, event->xclient.data.l[1]))
		{
		    w->activate ();
		}
	    }
	}
	else if (event->xclient.message_type == Atoms::winState)
	{
	    w = findWindow (event->xclient.window);
	    if (w)
	    {
		unsigned long wState, state;
		int	      i;

		wState = w->state ();

		for (i = 1; i < 3; i++)
		{
		    state = cps::windowStateMask (event->xclient.data.l[i]);
		    if (state & ~CompWindowStateHiddenMask)
		    {

static const unsigned short _NET_WM_STATE_REMOVE = 0;
static const unsigned short _NET_WM_STATE_ADD    = 1;
static const unsigned short _NET_WM_STATE_TOGGLE = 2;

			switch (event->xclient.data.l[0]) {
			case _NET_WM_STATE_REMOVE:
			    wState &= ~state;
			    break;
			case _NET_WM_STATE_ADD:
			    wState |= state;
			    break;
			case _NET_WM_STATE_TOGGLE:
			    wState ^= state;
			    break;
			}
		    }
		}

		wState = CompWindow::constrainWindowState (wState,
							   w->actions ());
		if (w->id () == privateScreen.orphanData.activeWindow)
		    wState &= ~CompWindowStateDemandsAttentionMask;

		if (wState != w->state ())
		{
		    CompStackingUpdateMode stackingUpdateMode;
		    unsigned long          dState = wState ^ w->state ();

		    stackingUpdateMode = CompStackingUpdateModeNone;

		    /* raise the window whenever its fullscreen state,
		       above/below state or maximization state changed */
		    if (dState & (CompWindowStateFullscreenMask |
				  CompWindowStateAboveMask |
				  CompWindowStateBelowMask |
				  CompWindowStateMaximizedHorzMask |
				  CompWindowStateMaximizedVertMask))
			stackingUpdateMode = CompStackingUpdateModeNormal;

		    w->changeState (wState);

		    w->updateAttributes (stackingUpdateMode);
		}
	    }
	}
	else if (event->xclient.message_type == Atoms::wmProtocols)
	{
	    if ((unsigned long) event->xclient.data.l[0] == Atoms::wmPing)
	    {
		w = findWindow (event->xclient.data.l[2]);
		if (w)
		    w->priv->handlePing (lastPing ());
	    }
	}
	else if (event->xclient.message_type == Atoms::closeWindow)
	{
	    w = findWindow (event->xclient.window);
	    if (w)
		w->close (event->xclient.data.l[0]);
	}
	else if (event->xclient.message_type == Atoms::desktopGeometry)
	{
	    if (event->xclient.window == privateScreen.rootWindow())
	    {
		CompOption::Value value;

		value.set ((int) (event->xclient.data.l[0] /
			   width ()));

		setOptionForPlugin ("core", "hsize", value);

		value.set ((int) (event->xclient.data.l[1] /
			   height ()));

		setOptionForPlugin ("core", "vsize", value);
	    }
	}
	else if (event->xclient.message_type == Atoms::moveResizeWindow)
	{
	    w = findWindow (event->xclient.window);
	    if (w)
	    {
		unsigned int   xwcm = 0;
		XWindowChanges xwc;
		int            gravity;
		int	       value_mask;
		unsigned int   source;

		gravity = (event->xclient.data.l[0] & 0xFF);		
		value_mask = (event->xclient.data.l[0] & 0xF00) >> 8;
		source = (event->xclient.data.l[0] & 0xF000) >> 12;

		memset (&xwc, 0, sizeof (xwc));

		if (value_mask & CWX)
		{
		    xwcm |= CWX;
		    xwc.x = event->xclient.data.l[1];
		}

		if (value_mask & CWY)
		{
		    xwcm |= CWY;
		    xwc.y = event->xclient.data.l[2];
		}

		if (value_mask & CWWidth)
		{
		    xwcm |= CWWidth;
		    xwc.width = event->xclient.data.l[3];
		}

		if (value_mask & CWHeight)
		{
		    xwcm |= CWHeight;
		    xwc.height = event->xclient.data.l[4];
		}

		w->moveResize (&xwc, xwcm, gravity, source);
	    }
	}
	else if (event->xclient.message_type == Atoms::restackWindow)
	{
	    w = findWindow (event->xclient.window);
	    if (w)
	    {
		/* TODO: other stack modes than Above and Below */
		if (event->xclient.data.l[1])
		{
		    CompWindow *sibling;

		    sibling = findWindow (event->xclient.data.l[1]);
		    if (sibling)
		    {
			if (event->xclient.data.l[2] == Above)
			    w->restackAbove (sibling);
			else if (event->xclient.data.l[2] == Below)
			    w->restackBelow (sibling);
		    }
		}
		else
		{
		    if (event->xclient.data.l[2] == Above)
			w->raise ();
		    else if (event->xclient.data.l[2] == Below)
			w->lower ();
		}
	    }
	}
	else if (event->xclient.message_type == Atoms::wmChangeState)
	{
	    w = findWindow (event->xclient.window);
	    if (w)
	    {
		if (event->xclient.data.l[0] == IconicState)
		{
		    if (w->actions () & CompWindowActionMinimizeMask)
			w->minimize ();
		}
		else if (event->xclient.data.l[0] == NormalState)
		{
		    w->unminimize ();
		}
	    }
	}
	else if (event->xclient.message_type == Atoms::showingDesktop)
	{
	    if (event->xclient.window == privateScreen.rootWindow() ||
		event->xclient.window == None)
	    {
		if (event->xclient.data.l[0])
		    enterShowDesktopMode ();
		else
		    leaveShowDesktopMode (NULL);
	    }
	}
	else if (event->xclient.message_type == Atoms::numberOfDesktops)
	{
	    if (event->xclient.window == privateScreen.rootWindow())
	    {
		CompOption::Value value;

		value.set ((int) event->xclient.data.l[0]);

		setOptionForPlugin ("core", "number_of_desktops", value);
	    }
	}
	else if (event->xclient.message_type == Atoms::currentDesktop)
	{
	    if (event->xclient.window == privateScreen.rootWindow())
		privateScreen.setCurrentDesktop (event->xclient.data.l[0]);
	}
	else if (event->xclient.message_type == Atoms::winDesktop)
	{
	    w = findWindow (event->xclient.window);
	    if (w)
		w->setDesktop (event->xclient.data.l[0]);
	}
	else if (event->xclient.message_type == Atoms::wmFullscreenMonitors)
	{
	    w = findWindow (event->xclient.window);
	    if (w)
	    {
		CompFullscreenMonitorSet monitors;

		monitors.top    = event->xclient.data.l[0];
		monitors.bottom = event->xclient.data.l[1];
		monitors.left   = event->xclient.data.l[2];
		monitors.right  = event->xclient.data.l[3];

		w->priv->setFullscreenMonitors (&monitors);
	    }
	}
	break;
    case MappingNotify:
	modHandler->updateModifierMappings ();
	break;
    case MapRequest:
	w = screen->findWindow (event->xmaprequest.window);

	if (w)
	{
	    XWindowAttributes attr;
	    bool              doMapProcessing = true;

	    /* We should check the override_redirect flag here, because the
	       client might have changed it while being unmapped. */
	    if (XGetWindowAttributes (privateScreen.dpy, w->id (), &attr))
		w->priv->setOverrideRedirect (attr.override_redirect != 0);

	    if (w->state () & CompWindowStateHiddenMask)
		if (!w->minimized () && !w->inShowDesktopMode ())
		    doMapProcessing = false;

	    if (doMapProcessing)
		w->priv->processMap ();

	    w->priv->managed = true;

	    setWindowProp (w->id (), Atoms::winDesktop, w->desktop ());
	}
	else
	{
	    XMapWindow (privateScreen.dpy, event->xmaprequest.window);
	}
	break;
    case ConfigureRequest:
	w = findWindow (event->xconfigurerequest.window);
	if (w && w->managed ())
	{
	    XWindowChanges xwc;

	    memset (&xwc, 0, sizeof (xwc));

	    xwc.x	     = event->xconfigurerequest.x;
	    xwc.y	     = event->xconfigurerequest.y;
	    xwc.width	     = event->xconfigurerequest.width;
	    xwc.height       = event->xconfigurerequest.height;
	    xwc.border_width = event->xconfigurerequest.border_width;

	    w->moveResize (&xwc, event->xconfigurerequest.value_mask,
			   0, ClientTypeUnknown);

	    if (event->xconfigurerequest.value_mask & CWStackMode)
	    {
		Window     above    = None;
		CompWindow *sibling = NULL;

		if (event->xconfigurerequest.value_mask & CWSibling)
		{
		    above   = event->xconfigurerequest.above;
		    sibling = findTopLevelWindow (above);
		}

		switch (event->xconfigurerequest.detail) {
		case Above:
		    if (w->priv->allowWindowFocus (NO_FOCUS_MASK, 0))
		    {
			if (above)
			{
			    if (sibling)
				w->restackAbove (sibling);
			}
			else
			    w->raise ();
		    }
		    break;
		case Below:
		    if (above)
		    {
			if (sibling)
			    w->restackBelow (sibling);
		    }
		    else
			w->lower ();
		    break;
		default:
		    /* no handling of the TopIf, BottomIf, Opposite cases -
		       there will hardly be any client using that */
		    break;
		}
	    }
	}
	else
	{
	    XWindowChanges xwc;
	    unsigned int   xwcm;

	    xwcm = event->xconfigurerequest.value_mask &
		(CWX | CWY | CWWidth | CWHeight | CWBorderWidth);

	    xwc.x	     = event->xconfigurerequest.x;
	    xwc.y	     = event->xconfigurerequest.y;
	    xwc.width	     = event->xconfigurerequest.width;
	    xwc.height	     = event->xconfigurerequest.height;
	    xwc.border_width = event->xconfigurerequest.border_width;

	    if (w)
	    {
		/* Any window that receives a ConfigureRequest
		 * is not override redirect, and may have changed
		 * to being not override redirect */
		w->priv->setOverrideRedirect (false);
		w->configureXWindow (xwcm, &xwc);
	    }
	    else
		XConfigureWindow (privateScreen.dpy, event->xconfigurerequest.window,
				  xwcm, &xwc);
	}
	break;
    case CirculateRequest:
	break;
    case FocusIn:
    {
	if (!XGetWindowAttributes (privateScreen.dpy, event->xfocus.window, &wa))
	    privateScreen.setDefaultWindowAttributes (&wa);

        /* If the call to XGetWindowAttributes failed it means
         * the window was destroyed, so track the focus change
         * anyways since we need to increment activeNum
         * and the passive button grabs and then we will
         * get the DestroyNotify later and change the focus
         * there
	 */

	if (wa.root == privateScreen.rootWindow())
	{
	    if (event->xfocus.mode == NotifyGrab)
		privateScreen.eventManager.grabNotified ();
	    else if (event->xfocus.mode == NotifyUngrab)
		privateScreen.eventManager.ungrabNotified ();
	    else
	    {
		CompWindowList dockWindows;
		XWindowChanges xwc;
		unsigned int   mask;

		w = findTopLevelWindow (event->xfocus.window);
		if (w && w->managed ())
		{
		    unsigned int state = w->state ();

		    if (getNextActiveWindow() == event->xfocus.window)
			setNextActiveWindow(None);

		    if (w->id () != privateScreen.orphanData.activeWindow)
		    {
			CompWindow     *active = screen->findWindow (privateScreen.orphanData.activeWindow);

			privateScreen.orphanData.activeWindow = w->id ();
			w->priv->activeNum = nextActiveNum();

			if (active)
			{
			    CompWindowList windowsLostFocus;
			    /* If this window lost focus and was above a fullscreen window
			     * and is no longer capable of being focused (eg, it is
			     * not visible on this viewport) then we need to check if
			     * any other windows below it are also now no longer capable
			     * of being focused and restack them in the highest position
			     * below docks that they are allowed to take */
			    if (!active->focus ())
			    {
				windowsLostFocus.push_back (active);
				for (CompWindow *fsw = active->prev; fsw; fsw = fsw->prev)
				{
				    if (!fsw->focus () &&
					fsw->managed () &&
					!(fsw->type () & (CompWindowTypeDockMask |
							  CompWindowTypeFullscreenMask)) &&
					!fsw->overrideRedirect ())
					windowsLostFocus.push_back (fsw);

				    if (fsw->type () & CompWindowTypeFullscreenMask)
				    {
					/* This will be the window that we must lower relative to */
					CompWindow *sibling = PrivateWindow::findValidStackSiblingBelow (active, fsw);

					if (sibling)
					{
					    for (CompWindowList::reverse_iterator rit = windowsLostFocus.rbegin ();
						 rit != windowsLostFocus.rend (); ++rit)
					    {
						(*rit)->restackAbove (sibling);
					    }
					}

					break;
				    }
				}
			    }

			    active->changeState (active->focused () ?
						 active->state () | CompWindowStateFocusedMask :
						 active->state () & ~CompWindowStateFocusedMask);

			    active->priv->updatePassiveButtonGrabs ();
			}

			if (w->focused ())
			    state |= w->state () | CompWindowStateFocusedMask;
			else
			    state &= w->state () & ~CompWindowStateFocusedMask;

			w->priv->updatePassiveButtonGrabs ();

			addToCurrentActiveWindowHistory (w->id ());

			XChangeProperty (privateScreen.dpy , privateScreen.rootWindow(),
					 Atoms::winActive,
					 XA_WINDOW, 32, PropModeReplace,
					 (unsigned char *) &privateScreen.orphanData.activeWindow, 1);

			w->windowNotify (CompWindowNotifyFocusChange);
		    }

		    state &= ~CompWindowStateDemandsAttentionMask;
		    w->changeState (state);
	        }
		else if (event->xfocus.window == privateScreen.rootWindow())
		{
		    /* Don't ever let the focus go to the root
		     * window except in grab cases
		     *
		     * FIXME: There might be a case where we have to
		     * handle root windows of other screens here, but
		     * the other window managers should handle that
		     */

		    if (event->xfocus.detail == NotifyDetailNone ||
			(event->xfocus.mode == NotifyNormal &&
			 event->xfocus.detail == NotifyInferior))
		    {
			privateScreen.orphanData.activeWindow = None;

			if (event->xfocus.detail == NotifyDetailNone ||
			    (event->xfocus.mode == NotifyNormal &&
			     event->xfocus.detail == NotifyInferior))
			{
			    screen->focusDefaultWindow ();
			}
		    }
		}

		/* Ensure that docks are stacked in the right place
		 *
		 * When a normal window gets the focus and is above a
		 * fullscreen window, restack the docks to be above
		 * the highest level mapped and visible normal window,
		 * otherwise put them above the highest fullscreen window
		 */
		if (w)
		{
		    if (PrivateWindow::stackDocks (w, dockWindows, &xwc, &mask))
		    {
			Window sibling = xwc.sibling;
			xwc.stack_mode = Above;

			/* Then update the dock windows */
			foreach (CompWindow *dw, dockWindows)
			{
			    xwc.sibling = sibling;
			    dw->configureXWindow (mask, &xwc);
			}
		    }
		}

	    }
	}
	else
	{
	    CompWindow *w;

	    w = screen->findWindow (privateScreen.orphanData.activeWindow);

	    setNextActiveWindow(None);
	    privateScreen.orphanData.activeWindow = None;

	    if (w)
		w->priv->updatePassiveButtonGrabs ();
	}
    }
    break;
    case FocusOut:
	if (event->xfocus.mode == NotifyUngrab)
	    privateScreen.eventManager.ungrabNotified ();
	break;
    case EnterNotify:
	if (event->xcrossing.root == privateScreen.rootWindow())
	    w = findTopLevelWindow (event->xcrossing.window);
	else
	    w = NULL;

	if (w && w->id () != below)
	{
	    below = w->id ();

	    if (!privateScreen.optionGetClickToFocus () &&
		privateScreen.eventManager.grabsEmpty () &&
		event->xcrossing.mode   != NotifyGrab                &&
		event->xcrossing.detail != NotifyInferior)
	    {
		bool raise;
		int  delay;

		raise = privateScreen.optionGetAutoraise ();
		delay = privateScreen.optionGetAutoraiseDelay ();

		if (autoRaiseTimer_.active () &&
		    autoRaiseWindow_ != w->id ())
		{
		    autoRaiseTimer_.stop ();
		}

		if (w->type () & ~(CompWindowTypeDockMask |
				   CompWindowTypeDesktopMask))
		{
		    w->moveInputFocusTo ();

		    if (raise)
		    {
			if (delay > 0)
			{
			    autoRaiseWindow_ = w->id ();
			    autoRaiseTimer_.start (
				boost::bind (autoRaiseTimeout, this),
				delay, (unsigned int) ((float) delay * 1.2));
			}
			else
			{
			    CompStackingUpdateMode mode =
				CompStackingUpdateModeNormal;

			    w->updateAttributes (mode);
			}
		    }
		}
	    }
	}
	break;
    case LeaveNotify:
	if (event->xcrossing.detail != NotifyInferior)
	{
	    if (event->xcrossing.window == below)
		below = None;
	}
	break;
    default:
	if (privateScreen.xShape.isEnabled () &&
		 event->type == privateScreen.xShape.get () + ShapeNotify)
	{
	    w = findWindow (((XShapeEvent *) event)->window);
	    if (w)
	    {
		if (w->mapNum ())
		    w->priv->updateRegion ();
	    }
	}
	else if (event->type == privateScreen.xSync.get () + XSyncAlarmNotify)
	{
	    XSyncAlarmNotifyEvent *sa;

	    sa = (XSyncAlarmNotifyEvent *) event;


	    for (cps::WindowManager::iterator i = windowManager.begin(); i != windowManager.end(); ++i)
	    {
		CompWindow* const w(*i);
		if (w->priv->syncAlarm == sa->alarm)
		{
		    w->priv->handleSyncAlarm ();
		    break;
		}
	    }
	}
	break;
    }
}
