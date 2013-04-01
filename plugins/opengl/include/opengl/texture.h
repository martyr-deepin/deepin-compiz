/*
 * Copyright © 2008 Dennis Kasprzyk
 * Copyright © 2007 Novell, Inc.
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
 */

#ifndef _GLTEXTURE_H
#define _GLTEXTURE_H

#include "core/region.h"
#include "core/string.h"

#include <X11/Xlib-xcb.h>

#ifdef USE_GLES
#include <GLES2/gl2.h>
#else
#include <GL/gl.h>
#endif

#include <boost/function.hpp>

#include <vector>

#include <opengl/pixmapsource.h>


#define POWER_OF_TWO(v) ((v & (v - 1)) == 0)

/**
 * Returns a 2D matrix adjusted texture co-ordinate x
 */
#define COMP_TEX_COORD_X(m, vx) ((m).xx * (vx) + (m).x0)
/**
 * Returns a 2D matrix adjusted texture co-ordinate y
 */
#define COMP_TEX_COORD_Y(m, vy) ((m).yy * (vy) + (m).y0)

/**
 * Returns a 2D matrix adjusted texture co-ordinate xy
 */
#define COMP_TEX_COORD_XY(m, vx, vy)		\
    ((m).xx * (vx) + (m).xy * (vy) + (m).x0)
/**
 * Returns a 2D matrix adjusted texture co-ordinate yx
 */
#define COMP_TEX_COORD_YX(m, vx, vy)		\
    ((m).yx * (vx) + (m).yy * (vy) + (m).y0)

class PrivateTexture;

/**
 * Class which represents an openGL texture
 */
class GLTexture : public CompRect {
    public:

	typedef enum {
	    Fast,
	    Good
	} Filter;

	typedef struct {
	    float xx; float yx;
	    float xy; float yy;
	    float x0; float y0;
	} Matrix;

	typedef std::vector<Matrix> MatrixList;

	/**
	 * Class which represents a list of openGL textures,
	 * usually used for texture tiling
	 */
	class List : public std::vector <GLTexture *> {

	    public:
		List ();
		List (unsigned int);
		List (const List &);
		~List ();

		List & operator= (const List &);

		void clear ();
	};

	typedef boost::function<List (Pixmap, int, int, int, compiz::opengl::PixmapSource)> BindPixmapProc;
	typedef unsigned int BindPixmapHandle;

    public:

	/**
	 * Returns the openGL texture name
	 */
	GLuint name () const;

	/**
	 * Returns the openGL texture target
	 */
	GLenum target () const;

	/**
	 * Returns the openGL texture filter
	 */
	GLenum filter () const;

	/**
	 * Returns a 2D 2x3 matrix describing the transformation of
	 * the texture
	 */
	const Matrix & matrix () const;

	/**
	 * Establishes the texture as the current drawing texture
	 * in the openGL context
	 *
	 * @param filter Defines what kind of filtering level this
	 * texture should be drawn with
	 */
	virtual void enable (Filter filter);

	/**
	 * Stops the textures from being the current drawing texture
	 * in the openGL context
	 */
	virtual void disable ();

	/**
	 * Returns true if this texture is MipMapped
	 */
	bool mipmap () const;

	/**
	 * Sets if this texture should be MipMapped
	 */
	void setMipmap (bool);

	/**
	 * Sets the openGL filter which should be used on this
	 * texture
	 */
	void setFilter (GLenum);
	void setWrap (GLenum);

	/**
	 * Increases the reference count of a texture
	 */
	static void incRef (GLTexture *);

	/**
	 * Decreases the reference count of a texture
	 */
	static void decRef (GLTexture *);

	/**
	 * Returns a GLTexture::List with the contents of
	 * some pixmap
	 *
	 * @param pixmap Specifies the pixmap data which should be converted
	 * into texture data
	 * @param width Specifies the width of the texture
	 * @param height Specifies the height of the texture
	 * @param depth Specifies the color depth of the texture
	 * @param source Whether the pixmap lifecycle is managed externall
	 */
	static List bindPixmapToTexture (Pixmap                       pixmap,
					 int                          width,
					 int                          height,
					 int                          depth,
					 compiz::opengl::PixmapSource source
					     = compiz::opengl::InternallyManaged);

	/**
	 * Returns a GLTexture::List with the contents of of
	 * a raw image buffer
	 *
	 * @param image Specifies a raw image buffer which should be converted
	 * into texture data
	 * @param size Specifies the size of this new texture
	 */
	static List imageBufferToTexture (const char *image,
					  CompSize   size);

	static List imageDataToTexture (const char *image,
					CompSize   size,
					GLenum     format,
					GLenum     type);

	/**
	 * Uses image loading plugins to read an image from the disk and
	 * return a GLTexture::List with its contents
	 *
	 * @param imageFileName The filename of the image
	 * @param pluginName	The name of the plugin, used to find
	 *			the default image path
	 * @param size		The size of this new texture
	 */
	static List readImageToTexture (CompString &imageFileName,
					CompString &pluginName,
					CompSize   &size);

	friend class PrivateTexture;

    protected:
	GLTexture ();
	virtual ~GLTexture ();

	void setData (GLenum target, Matrix &m, bool mipmap);

    private:
	PrivateTexture *priv;
};

#endif
