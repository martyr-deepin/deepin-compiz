#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <opengl/doublebuffer.h>

using namespace compiz::opengl;
using testing::_;
using testing::StrictMock;
using testing::Return;

class MockDoubleBuffer :
    public DoubleBuffer
{
    public:

	MOCK_CONST_METHOD0 (swap, void ());
	MOCK_CONST_METHOD0 (blitAvailable, bool ());
	MOCK_CONST_METHOD1 (blit, void (const CompRegion &));
	MOCK_CONST_METHOD0 (fallbackBlitAvailable, bool ());
	MOCK_CONST_METHOD1 (fallbackBlit, void (const CompRegion &));
	MOCK_CONST_METHOD0 (copyFrontToBack, void ());
};

class DoubleBufferTest :
    public ::testing::Test
{
    public:

	MockDoubleBuffer db;
	CompRegion	 blitRegion;

};

class CompizOpenGLDoubleBufferDeathTest :
    public DoubleBufferTest
{
};

TEST_F(DoubleBufferTest, TestPaintedFullAlwaysSwaps)
{
    EXPECT_CALL (db, swap ());
    EXPECT_CALL (db, copyFrontToBack ()).Times (0);

    db.render (blitRegion, true);
}

TEST_F(DoubleBufferTest, TestNoPaintedFullscreenOrFBOAlwaysBlitsSubBuffer)
{
    EXPECT_CALL (db, blitAvailable ()).WillOnce (Return (true));
    EXPECT_CALL (db, blit (_));
    EXPECT_CALL (db, copyFrontToBack ()).Times (0);

    db.render (blitRegion, false);
}

TEST_F(DoubleBufferTest, SwapWithoutFBO)
{
    db.set (DoubleBuffer::HAVE_PERSISTENT_BACK_BUFFER, false);
    db.set (DoubleBuffer::NEED_PERSISTENT_BACK_BUFFER, true);

    EXPECT_CALL (db, swap ());
    EXPECT_CALL (db, copyFrontToBack ()).Times (1);

    db.render (blitRegion, true);
}

TEST_F(DoubleBufferTest, BlitWithoutFBO)
{
    db.set (DoubleBuffer::HAVE_PERSISTENT_BACK_BUFFER, false);
    db.set (DoubleBuffer::NEED_PERSISTENT_BACK_BUFFER, false);

    EXPECT_CALL (db, blitAvailable ()).WillRepeatedly (Return (true));
    EXPECT_CALL (db, blit (_));
    EXPECT_CALL (db, swap ()).Times (0);
    EXPECT_CALL (db, copyFrontToBack ()).Times (0);

    db.render (blitRegion, false);
}

TEST_F(DoubleBufferTest, TestNoPaintedFullscreenOrFBODoesNotBlitIfNotSupported)
{

}

TEST_F(DoubleBufferTest, TestBlitExactlyWithRegionSpecified)
{
    CompRegion r1 (0, 0, 100, 100);
    CompRegion r2 (100, 100, 100, 100);
    CompRegion r3 (200, 200, 100, 100);

    EXPECT_CALL (db, blitAvailable ()).WillRepeatedly (Return (true));

    EXPECT_CALL (db, blit (r1));
    db.render (r1, false);

    EXPECT_CALL (db, blit (r2));
    db.render (r2, false);

    EXPECT_CALL (db, blit (r3));
    db.render (r3, false);
}

TEST_F(CompizOpenGLDoubleBufferDeathTest, TestNoPaintedFullscreenOrFBODoesNotBlitOrCopyIfNotSupportedAndDies)
{
    StrictMock <MockDoubleBuffer> dbStrict;

    ON_CALL (dbStrict, blitAvailable ()).WillByDefault (Return (false));
    ON_CALL (dbStrict, fallbackBlitAvailable ()).WillByDefault (Return (false));

    ASSERT_DEATH ({
		    dbStrict.render (blitRegion, false);
		  },
		  ".*");
}

TEST_F(DoubleBufferTest, TestSubBufferCopyIfNoFBOAndNoSubBufferBlit)
{
    StrictMock <MockDoubleBuffer> dbStrict;

    EXPECT_CALL (dbStrict, blitAvailable ()).WillOnce (Return (false));
    EXPECT_CALL (dbStrict, fallbackBlitAvailable ()).WillOnce (Return (true));
    EXPECT_CALL (dbStrict, fallbackBlit (blitRegion));

    dbStrict.render (blitRegion, false);
}
