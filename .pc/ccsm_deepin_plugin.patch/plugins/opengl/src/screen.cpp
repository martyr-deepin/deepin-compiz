/*
 * Copyright © 2011 Linaro Ltd.
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
 *          Travis Watkins <travis.watkins@linaro.org>
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>

#include "privates.h"

#include <dlfcn.h>
#include <math.h>

using namespace compiz::opengl;

namespace GL {
    #ifdef USE_GLES
    EGLCreateImageKHRProc  createImage;
    EGLDestroyImageKHRProc destroyImage;

    GLEGLImageTargetTexture2DOESProc eglImageTargetTexture;

    EGLPostSubBufferNVProc postSubBuffer = NULL;
    #else

    typedef int (*GLXSwapIntervalProc) (int interval);

    GLXBindTexImageProc      bindTexImage = NULL;
    GLXReleaseTexImageProc   releaseTexImage = NULL;
    GLXQueryDrawableProc     queryDrawable = NULL;
    GLXCopySubBufferProc     copySubBuffer = NULL;
    GLXGetVideoSyncProc      getVideoSync = NULL;
    GLXWaitVideoSyncProc     waitVideoSync = NULL;
    GLXSwapIntervalProc      swapInterval = NULL;
    GLXGetFBConfigsProc      getFBConfigs = NULL;
    GLXGetFBConfigAttribProc getFBConfigAttrib = NULL;
    GLXCreatePixmapProc      createPixmap = NULL;
    GLXDestroyPixmapProc     destroyPixmap = NULL;
    GLGenProgramsProc	     genPrograms = NULL;
    GLDeleteProgramsProc     deletePrograms = NULL;
    GLBindProgramProc	     bindProgram = NULL;
    GLProgramStringProc	     programString = NULL;
    GLProgramParameter4fProc programEnvParameter4f = NULL;
    GLProgramParameter4fProc programLocalParameter4f = NULL;
    #endif

    GLActiveTextureProc       activeTexture = NULL;
    GLClientActiveTextureProc clientActiveTexture = NULL;
    GLMultiTexCoord2fProc     multiTexCoord2f = NULL;

    GLGenFramebuffersProc        genFramebuffers = NULL;
    GLDeleteFramebuffersProc     deleteFramebuffers = NULL;
    GLBindFramebufferProc        bindFramebuffer = NULL;
    GLCheckFramebufferStatusProc checkFramebufferStatus = NULL;
    GLFramebufferTexture2DProc   framebufferTexture2D = NULL;
    GLGenerateMipmapProc         generateMipmap = NULL;

    GLBindBufferProc    bindBuffer = NULL;
    GLDeleteBuffersProc deleteBuffers = NULL;
    GLGenBuffersProc    genBuffers = NULL;
    GLBufferDataProc    bufferData = NULL;
    GLBufferSubDataProc bufferSubData = NULL;

    GLGetShaderivProc        getShaderiv = NULL;
    GLGetShaderInfoLogProc   getShaderInfoLog = NULL;
    GLGetProgramivProc       getProgramiv = NULL;
    GLGetProgramInfoLogProc  getProgramInfoLog = NULL;
    GLCreateShaderProc       createShader = NULL;
    GLShaderSourceProc       shaderSource = NULL;
    GLCompileShaderProc      compileShader = NULL;
    GLCreateProgramProc      createProgram = NULL;
    GLAttachShaderProc       attachShader = NULL;
    GLLinkProgramProc        linkProgram = NULL;
    GLValidateProgramProc    validateProgram = NULL;
    GLDeleteShaderProc       deleteShader = NULL;
    GLDeleteProgramProc      deleteProgram = NULL;
    GLUseProgramProc         useProgram = NULL;
    GLGetUniformLocationProc getUniformLocation = NULL;
    GLUniform1fProc          uniform1f = NULL;
    GLUniform1iProc          uniform1i = NULL;
    GLUniform2fProc          uniform2f = NULL;
    GLUniform2iProc          uniform2i = NULL;
    GLUniform3fProc          uniform3f = NULL;
    GLUniform3iProc          uniform3i = NULL;
    GLUniform4fProc          uniform4f = NULL;
    GLUniform4iProc          uniform4i = NULL;
    GLUniformMatrix4fvProc   uniformMatrix4fv = NULL;
    GLGetAttribLocationProc  getAttribLocation = NULL;

#ifndef USE_GLES

    GLCreateShaderObjectARBProc createShaderObjectARB = NULL;
    GLCreateProgramObjectARBProc createProgramObjectARB = NULL;
    GLCompileShaderARBProc compileShaderARB = NULL;
    GLShaderSourceARBProc shaderSourceARB = NULL;
    GLValidateProgramARBProc validateProgramARB = NULL;
    GLDeleteObjectARBProc deleteObjectARB = NULL;
    GLAttachObjectARBProc attachObjectARB = NULL;
    GLLinkProgramARBProc linkProgramARB = NULL;
    GLUseProgramObjectARBProc useProgramObjectARB = NULL;
    GLGetUniformLocationARBProc getUniformLocationARB = NULL;
    GLGetAttribLocationARBProc getAttribLocationARB = NULL;
    GLGetObjectParameterivProc getObjectParameteriv = NULL;
    GLGetInfoLogProc         getInfoLog = NULL;

#endif

    GLEnableVertexAttribArrayProc  enableVertexAttribArray = NULL;
    GLDisableVertexAttribArrayProc disableVertexAttribArray = NULL;
    GLVertexAttribPointerProc      vertexAttribPointer = NULL;

    GLGenRenderbuffersProc genRenderbuffers = NULL;
    GLDeleteRenderbuffersProc deleteRenderbuffers = NULL;
    GLFramebufferRenderbufferProc framebufferRenderbuffer = NULL;
    GLBindRenderbufferProc bindRenderbuffer = NULL;
    GLRenderbufferStorageProc renderbufferStorage = NULL;

    bool  textureFromPixmap = true;
    bool  textureRectangle = false;
    bool  textureNonPowerOfTwo = false;
    bool  textureNonPowerOfTwoMipmap = false;
    bool  textureEnvCombine = false;
    bool  textureEnvCrossbar = false;
    bool  textureBorderClamp = false;
    bool  textureCompression = false;
    GLint maxTextureSize = 0;
    bool  fboSupported = false;
    bool  fboEnabled = false;
    bool  fboStencilSupported = false;
    bool  vboSupported = false;
    bool  vboEnabled = false;
    bool  shaders = false;
    GLint maxTextureUnits = 1;

    bool canDoSaturated = false;
    bool canDoSlightlySaturated = false;

    unsigned int vsyncCount = 0;
    unsigned int unthrottledFrames = 0;

    bool stencilBuffer = false;
#ifndef USE_GLES

    GLuint createShaderARBWrapper (GLenum type)
    {
	return static_cast <GLuint> ((GL::createShaderObjectARB) (type));
    }

    GLuint createProgramARBWrapper (GLenum type)
    {
	return static_cast <GLuint> ((GL::createProgramObjectARB) ());
    }

    void shaderSourceARBWrapper (GLuint shader, GLsizei count, const GLchar **string, const GLint *length)
    {
	(GL::shaderSourceARB) (static_cast <GLhandleARB> (shader), count, string, length);
    }

    void compileShaderARBWrapper (GLuint shader)
    {
	(GL::compileShaderARB) (static_cast <GLhandleARB> (shader));
    }

    void validateProgramARBWrapper (GLuint program)
    {
	(GL::validateProgramARB) (static_cast <GLhandleARB> (program));
    }

    void deleteShaderARBWrapper (GLuint shader)
    {
	(GL::deleteObjectARB) (static_cast <GLhandleARB> (shader));
    }

    void deleteProgramARBWrapper (GLuint program)
    {
	(GL::deleteObjectARB) (static_cast <GLhandleARB> (program));
    }

    void attachShaderARBWrapper (GLuint program, GLuint shader)
    {
	(GL::attachObjectARB) (static_cast <GLhandleARB> (program), static_cast <GLhandleARB> (shader));
    }

    void linkProgramARBWrapper (GLuint program)
    {
	(GL::linkProgramARB) (static_cast <GLhandleARB> (program));
    }

    void useProgramARBWrapper (GLuint program)
    {
	(GL::useProgramObjectARB) (static_cast <GLhandleARB> (program));
    }

    int getUniformLocationARBWrapper (GLuint program, const GLchar *name)
    {
	return (GL::getUniformLocationARB) (static_cast <GLhandleARB> (program), name);
    }

    int getAttribLocationARBWrapper (GLuint program, const GLchar *name)
    {
	return (GL::getAttribLocationARB) (static_cast <GLhandleARB> (program), name);
    }

    void getProgramInfoLogARBWrapper (GLuint object, int maxLen, int *len, char *log)
    {
	(GL::getInfoLog) (static_cast <GLhandleARB> (object), maxLen, len, log);
    }

    void getShaderInfoLogARBWrapper (GLuint object, int maxLen, int *len, char *log)
    {
	(GL::getInfoLog) (static_cast <GLhandleARB> (object), maxLen, len, log);
    }

    void getShaderivARBWrapper (GLuint object, GLenum type, int *param)
    {
	(GL::getObjectParameteriv) (static_cast <GLhandleARB> (object), type, param);
    }

    void getProgramivARBWrapper (GLuint object, GLenum type, int *param)
    {
	(GL::getObjectParameteriv) (static_cast <GLhandleARB> (object), type, param);
    }

#endif
}

CompOutput *targetOutput = NULL;

/**
 * Callback object to create GLPrograms automatically when using GLVertexBuffer.
 */
class GLScreenAutoProgram : public GLVertexBuffer::AutoProgram
{
public:
    GLScreenAutoProgram (GLScreen *gScreen) : gScreen(gScreen) {}

    GLProgram *getProgram (GLShaderParameters &params)
    {
        const GLShaderData *shaderData = gScreen->getShaderData (params);
        std::list<const GLShaderData *> tempShaders;
        tempShaders.push_back (shaderData);
        return gScreen->getProgram (tempShaders);
    }

    GLScreen *gScreen;
};

bool
GLScreen::glInitContext (XVisualInfo *visinfo)
{
    #ifdef USE_GLES
    Display             *xdpy;
    Window               overlay;
    EGLDisplay           dpy;
    EGLConfig            config;
    EGLint               major, minor;
    const char		*eglExtensions, *glExtensions;
    XWindowAttributes    attr;
    EGLint               count, visualid;
    EGLConfig            configs[1024];
    CompOption::Vector   o (0);

    const EGLint config_attribs[] = {
	EGL_SURFACE_TYPE,         EGL_WINDOW_BIT,
	EGL_RED_SIZE,             1,
	EGL_GREEN_SIZE,           1,
	EGL_BLUE_SIZE,            1,
	EGL_ALPHA_SIZE,           0,
	EGL_RENDERABLE_TYPE,      EGL_OPENGL_ES2_BIT,
	EGL_CONFIG_CAVEAT,        EGL_NONE,
	EGL_STENCIL_SIZE,	  1,
	EGL_NONE
    };

    const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    xdpy = screen->dpy ();
    dpy = eglGetDisplay ((EGLNativeDisplayType)xdpy);
    if (!eglInitialize (dpy, &major, &minor))
    {
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	return false;
    }

    eglBindAPI (EGL_OPENGL_ES_API);

    if (!eglChooseConfig (dpy, config_attribs, configs, 1024, &count))
    {
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	return false;
    }

    if (!XGetWindowAttributes (xdpy, screen->root (), &attr))
    {
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	return false;
    }

    visualid = XVisualIDFromVisual (attr.visual);
    config = configs[0];
    for (int i = 0; i < count; i++) {
        EGLint val;
        eglGetConfigAttrib (dpy, configs[i], EGL_NATIVE_VISUAL_ID, &val);
        if (visualid == val) {
            config = configs[i];
            break;
        }
    }

    overlay = CompositeScreen::get (screen)->overlay ();
    priv->surface = eglCreateWindowSurface (dpy, config, overlay, 0);
    if (priv->surface == EGL_NO_SURFACE)
    {
	compLogMessage ("opengl", CompLogLevelFatal,
	                "eglCreateWindowSurface failed");
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	return false;
    }

    // Do not preserve buffer contents on swap
    eglSurfaceAttrib (dpy, priv->surface, EGL_SWAP_BEHAVIOR, EGL_BUFFER_DESTROYED);

    priv->ctx = eglCreateContext (dpy, config, EGL_NO_CONTEXT, context_attribs);
    if (priv->ctx == EGL_NO_CONTEXT)
    {
	compLogMessage ("opengl", CompLogLevelFatal, "eglCreateContext failed");
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	return false;
    }

    if (!eglMakeCurrent (dpy, priv->surface, priv->surface, priv->ctx))
    {
	compLogMessage ("opengl", CompLogLevelFatal,
	                "eglMakeCurrent failed");
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	return false;
    }

    eglExtensions = (const char *) eglQueryString (dpy, EGL_EXTENSIONS);
    glExtensions = (const char *) glGetString (GL_EXTENSIONS);

    if (!glExtensions || !eglExtensions)
    {
	compLogMessage ("opengl", CompLogLevelFatal,
			"No valid GL extensions string found.");
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	return false;
    }

    GL::textureFromPixmap = true;
    GL::textureNonPowerOfTwo = true;
    GL::fboSupported = true;
    GL::fboEnabled = true;
    GL::vboSupported = true;
    GL::vboEnabled = true;
    GL::shaders = true;
    GL::stencilBuffer = true;
    GL::maxTextureUnits = 4;
    glGetIntegerv (GL_MAX_TEXTURE_SIZE, &GL::maxTextureSize);

    GL::createImage = (GL::EGLCreateImageKHRProc)
	eglGetProcAddress ("eglCreateImageKHR");
    GL::destroyImage = (GL::EGLDestroyImageKHRProc)
	eglGetProcAddress ("eglDestroyImageKHR");
    GL::eglImageTargetTexture = (GL::GLEGLImageTargetTexture2DOESProc)
	eglGetProcAddress ("glEGLImageTargetTexture2DOES");

    if (!strstr (eglExtensions, "EGL_KHR_image_pixmap") ||
        !strstr (glExtensions, "GL_OES_EGL_image") ||
	!GL::createImage || !GL::destroyImage || !GL::eglImageTargetTexture)
    {
	compLogMessage ("opengl", CompLogLevelFatal,
			"GL_OES_EGL_image is missing");
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	return false;
    }

// work around efika supporting GL_BGRA directly instead of via this extension
#ifndef GL_BGRA
    if (!strstr (glExtensions, "GL_EXT_texture_format_BGRA8888"))
    {
	compLogMessage ("opengl", CompLogLevelFatal,
			"GL_EXT_texture_format_BGRA8888 is missing");
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	return false;
    }
#endif

    if (strstr (glExtensions, "GL_OES_texture_npot"))
	GL::textureNonPowerOfTwoMipmap = true;

    if (strstr (eglExtensions, "EGL_NV_post_sub_buffer"))
	GL::postSubBuffer = (GL::EGLPostSubBufferNVProc)
	    eglGetProcAddress ("eglPostSubBufferNV");

    GL::fboStencilSupported = GL::fboSupported &&
        strstr (glExtensions, "GL_OES_packed_depth_stencil");

    if (!GL::fboSupported &&
	!GL::postSubBuffer)
    {
	compLogMessage ("opengl", CompLogLevelFatal,
			"GL_EXT_framebuffer_object or EGL_NV_post_sub_buffer are required");
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	return false;
    }

    GL::activeTexture = glActiveTexture;
    GL::genFramebuffers = glGenFramebuffers;
    GL::deleteFramebuffers = glDeleteFramebuffers;
    GL::bindFramebuffer = glBindFramebuffer;
    GL::checkFramebufferStatus = glCheckFramebufferStatus;
    GL::framebufferTexture2D = glFramebufferTexture2D;
    GL::generateMipmap = glGenerateMipmap;

    GL::bindBuffer = glBindBuffer;
    GL::deleteBuffers = glDeleteBuffers;
    GL::genBuffers = glGenBuffers;
    GL::bufferData = glBufferData;
    GL::bufferSubData = glBufferSubData;

    GL::getShaderiv = glGetShaderiv;
    GL::getShaderInfoLog = glGetShaderInfoLog;
    GL::getProgramiv = glGetProgramiv;
    GL::getProgramInfoLog = glGetProgramInfoLog;
    GL::createShader = glCreateShader;
    GL::shaderSource = glShaderSource;
    GL::compileShader = glCompileShader;
    GL::createProgram = glCreateProgram;
    GL::attachShader = glAttachShader;
    GL::linkProgram = glLinkProgram;
    GL::validateProgram = glValidateProgram;
    GL::deleteShader = glDeleteShader;
    GL::deleteProgram = glDeleteProgram;
    GL::useProgram = glUseProgram;
    GL::getUniformLocation = glGetUniformLocation;
    GL::uniform1f = glUniform1f;
    GL::uniform1i = glUniform1i;
    GL::uniform2f = glUniform2f;
    GL::uniform2i = glUniform2i;
    GL::uniform3f = glUniform3f;
    GL::uniform3i = glUniform3i;
    GL::uniform4f = glUniform4f;
    GL::uniform4i = glUniform4i;
    GL::uniformMatrix4fv = glUniformMatrix4fv;
    GL::getAttribLocation = glGetAttribLocation;

    GL::enableVertexAttribArray = glEnableVertexAttribArray;
    GL::disableVertexAttribArray = glDisableVertexAttribArray;
    GL::vertexAttribPointer = glVertexAttribPointer;

    GL::genRenderbuffers = glGenRenderbuffers;
    GL::deleteRenderbuffers = glDeleteRenderbuffers;
    GL::bindRenderbuffer = glBindRenderbuffer;
    GL::framebufferRenderbuffer = glFramebufferRenderbuffer;
    GL::renderbufferStorage = glRenderbufferStorage;

    glClearColor (0.0, 0.0, 0.0, 1.0);
    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_BLEND);
    glEnable (GL_CULL_FACE);

    priv->updateView ();

    priv->lighting = false;

    priv->filter[NOTHING_TRANS_FILTER] = GLTexture::Fast;
    priv->filter[SCREEN_TRANS_FILTER]  = GLTexture::Good;
    priv->filter[WINDOW_TRANS_FILTER]  = GLTexture::Good;

    if (GL::textureFromPixmap)
	registerBindPixmap (EglTexture::bindPixmapToTexture);

    priv->incorrectRefreshRate = false;

    #else

    Display		 *dpy = screen->dpy ();
    const char		 *glExtensions;
    GLfloat		 globalAmbient[]  = { 0.1f, 0.1f,  0.1f, 0.1f };
    GLfloat		 ambientLight[]   = { 0.0f, 0.0f,  0.0f, 0.0f };
    GLfloat		 diffuseLight[]   = { 0.9f, 0.9f,  0.9f, 0.9f };
    GLfloat		 light0Position[] = { -0.5f, 0.5f, -9.0f, 1.0f };
    const char           *glRenderer;
    const char           *glVendor;
    CompOption::Vector o (0);

    priv->ctx = glXCreateContext (dpy, visinfo, NULL, True);
    if (!priv->ctx)
    {
	compLogMessage ("opengl", CompLogLevelWarn,
			"glXCreateContext with direct rendering failed - trying indirect");

	/* force Mesa libGL into indirect rendering mode, because
	   glXQueryExtensionsString is context-independant */
	setenv ("LIBGL_ALWAYS_INDIRECT", "1", True);
	priv->ctx = glXCreateContext(dpy, visinfo, NULL, true);

	if (!priv->ctx)
	{
	    compLogMessage ("opengl", CompLogLevelWarn,
			    "glXCreateContext failed");

	    XFree (visinfo);

	    screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	    return false;
	}
    }

    XFree (visinfo);

    glXMakeCurrent (dpy, CompositeScreen::get (screen)->output (), priv->ctx);

    glExtensions = (const char *) glGetString (GL_EXTENSIONS);
    if (!glExtensions)
    {
	compLogMessage ("opengl", CompLogLevelFatal,
			"No valid GL extensions string found.");
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	return false;
    }

    glRenderer = (const char *) glGetString (GL_RENDERER);
    glVendor = (const char *) glGetString (GL_VENDOR);
    if (glRenderer != NULL &&
	(strcmp (glRenderer, "Software Rasterizer") == 0 ||
	 strcmp (glRenderer, "Mesa X11") == 0))
    {
	compLogMessage ("opengl",
			CompLogLevelFatal,
			"Software rendering detected");
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	return false;
    }

    priv->commonFrontbuffer = true;
    priv->incorrectRefreshRate = false;
    if (glRenderer != NULL && strstr (glRenderer, "on llvmpipe"))
    {
	/*
	 * Most drivers use the same frontbuffer infrastructure for
	 * swapbuffers as well as subbuffer copying. However there are some
	 * odd exceptions like LLVMpipe (and SGX-something?) that use separate
	 * buffers, so we can't dynamically switch between buffer swapping and
	 * copying in those cases.
	 */
	priv->commonFrontbuffer = false;
    }

    if (glVendor != NULL && strstr (glVendor, "NVIDIA"))
    {
	/*
	 * NVIDIA provides an incorrect refresh rate, we need to
	 * force 60Hz */
	priv->incorrectRefreshRate = true;
    }

    if (strstr (glExtensions, "GL_ARB_texture_non_power_of_two"))
	GL::textureNonPowerOfTwo = true;
    GL::textureNonPowerOfTwoMipmap = GL::textureNonPowerOfTwo;

    glGetIntegerv (GL_MAX_TEXTURE_SIZE, &GL::maxTextureSize);

    if (strstr (glExtensions, "GL_NV_texture_rectangle")  ||
	strstr (glExtensions, "GL_EXT_texture_rectangle") ||
	strstr (glExtensions, "GL_ARB_texture_rectangle"))
    {
	GL::textureRectangle = true;

	if (!GL::textureNonPowerOfTwo)
	{
	    GLint maxTextureSize;

	    glGetIntegerv (GL_MAX_RECTANGLE_TEXTURE_SIZE_NV, &maxTextureSize);
	    if (maxTextureSize > GL::maxTextureSize)
		GL::maxTextureSize = maxTextureSize;
	}
    }

    if (!(GL::textureRectangle || GL::textureNonPowerOfTwo))
    {
	compLogMessage ("opengl", CompLogLevelFatal,
			"Support for non power of two textures missing");
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	return false;
    }

    if (strstr (glExtensions, "GL_ARB_texture_env_combine"))
    {
	GL::textureEnvCombine = true;

	/* XXX: GL_NV_texture_env_combine4 need special code but it seams to
	   be working anyway for now... */
	if (strstr (glExtensions, "GL_ARB_texture_env_crossbar") ||
	    strstr (glExtensions, "GL_NV_texture_env_combine4"))
	    GL::textureEnvCrossbar = true;
    }

    if (strstr (glExtensions, "GL_ARB_texture_border_clamp") ||
	strstr (glExtensions, "GL_SGIS_texture_border_clamp"))
	GL::textureBorderClamp = true;

    GL::maxTextureUnits = 1;
    if (strstr (glExtensions, "GL_ARB_multitexture"))
    {
	GL::activeTexture = (GL::GLActiveTextureProc)
	    getProcAddress ("glActiveTexture");
	GL::clientActiveTexture = (GL::GLClientActiveTextureProc)
	    getProcAddress ("glClientActiveTexture");
	GL::multiTexCoord2f = (GL::GLMultiTexCoord2fProc)
	    getProcAddress ("glMultiTexCoord2f");

	if (GL::activeTexture && GL::clientActiveTexture && GL::multiTexCoord2f)
	    glGetIntegerv (GL_MAX_TEXTURE_UNITS_ARB, &GL::maxTextureUnits);
    }

    if (strstr (glExtensions, "GL_EXT_framebuffer_object"))
    {
	GL::genFramebuffers = (GL::GLGenFramebuffersProc)
	    getProcAddress ("glGenFramebuffersEXT");
	GL::deleteFramebuffers = (GL::GLDeleteFramebuffersProc)
	    getProcAddress ("glDeleteFramebuffersEXT");
	GL::bindFramebuffer = (GL::GLBindFramebufferProc)
	    getProcAddress ("glBindFramebufferEXT");
	GL::checkFramebufferStatus = (GL::GLCheckFramebufferStatusProc)
	    getProcAddress ("glCheckFramebufferStatusEXT");
	GL::framebufferTexture2D = (GL::GLFramebufferTexture2DProc)
	    getProcAddress ("glFramebufferTexture2DEXT");
	GL::generateMipmap = (GL::GLGenerateMipmapProc)
	    getProcAddress ("glGenerateMipmapEXT");
	GL::genRenderbuffers = (GL::GLGenRenderbuffersProc)
	    getProcAddress ("glGenRenderbuffersEXT");
	GL::deleteRenderbuffers = (GL::GLDeleteRenderbuffersProc)
	    getProcAddress ("glDeleteRenderbuffersEXT");
	GL::bindRenderbuffer = (GL::GLBindRenderbufferProc)
	    getProcAddress ("glBindRenderbufferEXT");
	GL::framebufferRenderbuffer = (GL::GLFramebufferRenderbufferProc)
	    getProcAddress ("glFramebufferRenderbufferEXT");
	GL::renderbufferStorage = (GL::GLRenderbufferStorageProc)
	    getProcAddress ("glRenderbufferStorageEXT");

	if (GL::genFramebuffers        &&
	    GL::deleteFramebuffers     &&
	    GL::bindFramebuffer        &&
	    GL::checkFramebufferStatus &&
	    GL::framebufferTexture2D   &&
	    GL::generateMipmap          &&
	    GL::genRenderbuffers        &&
	    GL::deleteRenderbuffers     &&
	    GL::bindRenderbuffer        &&
	    GL::framebufferRenderbuffer &&
	    GL::renderbufferStorage
	    )
	    GL::fboSupported = true;
    }

    GL::fboStencilSupported = GL::fboSupported &&
        strstr (glExtensions, "GL_EXT_packed_depth_stencil");

    if (strstr (glExtensions, "GL_ARB_vertex_buffer_object"))
    {
	GL::bindBuffer = (GL::GLBindBufferProc)
	    getProcAddress ("glBindBufferARB");
	GL::deleteBuffers = (GL::GLDeleteBuffersProc)
	    getProcAddress ("glDeleteBuffersARB");
	GL::genBuffers = (GL::GLGenBuffersProc)
	    getProcAddress ("glGenBuffersARB");
	GL::bufferData = (GL::GLBufferDataProc)
	    getProcAddress ("glBufferDataARB");
	GL::bufferSubData = (GL::GLBufferSubDataProc)
	    getProcAddress ("glBufferSubDataARB");

	if (GL::bindBuffer    &&
	    GL::deleteBuffers &&
	    GL::genBuffers    &&
	    GL::bufferData    &&
	    GL::bufferSubData)
	    GL::vboSupported = true;
    }

    priv->updateRenderMode ();

    /*
     * !!! WARNING for users of the ATI/AMD fglrx driver !!!
     *
     * fglrx contains a hack which hides GL_ARB_shading_language_100 if
     * your argv[0]=="compiz" for stupid historical reasons, so you won't
     * get shader support by default when using fglrx.
     *
     * Workaround: Rename or link your "compiz" binary to "Compiz".
     */
    if (strstr (glExtensions, "GL_ARB_fragment_shader") &&
        strstr (glExtensions, "GL_ARB_vertex_shader") &&
	strstr (glExtensions, "GL_ARB_shader_objects") &&
	strstr (glExtensions, "GL_ARB_shading_language_100"))
    {
	GL::getShaderiv = (GL::GLGetShaderivProc) GL::getShaderivARBWrapper;
	GL::getShaderInfoLog = (GL::GLGetShaderInfoLogProc) GL::getShaderInfoLogARBWrapper;
	GL::getProgramiv = (GL::GLGetProgramivProc) GL::getProgramivARBWrapper;
	GL::getProgramInfoLog = (GL::GLGetProgramInfoLogProc) GL::getProgramInfoLogARBWrapper;
	GL::getObjectParameteriv = (GL::GLGetObjectParameterivProc) getProcAddress ("glGetObjectParameterivARB");
	GL::getInfoLog = (GL::GLGetInfoLogProc) getProcAddress ("glGetInfoLogARB");
	GL::createShader = (GL::GLCreateShaderProc) GL::createShaderARBWrapper;
	GL::createShaderObjectARB = (GL::GLCreateShaderObjectARBProc) getProcAddress ("glCreateShaderObjectARB");
	GL::shaderSource = (GL::GLShaderSourceProc) GL::shaderSourceARBWrapper;
	GL::shaderSourceARB = (GL::GLShaderSourceARBProc) getProcAddress ("glShaderSourceARB");
	GL::compileShader = (GL::GLCompileShaderProc) GL::compileShaderARBWrapper;
	GL::compileShaderARB = (GL::GLCompileShaderARBProc) getProcAddress ("glCompileShaderARB");
	GL::createProgram = (GL::GLCreateProgramProc) GL::createProgramARBWrapper;
	GL::createProgramObjectARB = (GL::GLCreateProgramObjectARBProc) getProcAddress ("glCreateProgramObjectARB");
	GL::attachShader = GL::attachShaderARBWrapper;
	GL::attachObjectARB = (GL::GLAttachObjectARBProc) getProcAddress ("glAttachObjectARB");
	GL::linkProgram = GL::linkProgramARBWrapper;
	GL::linkProgramARB = (GL::GLLinkProgramARBProc) getProcAddress ("glLinkProgramARB");
	GL::validateProgram = GL::validateProgramARBWrapper;
	GL::validateProgramARB = (GL::GLValidateProgramARBProc) getProcAddress ("glValidateProgramARB");
	GL::deleteShader = GL::deleteShaderARBWrapper;
	GL::deleteProgram = GL::deleteProgramARBWrapper;
	GL::deleteObjectARB = (GL::GLDeleteObjectARBProc) getProcAddress ("glDeleteObjectARB");
	GL::useProgram = GL::useProgramARBWrapper;
	GL::useProgramObjectARB = (GL::GLUseProgramObjectARBProc) getProcAddress ("glUseProgramObjectARB");
	GL::getUniformLocation = GL::getUniformLocationARBWrapper;
	GL::getUniformLocationARB = (GL::GLGetUniformLocationARBProc) getProcAddress ("glGetUniformLocationARB");
	GL::uniform1f = (GL::GLUniform1fProc) getProcAddress ("glUniform1fARB");
	GL::uniform1i = (GL::GLUniform1iProc) getProcAddress ("glUniform1iARB");
	GL::uniform2f = (GL::GLUniform2fProc) getProcAddress ("glUniform2fARB");
	GL::uniform2i = (GL::GLUniform2iProc) getProcAddress ("glUniform2iARB");
	GL::uniform3f = (GL::GLUniform3fProc) getProcAddress ("glUniform3fARB");
	GL::uniform3i = (GL::GLUniform3iProc) getProcAddress ("glUniform3iARB");
	GL::uniform4f = (GL::GLUniform4fProc) getProcAddress ("glUniform4fARB");
	GL::uniform4i = (GL::GLUniform4iProc) getProcAddress ("glUniform4iARB");
	GL::uniformMatrix4fv = (GL::GLUniformMatrix4fvProc) getProcAddress ("glUniformMatrix4fvARB");
	GL::getAttribLocation = (GL::GLGetAttribLocationProc) GL::getAttribLocationARBWrapper;
	GL::getAttribLocationARB = (GL::GLGetAttribLocationARBProc) getProcAddress ("glGetAttribLocationARB");

	GL::enableVertexAttribArray = (GL::GLEnableVertexAttribArrayProc) getProcAddress ("glEnableVertexAttribArrayARB");
	GL::disableVertexAttribArray = (GL::GLDisableVertexAttribArrayProc) getProcAddress ("glDisableVertexAttribArrayARB");
	GL::vertexAttribPointer = (GL::GLVertexAttribPointerProc) getProcAddress ("glVertexAttribPointerARB");

	GL::shaders = true;
    }

    if (strstr (glExtensions, "GL_ARB_texture_compression"))
	GL::textureCompression = true;

    glClearColor (0.0, 0.0, 0.0, 1.0);
    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_CULL_FACE);
    glDisable (GL_BLEND);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glColor4usv (defaultColor);

    if (GL::textureEnvCombine && GL::maxTextureUnits >= 2)
    {
	GL::canDoSaturated = true;
	if (GL::textureEnvCrossbar && GL::maxTextureUnits >= 4)
	    GL::canDoSlightlySaturated = true;
    }

    priv->updateView ();

    glLightModelfv (GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    glEnable (GL_LIGHT0);
    glLightfv (GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv (GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv (GL_LIGHT0, GL_POSITION, light0Position);

    glColorMaterial (GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glNormal3f (0.0f, 0.0f, -1.0f);

    priv->lighting = false;


    priv->filter[NOTHING_TRANS_FILTER] = GLTexture::Fast;
    priv->filter[SCREEN_TRANS_FILTER]  = GLTexture::Good;
    priv->filter[WINDOW_TRANS_FILTER]  = GLTexture::Good;

    if (GL::textureFromPixmap)
	registerBindPixmap (TfpTexture::bindPixmapToTexture);
#endif

    if (GL::fboSupported)
    {
	priv->scratchFbo = new GLFramebufferObject;
	priv->scratchFbo->allocate (*screen, NULL, GL_BGRA);
    }

    GLVertexBuffer::streamingBuffer ()->setAutoProgram (priv->autoProgram);

    return true;
}


template class PluginClassHandler<GLScreen, CompScreen, COMPIZ_OPENGL_ABI>;

GLScreen::GLScreen (CompScreen *s) :
    PluginClassHandler<GLScreen, CompScreen, COMPIZ_OPENGL_ABI> (s),
    priv (new PrivateGLScreen (this))
{
    XVisualInfo		 *visinfo = NULL;
#ifndef USE_GLES
    Display		 *dpy = s->dpy ();
    XVisualInfo		 templ;
    GLXFBConfig		 *fbConfigs;
    int			 defaultDepth, nvisinfo, nElements, value, i;
    const char		 *glxExtensions;
    XWindowAttributes    attr;
    CompOption::Vector o (0);

    if (!XGetWindowAttributes (dpy, s->root (), &attr))
    {
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	setFailed ();
	return;
    }

    templ.visualid = XVisualIDFromVisual (attr.visual);

    visinfo = XGetVisualInfo (dpy, VisualIDMask, &templ, &nvisinfo);
    if (!nvisinfo)
    {
	compLogMessage ("opengl", CompLogLevelFatal,
			"Couldn't get visual info for default visual");
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	setFailed ();
	return;
    }

    defaultDepth = visinfo->depth;

    glXGetConfig (dpy, visinfo, GLX_USE_GL, &value);
    if (!value)
    {
	compLogMessage ("opengl", CompLogLevelFatal,
			"Root visual is not a GL visual");
	XFree (visinfo);
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	setFailed ();
	return;
    }

    glXGetConfig (dpy, visinfo, GLX_DOUBLEBUFFER, &value);
    if (!value)
    {
	compLogMessage ("opengl", CompLogLevelFatal,
			"Root visual is not a double buffered GL visual");
	XFree (visinfo);
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	setFailed ();
	return;
    }

    glxExtensions = glXQueryExtensionsString (dpy, s->screenNum ());

    if (glxExtensions == NULL)
    {
	compLogMessage ("opengl", CompLogLevelFatal,
	                "glXQueryExtensionsString is NULL for screen %d",
	                s->screenNum ());
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	setFailed ();
	return;
    }

    if (!strstr (glxExtensions, "GLX_SGIX_fbconfig"))
    {
	compLogMessage ("opengl", CompLogLevelFatal,
			"GLX_SGIX_fbconfig is missing");
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	setFailed ();
	return;
    }

    priv->getProcAddress = (GL::GLXGetProcAddressProc)
	getProcAddress ("glXGetProcAddressARB");
    GL::bindTexImage = (GL::GLXBindTexImageProc)
	getProcAddress ("glXBindTexImageEXT");
    GL::releaseTexImage = (GL::GLXReleaseTexImageProc)
	getProcAddress ("glXReleaseTexImageEXT");
    GL::queryDrawable = (GL::GLXQueryDrawableProc)
	getProcAddress ("glXQueryDrawable");
    GL::getFBConfigs = (GL::GLXGetFBConfigsProc)
	getProcAddress ("glXGetFBConfigs");
    GL::getFBConfigAttrib = (GL::GLXGetFBConfigAttribProc)
	getProcAddress ("glXGetFBConfigAttrib");
    GL::createPixmap = (GL::GLXCreatePixmapProc)
	getProcAddress ("glXCreatePixmap");
    GL::destroyPixmap = (GL::GLXDestroyPixmapProc)
    	getProcAddress ("glXDestroyPixmap");

    if (!strstr (glxExtensions, "GLX_EXT_texture_from_pixmap") ||
        !GL::bindTexImage || !GL::releaseTexImage)
    {
	compLogMessage ("opengl", CompLogLevelFatal,
			"GLX_EXT_texture_from_pixmap is missing");
	GL::textureFromPixmap = false;
    }
    else
	GL::textureFromPixmap = true;

    if (!GL::queryDrawable     ||
	!GL::getFBConfigs      ||
	!GL::getFBConfigAttrib ||
	!GL::createPixmap      ||
	!GL::destroyPixmap)
    {
	compLogMessage ("opengl", CompLogLevelFatal,
			"fbconfig functions missing");
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	setFailed ();
	return;
    }

    if (strstr (glxExtensions, "GLX_MESA_copy_sub_buffer"))
	GL::copySubBuffer = (GL::GLXCopySubBufferProc)
	    getProcAddress ("glXCopySubBufferMESA");

    if (strstr (glxExtensions, "GLX_SGI_video_sync"))
    {
	GL::getVideoSync = (GL::GLXGetVideoSyncProc)
	    getProcAddress ("glXGetVideoSyncSGI");

	GL::waitVideoSync = (GL::GLXWaitVideoSyncProc)
	    getProcAddress ("glXWaitVideoSyncSGI");
    }

    if (strstr (glxExtensions, "GLX_SGI_swap_control"))
    {
	GL::swapInterval = (GL::GLXSwapIntervalProc)
	    getProcAddress ("glXSwapIntervalSGI");
    }

    fbConfigs = (*GL::getFBConfigs) (dpy, s->screenNum (), &nElements);

    GL::stencilBuffer = false;

    for (i = 0; i <= MAX_DEPTH; i++)
    {
	int j, db, stencil, depth, alpha, mipmap, rgba;

	priv->glxPixmapFBConfigs[i].fbConfig       = NULL;
	priv->glxPixmapFBConfigs[i].mipmap         = 0;
	priv->glxPixmapFBConfigs[i].yInverted      = 0;
	priv->glxPixmapFBConfigs[i].textureFormat  = 0;
	priv->glxPixmapFBConfigs[i].textureTargets = 0;

	db      = MAXSHORT;
	stencil = MAXSHORT;
	depth   = MAXSHORT;
	mipmap  = 0;
	rgba    = 0;

	for (j = 0; j < nElements; j++)
	{
	    XVisualInfo *vi;
	    int		visualDepth;

	    vi = glXGetVisualFromFBConfig (dpy, fbConfigs[j]);
	    if (vi == NULL)
		continue;

	    visualDepth = vi->depth;

	    XFree (vi);

	    if (visualDepth != i)
		continue;

	    (*GL::getFBConfigAttrib) (dpy, fbConfigs[j],
				      GLX_ALPHA_SIZE, &alpha);
	    (*GL::getFBConfigAttrib) (dpy, fbConfigs[j],
				      GLX_BUFFER_SIZE, &value);
	    if (value != i && (value - alpha) != i)
		continue;

	    value = 0;
	    if (i == 32)
	    {
		(*GL::getFBConfigAttrib) (dpy, fbConfigs[j],
					  GLX_BIND_TO_TEXTURE_RGBA_EXT, &value);

		if (value)
		{
		    rgba = 1;

		    priv->glxPixmapFBConfigs[i].textureFormat =
			GLX_TEXTURE_FORMAT_RGBA_EXT;
		}
	    }

	    if (!value)
	    {
		if (rgba)
		    continue;

		(*GL::getFBConfigAttrib) (dpy, fbConfigs[j],
					  GLX_BIND_TO_TEXTURE_RGB_EXT, &value);
		if (!value)
		    continue;

		priv->glxPixmapFBConfigs[i].textureFormat =
		    GLX_TEXTURE_FORMAT_RGB_EXT;
	    }

	    (*GL::getFBConfigAttrib) (dpy, fbConfigs[j],
				      GLX_DOUBLEBUFFER, &value);
	    if (value > db)
		continue;

	    db = value;

	    (*GL::getFBConfigAttrib) (dpy, fbConfigs[j],
				      GLX_STENCIL_SIZE, &value);
	    if (value > stencil)
		continue;

	    stencil = value;

	    (*GL::getFBConfigAttrib) (dpy, fbConfigs[j],
				      GLX_DEPTH_SIZE, &value);
	    if (value > depth)
		continue;

	    depth = value;

	    (*GL::getFBConfigAttrib) (dpy, fbConfigs[j],
	                              GLX_BIND_TO_MIPMAP_TEXTURE_EXT, &value);
	    if (value < mipmap)
		continue;

	    mipmap = value;

	    (*GL::getFBConfigAttrib) (dpy, fbConfigs[j],
				      GLX_Y_INVERTED_EXT, &value);

	    priv->glxPixmapFBConfigs[i].yInverted = value;

	    (*GL::getFBConfigAttrib) (dpy, fbConfigs[j],
				      GLX_BIND_TO_TEXTURE_TARGETS_EXT, &value);

	    priv->glxPixmapFBConfigs[i].textureTargets = value;

	    priv->glxPixmapFBConfigs[i].fbConfig = fbConfigs[j];
	    priv->glxPixmapFBConfigs[i].mipmap   = mipmap;
	}

	if (i == defaultDepth)
	    if (stencil != MAXSHORT)
		GL::stencilBuffer = true;
    }

    if (nElements)
	XFree (fbConfigs);

    if (!priv->glxPixmapFBConfigs[defaultDepth].fbConfig)
    {
	compLogMessage ("opengl", CompLogLevelFatal,
			"No GLXFBConfig for default depth, "
			"this isn't going to work.");
	screen->handleCompizEvent ("opengl", "fatal_fallback", o);
	setFailed ();
    }

#endif
    if (!glInitContext (visinfo))
	setFailed ();
}

GLScreen::~GLScreen ()
{
    if (priv->hasCompositing)
	CompositeScreen::get (screen)->unregisterPaintHandler ();

    #ifdef USE_GLES
    Display *xdpy = screen->dpy ();
    EGLDisplay dpy = eglGetDisplay (xdpy);

    eglMakeCurrent (dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (priv->ctx != EGL_NO_CONTEXT)
	eglDestroyContext (dpy, priv->ctx);
    eglDestroySurface (dpy, priv->surface);
    eglTerminate (dpy);
    eglReleaseThread ();
    #else

    if (priv->ctx)
	glXDestroyContext (screen->dpy (), priv->ctx);
    #endif

    if (priv->scratchFbo)
	delete priv->scratchFbo;

    delete priv;
}

PrivateGLScreen::PrivateGLScreen (GLScreen   *gs) :
    gScreen (gs),
    cScreen (CompositeScreen::get (screen)),
    textureFilter (GL_LINEAR),
    backgroundTextures (),
    backgroundLoaded (false),
    rasterPos (0, 0),
    projection (NULL),
    clearBuffers (true),
    lighting (false),
    #ifndef USE_GLES
    ctx (NULL),
    getProcAddress (0),
    doubleBuffer (screen->dpy (), *screen, cScreen->output ()),
    #else
    ctx (EGL_NO_CONTEXT),
    doubleBuffer (screen->dpy (), *screen, surface),
    #endif
    scratchFbo (NULL),
    outputRegion (),
    lastMask (0),
    bindPixmap (),
    hasCompositing (false),
    commonFrontbuffer (true),
    programCache (new GLProgramCache (30)),
    shaderCache (),
    autoProgram (new GLScreenAutoProgram(gs)),
    rootPixmapCopy (None),
    rootPixmapSize ()
{
    ScreenInterface::setHandler (screen);
}

PrivateGLScreen::~PrivateGLScreen ()
{
    delete programCache;
    delete autoProgram;
    if (rootPixmapCopy)
	XFreePixmap (screen->dpy (), rootPixmapCopy);

}

GLushort defaultColor[4] = { 0xffff, 0xffff, 0xffff, 0xffff };



GLenum
GLScreen::textureFilter ()
{
    return priv->textureFilter;
}

void
GLScreen::setTextureFilter (GLenum filter)
{
    priv->textureFilter = filter;
}

void
PrivateGLScreen::handleEvent (XEvent *event)
{
    CompWindow *w;

    screen->handleEvent (event);

    switch (event->type) {
	case ConfigureNotify:
	    if (event->xconfigure.window == screen->root ())
		updateScreenBackground ();
	    break;
	case PropertyNotify:
	    if (event->xproperty.atom == Atoms::xBackground[0] ||
		event->xproperty.atom == Atoms::xBackground[1])
	    {
		if (event->xproperty.window == screen->root ())
		    gScreen->updateBackground ();
	    }
	    else if (event->xproperty.atom == Atoms::winOpacity ||
		     event->xproperty.atom == Atoms::winBrightness ||
		     event->xproperty.atom == Atoms::winSaturation)
	    {
		w = screen->findWindow (event->xproperty.window);
		if (w)
		    GLWindow::get (w)->updatePaintAttribs ();
	    }
	    else if (event->xproperty.atom == Atoms::wmIcon)
	    {
		w = screen->findWindow (event->xproperty.window);
		if (w)
		    GLWindow::get (w)->priv->icons.clear ();
	    }
	    break;

	default:
	    if (event->type == cScreen->damageEvent () + XDamageNotify)
	    {
		XDamageNotifyEvent *de = (XDamageNotifyEvent *) event;

		#ifdef USE_GLES
		std::map<Damage, EglTexture*>::iterator it =
		    boundPixmapTex.find (de->damage);
		#else
		std::map<Damage, TfpTexture*>::iterator it =
		    boundPixmapTex.find (de->damage);
		#endif
		if (it != boundPixmapTex.end ())
		{
		    it->second->damaged = true;
		}

		/* XXX: It would be nice if we could also update
		 * the background of the root window when the root
		 * window pixmap changes, but unfortunately XDamage
		 * reports damage events any time a child of the root
		 * window gets a damage event, which means that we'd
		 * be recopying the root window pixmap all the time
		 * which is no good, so don't do that */
	    }
	    break;
    }
}

void
GLScreen::clearTargetOutput (unsigned int mask)
{
    clearOutput (targetOutput, mask);
}


static void
frustum (GLfloat *m,
	 GLfloat left,
	 GLfloat right,
	 GLfloat bottom,
	 GLfloat top,
	 GLfloat nearval,
	 GLfloat farval)
{
    GLfloat x, y, a, b, c, d;

    x = (2.0 * nearval) / (right - left);
    y = (2.0 * nearval) / (top - bottom);
    a = (right + left) / (right - left);
    b = (top + bottom) / (top - bottom);
    c = -(farval + nearval) / ( farval - nearval);
    d = -(2.0 * farval * nearval) / (farval - nearval);

#define M(row,col)  m[col * 4 + row]
    M(0,0) = x;     M(0,1) = 0.0f;  M(0,2) = a;      M(0,3) = 0.0f;
    M(1,0) = 0.0f;  M(1,1) = y;     M(1,2) = b;      M(1,3) = 0.0f;
    M(2,0) = 0.0f;  M(2,1) = 0.0f;  M(2,2) = c;      M(2,3) = d;
    M(3,0) = 0.0f;  M(3,1) = 0.0f;  M(3,2) = -1.0f;  M(3,3) = 0.0f;
#undef M

}

static void
perspective (GLfloat *m,
	     GLfloat fovy,
	     GLfloat aspect,
	     GLfloat zNear,
	     GLfloat zFar)
{
    GLfloat xmin, xmax, ymin, ymax;

    ymax = zNear * tan (fovy * M_PI / 360.0);
    ymin = -ymax;
    xmin = ymin * aspect;
    xmax = ymax * aspect;

    frustum (m, xmin, xmax, ymin, ymax, zNear, zFar);
}

void
PrivateGLScreen::updateView ()
{
    GLfloat projection_array[16];

    #ifndef USE_GLES
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    glDepthRange (0, 1);
    glRasterPos2f (0, 0);
    #endif
    glViewport (-1, -1, 2, 2);

    rasterPos = CompPoint (0, 0);

    perspective (projection_array, 60.0f, 1.0f, 0.1f, 100.0f);

    if (projection != NULL)
	delete projection;
    projection = new GLMatrix (projection_array);

    #ifndef USE_GLES
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glMultMatrixf (projection_array);
    glMatrixMode (GL_MODELVIEW);
    #endif

    CompRegion region (screen->region ());
    /* remove all output regions from visible screen region */
    foreach (CompOutput &o, screen->outputDevs ())
	region -= o;

    /* we should clear color buffers before swapping if we have visible
	regions without output */
    clearBuffers = !region.isEmpty ();

    gScreen->setDefaultViewport ();
}

void
PrivateGLScreen::outputChangeNotify ()
{
    screen->outputChangeNotify ();

    if (scratchFbo)
	scratchFbo->allocate (*screen, NULL, GL_BGRA);
    updateView ();
}

#ifndef USE_GLES
GL::FuncPtr
GLScreen::getProcAddress (const char *name)
{
    static void *dlhand = NULL;
    GL::FuncPtr funcPtr = NULL;

    if (priv->getProcAddress)
	funcPtr = priv->getProcAddress ((GLubyte *) name);

    if (!funcPtr)
    {
	if (!dlhand)
	    dlhand = dlopen ("libopengl.so", RTLD_LAZY);

	if (dlhand)
	{
	    dlerror ();
	    funcPtr = (GL::FuncPtr) dlsym (dlhand, name);
	    if (dlerror () != NULL)
		funcPtr = NULL;
	}
    }

    return funcPtr;
}
#endif

void
PrivateGLScreen::updateScreenBackground ()
{
    Display	  *dpy = screen->dpy ();
    Atom	  pixmapAtom, actualType;
    int		  actualFormat, i, status;
    unsigned int  width = 1, height = 1, depth = 0;
    unsigned long nItems;
    unsigned long bytesAfter;
    unsigned char *prop;
    Pixmap	  pixmap = None;

    pixmapAtom = XInternAtom (dpy, "PIXMAP", false);

    for (i = 0; pixmap == 0 && i < 2; i++)
    {
	status = XGetWindowProperty (dpy, screen->root (),
				     Atoms::xBackground[i],
				     0, 4, false, AnyPropertyType,
				     &actualType, &actualFormat, &nItems,
				     &bytesAfter, &prop);

	if (status == Success && nItems && prop)
	{
	    if (actualType   == pixmapAtom &&
		actualFormat == 32         &&
		nItems	     == 1)
	    {
		Pixmap p = None;

		memcpy (&p, prop, 4);

		if (p)
		{
		    unsigned int ui;
		    int		 i;
		    Window	 w;

		    if (XGetGeometry (dpy, p, &w, &i, &i,
				      &width, &height, &ui, &depth))
		    {
			if ((int) depth == screen->attrib ().depth)
			    pixmap = p;
		    }
		}
	    }

	    XFree (prop);
	}
    }

    if (pixmap)
    {
	backgroundTextures =
	    GLTexture::bindPixmapToTexture (pixmap, width, height, depth);
	if (backgroundTextures.empty ())
	{
	    compLogMessage ("core", CompLogLevelWarn,
			    "Couldn't bind background pixmap 0x%x to "
			    "texture", (int) pixmap);
	}
    }
    else
    {
	backgroundTextures.clear ();
    }

    if (backgroundTextures.empty ())
    {
	CompSize   size;
	/* Try to get the root window background */
	XGCValues gcv;
	GC        gc;

	gcv.graphics_exposures = false;
	gcv.subwindow_mode = IncludeInferiors;
	gc = XCreateGC (screen->dpy (), screen->root (),
			GCGraphicsExposures | GCSubwindowMode, &gcv);

	if (rootPixmapSize.width () != screen->width () ||
	    rootPixmapSize.height () != screen->height ())
	{
	    if (rootPixmapCopy)
		XFreePixmap (screen->dpy (), rootPixmapCopy);

	    rootPixmapSize = CompSize (screen->width (), screen->height ());

	    rootPixmapCopy = XCreatePixmap (screen->dpy (), screen->root (),
					    rootPixmapSize.width (), rootPixmapSize.height (),
					    DefaultDepth (screen->dpy (), DefaultScreen (screen->dpy ())));

	    backgroundTextures =
	    GLTexture::bindPixmapToTexture (rootPixmapCopy, rootPixmapSize.width (), rootPixmapSize.height (),
					    DefaultDepth (screen->dpy (), DefaultScreen (screen->dpy ())));

	    if (backgroundTextures.empty ())
	    {
		compLogMessage ("core", CompLogLevelWarn,
				"Couldn't bind background pixmap 0x%x to "
				"texture", (int) screen->width ());
	    }
	}

	if (rootPixmapCopy)
	{
	    XCopyArea (screen->dpy (), screen->root (), rootPixmapCopy, gc,
		       0, 0, screen->width (), screen->height (), 0, 0);
	    XSync (screen->dpy (), false);
	}
	else
	{
	    backgroundTextures.clear ();
	}

	XFreeGC(dpy, gc);
    }
}

void
GLScreen::setTexEnvMode (GLenum mode)
{
    #ifndef USE_GLES
    if (priv->lighting)
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    else
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
    #endif
}

void
GLScreen::setLighting (bool lighting)
{
    #ifndef USE_GLES
    if (priv->lighting != lighting)
    {
	if (!priv->optionGetLighting ())
	    lighting = false;

	if (lighting)
	{
	    glEnable (GL_COLOR_MATERIAL);
	    glEnable (GL_LIGHTING);
	}
	else
	{
	    glDisable (GL_COLOR_MATERIAL);
	    glDisable (GL_LIGHTING);
	}

	priv->lighting = lighting;

	setTexEnvMode (GL_REPLACE);
    }
    #endif
}

bool
GLScreenInterface::glPaintOutput (const GLScreenPaintAttrib &sAttrib,
			          const GLMatrix            &transform,
			          const CompRegion          &region,
			          CompOutput                *output,
			          unsigned int              mask)
    WRAPABLE_DEF (glPaintOutput, sAttrib, transform, region, output, mask)

void
GLScreenInterface::glPaintTransformedOutput (const GLScreenPaintAttrib &sAttrib,
					     const GLMatrix            &transform,
					     const CompRegion          &region,
					     CompOutput                *output,
					     unsigned int              mask)
    WRAPABLE_DEF (glPaintTransformedOutput, sAttrib, transform, region,
		  output, mask)

void
GLScreenInterface::glApplyTransform (const GLScreenPaintAttrib &sAttrib,
				     CompOutput                *output,
				     GLMatrix                  *transform)
    WRAPABLE_DEF (glApplyTransform, sAttrib, output, transform)

void
GLScreenInterface::glEnableOutputClipping (const GLMatrix   &transform,
				           const CompRegion &region,
				           CompOutput       *output)
    WRAPABLE_DEF (glEnableOutputClipping, transform, region, output)

void
GLScreenInterface::glDisableOutputClipping ()
    WRAPABLE_DEF (glDisableOutputClipping)

GLMatrix *
GLScreenInterface::projectionMatrix ()
    WRAPABLE_DEF (projectionMatrix)

void
GLScreenInterface::glPaintCompositedOutput (const CompRegion    &region,
					    GLFramebufferObject *fbo,
					    unsigned int         mask)
    WRAPABLE_DEF (glPaintCompositedOutput, region, fbo, mask)

void
GLScreenInterface::glBufferStencil(const GLMatrix &matrix,
				   GLVertexBuffer &vertexBuffer,
				   CompOutput *output)
    WRAPABLE_DEF (glBufferStencil, matrix, vertexBuffer, output)

GLMatrix *
GLScreen::projectionMatrix ()
{
    WRAPABLE_HND_FUNCTN_RETURN (GLMatrix *, projectionMatrix)

    return priv->projection;
}

void
GLScreen::updateBackground ()
{
    priv->backgroundTextures.clear ();

    if (priv->backgroundLoaded)
    {
	priv->backgroundLoaded = false;
	CompositeScreen::get (screen)->damageScreen ();
    }
}

bool
GLScreen::lighting ()
{
    return priv->lighting;
}

GLTexture::Filter
GLScreen::filter (int filter)
{
    return priv->filter[filter];
}

void
GLScreen::setFilter (int num, GLTexture::Filter filter)
{
    priv->filter[num] = filter;
}

#ifndef USE_GLES
GLFBConfig*
GLScreen::glxPixmapFBConfig (unsigned int depth)
{
    return &priv->glxPixmapFBConfigs[depth];
}
#endif

void
GLScreen::clearOutput (CompOutput   *output,
		       unsigned int mask)
{
    BoxPtr pBox = &output->region ()->extents;

    if (pBox->x1 != 0	     ||
	pBox->y1 != 0	     ||
	pBox->x2 != (int) screen->width () ||
	pBox->y2 != (int) screen->height ())
    {
	glEnable (GL_SCISSOR_TEST);
	glScissor (pBox->x1,
		   screen->height () - pBox->y2,
		   pBox->x2 - pBox->x1,
		   pBox->y2 - pBox->y1);
	glClear (mask);
	glDisable (GL_SCISSOR_TEST);
    }
    else
    {
	glClear (mask);
    }
}

void
GLScreen::setDefaultViewport ()
{
    priv->lastViewport.x      = screen->outputDevs ()[0].x1 ();
    priv->lastViewport.y      = screen->height () -
				screen->outputDevs ()[0].y2 ();
    priv->lastViewport.width  = screen->outputDevs ()[0].width ();
    priv->lastViewport.height = screen->outputDevs ()[0].height ();

    glViewport (priv->lastViewport.x,
		priv->lastViewport.y,
		priv->lastViewport.width,
		priv->lastViewport.height);
}

#ifdef USE_GLES
EGLContext
GLScreen::getEGLContext ()
{
    return priv->ctx;
}
#endif

GLProgram *
GLScreen::getProgram (std::list<const GLShaderData*> shaders)
{
    return (*priv->programCache)(shaders);
}

const GLShaderData *
GLScreen::getShaderData (GLShaderParameters &params)
{
    return &priv->shaderCache.getShaderData(params);
}

namespace GL
{

void
fastSwapInterval (Display *dpy, int interval)
{
    static int prev = -1;
#ifndef USE_GLES
    bool       hasSwapInterval = GL::swapInterval ? true : false;
#else
    bool       hasSwapInterval = true;
#endif

    if (hasSwapInterval && interval != prev)
    {
#ifndef USE_GLES
	(*GL::swapInterval) (interval);
#else
	eglSwapInterval (eglGetDisplay (dpy), interval);
#endif
	prev = interval;
    }
}

void
waitForVideoSync ()
{
#ifndef USE_GLES
    GL::unthrottledFrames++;
    if (GL::waitVideoSync)
    {
	// Don't wait twice. Just in case.
	fastSwapInterval (screen->dpy (), 0);

	/*
	 * While glXSwapBuffers/glXCopySubBufferMESA are meant to do a
	 * flush before they blit, it is best to not let that happen.
	 * Because that flush would occur after GL::waitVideoSync, causing
	 * a delay and the final blit to be slightly out of sync resulting
	 * in tearing. So we need to do a glFinish before we wait for
	 * vsync, to absolutely minimize tearing.
	 */
	glFinish ();

	// Docs: http://www.opengl.org/registry/specs/SGI/video_sync.txt
	unsigned int oldCount = GL::vsyncCount;
	(*GL::waitVideoSync) (1, 0, &GL::vsyncCount);

	if (GL::vsyncCount != oldCount)
	    GL::unthrottledFrames = 0;
    }
#endif
}

void
controlSwapVideoSync (bool sync)
{
#ifndef USE_GLES
    // Docs: http://www.opengl.org/registry/specs/SGI/swap_control.txt
    if (GL::swapInterval)
    {
	fastSwapInterval (screen->dpy (), sync ? 1 : 0);
	GL::unthrottledFrames++;
    }
    else if (sync)
	waitForVideoSync ();
#else
    fastSwapInterval (screen->dpy (), sync ? 1 : 0);
    GL::unthrottledFrames++;
#endif
}

} // namespace GL

GLDoubleBuffer::GLDoubleBuffer (Display *d, const CompSize &s) :
    mDpy (d),
    mSize (s)
{
}

#ifndef USE_GLES

void
GLXDoubleBuffer::copyFrontToBack() const
{
    int w = screen->width ();
    int h = screen->height ();

    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity ();
    glOrtho (0, w, 0, h, -1.0, 1.0);
    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glLoadIdentity ();

    glReadBuffer (GL_FRONT);
    glRasterPos2i (0, 0);
    glCopyPixels (0, 0, w, h, GL_COLOR);
    glReadBuffer (GL_BACK);

    glPopMatrix ();
    glMatrixMode (GL_PROJECTION);
    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);
}

GLXDoubleBuffer::GLXDoubleBuffer (Display *d,
			      const CompSize &s,
			      Window output) :
    GLDoubleBuffer (d, s),
    mOutput (output)
{
}

void
GLXDoubleBuffer::swap () const
{
    GL::controlSwapVideoSync (setting[VSYNC]);

    glXSwapBuffers (mDpy, mOutput);
}

bool
GLXDoubleBuffer::blitAvailable () const
{
    return GL::copySubBuffer ? true : false;
}

void
GLXDoubleBuffer::blit (const CompRegion &region) const
{
    const CompRect::vector &blitRects (region.rects ());

    if (setting[VSYNC])
        GL::waitForVideoSync ();

    foreach (const CompRect &r, blitRects)
    {
	int y = mSize.height () - r.y2 ();
	(*GL::copySubBuffer) (screen->dpy (), mOutput,
			      r.x1 (), y, r.width (), r.height ());
    }
}

bool
GLXDoubleBuffer::fallbackBlitAvailable () const
{
    return true;
}

void
GLXDoubleBuffer::fallbackBlit (const CompRegion &region) const
{
    const CompRect::vector &blitRects (region.rects ());
    int w = screen->width ();
    int h = screen->height ();

    if (setting[VSYNC])
	GL::waitForVideoSync ();

    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity ();
    glOrtho (0, w, 0, h, -1.0, 1.0);
    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glLoadIdentity ();

    glDrawBuffer (GL_FRONT);
    foreach (const CompRect &r, blitRects)
    {
	int x = r.x1 ();
	int y = h - r.y2();
	glRasterPos2i (x, y);
	glCopyPixels (x, y, w, h, GL_COLOR);
    }
    glDrawBuffer (GL_BACK);

    glPopMatrix ();
    glMatrixMode (GL_PROJECTION);
    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);

    glFlush ();

}

#else

EGLDoubleBuffer::EGLDoubleBuffer (Display *d,
			      const CompSize &s,
			      EGLSurface const & surface) :
    GLDoubleBuffer (d, s),
    mSurface (surface)
{
}

void
EGLDoubleBuffer::swap () const
{
    GL::controlSwapVideoSync (setting[VSYNC]);

    eglSwapBuffers (eglGetDisplay (mDpy), mSurface);
    eglWaitGL ();
    XFlush (mDpy);
}

bool
EGLDoubleBuffer::blitAvailable () const
{
    return GL::postSubBuffer ? true : false;
}

void
EGLDoubleBuffer::blit (const CompRegion &region) const
{
    CompRect::vector blitRects (region.rects ());
    int		     y = 0;

    GL::controlSwapVideoSync (setting[VSYNC]);

    foreach (const CompRect &r, blitRects)
    {
	y = mSize.height () - r.y2 ();

	(*GL::postSubBuffer) (eglGetDisplay (screen->dpy ()),
			      mSurface,
			      r.x1 (), y,
			      r.width (),
			      r.height ());
    }

    eglWaitGL ();
    XFlush (screen->dpy ());
}

bool
EGLDoubleBuffer::fallbackBlitAvailable () const
{
    return false;
}

void
EGLDoubleBuffer::fallbackBlit (const CompRegion &region) const
{
}

void
EGLDoubleBuffer::copyFrontToBack() const
{
}

#endif

void
PrivateGLScreen::paintOutputs (CompOutput::ptrList &outputs,
			       unsigned int        mask,
			       const CompRegion    &region)
{
    if (clearBuffers)
    {
	if (mask & COMPOSITE_SCREEN_DAMAGE_ALL_MASK)
	    glClear (GL_COLOR_BUFFER_BIT);
    }

    // Disable everything that we don't usually need and could slow us down
    glDisable (GL_BLEND);
    glDisable (GL_STENCIL_TEST);
    glDisable (GL_DEPTH_TEST);
    glDepthMask (GL_FALSE);
    glStencilMask (0);

    GLFramebufferObject *oldFbo = NULL;
    bool useFbo = false;

    /* Clear the color buffer where appropriate */
    if (GL::fboEnabled && scratchFbo)
    {
	oldFbo = scratchFbo->bind ();
	useFbo = scratchFbo->checkStatus () && scratchFbo->tex ();
	if (!useFbo)
	    GLFramebufferObject::rebind (oldFbo);
    }

#ifdef UNSAFE_ARM_SGX_FIXME
    refreshSubBuffer = ((lastMask & COMPOSITE_SCREEN_DAMAGE_ALL_MASK) &&
                        !(mask & COMPOSITE_SCREEN_DAMAGE_ALL_MASK) &&
                        (mask & COMPOSITE_SCREEN_DAMAGE_REGION_MASK));

    if (refreshSubBuffer)
    {
	// FIXME: We shouldn't have to substract a 1X1 pixel region here !!
	// This is an ugly workaround for what appears to be a bug in the SGX
	// X11 driver (e.g. on Pandaboard OMAP4 platform).
	// Posting a fullscreen damage region to the SGX seems to reset the
	// framebuffer, causing the screen to blackout.
	cScreen->damageRegion (CompRegion (screen->fullscreenOutput ()) -
                               CompRegion (CompRect(0, 0, 1, 1)));
    }
#endif

    CompRegion tmpRegion = (mask & COMPOSITE_SCREEN_DAMAGE_ALL_MASK) ?
                           screen->region () : region;

    foreach (CompOutput *output, outputs)
    {
	XRectangle r;
	targetOutput = output;

	r.x	 = output->x1 ();
	r.y	 = screen->height () - output->y2 ();
	r.width  = output->width ();
	r.height = output->height ();

	if (lastViewport.x      != r.x     ||
	    lastViewport.y      != r.y     ||
	    lastViewport.width  != r.width ||
	    lastViewport.height != r.height)
	{
	    glViewport (r.x, r.y, r.width, r.height);
	    lastViewport = r;
	}

	if (mask & COMPOSITE_SCREEN_DAMAGE_ALL_MASK)
	{
	    GLMatrix identity;

	    gScreen->glPaintOutput (defaultScreenPaintAttrib,
				    identity,
				    CompRegion (*output), output,
				    PAINT_SCREEN_REGION_MASK |
				    PAINT_SCREEN_FULL_MASK);
	}
	else if (mask & COMPOSITE_SCREEN_DAMAGE_REGION_MASK)
	{
	    GLMatrix identity;

#ifdef UNSAFE_ARM_SGX_FIXME
	    /*
	     * FIXME:
	     * This code is unsafe and causes Unity bug 1036520.
	     * So it probably needs to be replaced with something else
	     * on platforms where it is required.
	     *
	     * We should NEVER be extending tmpRegion, because that's
	     * telling windows/plugins it is OK to paint outside the
	     * damaged region.
	     */
	    if (refreshSubBuffer)
		tmpRegion = CompRegion (*output);
#endif

	    outputRegion = tmpRegion & CompRegion (*output);

	    if (!gScreen->glPaintOutput (defaultScreenPaintAttrib,
					 identity,
					 outputRegion, output,
					 PAINT_SCREEN_REGION_MASK))
	    {
		identity.reset ();

		gScreen->glPaintOutput (defaultScreenPaintAttrib,
					identity,
					CompRegion (*output), output,
					PAINT_SCREEN_FULL_MASK);

		tmpRegion += *output;
	    }
	}
    }

    targetOutput = &screen->outputDevs ()[0];

    glViewport (0, 0, screen->width (), screen->height ());

    if (useFbo)
    {
	GLFramebufferObject::rebind (oldFbo);

	// FIXME: does not work if screen dimensions exceed max texture size
	//        We should try to use glBlitFramebuffer instead.
	gScreen->glPaintCompositedOutput (screen->region (), scratchFbo, mask);
    }

    if (cScreen->outputWindowChanged ())
    {
	/*
	 * Changes to the composite output window seem to take a whole frame
	 * to take effect. So to avoid a visible flicker, we skip this frame
	 * and do a full redraw next time.
	 */
	cScreen->damageScreen ();
	return;
    }

    bool alwaysSwap = optionGetAlwaysSwapBuffers ();
    bool fullscreen = useFbo ||
                      alwaysSwap ||
                      ((mask & COMPOSITE_SCREEN_DAMAGE_ALL_MASK) &&
                       commonFrontbuffer);

    doubleBuffer.set (DoubleBuffer::VSYNC, optionGetSyncToVblank ());
    doubleBuffer.set (DoubleBuffer::HAVE_PERSISTENT_BACK_BUFFER, useFbo);
    doubleBuffer.set (DoubleBuffer::NEED_PERSISTENT_BACK_BUFFER, alwaysSwap);
    doubleBuffer.render (tmpRegion, fullscreen);

    lastMask = mask;
}

bool
PrivateGLScreen::hasVSync ()
{
   #ifdef USE_GLES
   return false;
   #else
    return GL::waitVideoSync && optionGetSyncToVblank () && 
           GL::unthrottledFrames < 5;
   #endif
}

bool
PrivateGLScreen::requiredForcedRefreshRate ()
{
    return incorrectRefreshRate;
}

bool
PrivateGLScreen::compositingActive ()
{
    return true;
}

void
PrivateGLScreen::updateRenderMode ()
{
#ifndef USE_GLES
    GL::fboEnabled = GL::fboSupported && optionGetFramebufferObject ();
    GL::vboEnabled = GL::vboSupported && optionGetVertexBufferObject ();
#endif
}

void
PrivateGLScreen::prepareDrawing ()
{
    bool wasFboEnabled = GL::fboEnabled;
    updateRenderMode ();
    if (wasFboEnabled != GL::fboEnabled)
	CompositeScreen::get (screen)->damageScreen ();
}

GLTexture::BindPixmapHandle
GLScreen::registerBindPixmap (GLTexture::BindPixmapProc proc)
{
    priv->bindPixmap.push_back (proc);
    if (!priv->hasCompositing &&
	CompositeScreen::get (screen)->registerPaintHandler (priv))
	priv->hasCompositing = true;
    return priv->bindPixmap.size () - 1;
}

void
GLScreen::unregisterBindPixmap (GLTexture::BindPixmapHandle hnd)
{
    bool hasBP = false;
    priv->bindPixmap[hnd].clear ();
    for (unsigned int i = 0; i < priv->bindPixmap.size (); i++)
	if (!priv->bindPixmap[i].empty ())
	    hasBP = true;
    if (!hasBP && priv->hasCompositing)
    {
	CompositeScreen::get (screen)->unregisterPaintHandler ();
	priv->hasCompositing = false;
    }
}

GLFramebufferObject *
GLScreen::fbo ()
{
    return priv->scratchFbo;
}

GLTexture *
GLScreen::defaultIcon ()
{
    CompIcon *i = screen->defaultIcon ();
    CompSize size;

    if (!i)
	return NULL;

    if (!i->width () || !i->height ())
	return NULL;

    if (priv->defaultIcon.icon == i)
	return priv->defaultIcon.textures[0];

    priv->defaultIcon.textures =
	GLTexture::imageBufferToTexture ((char *) i->data (), *i);

    if (priv->defaultIcon.textures.size () == 1)
	priv->defaultIcon.icon = i;
    else
    {
	priv->defaultIcon.icon = NULL;
	priv->defaultIcon.textures.clear ();
    }

    return priv->defaultIcon.textures[0];
}

void
GLScreen::resetRasterPos ()
{
    #ifndef USE_GLES
    glRasterPos2f (0, 0);
    #endif
    priv->rasterPos.setX (0);
    priv->rasterPos.setY (0);
}

