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

#ifndef _VERTEXBUFFER_PRIVATE_H
#define _VERTEXBUFFER_PRIVATE_H

#ifdef USE_GLES
#include <GLES2/gl2.h>
#else
#include <GL/gl.h>
#endif

#include <opengl/program.h>
#include <typeinfo>

class GLVertexBuffer;

class AbstractUniform
{
   public:
       void virtual set(GLProgram* program) = 0;
};

template < typename T, int C >
class Uniform: public AbstractUniform
{
    public:
	Uniform(const char *_name, ... );
	void set(GLProgram* program);

    public:
	T a[C];
	std::string name;
};

template < typename T, int C >
Uniform< T, C >::Uniform(const char *_name, ... )
{
    va_list arg_list;
    va_start( arg_list, _name );
    name = _name;
    for( int i = 0; i < C; i++ )
	a[i] = va_arg( arg_list, T );
    va_end( arg_list );
}

template < typename T, int C >
void Uniform< T, C >::set(GLProgram* prog)
{
    const char* n = name.c_str();

    // This will only get called from privateVertexBuffer::render
    // so we know we've got a valid, bound program here
    if (typeid(a[0]) == typeid(double))
    {
	switch (C)
	{
	    case 1: prog->setUniform   (n, (GLfloat) a[0]); break;
	    case 2: prog->setUniform2f (n, a[0], a[1]); break;
	    case 3: prog->setUniform3f (n, a[0], a[1], a[2]); break;
	    case 4: prog->setUniform4f (n, a[0], a[1], a[2], a[3]); break;
	}
    } else if (typeid(a[0]) == typeid(int))
    {
	switch (C)
	{
	    case 1: prog->setUniform   (n, (GLint) a[0]); break;
	    case 2: prog->setUniform2i (n, a[0], a[1]); break;
	    case 3: prog->setUniform3i (n, a[0], a[1], a[2]); break;
	    case 4: prog->setUniform4i (n, a[0], a[1], a[2], a[3]); break;
	}
    } else
    {
	compLogMessage ("opengl", CompLogLevelError, "Unknown uniform type!");
    }
}

class GLVertexBuffer;

class PrivateVertexBuffer
{
    public:
	PrivateVertexBuffer ();
	~PrivateVertexBuffer ();

	int render (const GLMatrix            *projection,
	            const GLMatrix            *modelview,
	            const GLWindowPaintAttrib *attrib);
	int legacyRender (const GLMatrix            &projection,
	                  const GLMatrix            &modelview,
	                  const GLWindowPaintAttrib &attrib);

    public:
	static GLVertexBuffer *streamingBuffer;

	std::vector<GLfloat> vertexData;
	std::vector<GLfloat> normalData;
	std::vector<GLfloat> colorData;

	enum
	{
	    MAX_TEXTURES = 4
	};
	std::vector<GLfloat> textureData[MAX_TEXTURES];
	GLuint nTextures;

	GLfloat color[4];

	GLuint vertexOffset;
	GLint  maxVertices;

	GLProgram *program;
	GLenum primitiveType;
	GLenum usage;

	GLuint vertexBuffer;
	GLuint normalBuffer;
	GLuint colorBuffer;
	GLuint textureBuffers[4];
	std::vector<AbstractUniform*> uniforms;

	GLVertexBuffer::AutoProgram *autoProgram;
};

#endif //_VERTEXBUFFER_PRIVATE_H

