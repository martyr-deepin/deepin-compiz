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
#ifndef _COMPIZ_OPENGL_GLX_TFP_BIND_H
#define _COMPIZ_OPENGL_GLX_TFP_BIND_H

#ifndef LP_1030891_NOT_FIXED
#define LP_1030891_NOT_FIXED
#endif

#include <opengl/pixmapsource.h>
#include <boost/function.hpp>

class ServerGrabInterface;
typedef unsigned long Pixmap;
typedef unsigned long GLXPixmap;

namespace compiz
{
    namespace opengl
    {
	typedef boost::function <bool (Pixmap)> PixmapCheckValidityFunc;
	typedef boost::function <void (GLXPixmap)> BindTexImageEXTFunc;
	typedef boost::function <void ()> WaitGLXFunc;

	bool bindTexImageGLX (ServerGrabInterface           *,
			      Pixmap,
			      GLXPixmap,
			      const PixmapCheckValidityFunc &,
			      const BindTexImageEXTFunc     &,
			      const WaitGLXFunc             &,
			      PixmapSource);
			      
    } // namespace opengl
} // namespace compiz
#endif
