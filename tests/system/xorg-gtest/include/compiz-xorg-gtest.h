/*
 * Compiz XOrg GTest
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
#ifndef _COMPIZ_XORG_GTEST_H
#define _COMPIZ_XORG_GTEST_H
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <xorg/gtest/xorg-gtest.h>

using ::testing::MatcherInterface;

namespace compiz
{
    namespace testing
    {
	typedef  ::testing::MatcherInterface <const XEvent &> XEventMatcher;

	std::list <Window> NET_CLIENT_LIST_STACKING (Display *);
	bool WaitForEventOfTypeOnWindow (Display *dpy,
					 Window  w,
					 int     type,
					 int     ext,
					 int     extType,
					 int     timeout = 1000);
	bool WaitForEventOfTypeOnWindowMatching (Display             *dpy,
						 Window              w,
						 int                 type,
						 int                 ext,
						 int                 extType,
						 const XEventMatcher &matcher,
						 int                 timeout = 1000);

	class XorgSystemTest :
	    public xorg::testing::Test
	{
	    public:

		virtual void SetUp ();
		virtual void TearDown ();

	    private:

		xorg::testing::Process mCompizProcess;
	};
    }
}

#endif
