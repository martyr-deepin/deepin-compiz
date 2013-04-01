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
#include "clip-groups.h"

using ::testing::Invoke;
using ::testing::_;
using ::testing::StrictMock;

using namespace compiz::decor;

class CompDecorClipGroupsTest :
    public ::testing::Test
{
};

class FakeDecorClippable :
    public DecorClippableInterface
{
    public:
	FakeDecorClippable (const CompRegion &inputRegion,
			    const CompRegion &outputRegion,
			    DecorClippableInterface *parent,
			    bool	     matches) :
	    mInputRegion (inputRegion),
	    mOutputRegion (outputRegion),
	    mParent (parent),
	    mOwner (NULL),
	    mMatches (matches)
	{
	}

	~FakeDecorClippable ()
	{
	    if (mOwner)
		mOwner->popClippable (mParent);
	}

	const CompRegion &
	getShadowRegion ()
	{
	    return mShadowRegion;
	}

    private:

	void doUpdateShadow (const CompRegion &r)
	{
	    mShadowRegion = mOutputRegion.intersected (r - mInputRegion);
	}

	void doSetOwner (DecorClipGroupInterface *i)
	{
	    mOwner = i;
	}

	bool doMatches (const CompMatch &m)
	{
	    return mMatches;
	}

	const CompRegion & getOutputRegion () { return mOutputRegion; }
	const CompRegion & getInputRegion () { return mInputRegion; }

	void doUpdateGroupShadows ()
	{
	    if (mOwner)
		mOwner->updateAllShadows ();
	}

	CompRegion mInputRegion;
	CompRegion mOutputRegion;

	CompRegion mShadowRegion;

	DecorClippableInterface *mParent;
	DecorClipGroupInterface *mOwner;

	bool				       mMatches;
};

class MockDecorClippable :
    public DecorClippableInterface
{
    public:
	
	MOCK_METHOD1 (doUpdateShadow, void (const CompRegion &));
	MOCK_METHOD1 (doSetOwner, void (DecorClipGroupInterface *));
	MOCK_METHOD1 (doMatches, bool (const CompMatch &));
	MOCK_METHOD0 (getOutputRegion, const CompRegion & ());
	MOCK_METHOD0 (getInputRegion, const CompRegion & ());
	MOCK_METHOD0 (doUpdateGroupShadows, void ());

	void Delegate (DecorClippableInterface &other)
	{
	    ON_CALL (*this, doUpdateShadow (_)).WillByDefault (Invoke (&other, &DecorClippableInterface::updateShadow));
	    ON_CALL (*this, doSetOwner (_)).WillByDefault (Invoke (&other, &DecorClippableInterface::setOwner));
	    ON_CALL (*this, doMatches (_)).WillByDefault (Invoke (&other, &DecorClippableInterface::matches));
	    ON_CALL (*this, getOutputRegion ()).WillByDefault (Invoke (&other, &DecorClippableInterface::outputRegion));
	    ON_CALL (*this, getInputRegion ()).WillByDefault (Invoke (&other, &DecorClippableInterface::inputRegion));
	    ON_CALL (*this, doUpdateGroupShadows ()).WillByDefault (Invoke (&other, &DecorClippableInterface::updateGroupShadows));
	}
};

void PrintTo(const CompRegion &reg, ::std::ostream *os)
{
    const CompRect &br = reg.boundingRect ();
    *os << "Bounding Rect " << br.x () << " " << br.y () << " " << br.width () << " " << br.height () << std::endl;
    const CompRect::vector &rv = reg.rects ();
    for (CompRect::vector::const_iterator it = rv.begin ();
	 it != rv.end ();
	 ++it)
    {
	const CompRect &r = *it;
	*os << " - Rect : " << r.x () << " " << r.y () << " " << r.width () << " " << r.height () << std::endl;
    }
}

TEST_F(CompDecorClipGroupsTest, TestPushClippable)
{
    impl::GenericDecorClipGroup cg;
    StrictMock <MockDecorClippable> mdc;
    FakeDecorClippable fdc (CompRegion (50, 50, 100, 100),
			    CompRegion (0, 0, 100, 100),
			    &mdc,
			    true);

    mdc.Delegate (fdc);

    EXPECT_CALL (mdc, getInputRegion ());
    EXPECT_CALL (mdc, doSetOwner (_)).Times (2);
    EXPECT_CALL (mdc, doUpdateShadow (_));

    cg.pushClippable (&mdc);
}

TEST_F(CompDecorClipGroupsTest, TestPushClippableUpdatesRegion)
{
    impl::GenericDecorClipGroup cg;
    StrictMock <MockDecorClippable> mdc;
    FakeDecorClippable fdc (CompRegion (50, 50, 100, 100),
			    CompRegion (0, 0, 100, 100),
			    &mdc,
			    true);

    mdc.Delegate (fdc);

    EXPECT_CALL (mdc, getInputRegion ()).Times (2);
    EXPECT_CALL (mdc, doSetOwner (_)).Times (2);
    EXPECT_CALL (mdc, doUpdateShadow (_));

    cg.pushClippable (&mdc);

    EXPECT_EQ (cg.clipRegion (), mdc.inputRegion ());
}

MATCHER_P (CompRegionEq, other, "")
{
    return (arg ^ other).isEmpty ();
}

TEST_F(CompDecorClipGroupsTest, TestPush2ClippableUpdatesRegion)
{
    impl::GenericDecorClipGroup cg;
    StrictMock <MockDecorClippable> mdc;
    FakeDecorClippable fdc (CompRegion (50, 50, 100, 100),
			    CompRegion (0, 0, 100, 100),
			    &mdc,
			    true);

    mdc.Delegate (fdc);

    StrictMock <MockDecorClippable> mdc2;
    FakeDecorClippable fdc2 (CompRegion (250, 250, 100, 100),
			     CompRegion (200, 200, 100, 100),
			     &mdc2,
			     true);

    mdc2.Delegate (fdc2);

    EXPECT_CALL (mdc, getInputRegion ()).Times (4);
    EXPECT_CALL (mdc, doSetOwner (&cg));
    EXPECT_CALL (mdc, doSetOwner (NULL));
    EXPECT_CALL (mdc, doUpdateShadow (_)).Times (1);

    EXPECT_CALL (mdc2, getInputRegion ()).Times (2);
    EXPECT_CALL (mdc2, doSetOwner (&cg));
    EXPECT_CALL (mdc2, doSetOwner (NULL));
    EXPECT_CALL (mdc2, doUpdateShadow (_)).Times (1);

    cg.pushClippable (&mdc);
    cg.pushClippable (&mdc2);

    CompRegion accumulated;

    accumulated += mdc.inputRegion ();
    accumulated += mdc2.inputRegion ();

    EXPECT_THAT (cg.clipRegion (), CompRegionEq (accumulated));
}

TEST_F(CompDecorClipGroupsTest, TestPush3ClippableUpdatesRegion)
{
    impl::GenericDecorClipGroup cg;
    StrictMock <MockDecorClippable> mdc;
    FakeDecorClippable fdc (CompRegion (50, 50, 100, 100),
			    CompRegion (0, 0, 100, 100),
			    &mdc,
			    true);

    mdc.Delegate (fdc);

    StrictMock <MockDecorClippable> mdc2;
    FakeDecorClippable fdc2 (CompRegion (150, 150, 100, 100),
			     CompRegion (100, 100, 100, 100),
			     &mdc2,
			     true);

    mdc2.Delegate (fdc2);

    StrictMock <MockDecorClippable> mdc3;
    FakeDecorClippable fdc3 (CompRegion (250, 250, 100, 100),
			     CompRegion (200, 200, 100, 100),
			     &mdc3,
			     true);

    mdc3.Delegate (fdc3);

    EXPECT_CALL (mdc, getInputRegion ()).Times (6);
    EXPECT_CALL (mdc, doSetOwner (&cg));
    EXPECT_CALL (mdc, doSetOwner (NULL));
    EXPECT_CALL (mdc, doUpdateShadow (_)).Times (1);

    EXPECT_CALL (mdc2, getInputRegion ()).Times (4);
    EXPECT_CALL (mdc2, doSetOwner (&cg));
    EXPECT_CALL (mdc2, doSetOwner (NULL));
    EXPECT_CALL (mdc2, doUpdateShadow (_)).Times (1);

    EXPECT_CALL (mdc3, getInputRegion ()).Times (2);
    EXPECT_CALL (mdc3, doSetOwner (&cg));
    EXPECT_CALL (mdc3, doSetOwner (NULL));
    EXPECT_CALL (mdc3, doUpdateShadow (_)).Times (1);

    cg.pushClippable (&mdc);
    cg.pushClippable (&mdc2);
    cg.pushClippable (&mdc3);

    CompRegion accumulated;

    accumulated += mdc.inputRegion ();
    accumulated += mdc2.inputRegion ();
    accumulated += mdc3.inputRegion ();

    EXPECT_THAT (cg.clipRegion (), CompRegionEq (accumulated));
}

TEST_F(CompDecorClipGroupsTest, TestPush2ClippableUpdatesShadow)
{
    impl::GenericDecorClipGroup cg;
    StrictMock <MockDecorClippable> mdc;
    FakeDecorClippable fdc (CompRegion (50, 50, 25, 25),
			    CompRegion (25, 25, 100, 100),
			    &mdc,
			    true);

    mdc.Delegate (fdc);

    StrictMock <MockDecorClippable> mdc2;
    FakeDecorClippable fdc2 (CompRegion (75, 75, 50, 50),
			     CompRegion (25, 25, 100, 100),
			     &mdc2,
			     true);

    mdc2.Delegate (fdc2);

    EXPECT_CALL (mdc, getInputRegion ()).Times (4);
    EXPECT_CALL (mdc, doSetOwner (&cg));
    EXPECT_CALL (mdc, doSetOwner (NULL));
    EXPECT_CALL (mdc, doUpdateShadow (_)).Times (2);

    EXPECT_CALL (mdc2, getInputRegion ()).Times (2);
    EXPECT_CALL (mdc2, doSetOwner (&cg));
    EXPECT_CALL (mdc2, doSetOwner (NULL));
    EXPECT_CALL (mdc2, doUpdateShadow (_)).Times (2);

    cg.pushClippable (&mdc);
    cg.pushClippable (&mdc2);

    cg.updateAllShadows ();

    CompRegion accumulated;

    accumulated += mdc.inputRegion ();
    accumulated += mdc2.inputRegion ();

    EXPECT_THAT (cg.clipRegion (), CompRegionEq (accumulated));

    CompRegion fdcRegion (75, 75, 50, 50);

    EXPECT_THAT (fdc.getShadowRegion (), CompRegionEq (fdcRegion));

    CompRegion fdc2Region (50, 50, 25, 25);

    EXPECT_THAT (fdc2.getShadowRegion (), CompRegionEq (fdc2Region));
}

TEST_F(CompDecorClipGroupsTest, TestPush3ClippableUpdatesRegionPop1)
{
    impl::GenericDecorClipGroup cg;
    StrictMock <MockDecorClippable> mdc;
    FakeDecorClippable fdc (CompRegion (50, 50, 100, 100),
			    CompRegion (0, 0, 100, 100),
			    &mdc,
			    true);

    mdc.Delegate (fdc);

    StrictMock <MockDecorClippable> mdc2;
    FakeDecorClippable fdc2 (CompRegion (150, 150, 100, 100),
			     CompRegion (100, 100, 100, 100),
			     &mdc2,
			     true);

    mdc2.Delegate (fdc2);

    StrictMock <MockDecorClippable> mdc3;
    FakeDecorClippable fdc3 (CompRegion (250, 250, 100, 100),
			     CompRegion (200, 200, 100, 100),
			     &mdc3,
			     true);

    mdc3.Delegate (fdc3);

    EXPECT_CALL (mdc, getInputRegion ()).Times (7);
    EXPECT_CALL (mdc, doSetOwner (&cg));
    EXPECT_CALL (mdc, doSetOwner (NULL));
    EXPECT_CALL (mdc, doUpdateShadow (_)).Times (1);

    EXPECT_CALL (mdc2, getInputRegion ()).Times (5);
    EXPECT_CALL (mdc2, doSetOwner (&cg));
    EXPECT_CALL (mdc2, doSetOwner (NULL));
    EXPECT_CALL (mdc2, doUpdateShadow (_)).Times (1);

    EXPECT_CALL (mdc3, getInputRegion ()).Times (2);
    EXPECT_CALL (mdc3, doSetOwner (&cg));
    EXPECT_CALL (mdc3, doSetOwner (NULL));
    EXPECT_CALL (mdc3, doUpdateShadow (_)).Times (1);

    cg.pushClippable (&mdc);
    cg.pushClippable (&mdc2);
    cg.pushClippable (&mdc3);

    CompRegion accumulated;

    accumulated += mdc.inputRegion ();
    accumulated += mdc2.inputRegion ();
    accumulated += mdc3.inputRegion ();

    EXPECT_THAT (cg.clipRegion (), CompRegionEq (accumulated));

    cg.popClippable (&mdc3);

    accumulated = CompRegion ();

    accumulated += mdc.inputRegion ();
    accumulated += mdc2.inputRegion ();

    EXPECT_THAT (cg.clipRegion (), CompRegionEq (accumulated));
}

