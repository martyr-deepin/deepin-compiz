/*
 * Copyright (c) 2011 Collabora, Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Collabora Ltd. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Collabora Ltd. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * COLLABORA LTD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL COLLABORA LTD. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Pekka Paalanen <ppaalanen@gmail.com>
 */

#ifndef _COMPIZ_GLFRAMEBUFFEROBJECT_H
#define _COMPIZ_GLFRAMEBUFFEROBJECT_H

#include <opengl/opengl.h>

struct PrivateGLFramebufferObject;

/**
 * Class representing a framebuffer object in GL, supporting only one
 * color attachment as per GLES 2 spec. The color attachment is referred
 * to as the texture (of the FBO).
 *
 * Usage:
 * 1. create a GLFramebufferObject (requires a GL context)
 * 2. call allocate (size), and check status ()
 * 3. old = bind ()
 * 4. do your rendering
 * 5. rebind (old)
 * 6. use the rendered texture via tex ()
 * 7. go to 2 or 3, or delete to quit (requires a GL context)
 */
class GLFramebufferObject
{
    public:
	GLFramebufferObject ();
	~GLFramebufferObject ();

	/**
	 * Ensure the texture is of the given size, recreating it if needed,
	 * and replace the FBO color attachment with it. The texture contents
	 * become undefined, unless specified in the 'image' argument.
	 * When specifying 'image', it's also possible to pass-in the
	 * desired image's 'format' and 'type'.
	 *
	 * Returns true on success, and false on texture allocation failure.
	 */
	bool allocate (const CompSize &size,
		       const char *image = NULL,
		       GLenum format = GL_RGBA,
		       GLenum type = GL_UNSIGNED_BYTE);

	/**
	 * Bind this as the current FBO, previous binding in GL context is
	 * undone. GL rendering is now targeted to this FBO.
	 * Returns a pointer to the previously bound FBO, or NULL if
	 * the previous binding was zero (the window system provided
	 * framebuffer).
	 *
	 * The previous FBO is no longer bound, so you can use its
	 * texture. To restore the previous FBO, call rebind (FBO) with
	 * the returned pointer as the argument.
	 */
	GLFramebufferObject *bind ();

	/**
	 * Bind the given FBO as the current FBO, without looking up the
	 * previous binding. The argument can be NULL, in which case the
	 * window system provided framebuffer gets bound (FBO is unbound).
	 */
	static void rebind (GLFramebufferObject *fbo);

	/**
	 * Check the FBO completeness. Returns true on complete.
	 * Otherwise returns false and reports the error to log.
	 */
	bool checkStatus ();

	/**
	 * Return a pointer to the texture that is the color attachment.
	 * This will return NULL, if allocate () has not been called, or
	 * the last allocate () call failed.
	 */
	GLTexture *tex ();

    private:
	PrivateGLFramebufferObject *priv;
};

#endif // _COMPIZ_GLFRAMEBUFFEROBJECT_H
