/*
 * Compiz configuration system library
 *
 * Copyright (C) 2012 Canonical Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Authored By:
 * Sam Spilsbury <sam.spilsbury@canonical.com>
 */
#ifndef _COMPIZ_GLIB_GSLICE_OFF_ENV_H
#define _COMPIZ_GLIB_GSLICE_OFF_ENV_H

#include <glib.h>

class CompizGLibGSliceOffEnv
{
    public:

	virtual void SetUpEnv ()
	{
	    g_setenv ("G_SLICE", "always-malloc", 1);
	}

	virtual void TearDownEnv ()
	{
	    g_unsetenv ("G_SLICE");
	}
};

#endif
