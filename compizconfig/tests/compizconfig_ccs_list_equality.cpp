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
#include <ccs.h>
#include <compizconfig_ccs_list_equality.h>

using ::testing::MakeMatcher;

class PrivateListEqualityMatcher
{
    public:

	PrivateListEqualityMatcher (CCSSettingListInfo *info,
				    CCSSettingValueList cmp) :
	    mInfo (info),
	    mCmp (cmp)
	{
	}

	CCSSettingListInfo  *mInfo;
	CCSSettingValueList mCmp;
};

ListEqualityMatcher::ListEqualityMatcher (CCSSettingListInfo *info,
					  CCSSettingValueList cmp) :
    priv (new PrivateListEqualityMatcher (info, cmp))
{
}

bool
ListEqualityMatcher::MatchAndExplain (CCSSettingValueList x,
				      MatchResultListener *listener) const
{
    return ccsCompareLists (x, priv->mCmp, *priv->mInfo);
}

void
ListEqualityMatcher::DescribeTo (std::ostream *os) const
{
    *os << "lists are equal";
}

void
ListEqualityMatcher::DescribeNegationTo (std::ostream *os) const
{
    *os << "lists are not equal";
}

Matcher<CCSSettingValueList> ListEqual (CCSSettingListInfo *info,
					CCSSettingValueList cmp)
{
    return MakeMatcher (new ListEqualityMatcher (info, cmp));
}

