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

#ifndef _OPENGL_PRIVATES_H
#define _OPENGL_PRIVATES_H

#include <memory>

#include <composite/composite.h>
#include <opengl/opengl.h>
#include <core/atoms.h>

#ifdef USE_GLES
#include <opengl/framebufferobject.h>
#endif

#include <opengl/doublebuffer.h>

#include "privatetexture.h"
#include "privatevertexbuffer.h"
#include "opengl_options.h"

extern CompOutput *targetOutput;

class GLDoubleBuffer :
    public compiz::opengl::DoubleBuffer
{
    public:

	GLDoubleBuffer (Display                                             *,
			const CompSize                                      &,
			const compiz::opengl::impl::GLXSwapIntervalEXTFunc  &,
			const compiz::opengl::impl::GLXWaitVideoSyncSGIFunc &);

    protected:

	Display *mDpy;
	const CompSize &mSize;
};

#ifndef USE_GLES

class GLXDoubleBuffer :
    public GLDoubleBuffer
{
    public:

	GLXDoubleBuffer (Display *,
		       const CompSize &,
		       Window);

	void swap () const;
	bool blitAvailable () const;
	void blit (const CompRegion &region) const;
	bool fallbackBlitAvailable () const;
	void fallbackBlit (const CompRegion &region) const;
	void copyFrontToBack () const;

    protected:

	Window mOutput;
};

#else

class EGLDoubleBuffer :
    public GLDoubleBuffer
{
    public:

	EGLDoubleBuffer (Display *,
		       const CompSize &,
		       EGLSurface const &);

	void swap () const;
	bool blitAvailable () const;
	void blit (const CompRegion &region) const;
	bool fallbackBlitAvailable () const;
	void fallbackBlit (const CompRegion &region) const;
	void copyFrontToBack () const;

    private:

	EGLSurface const & mSurface;
};

#endif

class GLIcon
{
    public:
	GLIcon () : icon (NULL) {}

	CompIcon        *icon;
	GLTexture::List textures;
};

class PrivateGLScreen :
    public ScreenInterface,
    public compiz::composite::PaintHandler,
    public OpenglOptions
{
    public:
	PrivateGLScreen (GLScreen *gs);
	~PrivateGLScreen ();

	bool setOption (const CompString &name, CompOption::Value &value);

	void handleEvent (XEvent *event);

	void outputChangeNotify ();

	void paintOutputs (CompOutput::ptrList &outputs,
			   unsigned int        mask,
			   const CompRegion    &region);

	bool hasVSync ();
	bool requiredForcedRefreshRate ();

	void updateRenderMode ();

	void prepareDrawing ();

	bool compositingActive ();

	void paintBackground (const GLMatrix   &transform,
	                      const CompRegion &region,
			      bool             transformed);

	void paintOutputRegion (const GLMatrix   &transform,
			        const CompRegion &region,
			        CompOutput       *output,
			        unsigned int     mask);

	void updateScreenBackground ();

	void updateView ();

	bool driverIsBlacklisted (const char *regex) const;

    public:

	GLScreen        *gScreen;
	CompositeScreen *cScreen;

	GLenum textureFilter;

	#ifndef USE_GLES
	GLFBConfig      glxPixmapFBConfigs[MAX_DEPTH + 1];
	#endif

	GLTexture::List backgroundTextures;
	bool            backgroundLoaded;

	GLTexture::Filter filter[3];

	CompPoint rasterPos;

	GLMatrix *projection;

	bool clearBuffers;
	bool lighting;

	#ifdef USE_GLES
	EGLContext ctx;
	EGLSurface surface;
	EGLDoubleBuffer doubleBuffer;
	#else
	GLXContext ctx;

	GL::GLXGetProcAddressProc getProcAddress;
	GLXDoubleBuffer doubleBuffer;
	#endif

	GLFramebufferObject *scratchFbo;
	CompRegion outputRegion;

	XRectangle lastViewport;
	bool refreshSubBuffer;
	unsigned int lastMask;

	std::vector<GLTexture::BindPixmapProc> bindPixmap;
	bool hasCompositing;
	bool commonFrontbuffer;
	bool incorrectRefreshRate; // hack for NVIDIA specifying an incorrect
				   // refresh rate, causing us to miss vblanks

	GLIcon defaultIcon;

	Window saveWindow; // hack for broken applications, see:
			   // https://bugs.launchpad.net/ubuntu/+source/compiz/+bug/807487

	GLProgramCache *programCache;
	GLShaderCache   shaderCache;
	GLVertexBuffer::AutoProgram *autoProgram;

	Pixmap rootPixmapCopy;
	CompSize rootPixmapSize;

	const char *glVendor, *glRenderer, *glVersion;

	mutable CompString prevRegex;
	mutable bool       prevBlacklisted;
};

class PrivateGLWindow :
    public WindowInterface,
    public CompositeWindowInterface
{
    public:

	static const unsigned int UpdateRegion = 1 << 0;
	static const unsigned int UpdateMatrix = 1 << 1;

    public:
	PrivateGLWindow (CompWindow *w, GLWindow *gw);
	~PrivateGLWindow ();

	void windowNotify (CompWindowNotify n);
	void resizeNotify (int dx, int dy, int dwidth, int dheight);
	void moveNotify (int dx, int dy, bool now);
	void updateFrameRegion (CompRegion &region);

	void setWindowMatrix ();
	void updateWindowRegions ();

	void clearTextures ();

	CompWindow      *window;
	GLWindow        *gWindow;
	CompositeWindow *cWindow;
	GLScreen        *gScreen;

	GLTexture::List       textures;
	GLTexture::MatrixList matrices;
	CompRegion::Vector    regions;
	unsigned int          updateState;
	bool		      needsRebind;

	CompRegion    clip;

	bool	      bindFailed;
	bool	      overlayWindow;

	GLushort opacity;
	GLushort brightness;
	GLushort saturation;

	GLWindowPaintAttrib paint;
	GLWindowPaintAttrib lastPaint;

	unsigned int lastMask;

	GLVertexBuffer *vertexBuffer;

	// map of shaders, plugin name is key, pair of vertex and fragment
	// shader source code is value
	std::list<const GLShaderData*> shaders;
	GLVertexBuffer::AutoProgram *autoProgram;

	std::list<GLIcon> icons;
};

#endif

