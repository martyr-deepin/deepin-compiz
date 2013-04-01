/*
 * Copyright © 2008 Dennis Kasprzyk
 * Copyright © 2007 Novell, Inc.
 * Copyright © 2011 Linaro Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Dennis Kasprzyk not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Dennis Kasprzyk makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DENNIS KASPRZYK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DENNIS KASPRZYK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Dennis Kasprzyk <onestone@compiz-fusion.org>
 *          David Reveman <davidr@novell.com>
 *          Travis Watkins <travis.watkins@linaro.org>
 */

#ifndef _PRIVATETEXTURE_H
#define _PRIVATETEXTURE_H

#ifdef USE_GLES
#define SUPPORT_X11
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#else
#include <GL/gl.h>
#include <GL/glx.h>
#endif
#include <opengl/texture.h>
#include <X11/extensions/Xdamage.h>

#include <map>

class GLScreen;
class GLDisplay;

class PrivateTexture {
    public:
	PrivateTexture (GLTexture *);
	~PrivateTexture ();

	static GLTexture::List loadImageData (const char   *image,
					      unsigned int width,
					      unsigned int height,
					      GLenum       format,
					      GLenum       type);

    public:
	GLTexture         *texture;
	GLuint            name;
	GLenum            target;
	GLenum            filter;
	GLenum            wrap;
	GLTexture::Matrix matrix;
	bool              mipmap;
	bool              mipmapSupport;
	bool              initial;
	int               refCount;
};

#ifdef USE_GLES
class EglTexture : public GLTexture {
    public:
	EglTexture ();
	~EglTexture ();

	void enable (Filter filter);

	static List bindPixmapToTexture (Pixmap                       pixmap,
					 int                          width,
					 int                          height,
					 int                          depth,
					 compiz::opengl::PixmapSource source);

    public:
	bool        damaged;
	Damage      damage;
	bool        updateMipMap;
};

extern std::map<Damage, EglTexture*> boundPixmapTex;
#else

class TfpTexture : public GLTexture {
    public:
	TfpTexture ();
	~TfpTexture ();

	void enable (Filter filter);
	bool bindTexImage (const GLXPixmap &);
	void releaseTexImage ();

	static List bindPixmapToTexture (Pixmap                       pixmap,
					 int                          width,
					 int                          height,
					 int                          depth,
					 compiz::opengl::PixmapSource source);

    public:
	Pixmap                       x11Pixmap;
	GLXPixmap                    pixmap;
	bool                         damaged;
	Damage                       damage;
	bool                         updateMipMap;
	compiz::opengl::PixmapSource source;
};

extern std::map<Damage, TfpTexture*> boundPixmapTex;
#endif

#endif
