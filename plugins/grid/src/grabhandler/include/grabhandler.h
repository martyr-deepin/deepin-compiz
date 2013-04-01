/*
 * Compiz Fusion Grid plugin, GrabHandler class
 *
 * Copyright (c) 2011 Canonical Ltd.
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
 *
 * Authored By:
 * Sam Spilsbury <sam.spilsbury@canonical.com>
 */
#ifndef _COMPIZ_GRID_GRABHANDLER_H
#define _COMPIZ_GRID_GRABHANDLER_H

#include <boost/function.hpp>

#define CompWindowGrabKeyMask         (1 << 0)
#define CompWindowGrabButtonMask      (1 << 1)
#define CompWindowGrabMoveMask        (1 << 2)
#define CompWindowGrabResizeMask      (1 << 3)
#define CompWindowGrabExternalAppMask (1 << 4)

namespace compiz
{
namespace grid
{
namespace window
{

typedef boost::function <bool (const char *)> GrabActiveFunc;

class GrabWindowHandler
{
    public:

	GrabWindowHandler (unsigned int         mask,
			   const GrabActiveFunc &grabActive);
	~GrabWindowHandler ();

    	bool track ();
	bool resetResize ();

    private:

	unsigned int         mMask;
	GrabActiveFunc       mGrabActive;

};
}
}
}

#endif
