/*
 * Compiz vpswitch plugin
 *
 * vpswitch.cpp
 *
 * Copyright (c) 2007 Dennis Kasprzyk <onestone@opencompositing.org>
 *
 * Go-to-numbered-viewport functionality by
 * 
 * Copyright (c) 2007 Robert Carr <racarr@opencompositing.org>
 *           (c) 2007 Danny Baumann <maniac@opencompositing.org>
 *
 * Go-to-specific-viewport functionality by
 *
 * Copyright (c) 2007 Michael Vogt <mvo@ubuntu.com>
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

#include "vpswitch.h"

COMPIZ_PLUGIN_20090315 (vpswitch, VPSwitchPluginVTable);

bool
VPSwitchScreen::initPluginAction (CompAction         *action,
		  		  CompAction::State  state,
		  		  CompOption::Vector &options)
{
    GET_DATA;

    CompPlugin *plugin = CompPlugin::find (optionGetInitPlugin ().c_str ());
    bool       rv = false;

    if (!plugin)
	return false;

    foreach (CompOption &opt, plugin->vTable->getOptions ())
    {
	if (opt.type () == CompOption::TypeAction ||
	    opt.type () == CompOption::TypeKey ||
	    opt.type () == CompOption::TypeButton ||
	    opt.type () == CompOption::TypeEdge ||
	    opt.type () == CompOption::TypeBell)
	    if (opt.name () == optionGetInitAction () &&
		opt.value ().action ().initiate ())
	    {
		rv = opt.value ().action ().initiate ()
		     (action, state, options);
		break;
	    }
    }

    if (rv)
	action->setState(action->state () | CompAction::StateTermButton);

    return rv;
}

bool
VPSwitchScreen::termPluginAction (CompAction         *action,
		  		  CompAction::State  state,
		  		  CompOption::Vector &options)
{
    CompPlugin *plugin = CompPlugin::find (optionGetInitPlugin ().c_str ());
    bool       rv = false;

    if (!plugin)
	return false;

    foreach (CompOption &opt, plugin->vTable->getOptions ())
    {
	if (opt.type () == CompOption::TypeAction ||
	    opt.type () == CompOption::TypeKey ||
	    opt.type () == CompOption::TypeButton ||
	    opt.type () == CompOption::TypeEdge ||
	    opt.type () == CompOption::TypeBell)
	    if (opt.name () == optionGetInitAction () &&
		opt.value ().action ().terminate ())
	    {
		rv = opt.value ().action ().terminate ()
		     (action, state, options);
		break;
	    }
    }

    return rv;
}

void
VPSwitchScreen::gotovp (int x, int y)
{
    XEvent xev;

    xev.xclient.type    = ClientMessage;
    xev.xclient.display = screen->dpy ();
    xev.xclient.format  = 32;

    xev.xclient.message_type = Atoms::desktopViewport;
    xev.xclient.window       = screen->root ();

    xev.xclient.data.l[0] = x * screen->width ();
    xev.xclient.data.l[1] = y * screen->height ();
    xev.xclient.data.l[2] = 0;
    xev.xclient.data.l[3] = 0;
    xev.xclient.data.l[4] = 0;

    XSendEvent (screen->dpy (), screen->root (), false,
		SubstructureRedirectMask | SubstructureNotifyMask, &xev);
}

/* desktop mouse button initiated actions */

bool
VPSwitchScreen::next (CompAction         *action,
      		      CompAction::State  state,
      		      CompOption::Vector &options)
{
    int targetX, targetY;
    CompPoint vp (screen->vp ());
    CompSize vpsize (screen->vpSize ());

    GET_DATA;

    targetX = vp.x () + 1;
    targetY = vp.y ();

    if (targetX >= vpsize.width ())
    {
	targetX = 0;
	targetY++;
    }
    if (targetY >= vpsize.height ())
	targetY = 0;

    gotovp (targetX, targetY);

    return true;
}

bool
VPSwitchScreen::prev (CompAction         *action,
      		      CompAction::State  state,
      		      CompOption::Vector &options)
{
    int targetX, targetY;
    CompPoint vp (screen->vp ());
    CompSize vpsize (screen->vpSize ());

    GET_DATA;

    targetX = vp.x () - 1;
    targetY = vp.y ();

    if (targetX < 0)
    {
	targetX = vpsize.width () - 1;
	targetY--;
    }
    if (targetY < 0)
	targetY = vpsize.height () - 1;

    gotovp (targetX, targetY);

    return true;
}

bool
VPSwitchScreen::movevp (CompAction	   *action,
			CompAction::State  state,
			CompOption::Vector &options,
			unsigned int dx, unsigned int dy)
{
    CompPoint vp (screen->vp ());
    CompSize  vpsize (screen->vpSize ());

    GET_DATA;

    /* Check bounds */

    if (vp.x () + dx > (unsigned int) vpsize.width ())
	return false;

    if (vp.y () + dy > (unsigned int) vpsize.height ())
	return false;

    gotovp (vp.x () + dx, vp.y () + dy);

    return true;
}

/* Handle viewport number switch key events */
void
VPSwitchScreen::handleEvent (XEvent *event)
{
    switch (event->type)
    {
    case KeyPress:
	KeySym       pressedKeySym;
	unsigned int mods;
	int          i, row;

	if (!numberedActive)
	    break;

	pressedKeySym = XLookupKeysym (&event->xkey, 0);
	mods = modHandler->keycodeToModifiers (event->xkey.keycode);
	if (mods & CompNumLockMask)
	    row = 1; /* use first row of lookup table */
	else
	    row = 2;

	for (i = 0; i < 10; i++)
	{
	    /* first try to handle normal number keys */
	    if (numberKeySyms[0][i] == pressedKeySym)
	    {
		destination *= 10;
		destination += i;
		break;
	    }
	    else
	    {
		if (numberKeySyms[row][i] == pressedKeySym)
		{
		    destination *= 10;
		    destination += i;
		    break;
		}
	    }
	}
    }

    screen->handleEvent (event);
}


bool
VPSwitchScreen::initiateNumbered (CompAction         *action,
				  CompAction::State  state,
				  CompOption::Vector &options)
{

    numberedActive = true;

    if (state & CompAction::StateInitKey)
	action->setState (action->state () | CompAction::StateTermKey);

    return true;
}

bool
VPSwitchScreen::terminateNumbered (CompAction         *action,
				   CompAction::State  state,
				   CompOption::Vector &options)
{
    int        nx, ny;
    CompSize   vpsize (screen->vpSize ());

    if (!numberedActive)
	return false;

    numberedActive = false;

    if (destination < 1 ||
	destination > (int) (vpsize.width () * vpsize.height ()))
	return false;

    nx = (destination - 1 ) % vpsize.width ();
    ny = (destination - 1 ) / vpsize.width ();

    gotovp (nx, ny);

    return true;
}

/* switch-to-specific viewport stuff */

bool
VPSwitchScreen::switchto (CompAction         *action,
	  		  CompAction::State  state,
	  		  CompOption::Vector &options,
	  		  int num)
{

    destination = num;

    numberedActive = true;
    return terminateNumbered (action, state, options);
}

VPSwitchScreen::VPSwitchScreen (CompScreen *screen):
    PluginClassHandler <VPSwitchScreen, CompScreen> (screen),
    destination (0),
    numberedActive (false)
{
    ScreenInterface::setHandler (screen);

#define directionBind(name, dx, dy)					       \
    optionSet##name##ButtonInitiate (boost::bind (&VPSwitchScreen::movevp,     \
						  this, _1, _2, _3, dx, dy))

    directionBind (Left, -1, 0);
    directionBind (Right, 1, 0);
    directionBind (Up, 0, -1);
    directionBind (Down, 0, 1);

#undef directionBind

#define numberedBind(num)						      \
    optionSetSwitchTo##num##KeyInitiate (boost::bind (&VPSwitchScreen::switchto, \
						      this, _1, _2, _3, num))

    /* Note: get actions in multi-lists, this is ugly */

    numberedBind (1);
    numberedBind (2);
    numberedBind (3);
    numberedBind (4);
    numberedBind (5);
    numberedBind (6);
    numberedBind (7);
    numberedBind (8);
    numberedBind (9);
    numberedBind (10);

#undef numberedBind

    optionSetBeginKeyInitiate (boost::bind(&VPSwitchScreen::initiateNumbered,
					   this, _1, _2, _3));
    optionSetBeginKeyTerminate (boost::bind (&VPSwitchScreen::terminateNumbered,
					      this, _1, _2, _3));

    optionSetNextButtonInitiate (boost::bind
					(&VPSwitchScreen::next,
					 this, _1, _2, _3));
    optionSetPrevButtonInitiate (boost::bind 
					(&VPSwitchScreen::prev,
					 this, _1, _2, _3));
    optionSetInitiateButtonInitiate (boost::bind 
					(&VPSwitchScreen::initPluginAction,
					 this, _1, _2, _3));
    optionSetInitiateButtonTerminate (boost::bind
					(&VPSwitchScreen::termPluginAction,
					 this, _1, _2, _3));
}

bool
VPSwitchPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    return true;
}
