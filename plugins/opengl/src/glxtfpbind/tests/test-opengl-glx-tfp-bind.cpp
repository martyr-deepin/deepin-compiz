#include <boost/bind.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <core/servergrab.h>
#include "glx-tfp-bind.h"

using ::testing::InSequence;
using ::testing::NiceMock;
using ::testing::StrictMock;
using ::testing::Return;

namespace cgl = compiz::opengl;

namespace
{
    const Pixmap pixmap = 1;
    const GLXPixmap glxPixmap = 2;

    void emptyWaitGLX () {}
    bool emptyCheckPixmap (Pixmap p) { return true; }
    void emptyBindTexImage (GLXPixmap p) {}

    cgl::WaitGLXFunc waitGLX () { return boost::bind (emptyWaitGLX); }
    cgl::PixmapCheckValidityFunc pixmapCheckValidity () { return boost::bind (emptyCheckPixmap, _1); }
    cgl::BindTexImageEXTFunc bindTexImageEXT () { return boost::bind (emptyBindTexImage, _1); }
}

class MockWaitGLX
{
    public:

	MOCK_METHOD0 (waitGLX, void ());
};

class MockPixmapCheckValidity
{
    public:

	MOCK_METHOD1 (checkValidity, bool (Pixmap));
};

class MockBindTexImageEXT
{
    public:

	MOCK_METHOD1 (bindTexImageEXT, void (GLXPixmap));
};

class MockServerGrab :
    public ServerGrabInterface
{
    public:

	MOCK_METHOD0 (grabServer, void ());
	MOCK_METHOD0 (syncServer, void ());
	MOCK_METHOD0 (ungrabServer, void ());
};

TEST (CompizOpenGLGLXTextureFromPixmapBindTest, TestTakesServerGrab)
{
    MockServerGrab mockServerGrab;
    InSequence     s;
#ifndef LP_1030891_NOT_FIXED
    EXPECT_CALL (mockServerGrab, grabServer ());
    EXPECT_CALL (mockServerGrab, syncServer ());
    EXPECT_CALL (mockServerGrab, ungrabServer ());
    EXPECT_CALL (mockServerGrab, syncServer ());
#endif

    cgl::bindTexImageGLX (&mockServerGrab,
			  pixmap,
			  glxPixmap,
			  pixmapCheckValidity (),
			  bindTexImageEXT (),
			  waitGLX (),
			  cgl::InternallyManaged);
}

#ifdef LP_1030891_NOT_FIXED
TEST (CompizOpenGLGLXTextureFromPixmapBindTest, TestTakesServerGrabLP1030891SpecialCase)
{
    MockServerGrab mockServerGrab;
    StrictMock <MockPixmapCheckValidity> mockPixmapCheck;
    StrictMock <MockBindTexImageEXT>     mockBindTexImage;

    cgl::PixmapCheckValidityFunc         pixmapCheckFunc (boost::bind (&MockPixmapCheckValidity::checkValidity, &mockPixmapCheck, _1));
    cgl::BindTexImageEXTFunc             bindTexImageEXTFunc (boost::bind (&MockBindTexImageEXT::bindTexImageEXT, &mockBindTexImage, _1));

    EXPECT_CALL (mockServerGrab, grabServer ());
    EXPECT_CALL (mockServerGrab, syncServer ()).Times (2);
    EXPECT_CALL (mockServerGrab, ungrabServer ());
    EXPECT_CALL (mockPixmapCheck, checkValidity (pixmap)).WillOnce (Return (true));
    EXPECT_CALL (mockBindTexImage, bindTexImageEXT (glxPixmap));
    cgl::bindTexImageGLX (&mockServerGrab,
			  pixmap,
			  glxPixmap,
			  pixmapCheckFunc,
			  bindTexImageEXTFunc,
			  waitGLX (),
			  cgl::ExternallyManaged);
}
#endif

TEST (CompizOpenGLGLXTextureFromPixmapBindTest, TestCallsWaitGLX)
{
    NiceMock <MockServerGrab> mockServerGrab;
    MockWaitGLX               mockWaitGLX;

    cgl::WaitGLXFunc          waitGLXFuncMock (boost::bind (&MockWaitGLX::waitGLX, &mockWaitGLX));

#ifndef LP_1030891_NOT_FIXED
    EXPECT_CALL (mockWaitGLX, waitGLX ());
#endif

    cgl::bindTexImageGLX (&mockServerGrab,
			  pixmap,
			  glxPixmap,
			  pixmapCheckValidity (),
			  bindTexImageEXT (),
			  waitGLXFuncMock,
			  cgl::InternallyManaged);
}

TEST (CompizOpenGLGLXTextureFromPixmapBindTest, TestNoCallToCheckValidityIfInternalAndImmediateBind)
{
    NiceMock <MockServerGrab>            mockServerGrab;
    StrictMock <MockPixmapCheckValidity> mockPixmapCheck;
    StrictMock <MockBindTexImageEXT>     mockBindTexImage;

    cgl::PixmapCheckValidityFunc         pixmapCheckFunc (boost::bind (&MockPixmapCheckValidity::checkValidity, &mockPixmapCheck, _1));
    cgl::BindTexImageEXTFunc             bindTexImageEXTFunc (boost::bind (&MockBindTexImageEXT::bindTexImageEXT, &mockBindTexImage, _1));

    EXPECT_CALL (mockBindTexImage, bindTexImageEXT (glxPixmap));

    EXPECT_TRUE (cgl::bindTexImageGLX (&mockServerGrab,
				       pixmap,
				       glxPixmap,
				       pixmapCheckFunc,
				       bindTexImageEXTFunc,
				       waitGLX (),
				       cgl::InternallyManaged));
}

TEST (CompizOpenGLGLXTextureFromPixmapBindTest, TestCheckValidityIfExternalNoBindIfInvalid)
{
    NiceMock <MockServerGrab>            mockServerGrab;
    StrictMock <MockPixmapCheckValidity> mockPixmapCheck;
    StrictMock <MockBindTexImageEXT>     mockBindTexImage;

    cgl::PixmapCheckValidityFunc         pixmapCheckFunc (boost::bind (&MockPixmapCheckValidity::checkValidity, &mockPixmapCheck, _1));
    cgl::BindTexImageEXTFunc             bindTexImageEXTFunc (boost::bind (&MockBindTexImageEXT::bindTexImageEXT, &mockBindTexImage, _1));

    EXPECT_CALL (mockPixmapCheck, checkValidity (pixmap)).WillOnce (Return (false));

    EXPECT_FALSE (cgl::bindTexImageGLX (&mockServerGrab,
					pixmap,
					glxPixmap,
					pixmapCheckFunc,
					bindTexImageEXTFunc,
					waitGLX (),
					cgl::ExternallyManaged));
}

TEST (CompizOpenGLGLXTextureFromPixmapBindTest, TestCheckValidityIfExternalBindIfValid)
{
    NiceMock <MockServerGrab>            mockServerGrab;
    StrictMock <MockPixmapCheckValidity> mockPixmapCheck;
    StrictMock <MockBindTexImageEXT>     mockBindTexImage;

    cgl::PixmapCheckValidityFunc         pixmapCheckFunc (boost::bind (&MockPixmapCheckValidity::checkValidity, &mockPixmapCheck, _1));
    cgl::BindTexImageEXTFunc             bindTexImageEXTFunc (boost::bind (&MockBindTexImageEXT::bindTexImageEXT, &mockBindTexImage, _1));

    EXPECT_CALL (mockPixmapCheck, checkValidity (pixmap)).WillOnce (Return (true));
    EXPECT_CALL (mockBindTexImage, bindTexImageEXT (glxPixmap));

    EXPECT_TRUE (cgl::bindTexImageGLX (&mockServerGrab,
				       pixmap,
				       glxPixmap,
				       pixmapCheckFunc,
				       bindTexImageEXTFunc,
				       waitGLX (),
				       cgl::ExternallyManaged));
}
