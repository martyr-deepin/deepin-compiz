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
#ifndef _COMPIZCONFIG_CCS_SETTING_VALUE_MATCHER_H
#define _COMPIZCONFIG_CCS_SETTING_VALUE_MATCHER_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs-defs.h>
#include <ccs-setting-types.h>

using ::testing::MatcherInterface;
using ::testing::Matcher;
using ::testing::MatchResultListener;

typedef struct _CCSSettingValue CCSSettingValue;
typedef union  _CCSSettingInfo  CCSSettingInfo;

class CCSSettingValueMatcher :
    public ::testing::MatcherInterface <CCSSettingValue>
{
    public:

	CCSSettingValueMatcher (const CCSSettingValue &match,
				CCSSettingType        type,
				CCSSettingInfo        *info);

	virtual bool MatchAndExplain (CCSSettingValue x, MatchResultListener *listener) const;
	virtual void DescribeTo (std::ostream *os) const;
	virtual void DescribeNegationTo (std::ostream *os) const;

    private:

	const CCSSettingValue &mMatch;
	CCSSettingType	  mType;
	CCSSettingInfo	  *mInfo;
};

Matcher <CCSSettingValue>
SettingValueMatch (const CCSSettingValue &match,
		   CCSSettingType	 type,
		   CCSSettingInfo	 *info);

#endif
