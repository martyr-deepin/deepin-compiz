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

#ifndef _COMPIZ_PLACE_CLAMP_TO_WORKAREA_H
#define _COMPIZ_PLACE_CLAMP_TO_WORKAREA_H

#include <core/rect.h>
#include <core/windowgeometry.h>
#include <core/windowextents.h>
#include <core/size.h>
#include <core/point.h>

namespace compiz
{
namespace place
{
extern unsigned int clampGeometrySizeOnly;
extern unsigned int clampGeometryToViewport;

void clampGeometryToWorkArea (compiz::window::Geometry &g,
			      const CompRect &workArea,
			      const CompWindowExtents &border,
			      unsigned int flags,
			      const CompSize &screenSize);
}
}

#endif
