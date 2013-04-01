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
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>
#include "compizconfig_ccs_setting_value_matcher.h"

CCSSettingValueMatcher::CCSSettingValueMatcher (const CCSSettingValue &match,
						CCSSettingType        type,
						CCSSettingInfo        *info) :
    mMatch (match),
    mType  (type),
    mInfo  (info)
{
}

bool
CCSSettingValueMatcher::MatchAndExplain (CCSSettingValue x, MatchResultListener *listener) const
{
    if (ccsCheckValueEq (&x,
			 mType,
			 mInfo,
			 &mMatch,
			 mType,
			 mInfo))
	return true;
    return false;
}

void
CCSSettingValueMatcher::DescribeTo (std::ostream *os) const
{
    *os << "Value Matches";
}

void
CCSSettingValueMatcher::DescribeNegationTo (std::ostream *os) const
{
    *os << "Value does not Match";
}

Matcher <CCSSettingValue>
SettingValueMatch (const CCSSettingValue &match,
		   CCSSettingType	     type,
		   CCSSettingInfo	     *info)
{
    return MakeMatcher (new CCSSettingValueMatcher (match, type, info));
}
