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

#ifndef __COMPIZ_OPENGL_FSREGION_H
#define __COMPIZ_OPENGL_FSREGION_H
#include "core/rect.h"
#include "core/region.h"

namespace compiz {
namespace opengl {

class FullscreenRegion
{
public:
    typedef enum
    {
	Desktop = 1,
	Alpha = 2
    } WinFlag;

    typedef unsigned int WinFlags;

    FullscreenRegion (const CompRect &rect);

    // isCoveredBy is called for windows from TOP to BOTTOM
    bool isCoveredBy (const CompRegion &region, WinFlags flags = 0);
    bool allowRedirection (const CompRegion &region);

private:
    CompRegion untouched;
    CompRegion orig;
};

} // namespace opengl
} // namespace compiz
#endif
