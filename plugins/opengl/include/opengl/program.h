/*
 * Copyright © 2011 Linaro Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Linaro Ltd. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Linaro Ltd. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * LINARO LTD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL LINARO LTD. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Travis Watkins <travis.watkins@linaro.org>
 */

#ifndef _COMPIZ_GLPROGRAM_H
#define _COMPIZ_GLPROGRAM_H

#ifdef USE_GLES
#include <GLES2/gl2.h>
#else
#include <GL/gl.h>
#endif

#include <core/core.h>
#include <opengl/matrix.h>

class PrivateProgram;

class GLProgram
{
    public:
	GLProgram (CompString &vertexShader, CompString &fragmentShader);
	~GLProgram ();

	bool valid ();
	void bind ();
	void unbind ();

	bool setUniform   (const char *name, GLfloat value);
	bool setUniform   (const char *name, GLint value);
	bool setUniform   (const char *name, const GLMatrix &value);
	bool setUniform2f (const char *name, GLfloat x, GLfloat y);
	bool setUniform3f (const char *name, GLfloat x, GLfloat y, GLfloat z);
	bool setUniform4f (const char *name,
	                   GLfloat x,
	                   GLfloat y,
	                   GLfloat z,
                           GLfloat w);
	bool setUniform2i (const char *name, GLint x, GLint y);
	bool setUniform3i (const char *name, GLint x, GLint y, GLint z);
	bool setUniform4i (const char *name,
	                   GLint x,
	                   GLint y,
	                   GLint z,
                           GLint w);

	GLuint attributeLocation (const char *name);

    private:
	PrivateProgram *priv;
};

#endif // _COMPIZ_GLPROGRAM_H

