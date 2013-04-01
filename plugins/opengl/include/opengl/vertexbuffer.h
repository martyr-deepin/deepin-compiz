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
 *          Frederic Plourde <frederic.plourde@collabora.co.uk>
 */

#ifndef _COMPIZ_GLVERTEXBUFFER_H
#define _COMPIZ_GLVERTEXBUFFER_H

#ifdef USE_GLES
#include <GLES2/gl2.h>
#else
#include <GL/gl.h>
#endif

#include <core/core.h>
#include <opengl/program.h>
#include <opengl/shadercache.h>

class PrivateVertexBuffer;
struct GLWindowPaintAttrib;

namespace compiz
{
    namespace gl
    {
	class AutoProgram
	{
	    public:
		virtual ~AutoProgram () {}

		virtual GLProgram *getProgram(GLShaderParameters &params) = 0;
	};
    }
}

class GLVertexBuffer
{
    public:

	static bool enabled ();

	GLVertexBuffer ();
	GLVertexBuffer (GLenum usage);
	~GLVertexBuffer ();

	typedef compiz::gl::AutoProgram AutoProgram;

	static GLVertexBuffer *streamingBuffer ();

	void begin (GLenum primitiveType = GL_TRIANGLES);
	bool end ();

	// vertices and normals are 3 parts, count is number of xyz groups
	void addVertices (GLuint nVertices, const GLfloat *vertices);
	GLfloat *getVertices () const;  // AKA GLWindow::Geometry::vertices
	int getVertexStride () const;   // AKA GLWindow::Geometry::vertexStride
	int countVertices () const;     // AKA GLWindow::Geometry::vCount

	void addNormals (GLuint nNormals, const GLfloat *normals);

	// color is always RGBA (4 parts), count is number of rgba groups
	void addColors (GLuint nColors, const GLushort *colors);

	void color4f (GLfloat r, GLfloat g, GLfloat b, GLfloat a);
	void colorDefault ();

	// texture is index, texcoords are 2 parts, count is number of pairs
	void addTexCoords (GLuint texture,
	                   GLuint nTexcoords,
	                   const GLfloat *texcoords);

	void addUniform (const char *name, GLfloat value);
	void addUniform (const char *name, GLint value);
	bool addUniform (const char *name, const GLMatrix &value);
	void addUniform2f (const char *name, GLfloat x, GLfloat y);
	void addUniform3f (const char *name, GLfloat x, GLfloat y, GLfloat z);
	void addUniform4f (const char *name, GLfloat x, GLfloat y,
			                     GLfloat z, GLfloat w);
	void addUniform2i (const char *name, GLint x, GLint y);
	void addUniform3i (const char *name, GLint x, GLint y, GLint z);
	void addUniform4i (const char *name, GLint x, GLint y,
			                     GLint z, GLint w);

	void setProgram (GLProgram *program);

	void setAutoProgram (AutoProgram *autoProgram);

	// This no-argument render () function is intended for use by plugins
	// that have custom programs.
	int render ();

	int render (const GLMatrix &modelview);

	int render (const GLMatrix            &modelview,
	            const GLWindowPaintAttrib &attrib);

	int render (const GLMatrix            &projection,
	            const GLMatrix            &modelview,
	            const GLWindowPaintAttrib &attrib);

	void setVertexOffset (GLuint vOffset);
	void setMaxVertices (GLint vMax);

    private:
	PrivateVertexBuffer *priv;
};

#endif // _COMPIZ_GLVERTEXBUFFER_H

