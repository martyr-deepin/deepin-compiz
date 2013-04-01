/*
 * Compiz Fusion Grid plugin
 *
 * Copyright (c) 2008 Stephen Kennedy <suasol@gmail.com>
 * Copyright (c) 2010 Scott Moreau <oreaus@gmail.com>
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
 * Description:
 *
 * Plugin to act like winsplit revolution (http://www.winsplit-revolution.com/)
 * use <Control><Alt>NUMPAD_KEY to move and tile your windows.
 *
 * Press the tiling keys several times to cycle through some tiling options.
 */

#include "grabhandler.h"

compiz::grid::window::GrabWindowHandler::GrabWindowHandler (unsigned int         mask,
							    const GrabActiveFunc &grabActive) :
    mMask (mask),
    mGrabActive (grabActive)
{
}

compiz::grid::window::GrabWindowHandler::~GrabWindowHandler ()
{
}

bool
compiz::grid::window::GrabWindowHandler::track ()
{
    if (mGrabActive ("expo"))
	return false;

    return ((mMask & (CompWindowGrabMoveMask | CompWindowGrabButtonMask)) &&
	    !(mMask & CompWindowGrabResizeMask));
}

bool
compiz::grid::window::GrabWindowHandler::resetResize ()
{
    return (mMask & CompWindowGrabResizeMask);
}
