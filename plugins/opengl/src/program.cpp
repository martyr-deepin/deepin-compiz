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

#include <iostream>
#include <fstream>
#include <opengl/opengl.h>

class PrivateProgram
{
    public:
	GLuint program;
	bool valid;
};


void printShaderInfoLog (GLuint shader)
{
    GLint   length = 0;
    GLint   chars  = 0;
    GLchar *infoLog;

    (*GL::getShaderiv) (shader, GL::INFO_LOG_LENGTH, &length);

    if (length > 0)
    {
	infoLog = new GLchar[length];
	(*GL::getShaderInfoLog) (shader, length, &chars, infoLog);
	std::cout << infoLog << std::endl;
	delete[] infoLog;
    }
}

void printProgramInfoLog(GLuint program)
{
    GLint   length = 0;
    GLint   chars  = 0;
    GLchar *infoLog;

    (*GL::getProgramiv) (program, GL::INFO_LOG_LENGTH, &length);

    if (length > 0)
    {
	infoLog = new GLchar[length];
	(*GL::getProgramInfoLog) (program, length, &chars, infoLog);
	std::cout << infoLog << std::endl;
	delete[] infoLog;
    }
}

static bool compileShader (GLuint *shader, GLenum type, CompString &source)
{
    const GLchar *data;
    GLint         status;

    data = (GLchar *)source.c_str ();

    *shader = (*GL::createShader) (type);
    (*GL::shaderSource) (*shader, 1, &data, NULL);
    (*GL::compileShader) (*shader);

    (*GL::getShaderiv) (*shader, GL::COMPILE_STATUS, &status);
    return (status == GL_TRUE);
}

GLProgram::GLProgram (CompString &vertexShader, CompString &fragmentShader) :
    priv (new PrivateProgram ())
{
    GLuint vertex, fragment;
    GLint status;

    priv->valid = false;
    priv->program = (*GL::createProgram) ();

    if (!compileShader (&vertex, GL::VERTEX_SHADER, vertexShader))
    {
	printShaderInfoLog (vertex);
	std::cout << vertexShader << std::endl << std::endl;
	return;
    }

    if (!compileShader (&fragment, GL::FRAGMENT_SHADER, fragmentShader))
    {
	printShaderInfoLog (fragment);
	std::cout << fragmentShader << std::endl << std::endl;
	return;
    }

    (*GL::attachShader) (priv->program, vertex);
    (*GL::attachShader) (priv->program, fragment);

    (*GL::linkProgram) (priv->program);
    (*GL::validateProgram) (priv->program);

    (*GL::getProgramiv) (priv->program, GL::LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
	printProgramInfoLog (priv->program);
	return;
    }

    (*GL::deleteShader) (vertex);
    (*GL::deleteShader) (fragment);

    priv->valid = true;
}

GLProgram::~GLProgram ()
{
    (*GL::deleteProgram) (priv->program);
    delete priv;
}

bool GLProgram::valid ()
{
    return priv->valid;
}

void GLProgram::bind ()
{
    (*GL::useProgram) (priv->program);
}

void GLProgram::unbind ()
{
    (*GL::useProgram) (0);
}

bool GLProgram::setUniform (const char *name, GLfloat value)
{
    GLint location = (*GL::getUniformLocation) (priv->program, name);
    if (location == -1)
	return false;

    (*GL::uniform1f) (location, value);
    return true;
}

bool GLProgram::setUniform (const char *name, GLint value)
{
    GLint location = (*GL::getUniformLocation) (priv->program, name);
    if (location == -1)
	return false;

    (*GL::uniform1i) (location, value);
    return true;
}

bool GLProgram::setUniform (const char *name, const GLMatrix &value)
{
    GLint location = (*GL::getUniformLocation) (priv->program, name);
    if (location == -1)
	return false;

    (*GL::uniformMatrix4fv) (location, 1, GL_FALSE, value.getMatrix ());
    return true;
}

bool GLProgram::setUniform2f (const char *name,
                              GLfloat x,
                              GLfloat y)
{
    GLint location = (*GL::getUniformLocation) (priv->program, name);
    if (location == -1)
	return false;

    (*GL::uniform2f) (location, x, y);
    return true;
}

bool GLProgram::setUniform3f (const char *name,
                              GLfloat x,
                              GLfloat y,
                              GLfloat z)
{
    GLint location = (*GL::getUniformLocation) (priv->program, name);
    if (location == -1)
	return false;

    (*GL::uniform3f) (location, x, y, z);
    return true;
}

bool GLProgram::setUniform4f (const char *name,
                              GLfloat x,
                              GLfloat y,
                              GLfloat z,
                              GLfloat w)
{
    GLint location = (*GL::getUniformLocation) (priv->program, name);
    if (location == -1)
	return false;

    (*GL::uniform4f) (location, x, y, z, w);
    return true;
}

bool GLProgram::setUniform2i (const char *name,
                              GLint x,
                              GLint y)
{
    GLint location = (*GL::getUniformLocation) (priv->program, name);
    if (location == -1)
	return false;

    (*GL::uniform2i) (location, x, y);
    return true;
}

bool GLProgram::setUniform3i (const char *name,
                              GLint x,
                              GLint y,
                              GLint z)
{
    GLint location = (*GL::getUniformLocation) (priv->program, name);
    if (location == -1)
	return false;

    (*GL::uniform3i) (location, x, y, z);
    return true;
}

bool GLProgram::setUniform4i (const char *name,
                              GLint x,
                              GLint y,
                              GLint z,
                              GLint w)
{
    GLint location = (*GL::getUniformLocation) (priv->program, name);
    if (location == -1)
	return false;

    (*GL::uniform4i) (location, x, y, z, w);
    return true;
}

GLuint GLProgram::attributeLocation (const char *name)
{
    return (*GL::getAttribLocation) (priv->program, name);
}

