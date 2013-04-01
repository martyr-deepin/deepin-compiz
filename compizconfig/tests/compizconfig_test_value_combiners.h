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
#ifndef _COMPIZCONFIG_TEST_VALUE_COMBINERS_H
#define _COMPIZCONFIG_TEST_VALUE_COMBINERS_H

#include <X11/keysym.h>
#include <ccs.h>
#include <gtest_shared_characterwrapper.h>

namespace compizconfig
{
    namespace test
    {
	namespace impl
	{
	    extern Bool boolValues[];
	    extern int intValues[];
	    extern float floatValues[];
	    extern const char * stringValues[];
	    extern const char * matchValues[];

	    extern CCSSettingKeyValue keyValue;
	    extern CCSSettingButtonValue buttonValue;

	    extern const unsigned int NUM_COLOR_VALUES;

	    CCSSettingColorValue *
	    getColorValueList ();

	    namespace populators
	    {
		namespace list
		{
		    CCSSettingValueList boolean (CCSSetting *setting);
		    CCSSettingValueList integer (CCSSetting *setting);
		    CCSSettingValueList doubleprecision (CCSSetting *setting);
		    CCSSettingValueList string (CCSSetting *setting);
		    CCSSettingValueList match (CCSSetting *setting);
		    CCSSettingValueList color (CCSSetting *setting);
		}
	    }
	}
    }
}

#endif
