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

#ifndef _COMPIZ_OPENGL_H
#define _COMPIZ_OPENGL_H

#ifdef USE_GLES
#define SUPPORT_X11
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#else
#include <GL/gl.h>
#include <GL/glx.h>
#endif

#include <core/size.h>
#include <core/pluginclasshandler.h>

#include <opengl/matrix.h>
#include <opengl/texture.h>
#include <opengl/framebufferobject.h>
#include <opengl/vertexbuffer.h>
#include <opengl/program.h>
#include <opengl/programcache.h>
#include <opengl/shadercache.h>

#define COMPIZ_OPENGL_ABI 6

/*
 * Some plugins check for #ifdef USE_MODERN_COMPIZ_GL. Support it for now, but
 * but the offending code should be changed to:  #if COMPIZ_OPENGL_ABI >= 5
 * Or the preprocessor checks should be removed altogether.
 */
#define USE_MODERN_COMPIZ_GL 1

#if !defined(GL_BGRA)
    #if !defined(GL_BGRA_EXT)
	#error GL_BGRA support is required
    #else
	#define GL_BGRA GL_BGRA_EXT
    #endif
#endif

#if !defined(GL_BGRA)
    #if !defined(GL_BGRA_EXT)
	#error GL_BGRA support is required
    #else
	#define GL_BGRA GL_BGRA_EXT
    #endif
#endif

/**
 * camera distance from screen, 0.5 * tan (FOV)
 */
extern const float DEFAULT_Z_CAMERA;

extern const float RED_SATURATION_WEIGHT;
extern const float GREEN_SATURATION_WEIGHT;
extern const float BLUE_SATURATION_WEIGHT;

class PrivateGLScreen;
class PrivateGLWindow;

extern GLushort   defaultColor[4];

#ifndef GLX_EXT_texture_from_pixmap
#define GLX_BIND_TO_TEXTURE_RGB_EXT        0x20D0
#define GLX_BIND_TO_TEXTURE_RGBA_EXT       0x20D1
#define GLX_BIND_TO_MIPMAP_TEXTURE_EXT     0x20D2
#define GLX_BIND_TO_TEXTURE_TARGETS_EXT    0x20D3
#define GLX_Y_INVERTED_EXT                 0x20D4
#define GLX_TEXTURE_FORMAT_EXT             0x20D5
#define GLX_TEXTURE_TARGET_EXT             0x20D6
#define GLX_MIPMAP_TEXTURE_EXT             0x20D7
#define GLX_TEXTURE_FORMAT_NONE_EXT        0x20D8
#define GLX_TEXTURE_FORMAT_RGB_EXT         0x20D9
#define GLX_TEXTURE_FORMAT_RGBA_EXT        0x20DA
#define GLX_TEXTURE_1D_BIT_EXT             0x00000001
#define GLX_TEXTURE_2D_BIT_EXT             0x00000002
#define GLX_TEXTURE_RECTANGLE_BIT_EXT      0x00000004
#define GLX_TEXTURE_1D_EXT                 0x20DB
#define GLX_TEXTURE_2D_EXT                 0x20DC
#define GLX_TEXTURE_RECTANGLE_EXT          0x20DD
#define GLX_FRONT_LEFT_EXT                 0x20DE
#endif

namespace GL {
    #ifdef USE_GLES
    typedef EGLImageKHR (*EGLCreateImageKHRProc)  (EGLDisplay dpy,
                                                   EGLContext ctx,
                                                   EGLenum target,
                                                   EGLClientBuffer buffer,
                                                   const EGLint *attrib_list);
    typedef EGLBoolean  (*EGLDestroyImageKHRProc) (EGLDisplay dpy,
                                                   EGLImageKHR image);

    typedef void (*GLEGLImageTargetTexture2DOESProc) (GLenum target,
                                                      GLeglImageOES image);

    typedef EGLBoolean (*EGLPostSubBufferNVProc) (EGLDisplay dpy,
						  EGLSurface surface,
						  EGLint x, EGLint y,
						  EGLint width, EGLint height);

    #else
    typedef void (*FuncPtr) (void);

    typedef FuncPtr (*GLXGetProcAddressProc) (const GLubyte *procName);

    typedef void    (*GLXBindTexImageProc)    (Display	 *display,
					       GLXDrawable	 drawable,
					       int		 buffer,
					       int		 *attribList);
    typedef void    (*GLXReleaseTexImageProc) (Display	 *display,
					       GLXDrawable	 drawable,
					       int		 buffer);
    typedef void    (*GLXQueryDrawableProc)   (Display	 *display,
					       GLXDrawable	 drawable,
					       int		 attribute,
					       unsigned int  *value);

    typedef void (*GLXCopySubBufferProc) (Display     *display,
					  GLXDrawable drawable,
					  int	  x,
					  int	  y,
					  int	  width,
					  int	  height);

    typedef int (*GLXGetVideoSyncProc)  (unsigned int *count);
    typedef int (*GLXWaitVideoSyncProc) (int	  divisor,
					 int	  remainder,
					 unsigned int *count);
    typedef int (*GLXSwapIntervalProc) (int interval);


    #ifndef GLX_VERSION_1_3
    typedef struct __GLXFBConfigRec *GLXFBConfig;
    #endif

    typedef GLXFBConfig *(*GLXGetFBConfigsProc) (Display *display,
						 int     screen,
						 int     *nElements);
    typedef int (*GLXGetFBConfigAttribProc) (Display     *display,
					     GLXFBConfig config,
					     int	     attribute,
					     int	     *value);
    typedef GLXPixmap (*GLXCreatePixmapProc) (Display     *display,
					      GLXFBConfig config,
					      Pixmap      pixmap,
					      const int   *attribList);
    typedef void      (*GLXDestroyPixmapProc) (Display *display,
    					       GLXPixmap pixmap);
    typedef void (*GLGenProgramsProc) (GLsizei n,
				       GLuint  *programs);
    typedef void (*GLDeleteProgramsProc) (GLsizei n,
					  GLuint  *programs);
    typedef void (*GLBindProgramProc) (GLenum target,
				       GLuint program);
    typedef void (*GLProgramStringProc) (GLenum	  target,
					 GLenum	  format,
					 GLsizei	  len,
					 const GLvoid *string);
    typedef void (*GLProgramParameter4fProc) (GLenum  target,
					      GLuint  index,
					      GLfloat x,
					      GLfloat y,
					      GLfloat z,
					      GLfloat w);
    typedef void (*GLGetProgramivProc) (GLenum target,
					GLenum pname,
					int    *params);
    #endif

    typedef void (*GLActiveTextureProc) (GLenum texture);
    typedef void (*GLClientActiveTextureProc) (GLenum texture);
    typedef void (*GLMultiTexCoord2fProc) (GLenum, GLfloat, GLfloat);

    typedef void (*GLGenFramebuffersProc) (GLsizei n,
					   GLuint  *framebuffers);
    typedef void (*GLDeleteFramebuffersProc) (GLsizei n,
					      const GLuint  *framebuffers);
    typedef void (*GLBindFramebufferProc) (GLenum target,
					   GLuint framebuffer);
    typedef GLenum (*GLCheckFramebufferStatusProc) (GLenum target);
    typedef void (*GLFramebufferTexture2DProc) (GLenum target,
						GLenum attachment,
						GLenum textarget,
						GLuint texture,
						GLint  level);
    typedef void (*GLGenerateMipmapProc) (GLenum target);

    typedef void (*GLBindBufferProc) (GLenum target,
                                      GLuint buffer);
    typedef void (*GLDeleteBuffersProc) (GLsizei n,
                                         const GLuint *buffers);
    typedef void (*GLGenBuffersProc) (GLsizei n,
                                      GLuint *buffers);
    typedef void (*GLBufferDataProc) (GLenum target,
                                      GLsizeiptr size,
                                      const GLvoid *data,
                                      GLenum usage);
    typedef void (*GLBufferSubDataProc) (GLenum target,
                                         GLintptr offset,
                                         GLsizeiptr size,
                                         const GLvoid *data);

    typedef void (*GLGetShaderivProc) (GLuint shader,
                                       GLenum pname,
                                       GLint *params);
    typedef void (*GLGetShaderInfoLogProc) (GLuint shader,
                                            GLsizei bufsize,
                                            GLsizei *length,
                                            GLchar *infoLog);
    typedef void (*GLGetProgramivProc) (GLuint program,
                                        GLenum pname,
                                        GLint* params);
    typedef void (*GLGetProgramInfoLogProc) (GLuint program,
                                             GLsizei bufsize,
                                             GLsizei *length,
                                             GLchar *infoLog);
    typedef GLuint (*GLCreateShaderProc) (GLenum type);
    typedef void (*GLShaderSourceProc) (GLuint shader,
                                        GLsizei count,
                                        const GLchar **string,
                                        const GLint* length);
    typedef void (*GLCompileShaderProc) (GLuint shader);
    typedef GLuint (*GLCreateProgramProc) ();
    typedef void (*GLAttachShaderProc) (GLuint program,
                                        GLuint shader);
    typedef void (*GLLinkProgramProc) (GLuint program);
    typedef void (*GLValidateProgramProc) (GLuint program);
    typedef void (*GLDeleteShaderProc) (GLuint shader);
    typedef void (*GLDeleteProgramProc) (GLuint program);
    typedef void (*GLUseProgramProc) (GLuint program);
    typedef int  (*GLGetUniformLocationProc) (GLuint program,
                                              const GLchar* name);
    typedef void (*GLUniform1fProc) (GLint location, GLfloat x);
    typedef void (*GLUniform1iProc) (GLint location, GLint x);
    typedef void (*GLUniform2fProc) (GLint location, GLfloat x, GLfloat y);
    typedef void (*GLUniform3fProc) (GLint location,
                                     GLfloat x,
                                     GLfloat y,
                                     GLfloat z);
    typedef void (*GLUniform4fProc) (GLint location,
                                     GLfloat x,
                                     GLfloat y,
                                     GLfloat z,
                                     GLfloat w);
    typedef void (*GLUniform2iProc) (GLint location, GLint x, GLint y);
    typedef void (*GLUniform3iProc) (GLint location,
                                     GLint x,
                                     GLint y,
                                     GLint z);
    typedef void (*GLUniform4iProc) (GLint location,
                                     GLint x,
                                     GLint y,
                                     GLint z,
                                     GLint w);
    typedef void (*GLUniformMatrix4fvProc) (GLint location,
                                            GLsizei count,
                                            GLboolean transpose,
                                            const GLfloat *value);
    typedef int (*GLGetAttribLocationProc) (GLuint program,
                                            const GLchar *name);

    typedef void (*GLEnableVertexAttribArrayProc) (GLuint index);
    typedef void (*GLDisableVertexAttribArrayProc) (GLuint index);
    typedef void (*GLVertexAttribPointerProc) (GLuint index,
                                               GLint size,
                                               GLenum type,
                                               GLboolean normalized,
                                               GLsizei stride,
                                               const GLvoid *ptr);

    typedef void (*GLGenRenderbuffersProc) (GLsizei n,
					GLuint  *rb);
    typedef void (*GLDeleteRenderbuffersProc) (GLsizei n,
					   const GLuint  *rb);
    typedef void (*GLBindRenderbufferProc) (GLenum target,
					GLuint renderbuffer);
    typedef void (*GLFramebufferRenderbufferProc) (GLenum target,
					       GLenum attachment,
					       GLenum renderbuffertarget,
					       GLuint renderbuffer);
    typedef void (*GLRenderbufferStorageProc) (GLenum target,
					   GLenum internalformat,
					   GLsizei width,
					   GLsizei height);


    /* GL_ARB_shader_objects */
    #ifndef USE_GLES
    typedef GLhandleARB (*GLCreateShaderObjectARBProc) (GLenum type);
    typedef GLhandleARB (*GLCreateProgramObjectARBProc) ();
    typedef void (*GLShaderSourceARBProc) (GLhandleARB shader,
                                        GLsizei count,
                                        const GLchar **string,
                                        const GLint* length);
    typedef void (*GLCompileShaderARBProc) (GLhandleARB shader);
    typedef void (*GLValidateProgramARBProc) (GLhandleARB program);
    typedef void (*GLDeleteObjectARBProc) (GLhandleARB object);
    typedef void (*GLAttachObjectARBProc) (GLhandleARB program,
                                        GLhandleARB shader);
    typedef void (*GLLinkProgramARBProc) (GLhandleARB program);
    typedef void (*GLUseProgramObjectARBProc) (GLhandleARB program);
    typedef int  (*GLGetUniformLocationARBProc) (GLhandleARB program,
                                              const GLchar* name);
    typedef int (*GLGetAttribLocationARBProc) (GLhandleARB program,
                                            const GLchar *name);

    typedef void (*GLGetObjectParameterivProc) (GLhandleARB object, GLenum type, int *param);
    typedef void (*GLGetInfoLogProc) (GLhandleARB object, int maxLen, int *len, char *log);
    #endif

    #ifdef USE_GLES
    extern EGLCreateImageKHRProc  createImage;
    extern EGLDestroyImageKHRProc destroyImage;

    extern GLEGLImageTargetTexture2DOESProc eglImageTargetTexture;

    #else
    extern GLXBindTexImageProc      bindTexImage;
    extern GLXReleaseTexImageProc   releaseTexImage;
    extern GLXQueryDrawableProc     queryDrawable;
    extern GLXCopySubBufferProc     copySubBuffer;
    extern GLXGetVideoSyncProc      getVideoSync;
    extern GLXWaitVideoSyncProc     waitVideoSync;
    extern GLXSwapIntervalProc      swapInterval;
    extern GLXGetFBConfigsProc      getFBConfigs;
    extern GLXGetFBConfigAttribProc getFBConfigAttrib;
    extern GLXCreatePixmapProc      createPixmap;
    extern GLXDestroyPixmapProc     destroyPixmap;
    extern GLGenProgramsProc        genPrograms;
    extern GLDeleteProgramsProc     deletePrograms;
    extern GLBindProgramProc        bindProgram;
    extern GLProgramStringProc      programString;
    extern GLProgramParameter4fProc programEnvParameter4f;
    extern GLProgramParameter4fProc programLocalParameter4f;
    extern GLGetProgramivProc       getProgramiv;
    #endif

    extern GLActiveTextureProc       activeTexture;
    extern GLClientActiveTextureProc clientActiveTexture;
    extern GLMultiTexCoord2fProc     multiTexCoord2f;

    extern GLGenFramebuffersProc        genFramebuffers;
    extern GLDeleteFramebuffersProc     deleteFramebuffers;
    extern GLBindFramebufferProc        bindFramebuffer;
    extern GLCheckFramebufferStatusProc checkFramebufferStatus;
    extern GLFramebufferTexture2DProc   framebufferTexture2D;
    extern GLGenerateMipmapProc         generateMipmap;

    extern GLBindBufferProc    bindBuffer;
    extern GLDeleteBuffersProc deleteBuffers;
    extern GLGenBuffersProc    genBuffers;
    extern GLBufferDataProc    bufferData;
    extern GLBufferSubDataProc bufferSubData;


    extern GLGetShaderivProc        getShaderiv;
    extern GLGetShaderInfoLogProc   getShaderInfoLog;
    extern GLGetProgramivProc       getProgramiv;
    extern GLGetProgramInfoLogProc  getProgramInfoLog;
    extern GLCreateShaderProc       createShader;
    extern GLShaderSourceProc       shaderSource;
    extern GLCompileShaderProc      compileShader;
    extern GLCreateProgramProc      createProgram;
    extern GLAttachShaderProc       attachShader;
    extern GLLinkProgramProc        linkProgram;
    extern GLValidateProgramProc    validateProgram;
    extern GLDeleteShaderProc       deleteShader;
    extern GLDeleteProgramProc      deleteProgram;
    extern GLUseProgramProc         useProgram;
    extern GLGetUniformLocationProc getUniformLocation;
    extern GLUniform1fProc          uniform1f;
    extern GLUniform1iProc          uniform1i;
    extern GLUniform2fProc          uniform2f;
    extern GLUniform2iProc          uniform2i;
    extern GLUniform3fProc          uniform3f;
    extern GLUniform3iProc          uniform3i;
    extern GLUniform4fProc          uniform4f;
    extern GLUniform4iProc          uniform4i;
    extern GLUniformMatrix4fvProc   uniformMatrix4fv;
    extern GLGetAttribLocationProc  getAttribLocation;

    extern GLEnableVertexAttribArrayProc  enableVertexAttribArray;
    extern GLDisableVertexAttribArrayProc disableVertexAttribArray;
    extern GLVertexAttribPointerProc      vertexAttribPointer;

    extern GLGenRenderbuffersProc genRenderbuffers;
    extern GLDeleteRenderbuffersProc deleteRenderbuffers;
    extern GLBindRenderbufferProc    bindRenderbuffer;
    extern GLFramebufferRenderbufferProc framebufferRenderbuffer;
    extern GLRenderbufferStorageProc renderbufferStorage;

    #ifndef USE_GLES
    extern GLCreateShaderObjectARBProc createShaderObjectARB;
    extern GLCreateProgramObjectARBProc createProgramObjectARB;
    extern GLShaderSourceARBProc shaderSourceARB;
    extern GLCompileShaderARBProc compileShaderARB;
    extern GLValidateProgramARBProc validateProgramARB;
    extern GLDeleteObjectARBProc deleteObjectARB;
    extern GLAttachObjectARBProc attachObjectARB;
    extern GLLinkProgramARBProc linkProgramARB;
    extern GLUseProgramObjectARBProc useProgramObjectARB;
    extern GLGetUniformLocationARBProc getUniformLocationARB;
    extern GLGetAttribLocationARBProc getAttribLocationARB;

    extern GLGetObjectParameterivProc   getObjectParameteriv;
    extern GLGetInfoLogProc	    getInfoLog;
    #endif

#ifdef USE_GLES

    static const GLenum 		    FRAMEBUFFER_BINDING = GL_FRAMEBUFFER_BINDING;
    static const GLenum 		    FRAMEBUFFER = GL_FRAMEBUFFER;
    static const GLenum			    RENDERBUFFER = GL_RENDERBUFFER;
    static const GLenum 		    COLOR_ATTACHMENT0 = GL_COLOR_ATTACHMENT0;
    static const GLenum 		    DEPTH_ATTACHMENT = GL_DEPTH_ATTACHMENT;
    static const GLenum			    STENCIL_ATTACHMENT = GL_STENCIL_ATTACHMENT;
    static const GLenum			    DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8_OES;

    /* OpenGL|ES does not support different draw/read framebuffers */
    static const GLenum 		    DRAW_FRAMEBUFFER = GL_FRAMEBUFFER;
    static const GLenum 		    READ_FRAMEBUFFER = GL_FRAMEBUFFER;

    static const GLenum 		    FRAMEBUFFER_COMPLETE = GL_FRAMEBUFFER_COMPLETE;
    static const GLenum 		    FRAMEBUFFER_UNDEFINED = 0;
    static const GLenum 		    FRAMEBUFFER_INCOMPLETE_ATTACHMENT = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
    static const GLenum 		    FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
    static const GLenum 		    FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER = 0;
    static const GLenum 		    FRAMEBUFFER_INCOMPLETE_READ_BUFFER = 0;
    static const GLenum 		    FRAMEBUFFER_UNSUPPORTED = GL_FRAMEBUFFER_UNSUPPORTED;
    static const GLenum 		    FRAMEBUFFER_INCOMPLETE_MULTISAMPLE = 0;
    static const GLenum 		    FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS = 0;
    static const GLenum 		    FRAMEBUFFER_INCOMPLETE_DIMENSIONS = GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS;

    static const GLenum 		    ARRAY_BUFFER = GL_ARRAY_BUFFER;
    static const GLenum 		    STATIC_DRAW = GL_STATIC_DRAW;
    static const GLenum 		    STREAM_DRAW = GL_STREAM_DRAW;
    static const GLenum 		    DYNAMIC_DRAW = GL_DYNAMIC_DRAW;

    static const GLenum 		    INFO_LOG_LENGTH = GL_INFO_LOG_LENGTH;
    static const GLenum 		    COMPILE_STATUS = GL_COMPILE_STATUS;
    static const GLenum 		    LINK_STATUS = GL_LINK_STATUS;
    static const GLenum 		    FRAGMENT_SHADER = GL_FRAGMENT_SHADER;
    static const GLenum 		    VERTEX_SHADER = GL_VERTEX_SHADER;

#else

    static const GLenum 		  FRAMEBUFFER_BINDING = GL_FRAMEBUFFER_BINDING_EXT;
    static const GLenum 		  FRAMEBUFFER = GL_FRAMEBUFFER_EXT;
    static const GLenum			  RENDERBUFFER = GL_RENDERBUFFER;
    static const GLenum 		  COLOR_ATTACHMENT0 = GL_COLOR_ATTACHMENT0_EXT;
    static const GLenum 		  DEPTH_ATTACHMENT = GL_DEPTH_ATTACHMENT_EXT;
    static const GLenum			  STENCIL_ATTACHMENT = GL_STENCIL_ATTACHMENT_EXT;
    static const GLenum			  DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8_EXT;

    static const GLenum 		  DRAW_FRAMEBUFFER = GL_DRAW_FRAMEBUFFER_EXT;
    static const GLenum 		  READ_FRAMEBUFFER = GL_READ_FRAMEBUFFER_EXT;
    static const GLenum 		  FRAMEBUFFER_COMPLETE = GL_FRAMEBUFFER_COMPLETE_EXT;
    static const GLenum 		  FRAMEBUFFER_UNDEFINED = GL_FRAMEBUFFER_UNDEFINED;
    static const GLenum 		  FRAMEBUFFER_INCOMPLETE_ATTACHMENT = GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT;
    static const GLenum 		  FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT = GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT;
    static const GLenum 		  FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER = GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT;
    static const GLenum 		  FRAMEBUFFER_INCOMPLETE_READ_BUFFER = GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT;
    static const GLenum 		  FRAMEBUFFER_UNSUPPORTED = GL_FRAMEBUFFER_UNSUPPORTED_EXT;
    static const GLenum 		  FRAMEBUFFER_INCOMPLETE_MULTISAMPLE = GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT;
    static const GLenum 		  FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS = GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS_EXT;
    static const GLenum 		  FRAMEBUFFER_INCOMPLETE_DIMENSIONS = 0;

    static const GLenum 		  ARRAY_BUFFER = GL_ARRAY_BUFFER_ARB;
    static const GLenum 		  STATIC_DRAW = GL_STATIC_DRAW_ARB;
    static const GLenum 		  STREAM_DRAW = GL_STREAM_DRAW_ARB;
    static const GLenum 		  DYNAMIC_DRAW = GL_DYNAMIC_DRAW_ARB;

    static const GLenum 		  INFO_LOG_LENGTH = GL_OBJECT_INFO_LOG_LENGTH_ARB;
    static const GLenum 		  COMPILE_STATUS = GL_OBJECT_COMPILE_STATUS_ARB;
    static const GLenum 		  LINK_STATUS = GL_OBJECT_LINK_STATUS_ARB;
    static const GLenum 		  FRAGMENT_SHADER = GL_FRAGMENT_SHADER_ARB;
    static const GLenum 		  VERTEX_SHADER = GL_VERTEX_SHADER_ARB;

#endif

    extern bool  textureFromPixmap;
    extern bool  textureRectangle;
    extern bool  textureNonPowerOfTwo;
    extern bool  textureNonPowerOfTwoMipmap;
    extern bool  textureEnvCombine;
    extern bool  textureEnvCrossbar;
    extern bool  textureBorderClamp;
    extern bool  textureCompression;
    extern GLint maxTextureSize;
    extern bool  fboSupported;
    extern bool  fboStencilSupported;
    extern bool  fboEnabled;
    extern bool  vboSupported;
    extern bool  vboEnabled;
    extern bool  shaders;
    extern bool  stencilBuffer;
    extern GLint maxTextureUnits;

    extern bool canDoSaturated;
    extern bool canDoSlightlySaturated;

#ifndef USE_GLES
    void getProgramInfoLogARBWrapper (GLuint object, int maxLen, int *len, char *log);
    void getShaderInfoLogARBWrapper (GLuint object, int maxLen, int *len, char *log);
    void getShaderivARBWrapper (GLuint object, GLenum type, int *param);
    void getProgramivARBWrapper (GLuint object, GLenum type, int *param);
    GLuint createShaderARBWrapper (GLenum type);
    GLuint createProgramARBWrapper (GLenum type);
    void shaderSourceARBWrapper (GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
    void compileShaderARBWrapper (GLuint shader);
    void validateProgramARBWrapper (GLuint program);
    void deleteShaderARBWrapper (GLuint shader);
    void deleteProgramARBWrapper (GLuint program);
    void attachShaderARBWrapper (GLuint program, GLuint shader);
    void linkProgramARBWrapper (GLuint program);
    void useProgramARBWrapper (GLuint program);
    int getUniformLocationARBWrapper (GLuint program, const GLchar *name);
    int getAttribLocationARBWrapper (GLuint program, const GLchar *name);
#endif
};

struct GLScreenPaintAttrib {
    GLfloat xRotate;
    GLfloat yRotate;
    GLfloat vRotate;
    GLfloat xTranslate;
    GLfloat yTranslate;
    GLfloat zTranslate;
    GLfloat zCamera;
};

#define MAX_DEPTH 32

#ifndef USE_GLES
struct GLFBConfig {
    GLXFBConfig fbConfig;
    int         yInverted;
    int         mipmap;
    int         textureFormat;
    int         textureTargets;
};
#endif

extern const unsigned short NOTHING_TRANS_FILTER;
extern const unsigned short SCREEN_TRANS_FILTER;
extern const unsigned short WINDOW_TRANS_FILTER;


extern GLScreenPaintAttrib defaultScreenPaintAttrib;

class GLScreen;
class GLFramebufferObject;
class GLScreenInterface;
extern template class WrapableInterface<GLScreen, GLScreenInterface>;

class GLScreenInterface :
    public WrapableInterface<GLScreen, GLScreenInterface>
{
    public:

	/**
	 * Hookable function used for plugins to use openGL to draw on an output
	 *
	 * @param attrib Describes some basic drawing attribs for the screen
	 * including translation, rotation and scale
	 * @param matrix Describes a 4x4 3D modelview matrix for which this
	 * screen should be drawn in
	 * @param region Describes the region of the screen being redrawn
	 * @param output Describes the output being redrawn
	 * @param mask   Bitmask which describes how the screen is being redrawn'
	 */
	virtual bool glPaintOutput (const GLScreenPaintAttrib &attrib,
				    const GLMatrix 	      &matrix,
				    const CompRegion 	      &region,
				    CompOutput 		      *output,
				    unsigned int	      mask);


	/**
	 * Hookable function used for plugins to use openGL to draw on an output
	 * when the screen is transformed
	 *
	 * There is little difference between this and glPaintOutput, however
	 * this will be called when the entire screen is being transformed
	 * (eg cube)
	 *
	 * @param attrib Describes some basic drawing attribs for the screen
	 * including translation, rotation and scale
	 * @param matrix Describes a 4x4 3D modelview matrix for which this
	 * screen should be drawn in
	 * @param region Describes the region of the screen being redrawn
	 * @param output Describes the output being redrawn
	 * @param mask   Bitmask which describes how the screen is being redrawn'
	 */
	virtual void glPaintTransformedOutput (const GLScreenPaintAttrib &attrib,
					       const GLMatrix 		 &matrix,
					       const CompRegion 	 &region,
					       CompOutput 		 *output,
					       unsigned int		 mask);

	/**
	 * Hookable function to apply elements from a GLScreenPaintAttrib
	 * to a GLMatrix in the context of a CompOutput
	 *
	 * @param attrib Describes the basic drawing attribs of a screen
	 * including translation, rotation and scale to be applies to a matrix
	 * @param output Describes the output in which these operations take
	 * place
	 * @param matrix Pointer to a matrix where transformations will
	 * be applied
	 */
	virtual void glApplyTransform (const GLScreenPaintAttrib &attrib,
				       CompOutput 		 *output,
				       GLMatrix 		 *mask);

	virtual void glEnableOutputClipping (const GLMatrix &,
					     const CompRegion &,
					     CompOutput *);
	virtual void glDisableOutputClipping ();

	virtual GLMatrix *projectionMatrix ();

	/**
	 * Hookable function used by plugins to shade the final composited
	 * Output.
	 *
	 * @param tmpRegion Describes the final composited output region
	 * @param scratchFbo Describes the final composited FBO that is
	 * to be rendered.
	 */
	virtual void glPaintCompositedOutput (const CompRegion    &region,
					      GLFramebufferObject *fbo,
					      unsigned int         mask);

	/**
	 * Hookable function used by plugins to determine stenciling mask
	 */
	virtual void glBufferStencil (const GLMatrix       &matrix,
				      GLVertexBuffer       &vertexBuffer,
				      CompOutput           *output);

};

extern template class PluginClassHandler<GLScreen, CompScreen, COMPIZ_OPENGL_ABI>;

class GLScreen :
    public WrapableHandler<GLScreenInterface, 8>,
    public PluginClassHandler<GLScreen, CompScreen, COMPIZ_OPENGL_ABI>,
    public CompOption::Class
{
    public:
	GLScreen (CompScreen *s);
	~GLScreen ();

	CompOption::Vector & getOptions ();
        bool setOption (const CompString &name, CompOption::Value &value);

	/**
	 * Returns the current compiz-wide openGL texture filter
	 */
	GLenum textureFilter ();

	/**
	 * Sets a new compiz-wide openGL texture filter
	 */
	void setTextureFilter (GLenum);

	void clearTargetOutput (unsigned int mask);

	/**
	 * Gets the libGL address of a particular openGL functor
	 */
	#ifndef USE_GLES
	GL::FuncPtr getProcAddress (const char *name);
	#endif

	void updateBackground ();

	/**
	 * Returns the current compiz-wide texture filter
	 */
	GLTexture::Filter filter (int);

	/**
	 * Sets a new compiz-wide texture filter
	 */
	void setFilter (int, GLTexture::Filter);

	/**
	 * Sets a new compiz-wid openGL texture environment mode
	 */
	void setTexEnvMode (GLenum mode);

	/**
	 * Turns lighting on and off
	 */
	void setLighting (bool lighting);

	/**
	 * Returns true if lighting is enabled
	 */
	bool lighting ();

	void clearOutput (CompOutput *output, unsigned int mask);

	void setDefaultViewport ();

	GLTexture::BindPixmapHandle registerBindPixmap (GLTexture::BindPixmapProc);
	void unregisterBindPixmap (GLTexture::BindPixmapHandle);

	#ifndef USE_GLES
	GLFBConfig * glxPixmapFBConfig (unsigned int depth);
	#endif

	#ifdef USE_GLES
	EGLContext getEGLContext ();
	#endif

	/**
	 * Returns a GLProgram from the cache or creates one and caches it
	 */
	GLProgram *getProgram (std::list<const GLShaderData*>);

	/**
	 * Returns a GLShaderData from the cache or creates one and caches it
	 */
	const GLShaderData *getShaderData (GLShaderParameters &params);

	/**
	 * Returns the FBO compiz is using for the screen
	 */
	GLFramebufferObject *fbo ();

	/**
	 * Returns a default icon texture
	 */
	GLTexture *defaultIcon ();

	void resetRasterPos ();

	bool glInitContext (XVisualInfo *);

	WRAPABLE_HND (0, GLScreenInterface, bool, glPaintOutput,
		      const GLScreenPaintAttrib &, const GLMatrix &,
		      const CompRegion &, CompOutput *, unsigned int);
	WRAPABLE_HND (1, GLScreenInterface, void, glPaintTransformedOutput,
		      const GLScreenPaintAttrib &,
		      const GLMatrix &, const CompRegion &, CompOutput *,
		      unsigned int);
	WRAPABLE_HND (2, GLScreenInterface, void, glApplyTransform,
		      const GLScreenPaintAttrib &, CompOutput *, GLMatrix *);

	WRAPABLE_HND (3, GLScreenInterface, void, glEnableOutputClipping,
		      const GLMatrix &, const CompRegion &, CompOutput *);
	WRAPABLE_HND (4, GLScreenInterface, void, glDisableOutputClipping);

	WRAPABLE_HND (5, GLScreenInterface, GLMatrix *, projectionMatrix);
	WRAPABLE_HND (6, GLScreenInterface, void, glPaintCompositedOutput,
		      const CompRegion &, GLFramebufferObject *, unsigned int);

	WRAPABLE_HND (7, GLScreenInterface, void, glBufferStencil, const GLMatrix &,
		      GLVertexBuffer &,
		      CompOutput *);

	friend class GLTexture;
	friend class GLWindow;

    private:
	PrivateGLScreen *priv;
};

struct GLWindowPaintAttrib {
    GLushort opacity;
    GLushort brightness;
    GLushort saturation;
    GLfloat  xScale;
    GLfloat  yScale;
    GLfloat  xTranslate;
    GLfloat  yTranslate;
};

class GLWindow;
class GLWindowInterface;
extern template class WrapableInterface<GLWindow, GLWindowInterface>;

class GLWindowInterface :
    public WrapableInterface<GLWindow, GLWindowInterface>
{
    public:

	/**
	 * Hookable function to paint a window on-screen
	 *
	 * @param attrib Describes basic drawing attribs of this window;
	 * opacity, brightness, saturation
	 * @param matrix A 4x4 matrix which describes the transformation of
	 * this window
	 * @param region Describes the region of the window being drawn
	 * @param mask   Bitmask which describes how this window is drawn
	 */
	virtual bool glPaint (const GLWindowPaintAttrib &attrib,
			      const GLMatrix 		&matrix,
			      const CompRegion 		&region,
			      unsigned int		mask);

	/**
	 * Hookable function to draw a window on-screen
	 *
	 * Unlike glPaint, when glDraw is called, the window is
	 * drawn immediately
	 *
	 * @param matrix A 4x4 matrix which describes the transformation of
	 * this window
	 * @param attrib A Fragment attrib which describes the texture
	 * modification state of this window
	 * @param region Describes which region will be drawn
	 * @param mask   Bitmask which describes how this window is drawn
	 */
	virtual bool glDraw (const GLMatrix 	&matrix,
			     const GLWindowPaintAttrib &attrib,
			     const CompRegion 	&region,
			     unsigned int	mask);

	/**
	 * Hookable function to add points to a window
	 * texture geometry
	 *
	 * This function adds rects to a window's texture geometry
	 * and modifies their points by the values in the GLTexture::MatrixList
	 *
	 * It is used for texture transformation to set points
	 * for where the texture should be skewed
	 *
	 * @param matrices Describes the matrices by which the texture exists
	 * @param region
	 * @param clipRegion
	 * @param min
	 * @param max
	 */
	virtual void glAddGeometry (const GLTexture::MatrixList &matrices,
				    const CompRegion 		&region,
				    const CompRegion 		&clipRegion,
				    unsigned int		min = MAXSHORT,
				    unsigned int		max = MAXSHORT);
	virtual void glDrawTexture (GLTexture *texture, const GLMatrix &,
	                            const GLWindowPaintAttrib &, unsigned int);
};

extern template class PluginClassHandler<GLWindow, CompWindow, COMPIZ_OPENGL_ABI>;

class GLWindow :
    public WrapableHandler<GLWindowInterface, 4>,
    public PluginClassHandler<GLWindow, CompWindow, COMPIZ_OPENGL_ABI>
{
    public:

	static GLWindowPaintAttrib defaultPaintAttrib;

    public:

	GLWindow (CompWindow *w);
	~GLWindow ();

	const CompRegion & clip () const;

	/**
	 * Returns the current paint attributes for this window
	 */
	GLWindowPaintAttrib & paintAttrib ();

	/**
	 * Returns the last paint attributes for this window
	 */
	GLWindowPaintAttrib & lastPaintAttrib ();

	unsigned int lastMask () const;

	/**
	 * Binds this window to an openGL texture
	 */
	bool bind ();

	/**
	 * Releases this window from an openGL texture
	 */
	void release ();

	/**
	 * Returns the tiled textures for this window
	 */
	const GLTexture::List &       textures () const;

	/**
	 * Returns the matrices for the tiled textures for this windwo
	 */
	const GLTexture::MatrixList & matrices () const;

	void updatePaintAttribs ();

	/**
	 * Returns the window vertex buffer object
	 */
	GLVertexBuffer * vertexBuffer ();

	/**
	 * Add a vertex and/or fragment shader function to the pipeline.
	 *
	 * @param name Name of the plugin adding the functions
	 * @param vertex_shader Function to add to the vertex shader
	 * @param fragment_shader Function to add to the fragment shader
	 */
	void addShaders (std::string name,
	                 std::string vertex_shader,
	                 std::string fragment_shader);

	GLTexture *getIcon (int width, int height);

	WRAPABLE_HND (0, GLWindowInterface, bool, glPaint,
		      const GLWindowPaintAttrib &, const GLMatrix &,
		      const CompRegion &, unsigned int);
	WRAPABLE_HND (1, GLWindowInterface, bool, glDraw, const GLMatrix &,
		      const GLWindowPaintAttrib &, const CompRegion &,
	              unsigned int);
	WRAPABLE_HND (2, GLWindowInterface, void, glAddGeometry,
		      const GLTexture::MatrixList &, const CompRegion &,
		      const CompRegion &,
		      unsigned int = MAXSHORT, unsigned int = MAXSHORT);
	WRAPABLE_HND (3, GLWindowInterface, void, glDrawTexture,
		      GLTexture *texture, const GLMatrix &,
	              const GLWindowPaintAttrib &, unsigned int);

	friend class GLScreen;
	friend class PrivateGLScreen;

    private:
	PrivateGLWindow *priv;
};

#endif

