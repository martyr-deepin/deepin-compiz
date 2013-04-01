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

#include "privates.h"


template class PluginClassHandler<GLWindow, CompWindow, COMPIZ_OPENGL_ABI>;

GLWindow::GLWindow (CompWindow *w) :
    PluginClassHandler<GLWindow, CompWindow, COMPIZ_OPENGL_ABI> (w),
    priv (new PrivateGLWindow (w, this))
{
    CompositeWindow *cw = CompositeWindow::get (w);

    priv->paint.opacity    = cw->opacity ();
    priv->paint.brightness = cw->brightness ();
    priv->paint.saturation = cw->saturation ();

    priv->lastPaint = priv->paint;

}

GLWindow::~GLWindow ()
{
    delete priv;
}


/**
 * Callback object to create GLPrograms automatically when using GLVertexBuffer.
 */
class GLWindowAutoProgram : public GLVertexBuffer::AutoProgram
{
public:
    GLWindowAutoProgram (PrivateGLWindow *pWindow) : pWindow(pWindow) {}

    GLProgram *getProgram (GLShaderParameters &params)
    {
	GLScreen *gScreen = pWindow->gScreen;

	const GLShaderData *shaderData = gScreen->getShaderData (params);
	pWindow->shaders.push_back (shaderData);
	return gScreen->getProgram (pWindow->shaders);
    }

    PrivateGLWindow *pWindow;

};

PrivateGLWindow::PrivateGLWindow (CompWindow *w,
				  GLWindow   *gw) :
    window (w),
    gWindow (gw),
    cWindow (CompositeWindow::get (w)),
    gScreen (GLScreen::get (screen)),
    textures (),
    regions (),
    updateState (UpdateRegion | UpdateMatrix),
    needsRebind (true),
    clip (),
    bindFailed (false),
    vertexBuffer (new GLVertexBuffer ()),
    autoProgram(new GLWindowAutoProgram(this)),
    icons ()
{
    paint.xScale	= 1.0f;
    paint.yScale	= 1.0f;
    paint.xTranslate	= 0.0f;
    paint.yTranslate	= 0.0f;

    WindowInterface::setHandler (w);
    CompositeWindowInterface::setHandler (cWindow);

    vertexBuffer->setAutoProgram(autoProgram);

    cWindow->setNewPixmapReadyCallback (boost::bind (&PrivateGLWindow::clearTextures, this));
}

PrivateGLWindow::~PrivateGLWindow ()
{
    delete vertexBuffer;
    delete autoProgram;
    cWindow->setNewPixmapReadyCallback (boost::function <void ()> ());
}

void
PrivateGLWindow::setWindowMatrix ()
{
    CompRect input (window->inputRect ());

    if (textures.size () != matrices.size ())
	matrices.resize (textures.size ());

    for (unsigned int i = 0; i < textures.size (); i++)
    {
	matrices[i] = textures[i]->matrix ();
	matrices[i].x0 -= (input.x () * matrices[i].xx);
	matrices[i].y0 -= (input.y () * matrices[i].yy);
    }

    updateState &= ~(UpdateMatrix);
}

void
PrivateGLWindow::clearTextures ()
{
    textures.clear ();
}

bool
GLWindow::bind ()
{
    if (priv->needsRebind)
    {
	if (!priv->cWindow->bind ())
	{
	    if (!priv->textures.empty ())
	    {
		/* Getting a new pixmap failed, recycle the old texture */
		priv->needsRebind = false;
		return true;
	    }
	    else
		return false;
	}

	GLTexture::List textures =
	    GLTexture::bindPixmapToTexture (priv->cWindow->pixmap (),
					    priv->cWindow->size ().width (),
					    priv->cWindow->size ().height (),
					    priv->window->depth ());
	if (textures.empty ())
	{
	    compLogMessage ("opengl", CompLogLevelInfo,
			    "Couldn't bind redirected window 0x%x to "
			    "texture\n", (int) priv->window->id ());

	    if (priv->cWindow->size ().width () > GL::maxTextureSize ||
		priv->cWindow->size ().height ()  > GL::maxTextureSize)
	    {
		compLogMessage ("opengl", CompLogLevelWarn,
				"Bug in window 0x%x (identifying as %s)", (int) priv->window->id (), priv->window->resName ().size () ? priv->window->resName ().c_str () : "(none available)");
		compLogMessage ("opengl", CompLogLevelWarn,
				"This window tried to create an absurdly large window %i x %i\n", priv->cWindow->size ().width (), priv->cWindow->size ().height ());
		compLogMessage ("opengl", CompLogLevelWarn,
				"Unforunately, that's not supported on your hardware, because you have a maximum texture size of %i", GL::maxTextureSize);
		compLogMessage ("opengl", CompLogLevelWarn, "you should probably file a bug against that application");
		compLogMessage ("opengl", CompLogLevelWarn, "for now, we're going to hide tht window so that it doesn't break your desktop\n");

		XReparentWindow (screen->dpy (), priv->window->id (), GLScreen::get (screen)->priv->saveWindow, 0, 0);
	    }
	    return false;
	}
	else
	{
	    priv->textures = textures;
	    priv->needsRebind = false;
	}
    }

    return true;
}

void
GLWindow::release ()
{
    if (!priv->cWindow->frozen ())
	priv->needsRebind = true;
}

bool
GLWindowInterface::glPaint (const GLWindowPaintAttrib &attrib,
			    const GLMatrix            &transform,
			    const CompRegion          &region,
			    unsigned int              mask)
    WRAPABLE_DEF (glPaint, attrib, transform, region, mask)

bool
GLWindowInterface::glDraw (const GLMatrix     &transform,
			   const GLWindowPaintAttrib &attrib,
			   const CompRegion   &region,
			   unsigned int       mask)
    WRAPABLE_DEF (glDraw, transform, attrib, region, mask)

void
GLWindowInterface::glAddGeometry (const GLTexture::MatrixList &matrix,
				  const CompRegion            &region,
				  const CompRegion            &clip,
				  unsigned int                maxGridWidth,
				  unsigned int                maxGridHeight)
    WRAPABLE_DEF (glAddGeometry, matrix, region, clip,
		  maxGridWidth, maxGridHeight)

void
GLWindowInterface::glDrawTexture (GLTexture          *texture,
                                  const GLMatrix            &transform,
				  const GLWindowPaintAttrib &attrib,
				  unsigned int       mask)
    WRAPABLE_DEF (glDrawTexture, texture, transform, attrib, mask)

const CompRegion &
GLWindow::clip () const
{
    return priv->clip;
}

GLWindowPaintAttrib &
GLWindow::paintAttrib ()
{
    return priv->paint;
}

GLWindowPaintAttrib &
GLWindow::lastPaintAttrib ()
{
    return priv->lastPaint;
}


void
PrivateGLWindow::resizeNotify (int dx, int dy, int dwidth, int dheight)
{
    window->resizeNotify (dx, dy, dwidth, dheight);
    updateState |= PrivateGLWindow::UpdateMatrix | PrivateGLWindow::UpdateRegion;
    gWindow->release ();
}

void
PrivateGLWindow::moveNotify (int dx, int dy, bool now)
{
    window->moveNotify (dx, dy, now);
    updateState |= PrivateGLWindow::UpdateMatrix;

    foreach (CompRegion &r, regions)
	r.translate (dx, dy);
}

void
PrivateGLWindow::windowNotify (CompWindowNotify n)
{
    switch (n)
    {
	case CompWindowNotifyUnmap:
	case CompWindowNotifyReparent:
	case CompWindowNotifyUnreparent:
	case CompWindowNotifyFrameUpdate:
	    gWindow->release ();
	    break;
	default:
	    break;
    }

    window->windowNotify (n);
}

void
GLWindow::updatePaintAttribs ()
{
    CompositeWindow *cw = CompositeWindow::get (priv->window);

    priv->paint.opacity    = cw->opacity ();
    priv->paint.brightness = cw->brightness ();
    priv->paint.saturation = cw->saturation ();
}

GLVertexBuffer *
GLWindow::vertexBuffer ()
{
    return priv->vertexBuffer;
}

const GLTexture::List &
GLWindow::textures () const
{
    static const GLTexture::List emptyList;

    /* No pixmap backs this window, let
     * users know that the window needs rebinding */
    if (priv->needsRebind)
	return emptyList;

    return priv->textures;
}

const GLTexture::MatrixList &
GLWindow::matrices () const
{
    return priv->matrices;
}

GLTexture *
GLWindow::getIcon (int width, int height)
{
    GLIcon   icon;
    CompIcon *i = priv->window->getIcon (width, height);

    if (!i)
	return NULL;

    if (!i->width () || !i->height ())
	return NULL;

    foreach (GLIcon &icon, priv->icons)
	if (icon.icon == i)
	    return icon.textures[0];

    icon.icon = i;
    icon.textures = GLTexture::imageBufferToTexture ((char *) i->data (), *i);

    if (icon.textures.size () > 1 || icon.textures.size () == 0)
	return NULL;

    priv->icons.push_back (icon);

    return icon.textures[0];
}

void
GLWindow::addShaders (std::string name,
                      std::string vertex_shader,
                      std::string fragment_shader)
{
    GLShaderData *data = new GLShaderData;
    data->name = name;
    data->vertexShader = vertex_shader;
    data->fragmentShader = fragment_shader;

    priv->shaders.push_back(data);
}

void
PrivateGLWindow::updateFrameRegion (CompRegion &region)
{
    window->updateFrameRegion (region);
    updateState |= PrivateGLWindow::UpdateRegion;
}

void
PrivateGLWindow::updateWindowRegions ()
{
    CompRect input (window->serverInputRect ());

    if (regions.size () != textures.size ())
	regions.resize (textures.size ());
    for (unsigned int i = 0; i < textures.size (); i++)
    {
	regions[i] = CompRegion (*textures[i]);
	regions[i].translate (input.x (), input.y ());
	regions[i] &= window->region ();
    }
    updateState &= ~(UpdateRegion);
}

unsigned int
GLWindow::lastMask () const
{
    return priv->lastMask;
}
