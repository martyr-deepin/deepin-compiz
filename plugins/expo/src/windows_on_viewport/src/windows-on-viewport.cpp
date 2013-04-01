/**
 * Copyright © 2012 Canonical Ltd.
 *
 * Authors:
 * Sam Spilsbury <sam.spilsbury@canonical.com>
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
#include <cstdio>
#include <algorithm>
#include "windows-on-viewport.h"
#include "viewport-member-window.h"

namespace compiz
{
    namespace expo
    {
	unsigned int countViewports (const CompSize &vpSize)
	{
	    return vpSize.width () * vpSize.height ();
	}

	void fillInNewViewportActiveData (unsigned int vpCount,
					  std::vector <bool> &vpActive)
	{
	    if (vpActive.size () < vpCount)
	    {
		unsigned int last = vpActive.size () - 1;
		vpActive.resize (vpCount);
		for (unsigned int i = last; i < vpActive.size (); i++)
		    vpActive[i] = false;
	    }
	}

	void activeViewportsForMembers (compiz::expo::ClientListGenerator &clientList,
					const CompPoint                   &cursor,
					const CompSize			  &vpSize,
					const CompSize			  &screenSize,
					std::vector <bool>		  &viewportActiveStates)
	{
	    compiz::expo::ViewportMemberWindow *vpMemberWindow = clientList.nextClient ();

	    fillInNewViewportActiveData (countViewports (vpSize), viewportActiveStates);
	    std::fill_n (viewportActiveStates.begin (), viewportActiveStates.size (), false);

	    while (vpMemberWindow)
	    {
		if (!vpMemberWindow->isDesktopOrDock ())
		{
		    CompPoint viewport;

		    /* If this is a dragged window, use the cursor position */
		    if (vpMemberWindow->dragged ())
			viewport.set (cursor.x () / screenSize.width (),
				      cursor.y () / screenSize.height ());
		    else
		    {
			const compiz::window::Geometry &geom = vpMemberWindow->absoluteGeometry ();
			viewport.set (geom.centerX () / screenSize.width (),
				      geom.centerY () / screenSize.height ());
		    }

		    unsigned int vpIndex = vpSize.width () * viewport.y () + viewport.x ();
		    viewportActiveStates[vpIndex] = true;
		}

		vpMemberWindow = clientList.nextClient ();
	    }
	}
    }
}
