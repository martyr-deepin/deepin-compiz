/*
 * Copyright © 2012 Canonical Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Canonical Ltd. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Canonical Ltd. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * CANONICAL, LTD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL CANONICAL, LTD. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authored by: Sam Spilsbury <sam.spilsbury@canonical.com>
 */
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "windows-on-viewport.h"
#include "client-list-generator.h"
#include "viewport-member-window.h"

using ::testing::InSequence;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

namespace
{
    namespace ce = compiz::expo;

    class MockViewportMemberWindow :
	public ce::ViewportMemberWindow
    {
	public:

	    MOCK_CONST_METHOD0 (absoluteGeometry, const compiz::window::Geometry & ());
	    MOCK_CONST_METHOD0 (isDesktopOrDock, bool ());
	    MOCK_CONST_METHOD0 (dragged, bool ());
    };

    class MockClientListGenerator :
	public ce::ClientListGenerator
    {
	public:

	    MOCK_METHOD0 (nextClient, ce::ViewportMemberWindow * ());
    };
}

class ExpoWindowsOnViewportTest :
    public ::testing::Test
{
    protected:

	MockClientListGenerator mockClientListGenerator;
	std::vector <bool>      activeStates;
};

namespace
{
    const CompSize vpSize (2, 2);
    const CompSize screenSize (1000, 1000);
}

TEST_F (ExpoWindowsOnViewportTest, TestNoDocksMakeViewportsActive)
{
    MockViewportMemberWindow mockViewportMembers[1];
    InSequence s;

    EXPECT_CALL (mockClientListGenerator, nextClient ()).WillOnce (Return (&mockViewportMembers[0]));
    EXPECT_CALL (mockViewportMembers[0], isDesktopOrDock ()).WillOnce (Return (true));
    EXPECT_CALL (mockClientListGenerator, nextClient ()).WillOnce (ReturnNull ());

    compiz::expo::activeViewportsForMembers (mockClientListGenerator,
					     CompPoint (1, 1),
					     vpSize,
					     screenSize,
					     activeStates);

    ASSERT_EQ (activeStates.size (), vpSize.width () * vpSize.height ());
    EXPECT_EQ (activeStates[0], false);
    EXPECT_EQ (activeStates[1], false);
    EXPECT_EQ (activeStates[2], false);
    EXPECT_EQ (activeStates[3], false);
}

TEST_F (ExpoWindowsOnViewportTest, TestGrabbedWindowUsesCursorPosition)
{
    MockViewportMemberWindow mockViewportMembers[1];
    InSequence s;

    EXPECT_CALL (mockClientListGenerator, nextClient ()).WillOnce (Return (&mockViewportMembers[0]));
    EXPECT_CALL (mockViewportMembers[0], isDesktopOrDock ()).WillOnce (Return (false));
    EXPECT_CALL (mockViewportMembers[0], dragged ()).WillOnce (Return (true));
    EXPECT_CALL (mockClientListGenerator, nextClient ()).WillOnce (ReturnNull ());

    compiz::expo::activeViewportsForMembers (mockClientListGenerator,
					     CompPoint (screenSize.width () * 1.5,
							screenSize.height () * 1.5),
					     vpSize,
					     screenSize,
					     activeStates);

    ASSERT_EQ (activeStates.size (), vpSize.width () * vpSize.height ());
    EXPECT_EQ (activeStates[0], false);
    EXPECT_EQ (activeStates[1], false);
    EXPECT_EQ (activeStates[2], false);
    EXPECT_EQ (activeStates[3], true); // 2,2 has the cursor of a dragged window
}

TEST_F (ExpoWindowsOnViewportTest, TestUngrabbedWindowUsesGeometry)
{
    MockViewportMemberWindow mockViewportMembers[1];
    InSequence s;

    compiz::window::Geometry vpMemberGeometry1 (screenSize.width () * 1.1,
						screenSize.height () * 1.1,
						screenSize.width () / 2,
						screenSize.height () / 2,
						0);

    EXPECT_CALL (mockClientListGenerator, nextClient ()).WillOnce (Return (&mockViewportMembers[0]));
    EXPECT_CALL (mockViewportMembers[0], isDesktopOrDock ()).WillOnce (Return (false));
    EXPECT_CALL (mockViewportMembers[0], dragged ()).WillOnce (Return (false));
    EXPECT_CALL (mockViewportMembers[0], absoluteGeometry ()).WillOnce (ReturnRef (vpMemberGeometry1));
    EXPECT_CALL (mockClientListGenerator, nextClient ()).WillOnce (ReturnNull ());

    compiz::expo::activeViewportsForMembers (mockClientListGenerator,
					     CompPoint (screenSize.width () * 1.5,
							screenSize.height () * 1.5),
					     vpSize,
					     screenSize,
					     activeStates);

    ASSERT_EQ (activeStates.size (), vpSize.width () * vpSize.height ());
    EXPECT_EQ (activeStates[0], false);
    EXPECT_EQ (activeStates[1], false);
    EXPECT_EQ (activeStates[2], false);
    EXPECT_EQ (activeStates[3], true); // 2,2 has a window on it
}
