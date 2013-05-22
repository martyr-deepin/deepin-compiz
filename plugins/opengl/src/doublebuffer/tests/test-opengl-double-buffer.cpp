#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <opengl/doublebuffer.h>

using namespace compiz::opengl;
using testing::_;
using testing::StrictMock;
using testing::Return;
using testing::DoAll;
using testing::SetArgReferee;
using testing::SetArgPointee;
using testing::InSequence;

char programName[] = "compiz_test_opengl_double_buffer";
bool debugOutput = false;

class MockDoubleBuffer :
    public DoubleBuffer
{
    public:

	MockDoubleBuffer (const impl::GLXSwapIntervalEXTFunc  &swapIntervalFunc,
			  const impl::GLXWaitVideoSyncSGIFunc &waitVideoSyncFunc) :
	    DoubleBuffer (swapIntervalFunc, waitVideoSyncFunc)
	{
	}

	MOCK_CONST_METHOD0 (swap, void ());
	MOCK_CONST_METHOD0 (blitAvailable, bool ());
	MOCK_CONST_METHOD1 (blit, void (const CompRegion &));
	MOCK_CONST_METHOD0 (fallbackBlitAvailable, bool ());
	MOCK_CONST_METHOD1 (fallbackBlit, void (const CompRegion &));
	MOCK_CONST_METHOD0 (copyFrontToBack, void ());
};

class MockVSyncDoubleBuffer :
    public MockDoubleBuffer
{
    public:

	static void stubSwapInterval (int) {}
	static int  stubWaitVideoSync (int, int, unsigned int *) { return 1; }

	MockVSyncDoubleBuffer () :
	    MockDoubleBuffer (boost::bind (stubSwapInterval, _1),
			      boost::bind (stubWaitVideoSync, _1, _2, _3))
	{
	}

	MOCK_METHOD2 (enableAsyncVideoSync, bool (FrontbufferRedrawType, FrameThrottleState &));
	MOCK_METHOD2 (enableBlockingVideoSync, bool (FrontbufferRedrawType, FrameThrottleState &));
	MOCK_METHOD0 (disableAsyncVideoSync, void ());
	MOCK_METHOD0 (disableBlockingVideoSync, void ());
};

class DoubleBufferTest :
    public ::testing::Test
{
    public:

	MockVSyncDoubleBuffer        db;
	CompRegion	             blitRegion;

};

class CompizOpenGLDoubleBufferDeathTest :
    public DoubleBufferTest
{
};

TEST_F(DoubleBufferTest, TestPaintedFullAlwaysSwaps)
{
    EXPECT_CALL (db, swap ());
    EXPECT_CALL (db, enableAsyncVideoSync (DoubleBuffer::Swap, _))
	    .WillOnce (Return (true));
    EXPECT_CALL (db, copyFrontToBack ()).Times (0);

    db.render (blitRegion, true);
}

TEST_F(DoubleBufferTest, TestNoPaintedFullscreenOrFBOAlwaysBlitsSubBuffer)
{
    EXPECT_CALL (db, blitAvailable ()).WillOnce (Return (true));
    EXPECT_CALL (db, blit (_));
    EXPECT_CALL (db, enableAsyncVideoSync (DoubleBuffer::Blit, _))
	    .WillOnce (Return (false));
    EXPECT_CALL (db, enableBlockingVideoSync (DoubleBuffer::Blit, _))
	    .WillOnce (Return (true));
    EXPECT_CALL (db, copyFrontToBack ()).Times (0);

    db.render (blitRegion, false);
}

TEST_F(DoubleBufferTest, SwapWithoutFBO)
{
    db.set (DoubleBuffer::HAVE_PERSISTENT_BACK_BUFFER, false);
    db.set (DoubleBuffer::NEED_PERSISTENT_BACK_BUFFER, true);

    EXPECT_CALL (db, swap ());
    EXPECT_CALL (db, enableAsyncVideoSync (DoubleBuffer::Swap, _))
	    .WillOnce (Return (true));
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
    EXPECT_CALL (db, enableAsyncVideoSync (DoubleBuffer::Blit, _))
	    .WillOnce (Return (false));
    EXPECT_CALL (db, enableBlockingVideoSync (DoubleBuffer::Blit, _))
	    .WillOnce (Return (true));
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
    EXPECT_CALL (db, enableAsyncVideoSync (DoubleBuffer::Blit, _))
	    .WillRepeatedly (Return (false));
    EXPECT_CALL (db, enableBlockingVideoSync (DoubleBuffer::Blit, _))
	    .WillRepeatedly (Return (true));

    EXPECT_CALL (db, blit (r1));
    db.render (r1, false);

    EXPECT_CALL (db, blit (r2));
    db.render (r2, false);

    EXPECT_CALL (db, blit (r3));
    db.render (r3, false);
}

TEST_F(CompizOpenGLDoubleBufferDeathTest, TestNoPaintedFullscreenOrFBODoesNotBlitOrCopyIfNotSupportedAndDies)
{
    StrictMock <MockVSyncDoubleBuffer> dbStrict;

    ON_CALL (dbStrict, blitAvailable ()).WillByDefault (Return (false));
    ON_CALL (dbStrict, fallbackBlitAvailable ()).WillByDefault (Return (false));

    ASSERT_DEATH ({
		    dbStrict.render (blitRegion, false);
		  },
		  ".*");
}

TEST_F(DoubleBufferTest, TestSubBufferCopyIfNoFBOAndNoSubBufferBlit)
{
    StrictMock <MockVSyncDoubleBuffer> dbStrict;

    EXPECT_CALL (dbStrict, blitAvailable ()).WillOnce (Return (false));
    EXPECT_CALL (dbStrict, fallbackBlitAvailable ()).WillOnce (Return (true));
    EXPECT_CALL (dbStrict, enableAsyncVideoSync (DoubleBuffer::Blit, _))
	    .WillOnce (Return (false));
    EXPECT_CALL (dbStrict, enableBlockingVideoSync (DoubleBuffer::Blit, _))
	    .WillOnce (Return (true));
    EXPECT_CALL (dbStrict, fallbackBlit (blitRegion));

    dbStrict.render (blitRegion, false);
}

TEST_F(DoubleBufferTest, TestCallWorkingStrategy)
{
    EXPECT_CALL (db, enableAsyncVideoSync (DoubleBuffer::Swap, _))
	    .WillOnce (Return (true));

    db.vsync (DoubleBuffer::Swap);
}

TEST_F(DoubleBufferTest, TestCallNextWorkingStrategy)
{
    /* This one fails */
    EXPECT_CALL (db, enableAsyncVideoSync (DoubleBuffer::Blit, _))
	    .WillOnce (Return (false));
    /* Try the next one */
    EXPECT_CALL (db, enableBlockingVideoSync (DoubleBuffer::Blit, _))
	    .WillOnce (Return (true));

    db.vsync (DoubleBuffer::Blit);
}

TEST_F(DoubleBufferTest, TestCallPrevCallNextPrevDeactivated)
{
    /* This one fails */
    EXPECT_CALL (db, enableAsyncVideoSync (DoubleBuffer::Blit, _))
	    .WillOnce (Return (false));
    /* Try the next one */
    EXPECT_CALL (db, enableBlockingVideoSync (DoubleBuffer::Blit, _))
	    .WillOnce (Return (true));

    db.vsync (DoubleBuffer::Blit);

    EXPECT_CALL (db, enableAsyncVideoSync (DoubleBuffer::Blit, _))
	    .WillOnce (Return (true));
    /* Previous one must be deactivated */
    EXPECT_CALL (db, disableBlockingVideoSync ());

    db.vsync (DoubleBuffer::Blit);
}

TEST_F(DoubleBufferTest, TestReportNoHardwareVSyncIfMoreThan5UnthrottledFrames)
{
    /* This one succeeds but fails to throttle */
    for (unsigned int i = 0; i < 5; ++i)
    {
	EXPECT_CALL (db, enableAsyncVideoSync (DoubleBuffer::Blit, _))
		    .WillOnce (Return (false));
	EXPECT_CALL (db, enableBlockingVideoSync (DoubleBuffer::Blit, _))
	    .WillOnce (DoAll (SetArgReferee <1> (DoubleBuffer::ExternalFrameThrottlingRequired),
				      Return (true)));

	db.vsync (DoubleBuffer::Blit);
    }

    EXPECT_FALSE (db.hardwareVSyncFunctional ());
}

TEST_F(DoubleBufferTest, TestRestoreReportHardwareVSync)
{
    /* This one succeeds but fails to throttle */
    for (unsigned int i = 0; i < 5; ++i)
    {
	EXPECT_CALL (db, enableAsyncVideoSync (DoubleBuffer::Blit, _))
		    .WillOnce (Return (false));
	EXPECT_CALL (db, enableBlockingVideoSync (DoubleBuffer::Blit, _))
	    .WillOnce (DoAll (SetArgReferee <1> (DoubleBuffer::ExternalFrameThrottlingRequired),
				      Return (true)));

	EXPECT_TRUE (db.hardwareVSyncFunctional ());

	db.vsync (DoubleBuffer::Blit);
    }

    EXPECT_FALSE (db.hardwareVSyncFunctional ());

    /* It works again */
    EXPECT_CALL (db, enableAsyncVideoSync (DoubleBuffer::Blit, _))
	    .WillOnce (Return (false));
    EXPECT_CALL (db, enableBlockingVideoSync (DoubleBuffer::Blit, _))
	.WillOnce (DoAll (SetArgReferee <1> (DoubleBuffer::FrameThrottledInternally),
			      Return (true)));

    db.vsync (DoubleBuffer::Blit);

    /* And should report to work for another 5 bad frames */
    for (unsigned int i = 0; i < 5; ++i)
    {
	EXPECT_CALL (db, enableAsyncVideoSync (DoubleBuffer::Blit, _))
		    .WillOnce (Return (false));
	EXPECT_CALL (db, enableBlockingVideoSync (DoubleBuffer::Blit, _))
	    .WillOnce (DoAll (SetArgReferee <1> (DoubleBuffer::ExternalFrameThrottlingRequired),
				      Return (true)));

	EXPECT_TRUE (db.hardwareVSyncFunctional ());

	db.vsync (DoubleBuffer::Blit);
    }

    EXPECT_FALSE (db.hardwareVSyncFunctional ());
}

namespace
{
class MockOpenGLFunctionsTable
{
    public:

	MOCK_METHOD3 (waitVideoSyncSGI, int (int, int, unsigned int *));
	MOCK_METHOD1 (swapIntervalEXT, void (int));
};

namespace cgl = compiz::opengl;
namespace cgli = compiz::opengl::impl;

cgli::GLXWaitVideoSyncSGIFunc
GetWaitVideoSyncFuncFromMock (MockOpenGLFunctionsTable &mock)
{
    return boost::bind (&MockOpenGLFunctionsTable::waitVideoSyncSGI, &mock, _1, _2, _3);
}

cgli::GLXSwapIntervalEXTFunc
GetSwapIntervalFuncFromMock (MockOpenGLFunctionsTable &mock)
{
    return boost::bind (&MockOpenGLFunctionsTable::swapIntervalEXT, &mock, _1);
}
}

class OpenGLVideoSyncTest :
    public ::testing::Test
{
    public:

	OpenGLVideoSyncTest () :
	    doubleBuffer (GetSwapIntervalFuncFromMock (functions),
			  GetWaitVideoSyncFuncFromMock (functions))
	{
	}

	MockDoubleBuffer         doubleBuffer;
	MockOpenGLFunctionsTable functions;
};

TEST_F (OpenGLVideoSyncTest, TestCallSwapIntervalOnVSyncForFlip)
{
    EXPECT_CALL (functions, swapIntervalEXT (1));
    doubleBuffer.vsync (cgl::DoubleBuffer::Swap);
}

TEST_F (OpenGLVideoSyncTest, TestCallSwapIntervalOnEnableForFlipOnlyOnce)
{
    EXPECT_CALL (functions, swapIntervalEXT (1)).Times (1);
    doubleBuffer.vsync (cgl::DoubleBuffer::Swap);
    doubleBuffer.vsync (cgl::DoubleBuffer::Swap);
}

TEST_F (OpenGLVideoSyncTest, TestCallSwapIntervalOnEnableForFlipAndZeroForDisable)
{
    EXPECT_CALL (functions, swapIntervalEXT (1));
    doubleBuffer.vsync (cgl::DoubleBuffer::Swap);
    EXPECT_CALL (functions, swapIntervalEXT (0));
    EXPECT_CALL (functions, waitVideoSyncSGI (1, 0, _));
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
}

TEST_F (OpenGLVideoSyncTest, TestCallSwapIntervalZeroForDisableOnce)
{
    /* Enable it */
    EXPECT_CALL (functions, swapIntervalEXT (1)).Times (1);
    doubleBuffer.vsync (cgl::DoubleBuffer::Swap);

    /* Disable it twice */
    EXPECT_CALL (functions, swapIntervalEXT (0)).Times (1);
    EXPECT_CALL (functions, waitVideoSyncSGI (1, 0, _)).Times (2);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
}

TEST_F (OpenGLVideoSyncTest, TestCallSwapIntervalFailsToEnableForCopy)
{
    EXPECT_CALL (functions, swapIntervalEXT (1)).Times (0);
    EXPECT_CALL (functions, waitVideoSyncSGI (1, _, _));
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
}

TEST_F (OpenGLVideoSyncTest, TestCallSwapIntervalUnthrottledWhereSuccess)
{
    EXPECT_CALL (functions, swapIntervalEXT (1));

    /* At the moment there's no way to test except for the general throttled method */
    doubleBuffer.vsync (cgl::DoubleBuffer::Swap);

    EXPECT_FALSE (doubleBuffer.hardwareVSyncFunctional ());
}

TEST_F (OpenGLVideoSyncTest, TestCallsGetVideoSyncAndWaitVideoSyncForCopy)
{
    EXPECT_CALL (functions, waitVideoSyncSGI (_, _, _));
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
}

TEST_F (OpenGLVideoSyncTest, TestCallsWaitVideoSyncAndThrottled)
{
    /* Frames 1-5 */
    ON_CALL (functions, waitVideoSyncSGI (1, _, _)).WillByDefault (DoAll (SetArgPointee<2> (0),
									  Return (0)));
    EXPECT_CALL (functions, waitVideoSyncSGI (1, _, _)).Times (5);
    /* Returned next frame, this frame was throttled */
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    EXPECT_FALSE (doubleBuffer.hardwareVSyncFunctional ());
}

TEST_F (OpenGLVideoSyncTest, TestCallsWaitVideoSyncAndThrottledEveryFrame)
{
    InSequence s;

    /* Frame 0 to frame 1 */
    EXPECT_CALL (functions, waitVideoSyncSGI (1, _, _)).WillOnce (DoAll (SetArgPointee<2> (1),
									 Return (0)));
    /* Frame 1 to frame 2 */
    EXPECT_CALL (functions, waitVideoSyncSGI (1, _, _)).WillOnce (DoAll (SetArgPointee<2> (2),
									 Return (0)));
    /* Frame 2 to frame 3 */
    EXPECT_CALL (functions, waitVideoSyncSGI (1, _, _)).WillOnce (DoAll (SetArgPointee<2> (3),
									 Return (0)));
    /* Frame 3 to frame 4 */
    EXPECT_CALL (functions, waitVideoSyncSGI (1, _, _)).WillOnce (DoAll (SetArgPointee<2> (4),
									 Return (0)));
    /* Frame 5 to frame 5 */
    EXPECT_CALL (functions, waitVideoSyncSGI (1, _, _)).WillOnce (DoAll (SetArgPointee<2> (5),
									 Return (0)));
    /* Returned next frame, this frame was throttled */
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    EXPECT_TRUE (doubleBuffer.hardwareVSyncFunctional ());
}

TEST_F (OpenGLVideoSyncTest, TestCallsWaitVideoSyncAndUnthrottledDueToBrokenWaitVSync)
{
    /* Frames 0 to 5 */
    ON_CALL (functions, waitVideoSyncSGI (1, _, _)).WillByDefault (DoAll (SetArgPointee<2> (0),
									  Return (0)));
    EXPECT_CALL (functions, waitVideoSyncSGI (1, _, _)).Times (5);
    /* Returned next frame, this frame was not throttled */
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    EXPECT_FALSE (doubleBuffer.hardwareVSyncFunctional ());

    InSequence s;

    /* Frame 0 to frame 1 */
    EXPECT_CALL (functions, waitVideoSyncSGI (1, _, _)).WillOnce (DoAll (SetArgPointee<2> (1),
									 Return (0)));
    /* Frame 1 to frame 2 */
    EXPECT_CALL (functions, waitVideoSyncSGI (1, _, _)).WillOnce (DoAll (SetArgPointee<2> (2),
									 Return (0)));
    /* Frame 2 to frame 3 */
    EXPECT_CALL (functions, waitVideoSyncSGI (1, _, _)).WillOnce (DoAll (SetArgPointee<2> (3),
									 Return (0)));
    /* Frame 3 to frame 4 */
    EXPECT_CALL (functions, waitVideoSyncSGI (1, _, _)).WillOnce (DoAll (SetArgPointee<2> (4),
									 Return (0)));
    /* Frame 5 to frame 5 (eg, working waitVideoSyncSGI) */
    EXPECT_CALL (functions, waitVideoSyncSGI (1, _, _)).WillOnce (DoAll (SetArgPointee<2> (5),
									 Return (0)));
    /* Returned next frame, this frame was throttled */
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    doubleBuffer.vsync (cgl::DoubleBuffer::Blit);
    EXPECT_TRUE (doubleBuffer.hardwareVSyncFunctional ());
}
