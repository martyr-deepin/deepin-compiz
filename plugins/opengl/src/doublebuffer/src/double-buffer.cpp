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

char programName[] = "compiz_test_opengl_double_buffer";
bool debugOutput = false;

namespace compiz
{
namespace opengl
{

DoubleBuffer::DoubleBuffer ()
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
	swap ();
	if (setting[NEED_PERSISTENT_BACK_BUFFER] &&
	    !setting[HAVE_PERSISTENT_BACK_BUFFER])
	{
	    copyFrontToBack ();
	}
    }
    else if (blitAvailable ())
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

} // namespace opengl
} // namespace compiz
