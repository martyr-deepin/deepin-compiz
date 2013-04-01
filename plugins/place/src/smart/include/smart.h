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

#ifndef _COMPIZ_PLACE_SMART_H
#define _COMPIZ_PLACE_SMART_H

#include <vector>
#include <core/rect.h>
#include <core/windowgeometry.h>
#include <core/windowextents.h>
#include <core/size.h>
#include <core/point.h>

namespace compiz
{
    namespace place
    {

	extern const unsigned int WindowAbove;
	extern const unsigned int WindowBelow;

	class Placeable
	{
	    public:

		typedef std::vector <Placeable *> Vector;

		const compiz::window::Geometry & geometry () const { return getGeometry (); }
		const compiz::window::extents::Extents & extents () const { return getExtents (); }
		const CompRect & workArea () const { return getWorkarea (); }

		unsigned int state () const { return getState (); }

		virtual ~Placeable () = 0;

	    protected:

		virtual const compiz::window::Geometry & getGeometry () const = 0;
		virtual const compiz::window::extents::Extents & getExtents () const = 0;
		virtual const CompRect & getWorkarea () const = 0;

		virtual unsigned int getState () const = 0;

		Placeable ();
	};

	void smart (Placeable                      *placeable,
		    CompPoint			   &pos,
		    const compiz::place::Placeable::Vector &placeables);

    }
}

#endif
