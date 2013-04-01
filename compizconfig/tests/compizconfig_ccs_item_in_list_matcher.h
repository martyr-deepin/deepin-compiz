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
#ifndef _COMPIZCONFIG_CCS_ITEM_IN_LIST_MATCHER_H
#define _COMPIZCONFIG_CCS_ITEM_IN_LIST_MATCHER_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <ccs.h>

using ::testing::Matcher;
using ::testing::MatcherInterface;
using ::testing::MatchResultListener;
using ::testing::MakeMatcher;

template <typename I, typename L>
class ItemInCCSListMatcher :
    public ::testing::MatcherInterface <L>
{
    public:

	ItemInCCSListMatcher (const Matcher<I> &matcher) :
	    mMatcher (matcher)
	{
	}

	virtual bool MatchAndExplain (L list, MatchResultListener *listener) const
	{
	    L iter = list;

	    while (iter)
	    {
		const I &i = *(reinterpret_cast <I *> (iter->data));

		if (mMatcher.MatchAndExplain (i, listener))
		    return true;

		iter = iter->next;
	    }

	    return false;
	}

	virtual void DescribeTo (std::ostream *os) const
	{
	    *os << "found in list (";
	    mMatcher.DescribeTo (os);
	    *os << ")";
	}

	virtual void DescribeNegationTo (std::ostream *os) const
	{
	    *os << "not found in list (";
	    mMatcher.DescribeNegationTo (os);
	    *os << ")";
	}

    private:

	Matcher<I> mMatcher;
};

template <typename I, typename L>
Matcher<L> IsItemInCCSList (const Matcher<I> &matcher)
{
    return MakeMatcher (new ItemInCCSListMatcher <I, L> (matcher));
}

typedef struct _CCSString	      CCSString;
typedef struct _CCSStringList *	      CCSStringList;
typedef struct _CCSSettingValue	      CCSSettingValue;
typedef struct _CCSSettingValueList * CCSSettingValueList;

/* A workaround for templates inside of macros not
 * expanding correctly */
Matcher <CCSStringList>
IsStringItemInStringCCSList (const Matcher <CCSString> &matcher);

Matcher <CCSSettingValueList>
IsSettingValueInSettingValueCCSList (const Matcher <CCSSettingValue> &matcher);

#endif
