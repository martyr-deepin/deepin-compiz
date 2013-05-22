/*
 * Compiz, opengl plugin, GLX_EXT_texture_from_pixmap rebind logic
 *
 * Copyright (c) 2012 Canonical Ltd.
 * Authors: Sam Spilsbury <sam.spilsbury@canonical.com>
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
#include <opengl/pixmapsource.h>
#include <core/servergrab.h>
#include "glx-tfp-bind.h"

namespace cgl = compiz::opengl;

bool
cgl::bindTexImageGLX (ServerGrabInterface                *serverGrabInterface,
		      Pixmap                             x11Pixmap,
		      GLXPixmap                          glxPixmap,
		      const cgl::PixmapCheckValidityFunc &checkPixmapValidity,
		      const cgl::BindTexImageEXTFunc     &bindTexImageEXT,
		      const cgl::WaitGLXFunc             &waitGLX,
		      cgl::PixmapSource                  source)
{
#ifndef LP_1030891_NOT_FIXED
    ServerLock lock (serverGrabInterface);

    waitGLX ();
#endif

    /* External pixmaps can disappear on us, but not
     * while we have a server grab at least */
    if (source == cgl::ExternallyManaged)
    {
#ifdef LP_1030891_NOT_FIXED
	ServerLock lock (serverGrabInterface);
#endif
	if (!checkPixmapValidity (x11Pixmap))
	    return false;

#ifdef LP_1030891_NOT_FIXED
	// We need to bind before the above ServerLock is lost
	bindTexImageEXT (glxPixmap);
	return true;
#endif
    }

    bindTexImageEXT (glxPixmap);

    return true;
}
