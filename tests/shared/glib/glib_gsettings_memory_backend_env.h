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
#ifndef _COMPIZ_GLIB_GSETTINGS_MEMORY_BACKEND_TEST_ENV_H
#define _COMPIZ_GLIB_GSETTINGS_MEMORY_BACKEND_TEST_ENV_H

#include <glib.h>

class CompizGLibGSettingsMemoryBackendTestingEnv
{
    public:

	virtual void SetUpEnv (const std::string &MOCK_PATH)
	{
	    g_setenv ("GSETTINGS_SCHEMA_DIR", MOCK_PATH.c_str (), true);
	    g_setenv ("GSETTINGS_BACKEND", "memory", 1);
	}

	virtual void TearDownEnv ()
	{
	    g_unsetenv ("GSETTINGS_BACKEND");
	    g_unsetenv ("GSETTINGS_SCHEMA_DIR");
	}
};

#endif
