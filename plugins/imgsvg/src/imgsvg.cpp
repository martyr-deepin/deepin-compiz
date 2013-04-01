/*
 * Copyright © 2006 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include "imgsvg.h"

#include <fstream>

COMPIZ_PLUGIN_20090315 (imgsvg, SvgPluginVTable)

static bool
svgSet (CompAction         *action,
	CompAction::State  state,
	CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");
    w   = screen->findWindow (xid);

    if (w)
    {
	decor_point_t p[2];
	CompString    data;

	SVG_WINDOW (w);

	memset (p, 0, sizeof (p));

	p[0].gravity = CompOption::getIntOptionNamed (options, "gravity0",
						      GRAVITY_NORTH | GRAVITY_WEST);

	p[0].x = CompOption::getIntOptionNamed (options, "x0");
	p[0].y = CompOption::getIntOptionNamed (options, "y0");

	p[1].gravity = CompOption::getIntOptionNamed (options, "gravity1",
						      GRAVITY_SOUTH | GRAVITY_EAST);

	p[1].x = CompOption::getIntOptionNamed (options, "x1");
	p[1].y = CompOption::getIntOptionNamed (options, "y1");

	data = CompOption::getStringOptionNamed (options, "data");
	sw->setSvg (data, p);
    }

    return false;
}


SvgScreen::SvgScreen (CompScreen *screen) :
    PluginClassHandler<SvgScreen, CompScreen> (screen)
{
    optionSetSetInitiate (svgSet);
    ScreenInterface::setHandler (screen, true);
}

SvgScreen::~SvgScreen ()
{
}

bool
SvgScreen::fileToImage (CompString &path,
			CompSize   &size,
			int        &stride,
			void       *&data)
{
    CompString fileName = path;
    bool       status = false;
    int        len = fileName.length ();

    if (len < 4 || fileName.substr (len - 4, 4) != ".svg")
	fileName += ".svg";

    status = readSvgToImage (fileName.c_str (), size, data);

    if (status)
    {
	stride = size.width () * 4;
	return true;
    }

    status = screen->fileToImage (path, size, stride, data);

    return status;
}

void
SvgScreen::handleCompizEvent (const char         *plugin,
			      const char         *event,
			      CompOption::Vector &options)
{
    screen->handleCompizEvent (plugin, event, options);

    if (strcmp (plugin, "zoom") == 0)
    {
	int output = CompOption::getIntOptionNamed (options, "output");
	if (output == 0)
	{
	    if (strcmp (event, "in") == 0)
	    {
		zoom.setGeometry (CompOption::getIntOptionNamed (options, "x1"),
				  CompOption::getIntOptionNamed (options, "y1"),
				  CompOption::getIntOptionNamed (options, "x2"),
				  CompOption::getIntOptionNamed (options, "y2"));
	    }
	    else if (strcmp (event, "out") == 0)
	    {
		zoom.setGeometry (0, 0, 0, 0);
	    }
	}
    }
}

bool
SvgScreen::readSvgToImage (const char *file,
			   CompSize   &size,
			   void       *&data)
{
    cairo_surface_t   *surface;
    std::ifstream     svgFile;
    GError	      *error = NULL;
    RsvgHandle	      *svgHandle;
    RsvgDimensionData svgDimension;

    svgFile.open (file);
    if (!svgFile.is_open ())
	return false;

    svgFile.close ();
    svgHandle = rsvg_handle_new_from_file (file, &error);
    if (!svgHandle)
	return false;

    rsvg_handle_get_dimensions (svgHandle, &svgDimension);

    size.setWidth (svgDimension.width);
    size.setHeight (svgDimension.height);

    data = malloc (svgDimension.width * svgDimension.height * 4);
    if (!data)
    {
	rsvg_handle_free (svgHandle);
	return false;
    }

    surface = cairo_image_surface_create_for_data ((unsigned char *) data,
						   CAIRO_FORMAT_ARGB32,
						   svgDimension.width,
						   svgDimension.height,
						   svgDimension.width * 4);
    if (surface)
    {
	cairo_t *cr;

	cr = cairo_create (surface);

	cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
	cairo_paint (cr);
	cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

	rsvg_handle_render_cairo (svgHandle, cr);

	cairo_destroy (cr);
	cairo_surface_destroy (surface);
    }

    rsvg_handle_free (svgHandle);

    return true;
}

SvgWindow::SvgWindow (CompWindow *window) :
    PluginClassHandler<SvgWindow, CompWindow> (window),
    source (NULL),
    context (NULL),
    sScreen (SvgScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    window (window),
    gWindow (GLWindow::get (window))
{
    if (gWindow)
	GLWindowInterface::setHandler (gWindow, false);
}

SvgWindow::~SvgWindow ()
{
    if (source)
    {
	rsvg_handle_free (source->svg);
	delete source;
    }

    if (context)
    {
	finiTexture (context->texture[0]);
	delete context;
    }
}

bool
SvgWindow::glDraw (const GLMatrix     &transform,
		   const GLWindowPaintAttrib &attrib,
		   const CompRegion   &region,
		   unsigned int       mask)
{
    bool status = gWindow->glDraw (transform, attrib, region, mask);

    if (!status)
	return status;

    const CompRegion &reg = (mask & PAINT_WINDOW_TRANSFORMED_MASK) ?
			    infiniteRegion : region;

    if (context && reg.numRects ())
    {
	GLTexture::MatrixList matrix (1);
	unsigned int          i, j;
	int		      x1, y1, x2, y2;
	CompRect              rect = context->box.boundingRect ();

	x1 = MIN (rect.x1 (), sScreen->zoom.x1 ());
	y1 = MIN (rect.y1 (), sScreen->zoom.y1 ());
	x2 = MAX (rect.x2 (), sScreen->zoom.x2 ());
	y2 = MAX (rect.y2 (), sScreen->zoom.y2 ());

	rect.setGeometry (x1, y1, x2 - x1, y2 - y1);

	for (i = 0; i < context->texture[0].textures.size (); i++)
	{
	    matrix[0] = context->texture[0].matrices[i];

	    gWindow->vertexBuffer ()->begin ();
	    gWindow->glAddGeometry (matrix, context->box, reg);
	    gWindow->vertexBuffer ()->end ();

	    if (mask & PAINT_WINDOW_TRANSLUCENT_MASK)
		mask |= PAINT_WINDOW_BLEND_MASK;

	    gWindow->glDrawTexture (context->texture[0].textures[i], transform,
	                            attrib, mask);

	    if (rect.width () > 0 && rect.height () > 0)
	    {
		float    xScale, yScale;
		float    dx, dy;
		int      width, height;

		rect.setGeometry (rect.x1 () - 1,
				  rect.y1 () - 1,
				  rect.width () + 1,
				  rect.height () + 1);

		xScale = screen->width  () /
		         (float) (sScreen->zoom.width ());
		yScale = screen->height () /
		         (float) (sScreen->zoom.height ());

		dx = rect.width ();
		dy = rect.height ();

		width  = dx * xScale + 0.5f;
		height = dy * yScale + 0.5f;

		if (rect   != context->rect          ||
		    width  != context->size.width () ||
		    height != context->size.height ())
		{
		    float x1, y1, x2, y2;

		    context->rect = rect;
		    context->size.setWidth (width);
		    context->size.setHeight (height);

		    dx = context->box.boundingRect ().width ();
		    dy = context->box.boundingRect ().height ();

		    x1 = (rect.x1 () - context->box.boundingRect ().x ()) / dx;
		    y1 = (rect.y1 () - context->box.boundingRect ().y ()) / dy;
		    x2 = (rect.x2 () - context->box.boundingRect ().x ()) / dx;
		    y2 = (rect.y2 () - context->box.boundingRect ().y ()) / dy;

		    finiTexture (context->texture[1]);

		    if (initTexture (context->source, context->texture[1],
				     context->size))
		    {
			renderSvg (context->source, context->texture[1],
				   context->size, x1, y1, x2, y2);

			updateSvgMatrix ();
		    }
		}

		for (j = 0; j < context->texture[1].textures.size (); j++)
		{
		    GLTexture::Filter saveFilter;
		    CompRegion        r (rect);

		    matrix[0] = context->texture[1].matrices[j];

		    saveFilter = gScreen->filter (SCREEN_TRANS_FILTER);
		    gScreen->setFilter (SCREEN_TRANS_FILTER, GLTexture::Good);

		    gWindow->vertexBuffer ()->begin ();
		    gWindow->glAddGeometry (matrix, r, reg);
		    gWindow->vertexBuffer ()->end ();

		    gWindow->glDrawTexture (context->texture[1].textures[j],
					    transform, attrib, mask);

		    gScreen->setFilter (SCREEN_TRANS_FILTER, saveFilter);
		}
	    }
	    else if (context->texture[1].size.width ())
	    {
		finiTexture (context->texture[1]);
		initTexture (source, context->texture[1], CompSize ());

		memset (&context->rect, 0, sizeof (BoxRec));
		context->size.setWidth (0);
		context->size.setHeight (0);
	    }
	}
    }

    return status;
}

void
SvgWindow::moveNotify (int  dx,
		       int  dy,
		       bool immediate)
{
    if (context)
    {
	context->box.translate (dx, dy);
	updateSvgMatrix ();
    }

    window->moveNotify (dx, dy, immediate);
}

void
SvgWindow::resizeNotify (int dx,
			 int dy,
			 int dwidth,
			 int dheight)
{
    if (source)
	updateSvgContext ();

    window->resizeNotify (dx, dy, dwidth, dheight);
}

void
SvgWindow::updateSvgMatrix ()
{
    SvgTexture        *texture;
    GLTexture::Matrix *m;
    unsigned int      i;
    CompRect          rect;

    rect = context->box.boundingRect ();
    texture = &context->texture[0];

    if (texture->matrices.size () != texture->textures.size ())
	texture->matrices.resize (texture->textures.size ());

    for (i = 0; i < texture->textures.size (); i++)
    {
	m = &texture->matrices[i];
	*m = texture->textures[i]->matrix ();

	m->xx *= (float) texture->size.width ()  / rect.width ();
	m->yy *= (float) texture->size.height () / rect.height ();

	m->x0 -= (rect.x () * m->xx);
	m->y0 -= (rect.y () * m->yy);
    }

    texture = &context->texture[1];

    if (texture->matrices.size () != texture->textures.size ())
	texture->matrices.resize (texture->textures.size ());

    for (i = 0; i < texture->textures.size (); i++)
    {
	m = &texture->matrices[i];
	*m = texture->textures[i]->matrix ();

	m->xx *= (float) texture->size.width ()  / context->rect.width ();
	m->yy *= (float) texture->size.height () / context->rect.height ();

	m->x0 -= (context->rect.x () * m->xx);
	m->y0 -= (context->rect.y () * m->yy);
    }
}

void
SvgWindow::updateSvgContext ()
{
    int      x1, y1, x2, y2;
    CompSize wSize;

    if (context)
    {
	finiTexture (context->texture[0]);
	finiTexture (context->texture[1]);
    }
    else
    {
	context = new SvgContext;
	if (!context)
	    return;
    }

    initTexture (source, context->texture[1], context->size);

    context->source = source;

    wSize.setWidth (window->geometry ().width ());
    wSize.setHeight (window->geometry ().height ());

    decor_apply_gravity (source->p1.gravity,
			 source->p1.x, source->p1.y,
			 wSize.width (), wSize.height (),
			 &x1, &y1);

    decor_apply_gravity (source->p2.gravity,
			 source->p2.x, source->p2.y,
			 wSize.width (), wSize.height (),
			 &x2, &y2);

    x1 = MAX (x1, 0);
    y1 = MAX (y1, 0);
    x2 = MIN (x2, wSize.width ());
    y2 = MIN (y2, wSize.height ());

    if (!initTexture (source, context->texture[0], wSize))
    {
	delete context;
	context = NULL;
    }
    else
    {
	renderSvg (source, context->texture[0], wSize, 0.0f, 0.0f, 1.0f, 1.0f);

	initTexture (source, context->texture[1], CompSize ());

	context->box += CompRect (x1, y1, x2 - x1, y2 - y1);
	context->box.translate (window->geometry ().x (), window->geometry ().y ());

	updateSvgMatrix ();
    }
}

void
SvgWindow::renderSvg (SvgSource  *source,
		      SvgTexture &texture,
		      CompSize   size,
		      float      x1,
		      float      y1,
		      float      x2,
		      float      y2)
{
    float w = x2 - x1;
    float h = y2 - y1;

    cairo_save (texture.cr);

    cairo_set_operator (texture.cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba (texture.cr, 1.0, 1.0, 1.0, 0.0);
    cairo_paint (texture.cr);
    cairo_set_operator (texture.cr, CAIRO_OPERATOR_OVER);

    cairo_scale (texture.cr, 1.0 / w, 1.0 / h);

    cairo_scale (texture.cr,
		 (double) size.width () / source->dimension.width,
		 (double) size.height () / source->dimension.height);

    cairo_translate (texture.cr,
		     -x1 * source->dimension.width,
		     -y1 * source->dimension.height);

    rsvg_handle_render_cairo (source->svg, texture.cr);

    cairo_restore (texture.cr);
}

bool
SvgWindow::initTexture (SvgSource  *source,
			SvgTexture &texture,
			CompSize   size)
{
    cairo_surface_t *surface;
    Display         *dpy = screen->dpy ();

    texture.size    = size;
    texture.pixmap  = None;
    texture.cr      = NULL;

    if (size.width () && size.height ())
    {
	XWindowAttributes attr;
	XGetWindowAttributes (dpy, window->id (), &attr);

	texture.pixmap = XCreatePixmap (dpy, screen->root (),
					size.width (), size.height (),
					attr.depth);

	texture.textures =
	    GLTexture::bindPixmapToTexture (texture.pixmap,
					    size.width (), size.height (), attr.depth);
	if (texture.textures.empty ())
	{
	    compLogMessage ("svg", CompLogLevelInfo,
			    "Couldn't bind pixmap 0x%x to texture",
			    (int) texture.pixmap);

	    XFreePixmap (dpy, texture.pixmap);

	    return false;
	}

	surface = cairo_xlib_surface_create (dpy, texture.pixmap, attr.visual,
					     size.width (), size.height ());
	texture.cr = cairo_create (surface);
	cairo_surface_destroy (surface);
    }

    return true;
}

void
SvgWindow::finiTexture (SvgTexture &texture)
{
    if (texture.cr)
	cairo_destroy (texture.cr);

    if (texture.pixmap)
	XFreePixmap (screen->dpy (), texture.pixmap);
}

void
SvgWindow::setSvg (CompString    &data,
		   decor_point_t p[2])
{
    RsvgHandle *svg = NULL;
    GError     *error = NULL;

    if (!gWindow)
	return;

    svg = rsvg_handle_new_from_data ((guint8 *) data.c_str (),
				     data.length (), &error);

    if (source)
    {
	rsvg_handle_free (source->svg);
	source->svg = svg;
    }
    else
    {
	source = new SvgSource;
	if (source)
	    source->svg = svg;
    }

    if (source && source->svg)
    {
	source->p1 = p[0];
	source->p2 = p[1];

	source->svg = svg;

	gWindow->glDrawSetEnabled (this, true);
	rsvg_handle_get_dimensions (svg, &source->dimension);

	updateSvgContext ();
    }
    else
    {
	if (svg)
	    rsvg_handle_free (svg);

	if (source)
	{
	    delete source;
	    source = NULL;
	}

	if (context)
	{
	    finiTexture (context->texture[0]);
	    delete context;
	    context = NULL;
	}

	gWindow->glDrawSetEnabled (this, false);
    }
}

bool
SvgPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    rsvg_init ();

    return true;
}

void
SvgPluginVTable::fini ()
{
    rsvg_term ();
}

