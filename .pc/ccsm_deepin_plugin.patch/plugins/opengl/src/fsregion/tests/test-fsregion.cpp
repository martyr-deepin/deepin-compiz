/*
 * Compiz opengl plugin, FullscreenRegion class
 *
 * Copyright (c) 2012 Canonical Ltd.
 * Author: Daniel van Vugt <daniel.van.vugt@canonical.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "gtest/gtest.h"
#include "fsregion.h"

using namespace compiz::opengl;

TEST (OpenGLFullscreenRegion, NoWindows)
{
    FullscreenRegion monitor (CompRect (0, 0, 1024, 768));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768),
                                       FullscreenRegion::Desktop));
}

TEST (OpenGLFullscreenRegion, OneFullscreen)
{
    FullscreenRegion monitor (CompRect (0, 0, 1024, 768));
    EXPECT_TRUE  (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768),
                                       FullscreenRegion::Desktop));
}

TEST (OpenGLFullscreenRegion, FullscreenNoDesktop)
{
    FullscreenRegion monitor (CompRect (0, 0, 1024, 768));
    EXPECT_TRUE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768)));
}

TEST (OpenGLFullscreenRegion, AlphaFullscreen)
{
    FullscreenRegion monitor (CompRect (0, 0, 1024, 768));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768),
                                       FullscreenRegion::Alpha));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768),
                                       FullscreenRegion::Desktop));
}

TEST (OpenGLFullscreenRegion, AlphaOverFullscreen)
{
    FullscreenRegion monitor (CompRect (0, 0, 1024, 768));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (50, 60, 70, 80),
                                       FullscreenRegion::Alpha));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768),
                                       FullscreenRegion::Desktop));
}

TEST (OpenGLFullscreenRegion, NormalWindows)
{
    FullscreenRegion monitor (CompRect (0, 0, 1024, 768));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (10, 10, 40, 30)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (20, 20, 50, 20)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768),
                                       FullscreenRegion::Desktop));
}

TEST (OpenGLFullscreenRegion, TwoFullscreen)
{
    FullscreenRegion monitor (CompRect (0, 0, 1024, 768));
    EXPECT_TRUE  (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (10, 10, 40, 30)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (20, 20, 50, 20)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768),
                                       FullscreenRegion::Desktop));
}

TEST (OpenGLFullscreenRegion, Offscreen)
{
    FullscreenRegion monitor (CompRect (0, 0, 1024, 768));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (-100, -100, 1, 1)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (2000, 2000, 123, 456)));
    EXPECT_TRUE  (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (10, 10, 40, 30)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (20, 20, 50, 20)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768),
                                       FullscreenRegion::Desktop));
}

TEST (OpenGLFullscreenRegion, CancelFullscreen1)
{
    FullscreenRegion monitor (CompRect (0, 0, 1024, 768));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (500, 500, 345, 234)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (10, 10, 40, 30)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (20, 20, 50, 20)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768),
                                       FullscreenRegion::Desktop));
}

TEST (OpenGLFullscreenRegion, CancelFullscreen2)
{
    FullscreenRegion monitor (CompRect (0, 0, 1024, 768));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (-100, -100, 1, 1)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (2000, 2000, 123, 456)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (500, 500, 345, 234)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (10, 10, 40, 30)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (20, 20, 50, 20)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768),
                                       FullscreenRegion::Desktop));
}

TEST (OpenGLFullscreenRegion, Overflow)
{
    FullscreenRegion monitor (CompRect (0, 0, 1024, 768));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (10, 10, 40, 30)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (-10, -10, 1044, 788)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768),
                                       FullscreenRegion::Desktop));
}

TEST (OpenGLFullscreenRegion, KeepUnredirectedStateIfNotOnMonitor)
{
    FullscreenRegion monitor (CompRect (0, 0, 1024, 768));
    CompRegion       window (1025, 0, 1024, 768);
    /* Eg, not covering the monitor, should be redirected */
    EXPECT_FALSE (monitor.isCoveredBy (window));
    /* Don't allow the redirection however, because we weren't
     * covering the monitor at all. */
    EXPECT_FALSE (monitor.allowRedirection (window));
}

TEST (OpenGLFullscreenRegion, MaximizedWithDocks)  // LP: #1053902
{
    FullscreenRegion monitor (CompRect (0, 0, 1024, 768));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 24)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 24, 64, 744)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (64, 24, 960, 744)));
    EXPECT_FALSE (monitor.isCoveredBy (CompRegion (0, 0, 1024, 768),
                                       FullscreenRegion::Desktop));
}

