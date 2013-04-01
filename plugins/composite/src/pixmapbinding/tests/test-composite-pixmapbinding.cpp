/*
 * Copyright © 2012 Canonical Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Dennis Kasprzyk not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Dennis Kasprzyk makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DENNIS KASPRZYK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DENNIS KASPRZYK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Sam Spilsbury <sam.spilsbury@canonical.com>
 */

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <core/servergrab.h>
#include "pixmapbinding.h"

using ::testing::Invoke;
using ::testing::Return;
using ::testing::_;

class CompositePixmapBinderTest :
    public ::testing::Test
{
};

class MockPixmapFreezer :
    public PixmapFreezerInterface
{
    public:

	MOCK_METHOD0 (frozen, bool ());
};

class MockWindowPixmapGet :
    public WindowPixmapGetInterface
{
    public:

	MOCK_METHOD0 (getPixmap, WindowPixmapInterface::Ptr ());
};

class FakeWindowAttributesGet :
    public WindowAttributesGetInterface
{
    public:

	FakeWindowAttributesGet (XWindowAttributes &wa) :
	    mAttributes (wa)
	{
	}

	bool getAttributes (XWindowAttributes &wa)
	{
	    wa = mAttributes;
	    return true;
	}

    private:

	XWindowAttributes mAttributes;
};

class MockWindowAttributesGet :
    public WindowAttributesGetInterface
{
    public:

	MOCK_METHOD1 (getAttributes, bool (XWindowAttributes &));
};

class MockPixmap :
    public WindowPixmapInterface
{
    public:

	typedef boost::shared_ptr <MockPixmap> Ptr;

	MOCK_CONST_METHOD0 (pixmap, Pixmap ());
	MOCK_METHOD0 (releasePixmap, void ());
};

class MockServerGrab :
    public ServerGrabInterface
{
    public:

	MOCK_METHOD0 (grabServer, void ());
	MOCK_METHOD0 (ungrabServer, void ());
	MOCK_METHOD0 (syncServer, void ());
};

class PixmapReadyInterface
{
    public:

	virtual ~PixmapReadyInterface () {}

	virtual void ready () = 0;
};

class MockPixmapReady :
    public PixmapReadyInterface
{
    public:

	MOCK_METHOD0 (ready, void ());
};

TEST(CompositePixmapBinderTest, TestInitialBindSuccess)
{
    /* Leave this here
     * There's a bug in Google Mock at the moment where
     * if a function with an expectation which returns
     * Mock B through a container is itself contained in
     * Mock A, then when Mock A is destroyed, it will first
     * lock a mutex, and then it will invoke
     * the destructor the container containing Mock B which
     * in turn might invoke Mock B's destructor and thus
     * lock the same mutex in the same thread (deadlock).
     *
     * Keeping this reference here ensures that MockPixmap
     * stays alive while MockWindowPixmapGet is being destroyed
     */
    MockPixmap::Ptr wp (boost::make_shared <MockPixmap> ());

    MockWindowPixmapGet mwpg;
    MockWindowAttributesGet mwag;
    MockServerGrab          msg;
    MockPixmapFreezer       mpf;
    XWindowAttributes xwa;

    xwa.width = 100;
    xwa.height = 200;
    xwa.map_state = IsViewable;
    xwa.border_width = 1;

    FakeWindowAttributesGet fwag (xwa);

    MockPixmapReady ready;

    boost::function <void ()> readyCb (boost::bind (&PixmapReadyInterface::ready, &ready));

    PixmapBinding pr (readyCb,
			&mwpg,
			&mwag,
			&mpf,
			&msg);

    EXPECT_CALL (msg, grabServer ());
    EXPECT_CALL (msg, syncServer ()).Times (2);
    EXPECT_CALL (mwag, getAttributes (_)).WillOnce (Invoke (&fwag, &FakeWindowAttributesGet::getAttributes));
    EXPECT_CALL (mwpg, getPixmap ()).WillOnce (Return (boost::shared_static_cast <WindowPixmapInterface> (wp)));

    EXPECT_CALL (*wp, pixmap ()).WillOnce (Return (1));

    EXPECT_CALL (ready, ready ());
    EXPECT_CALL (msg, ungrabServer ());

    pr.bind ();

    EXPECT_CALL (*wp, pixmap ()).WillOnce (Return (1));

    EXPECT_EQ (pr.pixmap (), 1);
    EXPECT_EQ (pr.size (), CompSize (102, 202));

    EXPECT_CALL (*wp, releasePixmap ());
}

TEST(CompositePixmapBinderTest, TestInitialBindSuccessNoRebind)
{
    MockPixmap::Ptr wp (boost::make_shared <MockPixmap> ());

    MockWindowPixmapGet mwpg;
    MockWindowAttributesGet mwag;
    MockServerGrab          msg;
    MockPixmapFreezer       mpf;
    XWindowAttributes xwa;

    xwa.width = 100;
    xwa.height = 200;
    xwa.map_state = IsViewable;
    xwa.border_width = 1;

    FakeWindowAttributesGet fwag (xwa);

    MockPixmapReady ready;

    boost::function <void ()> readyCb (boost::bind (&PixmapReadyInterface::ready, &ready));

    PixmapBinding pr (readyCb,
			&mwpg,
			&mwag,
			&mpf,
			&msg);

    EXPECT_CALL (msg, grabServer ());
    EXPECT_CALL (msg, syncServer ()).Times (2);
    EXPECT_CALL (mwag, getAttributes (_)).WillOnce (Invoke (&fwag, &FakeWindowAttributesGet::getAttributes));
    EXPECT_CALL (mwpg, getPixmap ()).WillOnce (Return (boost::shared_static_cast <WindowPixmapInterface> (wp)));

    EXPECT_CALL (*wp, pixmap ()).WillOnce (Return (1));

    EXPECT_CALL (ready, ready ());
    EXPECT_CALL (msg, ungrabServer ());

    EXPECT_TRUE (pr.bind ());
    EXPECT_TRUE (pr.bind ());

    EXPECT_CALL (*wp, pixmap ()).WillOnce (Return (1));

    EXPECT_EQ (pr.pixmap (), 1);
    EXPECT_EQ (pr.size (), CompSize (102, 202));

    EXPECT_CALL (*wp, releasePixmap ());
}

TEST(CompositePixmapBinderTest, TestRebindAfterRelease)
{
    MockPixmap::Ptr wp (boost::make_shared <MockPixmap> ());

    MockWindowPixmapGet mwpg;
    MockWindowAttributesGet mwag;
    MockServerGrab          msg;
    MockPixmapFreezer       mpf;
    XWindowAttributes xwa;

    xwa.width = 100;
    xwa.height = 200;
    xwa.map_state = IsViewable;
    xwa.border_width = 1;

    FakeWindowAttributesGet fwag (xwa);

    MockPixmapReady ready;

    boost::function <void ()> readyCb (boost::bind (&PixmapReadyInterface::ready, &ready));

    PixmapBinding pr (readyCb,
			&mwpg,
			&mwag,
			&mpf,
			&msg);

    EXPECT_CALL (msg, grabServer ());
    EXPECT_CALL (msg, syncServer ()).Times (2);
    EXPECT_CALL (mwag, getAttributes (_)).WillOnce (Invoke (&fwag, &FakeWindowAttributesGet::getAttributes));
    EXPECT_CALL (mwpg, getPixmap ()).WillOnce (Return (boost::shared_static_cast <WindowPixmapInterface> (wp)));

    EXPECT_CALL (*wp, pixmap ()).WillOnce (Return (1));

    EXPECT_CALL (ready, ready ());
    EXPECT_CALL (msg, ungrabServer ());

    EXPECT_TRUE (pr.bind ());

    EXPECT_CALL (mpf, frozen ()).WillOnce (Return (false));
    EXPECT_CALL (*wp, pixmap ()).WillOnce (Return (1));

    EXPECT_EQ (pr.pixmap (), 1);
    EXPECT_EQ (pr.size (), CompSize (102, 202));

    EXPECT_CALL (*wp, releasePixmap ());

    pr.release ();

    EXPECT_CALL (msg, grabServer ());
    EXPECT_CALL (msg, syncServer ()).Times (2);
    EXPECT_CALL (mwag, getAttributes (_)).WillOnce (Invoke (&fwag, &FakeWindowAttributesGet::getAttributes));
    EXPECT_CALL (mwpg, getPixmap ()).WillOnce (Return (boost::shared_static_cast <WindowPixmapInterface> (wp)));

    EXPECT_CALL (*wp, pixmap ()).WillOnce (Return (1));

    EXPECT_CALL (ready, ready ());
    EXPECT_CALL (msg, ungrabServer ());

    EXPECT_TRUE (pr.bind ());

    EXPECT_CALL (*wp, pixmap ()).WillOnce (Return (1));

    EXPECT_EQ (pr.pixmap (), 1);
    EXPECT_EQ (pr.size (), CompSize (102, 202));

    EXPECT_CALL (*wp, releasePixmap ());
}

TEST(CompositePixmapBinderTest, TestNoRebindAfterReleaseWhenFrozen)
{
    MockPixmap::Ptr wp (boost::make_shared <MockPixmap> ());

    MockWindowPixmapGet mwpg;
    MockWindowAttributesGet mwag;
    MockServerGrab          msg;
    MockPixmapFreezer       mpf;
    XWindowAttributes xwa;

    xwa.width = 100;
    xwa.height = 200;
    xwa.map_state = IsViewable;
    xwa.border_width = 1;

    FakeWindowAttributesGet fwag (xwa);

    MockPixmapReady ready;

    boost::function <void ()> readyCb (boost::bind (&PixmapReadyInterface::ready, &ready));

    PixmapBinding pr (readyCb,
			&mwpg,
			&mwag,
			&mpf,
			&msg);

    EXPECT_CALL (msg, grabServer ());
    EXPECT_CALL (msg, syncServer ()).Times (2);
    EXPECT_CALL (mwag, getAttributes (_)).WillOnce (Invoke (&fwag, &FakeWindowAttributesGet::getAttributes));
    EXPECT_CALL (mwpg, getPixmap ()).WillOnce (Return (boost::shared_static_cast <WindowPixmapInterface> (wp)));

    EXPECT_CALL (*wp, pixmap ()).WillOnce (Return (1));

    EXPECT_CALL (ready, ready ());
    EXPECT_CALL (msg, ungrabServer ());

    EXPECT_TRUE (pr.bind ());

    EXPECT_CALL (mpf, frozen ()).WillOnce (Return (true));
    EXPECT_CALL (*wp, pixmap ()).WillOnce (Return (1));

    EXPECT_EQ (pr.pixmap (), 1);
    EXPECT_EQ (pr.size (), CompSize (102, 202));

    pr.release ();

    EXPECT_TRUE (pr.bind ());

    EXPECT_CALL (*wp, pixmap ()).WillOnce (Return (1));

    EXPECT_EQ (pr.pixmap (), 1);
    EXPECT_EQ (pr.size (), CompSize (102, 202));

    EXPECT_CALL (*wp, releasePixmap ());
}

TEST(CompositePixmapBinderTest, TestInitialBindFailureWindowUnmapped)
{
    MockWindowPixmapGet mwpg;
    MockWindowAttributesGet mwag;
    MockServerGrab          msg;
    MockPixmapFreezer       mpf;
    XWindowAttributes xwa;

    xwa.width = 100;
    xwa.height = 200;
    xwa.map_state = IsUnmapped;
    xwa.border_width = 1;

    FakeWindowAttributesGet fwag (xwa);

    PixmapBinding pr (boost::function <void ()> (),
			&mwpg,
			&mwag,
			&mpf,
			&msg);

    EXPECT_CALL (msg, grabServer ());
    EXPECT_CALL (msg, syncServer ()).Times (2);
    EXPECT_CALL (mwag, getAttributes (_)).WillOnce (Invoke (&fwag, &FakeWindowAttributesGet::getAttributes));

    EXPECT_CALL (msg, ungrabServer ());

    EXPECT_FALSE (pr.bind ());

    EXPECT_EQ (pr.pixmap (), 0);
    EXPECT_EQ (pr.size (), CompSize (0, 0));
}

TEST(CompositePixmapBinderTest, TestInitialBindFailureWindowZeroSize)
{
    MockWindowPixmapGet mwpg;
    MockWindowAttributesGet mwag;
    MockServerGrab          msg;
    MockPixmapFreezer       mpf;
    XWindowAttributes xwa;

    xwa.width = 0;
    xwa.height = 0;
    xwa.map_state = IsViewable;
    xwa.border_width = 0;

    FakeWindowAttributesGet fwag (xwa);

    PixmapBinding pr (boost::function <void ()> (),
			&mwpg,
			&mwag,
			&mpf,
			&msg);

    EXPECT_CALL (msg, grabServer ());
    EXPECT_CALL (msg, syncServer ()).Times (2);
    EXPECT_CALL (mwag, getAttributes (_)).WillOnce (Invoke (&fwag, &FakeWindowAttributesGet::getAttributes));

    EXPECT_CALL (msg, ungrabServer ());

    EXPECT_FALSE (pr.bind ());

    EXPECT_EQ (pr.pixmap (), 0);
    EXPECT_EQ (pr.size (), CompSize (0, 0));
}

TEST(CompositePixmapBinderTest, TestInitialBindFailureNilPixmapReturned)
{
    MockPixmap::Ptr wp (boost::make_shared <MockPixmap> ());

    MockWindowPixmapGet mwpg;
    MockWindowAttributesGet mwag;
    MockServerGrab          msg;
    MockPixmapFreezer       mpf;
    XWindowAttributes xwa;

    xwa.width = 100;
    xwa.height =  200;
    xwa.map_state = IsViewable;
    xwa.border_width = 0;

    FakeWindowAttributesGet fwag (xwa);

    PixmapBinding pr (boost::function <void ()> (),
			&mwpg,
			&mwag,
			&mpf,
			&msg);

    EXPECT_CALL (msg, grabServer ());
    EXPECT_CALL (msg, syncServer ()).Times (2);
    EXPECT_CALL (mwag, getAttributes (_)).WillOnce (Invoke (&fwag, &FakeWindowAttributesGet::getAttributes));
    EXPECT_CALL (mwpg, getPixmap ()).WillOnce (Return (boost::shared_static_cast <WindowPixmapInterface> (wp)));

    EXPECT_CALL (*wp, pixmap ()).WillOnce (Return (0));

    EXPECT_CALL (msg, ungrabServer ());

    EXPECT_FALSE (pr.bind ());

    EXPECT_EQ (pr.pixmap (), 0);
    EXPECT_EQ (pr.size (), CompSize (0, 0));
}

TEST(CompositePixmapBinderTest, TestInitialBindFailureWindowUnmappedSuccessOnRemap)
{
    MockPixmap::Ptr wp (boost::make_shared <MockPixmap> ());

    MockWindowPixmapGet mwpg;
    MockWindowAttributesGet mwag;
    MockServerGrab          msg;
    MockPixmapFreezer       mpf;
    XWindowAttributes xwa;

    xwa.width = 100;
    xwa.height = 200;
    xwa.map_state = IsUnmapped;
    xwa.border_width = 1;

    FakeWindowAttributesGet fwag (xwa);

    PixmapBinding pr (boost::function <void ()> (),
			&mwpg,
			&mwag,
			&mpf,
			&msg);

    EXPECT_CALL (msg, grabServer ());
    EXPECT_CALL (msg, syncServer ()).Times (2);
    EXPECT_CALL (mwag, getAttributes (_)).WillOnce (Invoke (&fwag, &FakeWindowAttributesGet::getAttributes));

    EXPECT_CALL (msg, ungrabServer ());

    EXPECT_FALSE (pr.bind ());

    EXPECT_EQ (pr.pixmap (), 0);
    EXPECT_EQ (pr.size (), CompSize (0, 0));

    EXPECT_FALSE (pr.bind ());

    pr.allowFurtherRebindAttempts ();

    xwa.width = 100;
    xwa.height = 200;
    xwa.map_state = IsViewable;
    xwa.border_width = 1;

    FakeWindowAttributesGet fwag2 (xwa);

    EXPECT_CALL (msg, grabServer ());
    EXPECT_CALL (msg, syncServer ()).Times (2);
    EXPECT_CALL (mwag, getAttributes (_)).WillOnce (Invoke (&fwag2, &FakeWindowAttributesGet::getAttributes));
    EXPECT_CALL (mwpg, getPixmap ()).WillOnce (Return (boost::shared_static_cast <WindowPixmapInterface> (wp)));

    EXPECT_CALL (msg, ungrabServer ());

    EXPECT_CALL (*wp, pixmap ()).WillOnce (Return (1));

    EXPECT_TRUE (pr.bind ());

    EXPECT_CALL (*wp, pixmap ()).WillOnce (Return (1));
    EXPECT_EQ (pr.pixmap (), 1);
    EXPECT_EQ (pr.size (), CompSize (102, 202));
}
