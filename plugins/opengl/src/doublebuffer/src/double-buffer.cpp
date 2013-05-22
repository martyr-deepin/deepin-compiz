/*
 * Compiz, opengl plugin, DoubleBuffer class
 *
 * Copyright (c) 2012 Canonical Ltd.
 * Authors: Sam Spilsbury <sam.spilsbury@canonical.com>
 *          Daniel van Vugt <daniel.van.vugt@canonical.com>
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

#include <cstdlib>
#include <cassert>
#include "opengl/doublebuffer.h"

using namespace compiz::opengl;

namespace
{
const unsigned int UNTHROTTLED_FRAMES_MAX = 5;
}

namespace compiz
{
namespace opengl
{

DoubleBuffer::DoubleBuffer (const impl::GLXSwapIntervalEXTFunc  &swapIntervalFunc,
			    const impl::GLXWaitVideoSyncSGIFunc &waitVideoSyncFunc) :
    syncType (NoSync),
    bufferFrameThrottleState (FrameThrottledInternally),
    blockingVSyncUnthrottledFrames (0),
    swapIntervalFunc (swapIntervalFunc),
    waitVideoSyncFunc (waitVideoSyncFunc),
    lastVSyncCounter (0)
{
    setting[VSYNC] = true;
    setting[HAVE_PERSISTENT_BACK_BUFFER] = false;
    setting[NEED_PERSISTENT_BACK_BUFFER] = false;
}

DoubleBuffer::~DoubleBuffer ()
{
}

void
DoubleBuffer::set (Setting name, bool value)
{
    setting[name] = value;
}

void
DoubleBuffer::render (const CompRegion &region,
                      bool fullscreen)
{
    if (fullscreen)
    {
	if (setting[VSYNC])
	    vsync (Swap);

	swap ();

	if (setting[NEED_PERSISTENT_BACK_BUFFER] &&
	    !setting[HAVE_PERSISTENT_BACK_BUFFER])
	{
	    copyFrontToBack ();
	}
    }
    else
    {
	if (setting[VSYNC])
	    vsync (Blit);

	if (blitAvailable ())
	    blit (region);
	else if (fallbackBlitAvailable ())
	    fallbackBlit (region);
	else
	{
	    // This will never happen unless you make a porting mistake...
	    assert (false);
	    abort ();
	}
    }
}

void
DoubleBuffer::vsync (FrontbufferRedrawType redrawType)
{
    FrameThrottleState throttleState;
    SyncType           lastSyncType = syncType;

    if (enableAsyncVideoSync (redrawType, throttleState))
    {
	syncType = Async;

	if (lastSyncType == Blocking)
	    disableBlockingVideoSync ();

	/* Apply throttle */
	bufferFrameThrottleState = throttleState;
	blockingVSyncUnthrottledFrames = 0;
    }
    else if (enableBlockingVideoSync (redrawType, throttleState))
    {
	syncType = Blocking;
	if (lastSyncType == Async)
	    disableAsyncVideoSync ();

	/* Accumulate throttle */
	if (throttleState == ExternalFrameThrottlingRequired)
	    blockingVSyncUnthrottledFrames++;
	else
	    blockingVSyncUnthrottledFrames = 0;

	if (blockingVSyncUnthrottledFrames >= UNTHROTTLED_FRAMES_MAX)
	    bufferFrameThrottleState = ExternalFrameThrottlingRequired;
	else
	    bufferFrameThrottleState = FrameThrottledInternally;
    }
    else
    {
	syncType = NoSync;
	/* Throttle all rendering */
	bufferFrameThrottleState = ExternalFrameThrottlingRequired;
	blockingVSyncUnthrottledFrames = 0;
    }
}

bool
DoubleBuffer::hardwareVSyncFunctional ()
{
    return bufferFrameThrottleState == FrameThrottledInternally;
}

bool
DoubleBuffer::enableAsyncVideoSync (FrontbufferRedrawType swapType, FrameThrottleState &throttleState)
{
    /* Always consider these frames as un-throttled as the buffer
     * swaps are done asynchronously */
    throttleState = ExternalFrameThrottlingRequired;

    /* Can't use swapInterval unless using SwapBuffers */
    if (swapType != Swap)
	return false;

    /* Enable if not enabled */
    if (syncType != Async)
	swapIntervalFunc (1);

    return true;
}

void
DoubleBuffer::disableAsyncVideoSync ()
{
    /* Disable if enabled */
    swapIntervalFunc (0);
}

bool
DoubleBuffer::enableBlockingVideoSync (FrontbufferRedrawType swapType, FrameThrottleState &throttleState)
{
    unsigned int oldVideoSyncCounter = lastVSyncCounter;
    waitVideoSyncFunc (1, 0, &lastVSyncCounter);

    /* Check if this frame was actually throttled */
    if (lastVSyncCounter == oldVideoSyncCounter)
	throttleState = ExternalFrameThrottlingRequired;
    else
	throttleState = FrameThrottledInternally;

    return true;
}

void
DoubleBuffer::disableBlockingVideoSync ()
{
    blockingVSyncUnthrottledFrames = 0;
}

} // namespace opengl
} // namespace compiz
