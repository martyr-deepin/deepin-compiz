/**
 *
 * Compiz metacity like info during resize
 *
 * resizeinfo.c
 *
 * Copyright (c) 2007 Robert Carr <racarr@opencompositing.org>
 *
 * Compiz resize atom usage and general cleanups by
 * Copyright (c) 2007 Danny Baumann <maniac@opencompositing.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 **/

#include "resizeinfo.h"

COMPIZ_PLUGIN_20090315 (resizeinfo, InfoPluginVTable);

const unsigned short RESIZE_POPUP_WIDTH = 85;
const unsigned short RESIZE_POPUP_HEIGHT = 50;

const double PI = 3.14159265359f;

/* Set up an InfoLayer to build a cairo->opengl texture pipeline */
InfoLayer::~InfoLayer ()
{
    if (cr)
	cairo_destroy (cr);

    if (surface)
	cairo_surface_destroy (surface);

    if (pixmap)
	XFreePixmap (screen->dpy (), pixmap);
}

/* Here 's' is Screen * 'screen' is 'CompScreen *' */

InfoLayer::InfoLayer () :
    valid (false),
    s (ScreenOfDisplay (screen->dpy (), screen->screenNum ())),
    pixmap (None),
    surface (NULL),
    cr (NULL)
{
    format = XRenderFindStandardFormat (screen->dpy (), PictStandardARGB32);
    if (!format)
	return;

    pixmap = XCreatePixmap (screen->dpy (), screen->root (),
			    RESIZE_POPUP_WIDTH, RESIZE_POPUP_HEIGHT, 32);
    if (!pixmap)
	return;

    surface =
	cairo_xlib_surface_create_with_xrender_format (screen->dpy (),
						       pixmap, s,
						       format,
						       RESIZE_POPUP_WIDTH,
						       RESIZE_POPUP_HEIGHT);
    if (cairo_surface_status (surface) != CAIRO_STATUS_SUCCESS)
    {
	compLogMessage ("resizeinfo", CompLogLevelWarn,
			"Could not create cairo layer surface,");
	return;
    }

    texture = GLTexture::bindPixmapToTexture (pixmap,
					      RESIZE_POPUP_WIDTH,
					      RESIZE_POPUP_HEIGHT, 32);
    if (!texture.size ())
    {
	compLogMessage ("resizeinfo", CompLogLevelWarn,
			"Bind Pixmap to Texture failure");
	return;
    }

    cr = cairo_create (surface);
    if (cairo_status (cr) != CAIRO_STATUS_SUCCESS)
    {
	compLogMessage ("resizeinfo", CompLogLevelWarn,
			"Could not create cairo context");
	return;
    }

    valid = true;
}

/* Draw the window "size" derived from the window hints.
   We calculate width or height - base_width or base_height and divide
   it by the increment in each direction. For windows like terminals
   setting the proper size hints this gives us the number of columns/rows. */
void
InfoLayer::renderText ()
{
    unsigned int         baseWidth, baseHeight;
    unsigned int         widthInc, heightInc;
    unsigned int         width, height, xv, yv;
    unsigned short       *color;
    char                 info[50];
    PangoLayout          *layout;
    PangoFontDescription *font;
    int                  w, h;

    INFO_SCREEN (screen);

    if (!valid)
	return;

    baseWidth = is->pWindow->sizeHints ().base_width;
    baseHeight = is->pWindow->sizeHints ().base_height;
    widthInc = is->pWindow->sizeHints ().width_inc;
    heightInc = is->pWindow->sizeHints ().height_inc;
    width = is->resizeGeometry.width;
    height = is->resizeGeometry.height;
	
    color = is->optionGetTextColor ();

    xv = (widthInc > 1) ? (width - baseWidth) / widthInc : width;
    yv = (heightInc > 1) ? (height - baseHeight) / heightInc : height;

    /* Clear the context. */
    cairo_save (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint (cr);
    cairo_restore (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

    snprintf (info, 50, "%d x %d", xv, yv);

    font = pango_font_description_new ();
    layout = pango_cairo_create_layout (is->textLayer.cr);
  
    pango_font_description_set_family (font,"Sans");
    pango_font_description_set_absolute_size (font, 12 * PANGO_SCALE);
    pango_font_description_set_style (font, PANGO_STYLE_NORMAL);
    pango_font_description_set_weight (font, PANGO_WEIGHT_BOLD);
 
    pango_layout_set_font_description (layout, font);
    pango_layout_set_ellipsize (layout, PANGO_ELLIPSIZE_END);
    pango_layout_set_text (layout, info, -1);
  
    pango_layout_get_pixel_size (layout, &w, &h);
  
    cairo_move_to (cr, 
		   RESIZE_POPUP_WIDTH / 2.0f - w / 2.0f, 
		   RESIZE_POPUP_HEIGHT / 2.0f - h / 2.0f);
  
    pango_layout_set_width (layout, RESIZE_POPUP_WIDTH * PANGO_SCALE);
    pango_cairo_update_layout (cr, layout);
  
    cairo_set_source_rgba (cr, 
			   *(color)     / (float)0xffff,
			   *(color + 1) / (float)0xffff,
			   *(color + 2) / (float)0xffff,
			   *(color + 3) / (float)0xffff);

    pango_cairo_show_layout (cr, layout);

    pango_font_description_free (font);
    g_object_unref (layout);
}

/* Draw the background. We draw this on a second layer so that we do
   not have to draw it each time we have to update. Granted we could
   use some cairo trickery for this... */
void
InfoLayer::renderBackground ()
{
    cairo_pattern_t *pattern;	
    float           border = 7.5;
    int             height = RESIZE_POPUP_HEIGHT;
    int             width = RESIZE_POPUP_WIDTH;
    float           r, g, b, a;

    INFO_SCREEN (screen);

    if (!valid)
	return;

    cairo_set_line_width (cr, 1.0f);

    /* Clear */
    cairo_save (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint (cr);
    cairo_restore (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);

    /* Setup Gradient */
    pattern = cairo_pattern_create_linear (0, 0, width, height);

    r = is->optionGetGradient1Red () / (float)0xffff;
    g = is->optionGetGradient1Green () / (float)0xffff;
    b = is->optionGetGradient1Blue () / (float)0xffff;
    a = is->optionGetGradient1Alpha () / (float)0xffff;
    cairo_pattern_add_color_stop_rgba (pattern, 0.00f, r, g, b, a);

    r = is->optionGetGradient1Red () / (float)0xffff;
    g = is->optionGetGradient1Green () / (float)0xffff;
    b = is->optionGetGradient1Blue () / (float)0xffff;
    a = is->optionGetGradient1Alpha () / (float)0xffff;
    cairo_pattern_add_color_stop_rgba (pattern, 0.65f, r, g, b, a);

    r = is->optionGetGradient1Red () / (float)0xffff;
    g = is->optionGetGradient1Green () / (float)0xffff;
    b = is->optionGetGradient1Blue () / (float)0xffff;
    a = is->optionGetGradient1Alpha () / (float)0xffff;
    cairo_pattern_add_color_stop_rgba (pattern, 0.85f, r, g, b, a);
    cairo_set_source (cr, pattern);
	
    /* Rounded Rectangle! */
    cairo_arc (cr, border, border, border, PI, 1.5f * PI);
    cairo_arc (cr, border + width - 2 * border, border, border,
	       1.5f * PI, 2.0 * PI);
    cairo_arc (cr, width - border, height - border, border, 0, PI / 2.0f);
    cairo_arc (cr, border, height - border, border,  PI / 2.0f, PI);
    cairo_close_path (cr);
    cairo_fill_preserve (cr);
	
    /* Outline */
    cairo_set_source_rgba (cr, 0.9f, 0.9f, 0.9f, 1.0f);
    cairo_stroke (cr);
	
    cairo_pattern_destroy (pattern);
}

static void
gradientChanged (CompOption                 *o, 
		 ResizeinfoOptions::Options num)
{
    INFO_SCREEN (screen);

    is->backgroundLayer.renderBackground ();
}

void
InfoScreen::damagePaintRegion ()
{
    int    x, y;

    if (!fadeTime && !drawing)
	return;

    x = resizeGeometry.x + resizeGeometry.width / 2.0f -
	RESIZE_POPUP_WIDTH / 2.0f;
    y = resizeGeometry.y + resizeGeometry.height / 2.0f - 
	RESIZE_POPUP_HEIGHT / 2.0f;

    CompRegion reg (x - 5, y - 5,
		    (x + RESIZE_POPUP_WIDTH + 5),
		    (y + RESIZE_POPUP_HEIGHT + 5));

    cScreen->damageRegion (reg);
}

/*  Handle the fade in /fade out. */
void
InfoScreen::preparePaint (int ms)
{	
    if (fadeTime)
    {
	fadeTime -= ms;
	if (fadeTime < 0)
	    fadeTime = 0;
    }

    cScreen->preparePaint (ms);
}

void
InfoScreen::donePaint ()
{
    if (pWindow)
    {
	if (fadeTime)
	    damagePaintRegion ();
	
	if (!fadeTime && !drawing)
	{
	    pWindow = NULL;

	    cScreen->preparePaintSetEnabled (this, false);
	    gScreen->glPaintOutputSetEnabled (this, false);
	    cScreen->donePaintSetEnabled (this, false);
	}

    }

    cScreen->donePaint ();
}

void
InfoWindow::grabNotify (int          x,
		        int          y,
		        unsigned int state,
		        unsigned int mask)
{
    INFO_SCREEN (screen);

    if ((!is->pWindow || !is->drawing) &&
        ((window->state () & MAXIMIZE_STATE) != MAXIMIZE_STATE))
    {
	bool showInfo;
	showInfo = (((window->sizeHints ().width_inc != 1) && 
		     (window->sizeHints ().height_inc != 1)) ||
		    is->optionGetAlwaysShow ());

	if (showInfo && (mask & CompWindowGrabResizeMask))
	{
	    is->pWindow  = window;
	    is->drawing  = true;
	    is->fadeTime = is->optionGetFadeTime () - is->fadeTime;

	    is->resizeGeometry.x      = window->x ();
	    is->resizeGeometry.y      = window->y ();
	    is->resizeGeometry.width  = window->width ();
	    is->resizeGeometry.height = window->height ();

	    screen->handleEventSetEnabled (is, true);
	}
    }
	
    window->grabNotify (x, y, state, mask);
}

void
InfoWindow::ungrabNotify ()
{
    INFO_SCREEN (screen);

    if (window == is->pWindow)
    {
	is->drawing = false;
	is->fadeTime = is->optionGetFadeTime () - is->fadeTime;
	is->cScreen->damageScreen ();

	screen->handleEventSetEnabled (is, false);
	window->ungrabNotifySetEnabled (this, false);
    }
	
    window->ungrabNotify ();
}

/* Draw a texture at x/y on a quad of RESIZE_POPUP_WIDTH /
   RESIZE_POPUP_HEIGHT with the opacity in InfoScreen. */
void
InfoLayer::draw (const GLMatrix &transform,
                 int             x,
	   	 int y)
{
    BOX   box;
    float opacity;

    INFO_SCREEN (screen);

    if (!valid)
	return;

    for (unsigned int i = 0; i < texture.size (); i++)
    {
	GLushort           colorData[4];
	GLfloat            textureData[8];
	GLfloat            vertexData[12];
	GLTexture         *tex = texture[i];
	GLTexture::Matrix matrix = tex->matrix ();
	GLVertexBuffer *streamingBuffer = GLVertexBuffer::streamingBuffer ();

	tex->enable (GLTexture::Good);

	matrix.x0 -= x * matrix.xx;
	matrix.y0 -= y * matrix.yy;

	box.x1 = x;
	box.x2 = x + RESIZE_POPUP_WIDTH;
	box.y1 = y;
	box.y2 = y + RESIZE_POPUP_HEIGHT;

	opacity = (float) is->fadeTime / is->optionGetFadeTime ();
	if (is->drawing)
	    opacity = 1.0f - opacity;

	streamingBuffer->begin (GL_TRIANGLE_STRIP);

	colorData[0] = opacity * 65535;
	colorData[1] = opacity * 65535;
	colorData[2] = opacity * 65535;
	colorData[3] = opacity * 65535;

	textureData[0] = COMP_TEX_COORD_X (matrix, box.x1);
	textureData[1] = COMP_TEX_COORD_Y (matrix, box.y2);
	textureData[2] = COMP_TEX_COORD_X (matrix, box.x2);
	textureData[3] = COMP_TEX_COORD_Y (matrix, box.y2);
	textureData[4] = COMP_TEX_COORD_X (matrix, box.x1);
	textureData[5] = COMP_TEX_COORD_Y (matrix, box.y1);
	textureData[6] = COMP_TEX_COORD_X (matrix, box.x2);
	textureData[7] = COMP_TEX_COORD_Y (matrix, box.y1);

	vertexData[0]  = box.x1;
	vertexData[1]  = box.y2;
	vertexData[2]  = 0;
	vertexData[3]  = box.x2;
	vertexData[4]  = box.y2;
	vertexData[5]  = 0;
	vertexData[6]  = box.x1;
	vertexData[7]  = box.y1;
	vertexData[8]  = 0;
	vertexData[9]  = box.x2;
	vertexData[10] = box.y1;
	vertexData[11] = 0;

	streamingBuffer->addColors (1, colorData);
	streamingBuffer->addTexCoords (0, 4, textureData);
	streamingBuffer->addVertices (4, vertexData);

	streamingBuffer->end ();
	streamingBuffer->render (transform);

	tex->disable ();
    }
}

bool
InfoScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
			   const GLMatrix            &transform,
			   const CompRegion          &region,
			   CompOutput                *output,
			   unsigned int              mask)
{
    bool status;
  
    status = gScreen->glPaintOutput (attrib, transform, region, output, mask);

    if ((drawing || fadeTime) && pWindow)
    {
	GLMatrix sTransform = transform;
	int      x, y;

	x = resizeGeometry.x + resizeGeometry.width / 2.0f - 
	    RESIZE_POPUP_WIDTH / 2.0f;
	y = resizeGeometry.y + resizeGeometry.height / 2.0f - 
	    RESIZE_POPUP_HEIGHT / 2.0f;

	sTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);
      
	glEnable (GL_BLEND);
#ifndef USE_GLES
	gScreen->setTexEnvMode (GL_MODULATE);
#endif
	backgroundLayer.draw (sTransform, x, y);
	textLayer.draw (sTransform, x, y);
  
	gScreen->setTexEnvMode (GL_REPLACE);
	glDisable (GL_BLEND);

    }

    return status;
}

void
InfoScreen::handleEvent (XEvent *event)
{
    switch (event->type) {
    case ClientMessage:    
	if (event->xclient.message_type == resizeInfoAtom)
	{
	    CompWindow *w;

	    w = screen->findWindow (event->xclient.window);
	    if (w && w == pWindow)
	    {
		resizeGeometry.x      = event->xclient.data.l[0];
		resizeGeometry.y      = event->xclient.data.l[1];
		resizeGeometry.width  = event->xclient.data.l[2];
		resizeGeometry.height = event->xclient.data.l[3];

		textLayer.renderText ();

		cScreen->preparePaintSetEnabled (this, true);
		gScreen->glPaintOutputSetEnabled (this, true);
		cScreen->donePaintSetEnabled (this, true);

		w->ungrabNotifySetEnabled (InfoWindow::get (w), true);

		damagePaintRegion ();
	    }
	}
	break;
    default:
	break;
    }

    screen->handleEvent (event);
}

InfoScreen::InfoScreen (CompScreen *screen) :
    PluginClassHandler <InfoScreen, CompScreen> (screen),
    ResizeinfoOptions (),
    gScreen (GLScreen::get (screen)),
    cScreen (CompositeScreen::get (screen)),
    resizeInfoAtom (XInternAtom (screen->dpy (), "_COMPIZ_RESIZE_NOTIFY", 0)),
    pWindow (0),
    drawing (false),
    fadeTime (0)
{
    ScreenInterface::setHandler (screen);
    CompositeScreenInterface::setHandler (cScreen);
    GLScreenInterface::setHandler (gScreen);

    memset (&resizeGeometry, 0, sizeof (resizeGeometry));

    cScreen->preparePaintSetEnabled (this, false);
    gScreen->glPaintOutputSetEnabled (this, false);
    cScreen->donePaintSetEnabled (this, false);

    screen->handleEventSetEnabled (this, false);

    backgroundLayer.renderBackground ();

    optionSetGradient1Notify (gradientChanged);
    optionSetGradient2Notify (gradientChanged);
    optionSetGradient3Notify (gradientChanged);
}

InfoWindow::InfoWindow (CompWindow *window) :
    PluginClassHandler <InfoWindow, CompWindow> (window),
    window (window)
{
    WindowInterface::setHandler (window);

    window->ungrabNotifySetEnabled (this, false);
}

bool
InfoPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
	!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

    return true;
}
