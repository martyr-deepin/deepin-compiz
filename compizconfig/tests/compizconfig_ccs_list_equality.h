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
#ifndef _COMPIZCONFIG_CCS_LIST_EQUALITY_H
#define _COMPIZCONFIG_CCS_LIST_EQUALITY_H

#include <iosfwd>
#include <memory>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::MatcherInterface;
using ::testing::MatchResultListener;
using ::testing::Matcher;

typedef struct _CCSSettingValueList * CCSSettingValueList;
typedef struct _CCSSettingListInfo    CCSSettingListInfo;

class PrivateListEqualityMatcher;

class ListEqualityMatcher :
    public MatcherInterface <CCSSettingValueList>
{
    public:

	ListEqualityMatcher (CCSSettingListInfo  *info,
			     CCSSettingValueList cmp);

	virtual bool MatchAndExplain (CCSSettingValueList x, MatchResultListener *listener) const;
	virtual void DescribeTo (std::ostream *os) const;
	virtual void DescribeNegationTo (std::ostream *os) const;

    private:

	std::auto_ptr <PrivateListEqualityMatcher> priv;
};

Matcher<CCSSettingValueList> ListEqual (CCSSettingListInfo *info,
					CCSSettingValueList cmp);

#endif
