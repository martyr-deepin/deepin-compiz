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

#include <ccs.h>
#include <compizconfig_ccs_setting_value_operators.h>
#include <iostream>

bool
operator== (const CCSSettingColorValue &lhs,
	    const CCSSettingColorValue &rhs)
{
    if (ccsIsEqualColor (lhs, rhs))
	return true;
    return false;
}

std::ostream &
operator<< (std::ostream &os, const CCSSettingColorValue &v)
{
    return os << "Red: " << std::hex << v.color.red << "Blue: " << std::hex << v.color.blue << "Green: " << v.color.green << "Alpha: " << v.color.alpha
       << std::dec << std::endl;
}

bool
operator== (const CCSSettingKeyValue &lhs,
	    const CCSSettingKeyValue &rhs)
{
    if (ccsIsEqualKey (lhs, rhs))
	return true;
    return false;
}

std::ostream &
operator<< (std::ostream &os, const CCSSettingKeyValue &v)
{
    return os << "Keysym: " << v.keysym << " KeyModMask " << std::hex << v.keyModMask << std::dec << std::endl;
}

bool
operator== (const CCSSettingButtonValue &lhs,
	    const CCSSettingButtonValue &rhs)
{
    if (ccsIsEqualButton (lhs, rhs))
	return true;
    return false;
}

std::ostream &
operator<< (std::ostream &os, const CCSSettingButtonValue &v)
{
    return os << "Button " << v.button << "Button Key Mask: " << std::hex << v.buttonModMask << "Edge Mask: " << v.edgeMask << std::dec << std::endl;
}

bool
operator== (const CCSString &lhs,
	    const std::string &rhs)
{
    if (rhs == lhs.value)
	return true;

    return false;
}

bool
operator== (const std::string &lhs,
	    const CCSString &rhs)
{
    return rhs == lhs;
}

bool
operator== (const std::string &rhs,
	    CCSString	      *lhs)
{
    return *lhs == rhs;
}

std::ostream &
operator<< (std::ostream &os, CCSString &string)
{
    os << string.value << std::endl;
    return os;
}
