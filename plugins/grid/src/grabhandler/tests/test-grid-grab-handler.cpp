/*
 * Compiz Fusion Grid plugin, GrabHandler class
 *
 * Copyright (c) 2011 Canonical Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Description:
 *
 * Plugin to act like winsplit revolution (http://www.winsplit-revolution.com/)
 * use <Control><Alt>NUMPAD_KEY to move and tile your windows.
 *
 * Press the tiling keys several times to cycle through some tiling options.
 *
 * Authored By:
 * Sam Spilsbury <sam.spilsbury@canonical.com>
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/bind.hpp>

#include <grabhandler.h>

/* FIXME: Not entirely portable, but we can't
 * include window.h without pulling in bunch of
 * static initalizers */

#define CompWindowGrabKeyMask         (1 << 0)
#define CompWindowGrabButtonMask      (1 << 1)
#define CompWindowGrabMoveMask        (1 << 2)
#define CompWindowGrabResizeMask      (1 << 3)
#define CompWindowGrabExternalAppMask (1 << 4)

using testing::Eq;
using testing::Return;

namespace
{
    bool returnFalse () { return false; }

    class MockGrabExist
    {
	public:

	    MOCK_METHOD1 (grabExist, bool (const char *));
    };
}

TEST(GridGrabHandlerTest, TestMoveHandler)
{
    compiz::grid::window::GrabWindowHandler moveHandler (CompWindowGrabMoveMask |
							 CompWindowGrabButtonMask,
							 boost::bind (returnFalse));

    EXPECT_TRUE (moveHandler.track ());
    EXPECT_FALSE (moveHandler.resetResize ());
}

TEST(GridGrabHandlerTest, TestResizeHandler)
{
    compiz::grid::window::GrabWindowHandler resizeHandler (CompWindowGrabButtonMask |
						    	   CompWindowGrabResizeMask,
							   boost::bind (returnFalse));

    EXPECT_FALSE (resizeHandler.track ());
    EXPECT_TRUE (resizeHandler.resetResize ());
}

TEST(GridGrabHandlerTest, TestNoTrackOnExpoGrab)
{
    MockGrabExist                           mge;
    compiz::grid::window::GrabActiveFunc    grabExist (boost::bind (&MockGrabExist::grabExist,
								    &mge, _1));
    compiz::grid::window::GrabWindowHandler moveHandler (CompWindowGrabMoveMask |
							 CompWindowGrabButtonMask,
							 grabExist);

    EXPECT_CALL (mge, grabExist (Eq ("expo"))).WillOnce (Return (true));
    EXPECT_FALSE (moveHandler.track ());
}
