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

#include <compizconfig_ccs_item_in_list_matcher.h>

Matcher <CCSStringList>
IsStringItemInStringCCSList (const Matcher <CCSString> &matcher)
{
    return IsItemInCCSList <CCSString, CCSStringList> (matcher);
}

Matcher <CCSSettingValueList>
IsSettingValueInSettingValueCCSList (const Matcher <CCSSettingValue> &matcher)
{
    return IsItemInCCSList <CCSSettingValue, CCSSettingValueList> (matcher);
}
