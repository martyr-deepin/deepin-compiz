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
 *          Alexandros Frantzis <alexandros.frantzis@linaro.org>
 */

#include <vector>
#include <iostream>

#ifdef USE_GLES
#include <GLES2/gl2.h>
#else
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include <opengl/vertexbuffer.h>

#include "privates.h"

GLVertexBuffer *PrivateVertexBuffer::streamingBuffer = NULL;

bool GLVertexBuffer::enabled ()
{
    // FIXME: GL::shaders shouldn't be a requirement here. But for now,
    //        fglrx doesn't have GL::shaders and that causes blending problems.
    return GL::vboEnabled && GL::shaders;
}

GLVertexBuffer::GLVertexBuffer () :
    priv (new PrivateVertexBuffer ())
{
    priv->usage = GL::STATIC_DRAW;
    colorDefault ();
}

GLVertexBuffer::GLVertexBuffer (GLenum usage) :
    priv (new PrivateVertexBuffer ())
{
    if (usage != GL::STATIC_DRAW &&
        usage != GL::DYNAMIC_DRAW &&
        usage != GL::STREAM_DRAW)
	usage = GL::STATIC_DRAW;
    priv->usage = usage;
    colorDefault ();
}

GLVertexBuffer::~GLVertexBuffer ()
{
    delete priv;
}

GLVertexBuffer *GLVertexBuffer::streamingBuffer ()
{
    if (PrivateVertexBuffer::streamingBuffer == NULL)
	PrivateVertexBuffer::streamingBuffer = new GLVertexBuffer
							      (GL::STREAM_DRAW);
    return PrivateVertexBuffer::streamingBuffer;
}

void GLVertexBuffer::begin (GLenum primitiveType /* = GL_TRIANGLES */)
{
    priv->primitiveType = primitiveType;

    priv->vertexData.clear ();
    priv->vertexOffset = 0;
    priv->maxVertices = -1;
    priv->normalData.clear ();
    priv->colorData.clear ();
    priv->uniforms.clear ();

    priv->nTextures = 0;
    for (int i = 0; i < PrivateVertexBuffer::MAX_TEXTURES; i++)
	priv->textureData[i].clear ();
}

bool GLVertexBuffer::end ()
{
    if (priv->vertexData.empty ())
	return false;

    if (!enabled ())
	return true;

    GL::bindBuffer (GL_ARRAY_BUFFER, priv->vertexBuffer);
    GL::bufferData (GL_ARRAY_BUFFER,
                    sizeof(GLfloat) * priv->vertexData.size (),
                    &priv->vertexData[0], priv->usage);

    if (priv->normalData.size ())
    {
	GL::bindBuffer (GL_ARRAY_BUFFER, priv->normalBuffer);
	GL::bufferData (GL_ARRAY_BUFFER,
	                sizeof(GLfloat) * priv->normalData.size (),
	                &priv->normalData[0], priv->usage);
    }

    if (!priv->colorData.size ())
    {
	priv->colorData.resize (4);
	priv->colorData[0] = priv->color[0];
	priv->colorData[1] = priv->color[1];
	priv->colorData[2] = priv->color[2];
	priv->colorData[3] = priv->color[3];
    }

    if (priv->colorData.size ())
    {
	GL::bindBuffer (GL_ARRAY_BUFFER, priv->colorBuffer);
	GL::bufferData (GL_ARRAY_BUFFER,
	                sizeof(GLfloat) * priv->colorData.size (),
	                &priv->colorData[0], priv->usage);
    }

    for (GLuint i = 0; i < priv->nTextures; i++)
    {
	GL::bindBuffer (GL_ARRAY_BUFFER, priv->textureBuffers[i]);
	GL::bufferData (GL_ARRAY_BUFFER,
	                sizeof(GLfloat) * priv->textureData[i].size (),
	                &priv->textureData[i][0], priv->usage);
    }

    GL::bindBuffer (GL_ARRAY_BUFFER, 0);

    return true;
}

void GLVertexBuffer::addVertices (GLuint nVertices, const GLfloat *vertices)
{
    priv->vertexData.reserve (priv->vertexData.size () + (nVertices * 3));

    for (GLuint i = 0; i < nVertices * 3; i++)
    {
	priv->vertexData.push_back (vertices[i]);
    }
}

GLfloat *GLVertexBuffer::getVertices() const
{
    return &priv->vertexData[0];
}

int GLVertexBuffer::getVertexStride() const
{
    return 3; // as seen in addVertices
}

int GLVertexBuffer::countVertices() const
{
    return priv->vertexData.size() / 3;
}

void GLVertexBuffer::setVertexOffset (GLuint vOffset)
{
    priv->vertexOffset = vOffset;
}

void GLVertexBuffer::setMaxVertices (GLint vMax)
{
    priv->maxVertices = vMax;
}

void GLVertexBuffer::addNormals (GLuint nNormals, const GLfloat *normals)
{
    priv->normalData.reserve (priv->normalData.size () + (nNormals * 3));

    for (GLuint i = 0; i < nNormals * 3; i++)
    {
	priv->normalData.push_back (normals[i]);
    }
}

void GLVertexBuffer::addColors (GLuint nColors, const GLushort *colors)
{
    priv->colorData.reserve (priv->colorData.size () + (nColors * 4));

    for (GLuint i = 0; i < nColors * 4; i++)
    {
	priv->colorData.push_back (colors[i] / 65535.0f);
    }
}

void GLVertexBuffer::color4f (GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    priv->color[0] = r;
    priv->color[1] = g;
    priv->color[2] = b;
    priv->color[3] = a;
}

void GLVertexBuffer::colorDefault ()
{
    priv->color[0] = defaultColor[0] / 65535.0;
    priv->color[1] = defaultColor[1] / 65535.0;
    priv->color[2] = defaultColor[2] / 65535.0;
    priv->color[3] = defaultColor[3] / 65535.0;
}

void GLVertexBuffer::addTexCoords (GLuint texture,
                                   GLuint nTexcoords,
                                   const GLfloat *texcoords)
{
    if (texture >= PrivateVertexBuffer::MAX_TEXTURES)
	return;

    if (texture >= priv->nTextures)
	priv->nTextures = texture + 1;

    std::vector<GLfloat> &data = priv->textureData[texture];
    data.reserve (data.size () + (nTexcoords * 2));

    for (GLuint i = 0; i < nTexcoords * 2; i++)
	data.push_back (texcoords[i]);
}

void GLVertexBuffer::addUniform (const char *name, GLfloat value)
{
    // we're casting to double here to make our template va_arg happy
    Uniform<double, 1>* uniform = new Uniform<double, 1>(name, (double)value);
    priv->uniforms.push_back (uniform);
}

void GLVertexBuffer::addUniform (const char *name, GLint value)
{
    Uniform<GLint, 1>* uniform = new Uniform<GLint, 1>(name, value);
    priv->uniforms.push_back (uniform);
}

bool GLVertexBuffer::addUniform (const char *name, const GLMatrix &value)
{
    //#warning Add 'addUniform' support to GLMatrix type !
    return true;
}

void GLVertexBuffer::addUniform2f (const char *name,
                                   GLfloat x,
                                   GLfloat y)
{
    // we're casting to double here to make our template va_arg happy
    Uniform<double, 2>* uniform = new Uniform<double, 2>(name,
							 (double)x,
							 (double)y);
    priv->uniforms.push_back (uniform);
}

void GLVertexBuffer::addUniform3f (const char *name,
                                   GLfloat x,
                                   GLfloat y,
                                   GLfloat z)
{
     // we're casting to double here to make our template va_arg happy
    Uniform<double, 3>* uniform = new Uniform<double, 3>(name,
							 (double)x,
							 (double)y,
							 (double)z);
    priv->uniforms.push_back (uniform);
}

void GLVertexBuffer::addUniform4f (const char *name,
                                   GLfloat x,
                                   GLfloat y,
                                   GLfloat z,
                                   GLfloat w)
{
    // we're casting to double here to make our template va_arg happy
    Uniform<double, 4>* uniform = new Uniform<double, 4>(name,
							 (double)x,
							 (double)y,
							 (double)z,
							 (double)w);
    priv->uniforms.push_back (uniform);
}

void GLVertexBuffer::addUniform2i (const char *name,
                                   GLint x,
                                   GLint y)
{
    Uniform<GLint, 2>* uniform = new Uniform<GLint, 2>(name, x, y);
    priv->uniforms.push_back (uniform);
}

void GLVertexBuffer::addUniform3i (const char *name,
                                   GLint x,
                                   GLint y,
                                   GLint z)
{
    Uniform<GLint, 3>* uniform = new Uniform<GLint, 3>(name, x, y, z);
    priv->uniforms.push_back (uniform);
}

void GLVertexBuffer::addUniform4i (const char *name,
                                   GLint x,
                                   GLint y,
                                   GLint z,
                                   GLint w)
{
    Uniform<GLint, 4>* uniform = new Uniform<GLint, 4>(name, x, y, z, w);
    priv->uniforms.push_back (uniform);
}

void GLVertexBuffer::setProgram (GLProgram *program)
{
    priv->program = program;
}

void GLVertexBuffer::setAutoProgram (AutoProgram *autoProgram)
{
    priv->autoProgram = autoProgram;
}

int GLVertexBuffer::render ()
{
    if (enabled ())
	return priv->render (NULL, NULL, NULL);
    else
	return -1;
}

int GLVertexBuffer::render (const GLMatrix &modelview)
{
    const GLWindowPaintAttrib attrib = { OPAQUE, BRIGHT, COLOR, 0, 0, 0, 0 };

    return render (modelview, attrib);
}

int GLVertexBuffer::render (const GLMatrix            &modelview,
                            const GLWindowPaintAttrib &attrib)
{
    GLScreen *gScreen = GLScreen::get (screen);
    GLMatrix *projection = gScreen->projectionMatrix ();

    return render (*projection, modelview, attrib);
}


#if 0
#define PRINT_MATRIX(m) printMatrix ((m), #m)
static void printMatrix (const GLMatrix &matrix, const char *title = NULL)
{
    const float *m = matrix.getMatrix();
    printf ("--- %s ---\n", title ? title : "?");
    for (int y = 0; y < 4; y++)
	printf ("[%5.1f %5.1f %5.1f %5.1f]\n", m[y], m[y+4], m[y+8], m[y+12]);
}
#else
#define PRINT_MATRIX(m)
#endif

int GLVertexBuffer::render (const GLMatrix            &projection,
                            const GLMatrix            &modelview,
                            const GLWindowPaintAttrib &attrib)
{
    if (!priv->vertexData.size ())
	return -1;

    PRINT_MATRIX(modelview);
    PRINT_MATRIX(projection);

    if (enabled ())
	return priv->render (&projection, &modelview, &attrib);
    else
	return priv->legacyRender (projection, modelview, attrib);
}

PrivateVertexBuffer::PrivateVertexBuffer () :
    nTextures (0),
    vertexOffset (0),
    maxVertices (-1),
    program (NULL)
{
    if (!GL::genBuffers)
	return;

    GL::genBuffers (1, &vertexBuffer);
    GL::genBuffers (1, &normalBuffer);
    GL::genBuffers (1, &colorBuffer);
    GL::genBuffers (4, &textureBuffers[0]);
}

PrivateVertexBuffer::~PrivateVertexBuffer ()
{
    if (!GL::deleteBuffers)
	return;

    if (vertexBuffer)
	GL::deleteBuffers (1, &vertexBuffer);
    if (normalBuffer)
	GL::deleteBuffers (1, &normalBuffer);
    if (colorBuffer)
	GL::deleteBuffers (1, &colorBuffer);
    if (textureBuffers[0])
	GL::deleteBuffers (4, &textureBuffers[0]);
}

int PrivateVertexBuffer::render (const GLMatrix            *projection,
                                 const GLMatrix            *modelview,
                                 const GLWindowPaintAttrib *attrib)
{
    GLfloat attribs[3] = {1, 1, 1};
    GLint positionIndex = -1;
    GLint normalIndex = -1;
    GLint colorIndex = -1;
    GLint texCoordIndex[4] = {-1, -1, -1, -1};
    GLProgram *tmpProgram = program;

    // If we don't have an explicitly set program, try to get one
    // using the AutoProgram callback object.
    if (tmpProgram == NULL && autoProgram) {
	// Convert attrib to shader parameters
	GLShaderParameters params;

	params.opacity = attrib->opacity != OPAQUE;
	params.brightness = attrib->brightness != BRIGHT;
	params.saturation = attrib->saturation != COLOR;
	params.color = colorData.size () == 4 ? GLShaderVariableUniform :
	               colorData.size () >  4 ? GLShaderVariableVarying :
	                                        GLShaderVariableNone;
	params.normal = normalData.size () <= 4 ? GLShaderVariableUniform :
	                                          GLShaderVariableVarying;
	params.numTextures = nTextures;

	// Get a program matching the parameters
	tmpProgram = autoProgram->getProgram(params);
    }

    if (tmpProgram == NULL)
    {
	std::cerr << "no program defined!" << std::endl;
	return -1;
    }

    tmpProgram->bind ();
    if (!tmpProgram->valid ())
    {
	return -1;
    }

    if (projection)
	tmpProgram->setUniform ("projection", *projection);

    if (modelview)
	tmpProgram->setUniform ("modelview", *modelview);

    positionIndex = tmpProgram->attributeLocation ("position");
    (*GL::enableVertexAttribArray) (positionIndex);
    (*GL::bindBuffer) (GL::ARRAY_BUFFER, vertexBuffer);
    (*GL::vertexAttribPointer) (positionIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    (*GL::bindBuffer) (GL::ARRAY_BUFFER, 0);

    //use default normal
    if (normalData.empty ())
    {
	tmpProgram->setUniform3f ("singleNormal", 0.0f, 0.0f, -1.0f);
    }
    // special case a single normal and apply it to the entire operation
    else if (normalData.size () == 3)
    {
	tmpProgram->setUniform3f ("singleNormal",
	                       normalData[0], normalData[1], normalData[2]);
    }
    else if (normalData.size () > 3)
    {
	normalIndex = tmpProgram->attributeLocation ("normal");
	(*GL::enableVertexAttribArray) (normalIndex);
	(*GL::bindBuffer) (GL::ARRAY_BUFFER, normalBuffer);
	(*GL::vertexAttribPointer) (normalIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	(*GL::bindBuffer) (GL::ARRAY_BUFFER, 0);
    }

    // special case a single color and apply it to the entire operation
    if (colorData.size () == 4)
    {
	tmpProgram->setUniform4f ("singleColor", colorData[0],
	                       colorData[1], colorData[2], colorData[3]);
    }
    else if (colorData.size () > 4)
    {
	colorIndex = tmpProgram->attributeLocation ("color");
	(*GL::enableVertexAttribArray) (colorIndex);
	(*GL::bindBuffer) (GL::ARRAY_BUFFER, colorBuffer);
	(*GL::vertexAttribPointer) (colorIndex, 4, GL_FLOAT, GL_FALSE, 0, 0);
	(*GL::bindBuffer) (GL::ARRAY_BUFFER, 0);
    }

    for (int i = nTextures - 1; i >= 0; i--)
    {
	char name[10];

	snprintf (name, 10, "texCoord%d", i);
	texCoordIndex[i] = tmpProgram->attributeLocation (name);

	(*GL::enableVertexAttribArray) (texCoordIndex[i]);
	(*GL::bindBuffer) (GL::ARRAY_BUFFER, textureBuffers[i]);
	(*GL::vertexAttribPointer) (texCoordIndex[i], 2, GL_FLOAT, GL_FALSE, 0, 0);
	(*GL::bindBuffer) (GL::ARRAY_BUFFER, 0);

	snprintf (name, 9, "texture%d", i);
	tmpProgram->setUniform (name, i);
    }

    // set per-plugin uniforms
    for (unsigned int i = 0; i < uniforms.size (); i++)
    {
	uniforms[i]->set (program);
    }

    //convert paint attribs to 0-1 range
    if (attrib)
    {
	attribs[0] = attrib->opacity  / 65535.0f;
	attribs[1] = attrib->brightness / 65535.0f;
	attribs[2] = attrib->saturation / 65535.0f;
	tmpProgram->setUniform3f ("paintAttrib", attribs[0], attribs[1], attribs[2]);
    }


    glDrawArrays (primitiveType, vertexOffset, maxVertices > 0 ?
				    std::min (static_cast <int> (vertexData.size () / 3),
					      maxVertices) :
				    vertexData.size () / 3);
    for (int i = 0; i < 4; ++i)
    {
	if (texCoordIndex[i] != -1)
	    (*GL::disableVertexAttribArray) (texCoordIndex[i]);
    }

    if (colorIndex != -1)
	(*GL::disableVertexAttribArray) (colorIndex);

    if (normalIndex != -1)
	(*GL::disableVertexAttribArray) (normalIndex);

    (*GL::disableVertexAttribArray) (positionIndex);

    tmpProgram->unbind ();

    return 0;
}

int PrivateVertexBuffer::legacyRender (const GLMatrix            &projection,
                                       const GLMatrix            &modelview,
                                       const GLWindowPaintAttrib &attrib)
{
    #ifndef USE_GLES
    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glLoadMatrixf (projection.getMatrix ());

    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glLoadMatrixf (modelview.getMatrix ());

    glEnableClientState (GL_VERTEX_ARRAY);
    glVertexPointer (3, GL_FLOAT, 0, &vertexData[0]);

    //use default normal
    if (normalData.empty ())
    {
	glNormal3f (0.0f, 0.0f, -1.0f);
    }
    // special case a single normal and apply it to the entire operation
    else if (normalData.size () == 3)
    {
	glNormal3fv (&normalData[0]);
    }
    else if (normalData.size () > 3)
    {
	glEnableClientState (GL_NORMAL_ARRAY);
	glNormalPointer (GL_FLOAT, 0, &normalData[0]);
    }

    // special case a single color and apply it to the entire operation
    if (colorData.size () == 4)
    {
	glColor4fv (&colorData[0]);
    }
    else if (colorData.size () > 4)
    {
	glEnableClientState (GL_COLOR_ARRAY);
	glColorPointer (4, GL_FLOAT, 0, &colorData[0]);
    }

    for (int i = nTextures - 1; i >= 0; i--)
    {
	GL::clientActiveTexture (GL_TEXTURE0_ARB + i);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer (2, GL_FLOAT, 0, &textureData[i][0]);
    }

    glDrawArrays (primitiveType, vertexOffset, vertexData.size () / 3);

    glDisableClientState (GL_VERTEX_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);
    glDisableClientState (GL_COLOR_ARRAY);

    for (int i = nTextures; i > 0; i--)
    {
	GL::clientActiveTexture (GL_TEXTURE0_ARB + i);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
    }

    GL::clientActiveTexture (GL_TEXTURE0_ARB);

    glMatrixMode (GL_PROJECTION);
    glPopMatrix ();

    glMatrixMode (GL_MODELVIEW);
    glPopMatrix ();
    #endif

    return 0;
}

