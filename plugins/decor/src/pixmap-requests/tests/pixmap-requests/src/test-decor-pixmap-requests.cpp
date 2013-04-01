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
#include <iostream>
#include "pixmap-requests.h"

using ::testing::Return;

class DecorPixmapRequestsTest :
    public ::testing::Test
{
};

class MockDecorPixmapDeletor :
    public DecorPixmapDeletionInterface
{
    public:

	MOCK_METHOD1 (postDeletePixmap, int (Pixmap p));
};

class MockDecorPixmapReceiver :
    public DecorPixmapReceiverInterface
{
    public:

	MOCK_METHOD0 (pending, void ());
	MOCK_METHOD0 (update, void ());
};

class MockDecoration :
    public DecorationInterface
{
    public:

	MOCK_METHOD0 (receiverInterface, DecorPixmapReceiverInterface & ());
	MOCK_CONST_METHOD0 (getFrameType, unsigned int ());
	MOCK_CONST_METHOD0 (getFrameState, unsigned int ());
	MOCK_CONST_METHOD0 (getFrameActions, unsigned int ());
};

class MockDecorationListFindMatching :
    public DecorationListFindMatchingInterface
{
    public:

	MOCK_METHOD3 (findMatchingDecoration, DecorationInterface::Ptr (unsigned int, unsigned int, unsigned int));
};

class MockDecorPixmapRequestor :
    public DecorPixmapRequestorInterface
{
    public:

	MOCK_METHOD3 (postGenerateRequest, int (unsigned int, unsigned int, unsigned int));
	MOCK_METHOD1 (handlePending, void (long *));
};

TEST(DecorPixmapRequestsTest, TestDestroyPixmapDeletes)
{
    boost::shared_ptr <MockDecorPixmapDeletor> mockDeletor = boost::make_shared <MockDecorPixmapDeletor> ();
    DecorPixmap pm (1, boost::shared_static_cast<DecorPixmapDeletionInterface> (mockDeletor));

    EXPECT_CALL (*(mockDeletor.get ()), postDeletePixmap (1)).WillOnce (Return (1));
}

TEST(DecorPixmapRequestsTest, TestPendingGeneratesRequest)
{
    MockDecorPixmapRequestor mockRequestor;
    MockDecoration           mockDecoration;
    X11DecorPixmapReceiver   receiver (&mockRequestor, &mockDecoration);

    EXPECT_CALL (mockDecoration, getFrameActions ()).WillOnce (Return (3));
    EXPECT_CALL (mockDecoration, getFrameState ()).WillOnce (Return (2));
    EXPECT_CALL (mockDecoration, getFrameType ()).WillOnce (Return (1));

    EXPECT_CALL (mockRequestor, postGenerateRequest (1, 2, 3));

    receiver.pending ();
}

TEST(DecorPixmapRequestsTest, TestPendingGeneratesOnlyOneRequest)
{
    MockDecorPixmapRequestor mockRequestor;
    MockDecoration           mockDecoration;
    X11DecorPixmapReceiver   receiver (&mockRequestor, &mockDecoration);

    EXPECT_CALL (mockDecoration, getFrameActions ()).WillOnce (Return (3));
    EXPECT_CALL (mockDecoration, getFrameState ()).WillOnce (Return (2));
    EXPECT_CALL (mockDecoration, getFrameType ()).WillOnce (Return (1));

    EXPECT_CALL (mockRequestor, postGenerateRequest (1, 2, 3));

    receiver.pending ();
    receiver.pending ();
}

TEST(DecorPixmapRequestsTest, TestUpdateGeneratesRequestIfNewOnePending)
{
    MockDecorPixmapRequestor mockRequestor;
    MockDecoration           mockDecoration;
    X11DecorPixmapReceiver   receiver (&mockRequestor, &mockDecoration);

    EXPECT_CALL (mockDecoration, getFrameActions ()).WillOnce (Return (3));
    EXPECT_CALL (mockDecoration, getFrameState ()).WillOnce (Return (2));
    EXPECT_CALL (mockDecoration, getFrameType ()).WillOnce (Return (1));

    EXPECT_CALL (mockRequestor, postGenerateRequest (1, 2, 3));

    receiver.pending ();
    receiver.pending ();

    EXPECT_CALL (mockDecoration, getFrameActions ()).WillOnce (Return (3));
    EXPECT_CALL (mockDecoration, getFrameState ()).WillOnce (Return (2));
    EXPECT_CALL (mockDecoration, getFrameType ()).WillOnce (Return (1));

    EXPECT_CALL (mockRequestor, postGenerateRequest (1, 2, 3));

    receiver.update ();
}

TEST(DecorPixmapRequestsTest, TestUpdateGeneratesNoRequestIfNoNewOnePending)
{
    MockDecorPixmapRequestor mockRequestor;
    MockDecoration           mockDecoration;
    X11DecorPixmapReceiver   receiver (&mockRequestor, &mockDecoration);

    EXPECT_CALL (mockDecoration, getFrameActions ()).WillOnce (Return (3));
    EXPECT_CALL (mockDecoration, getFrameState ()).WillOnce (Return (2));
    EXPECT_CALL (mockDecoration, getFrameType ()).WillOnce (Return (1));

    EXPECT_CALL (mockRequestor, postGenerateRequest (1, 2, 3));

    receiver.pending ();
    receiver.update ();
}
