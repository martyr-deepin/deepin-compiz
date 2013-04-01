/*
 * Copyright (C) 2001 Havoc Pennington
 * Copyright (C) 2002, 2003 Red Hat, Inc.
 * Copyright (C) 2003 Rob Adams
 * Copyright (C) 2005 Novell, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef _COMPIZ_PLACE_SCREEN_SIZE_CHANGE_H
#define _COMPIZ_PLACE_SCREEN_SIZE_CHANGE_H

#include <core/rect.h>
#include <core/windowgeometry.h>
#include <core/windowgeometrysaver.h>
#include <core/windowextents.h>
#include <core/size.h>
#include <core/point.h>

#include "constrain-to-workarea.h"

namespace compiz
{
namespace place
{

class ScreenSizeChangeObject
{
    public:

	ScreenSizeChangeObject (const compiz::window::Geometry &g);
	virtual ~ScreenSizeChangeObject ();

	virtual const compiz::window::Geometry & getGeometry () const = 0;
	virtual void applyGeometry (compiz::window::Geometry &ng,
				    compiz::window::Geometry &og) = 0;
	virtual const CompPoint & getViewport () const = 0;
	virtual const CompRect & getWorkarea (const compiz::window::Geometry &g) const = 0;
	virtual const compiz::window::extents::Extents & getExtents () const = 0;

	compiz::window::Geometry adjustForSize (const CompSize &oldSize,
						const CompSize &newSize);

	void unset ();

    private:

	compiz::window::GeometrySaver mSaver;
};

}
}

#endif
