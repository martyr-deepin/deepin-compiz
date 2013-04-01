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

#include "annotate.h"

COMPIZ_PLUGIN_20090315 (annotate, AnnoPluginVTable)

#define DEG2RAD (M_PI / 180.0f)

void
AnnoScreen::cairoClear (cairo_t    *cr)
{
    cairo_save (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint (cr);
    cairo_restore (cr);

    content = false;
}

cairo_t *
AnnoScreen::cairoContext ()
{
    if (!cairo)
    {
	XRenderPictFormat *format;
	Screen		  *xScreen;
	int		  w, h;

	xScreen = ScreenOfDisplay (screen->dpy (), screen->screenNum ());

	w = screen->width ();
	h = screen->height ();

	format = XRenderFindStandardFormat (screen->dpy (),
					    PictStandardARGB32);

	pixmap = XCreatePixmap (screen->dpy (), screen->root (), w, h, 32);

	texture = GLTexture::bindPixmapToTexture (pixmap, w, h, 32);

	if (texture.empty ())
	{
	    compLogMessage ("annotate", CompLogLevelError,
			    "Couldn't bind pixmap 0x%x to texture",
			    (int) pixmap);

	    XFreePixmap (screen->dpy (), pixmap);

	    return NULL;
	}

	damage = XDamageCreate (screen->dpy (), pixmap,
				XDamageReportBoundingBox);

	surface =
	    cairo_xlib_surface_create_with_xrender_format (screen->dpy (),
							   pixmap, xScreen,
							   format, w, h);

	cairo = cairo_create (surface);
	
	if (cairoBuffer.size ())
	{
	    cairo_t *cr = cairo_create (surface);
	    int stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, w);
	    cairo_surface_t *raw_source =
		cairo_image_surface_create_for_data ((unsigned char *)
						     cairoBuffer.c_str (),
						     CAIRO_FORMAT_ARGB32,
						     w, h, stride);
	    
	    if (cr && raw_source)
	    {	      
		cairo_set_source_surface (cr, raw_source, 0, 0);
		cairo_paint (cr);
		
		cairo_surface_destroy (raw_source);
		cairo_destroy (cr);
		cairoBuffer.clear ();
	    }
	}
	else	    
	    cairoClear (cairo);
    }

    return cairo;
}

void
AnnoScreen::setSourceColor (cairo_t	   *cr,
		    	    unsigned short *color)
{
    cairo_set_source_rgba (cr,
			   (double) color[0] / 0xffff,
			   (double) color[1] / 0xffff,
			   (double) color[2] / 0xffff,
			   (double) color[3] / 0xffff);
}

void
AnnoScreen::drawEllipse (double		xc,
			 double		yc,
			 double		radiusX,
			 double		radiusY,
			 unsigned short	*fillColor,
			 unsigned short	*strokeColor,
			 double		strokeWidth)
{
    cairo_t *cr;

    cr = cairoContext ();
    if (cr)
    {
	setSourceColor (cr, fillColor);
	cairo_save (cr);
	cairo_translate (cr, xc, yc);
	if (radiusX > radiusY)
	{
	    cairo_scale (cr, 1.0, radiusY/radiusX);
	    cairo_arc (cr, 0, 0, radiusX, 0, 2 * M_PI);
	}
	else
	{
	    cairo_scale (cr, radiusX/radiusY, 1.0);
	    cairo_arc (cr, 0, 0, radiusY, 0, 2 * M_PI);
	}
	cairo_restore (cr);
	cairo_fill_preserve (cr);
	cairo_set_line_width (cr, strokeWidth);
	setSourceColor (cr, strokeColor);
	cairo_stroke (cr);

	content = true;
    }
}

void
AnnoScreen::drawRectangle (double	  x,
			   double	  y,
			   double	  w,
			   double	  h,
			   unsigned short *fillColor,
			   unsigned short *strokeColor,
			   double	  strokeWidth)
{
    cairo_t *cr;

    cr = cairoContext ();
    if (cr)
    {
	double  ex1, ey1, ex2, ey2;

	setSourceColor (cr, fillColor);
	cairo_rectangle (cr, x, y, w, h);
	cairo_fill_preserve (cr);
	cairo_set_line_width (cr, strokeWidth);
	cairo_stroke_extents (cr, &ex1, &ey1, &ex2, &ey2);
	setSourceColor (cr, strokeColor);
	cairo_stroke (cr);

	content = true;
    }
}

void
AnnoScreen::drawLine (double	     x1,
		      double	     y1,
		      double	     x2,
		      double	     y2,
		      double	     width,
		      unsigned short *color)
{
    cairo_t *cr;

    cr = cairoContext ();
    if (cr)
    {
	double ex1, ey1, ex2, ey2;

	cairo_set_line_width (cr, width);
	cairo_move_to (cr, x1, y1);
	cairo_line_to (cr, x2, y2);
	cairo_stroke_extents (cr, &ex1, &ey1, &ex2, &ey2);
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	setSourceColor (cr, color);
	cairo_stroke (cr);

	content = true;
    }
}

void
AnnoScreen::drawText (double	     		     x,
		      double	     		     y,
		      const char	     	     *text,
		      const char	     	     *fontFamily,
		      double	     		     fontSize,
		      cairo_font_slant_t	     fontSlant,
		      cairo_font_weight_t	     fontWeight,
		      unsigned short 		     *fillColor,
		      unsigned short 		     *strokeColor,
		      double	     		     strokeWidth)
{
    REGION reg;
    cairo_t *cr;

    cr = cairoContext ();
    if (cr)
    {
	cairo_text_extents_t extents;

	cairo_set_line_width (cr, strokeWidth);
	setSourceColor (cr, fillColor);
	cairo_select_font_face (cr, fontFamily, fontSlant, fontWeight);
	cairo_set_font_size (cr, fontSize);
	cairo_text_extents (cr, text, &extents);
	cairo_save (cr);
	cairo_move_to (cr, x, y);
	cairo_text_path (cr, text);
	cairo_fill_preserve (cr);
	setSourceColor (cr, strokeColor);
	cairo_stroke (cr);
	cairo_restore (cr);

	reg.rects    = &reg.extents;
	reg.numRects = 1;

	reg.extents.x1 = x;
	reg.extents.y1 = y + extents.y_bearing - 2.0;
	reg.extents.x2 = x + extents.width + 20.0;
	reg.extents.y2 = y + extents.height;

	content = true;
    }
}

/* DBUS Interface (TODO: plugin interface) */

/* Here, you can use DBUS or any other plugin via the action system to draw on
 * the screen using cairo. Parameters are as follows:
 * "tool": ["rectangle", "circle", "line", "text"] default: "line"
 * - This allows you to select what you want to draw
 * Tool-specific parameters:
 * - * "circle"
 * - * - "xc" float, default: 0 - X Center
 * - * - "yc" float, default: 0 - Y Center
 * - * - "radius" float, default: 0 - Radius
 * - * "rectangle"
 * - * - "x" float, default: 0 - X Point
 * - * - "y" float, default: 0 - Y Point
 * - * - "width" float, default: 0 - Width
 * - * - "height" float, default: 0 - Height
 * - * "line"
 * - * - "x1" float, default: 0 - X Point 1
 * - * - "y1" float, default: 0 - Y Point 1
 * - * - "x2" float, default: 0 - X Point 2
 * - * - "y2" float, default: 0 - Y Point 2
 * - * "text"
 * - * - "slant" string, default: "" - ["oblique", "italic", ""] - Text Slant
 * - * - "weight" string, default: " - ["bold", ""] - Text Weight
 * - * - "text" string, default: "" - Any Character - The text to display
 * - * - "family" float, default: "Sans" - The font family
 * - * - "size" float, default: 36.0 - Font Size
 * - * - "x" float, default: 0 - X Point
 * - * - "u" float, default: 0 - Y Point
 * Other parameters are:
 * - * - "fill_color" float, default: 0 - Drawing Fill Color
 * - * - "stroke_color" float, default: 0 - Drawing Border Color
 * - * - "line_width" float, default: 0 - Drawing Width
 * - * - "stroke_width" float, default: 0 - Drawing Height
 * - * - All of these are taken from the builtin options if not provided
 */ 

bool
AnnoScreen::draw (CompAction         *action,
		  CompAction::State  state,
		  CompOption::Vector& options)
{
    cairo_t *cr;

    cr = cairoContext ();
    if (cr)
    {
        const char	*tool;
        unsigned short	*fillColor, *strokeColor;
        double		strokeWidth;

        tool =
	 CompOption::getStringOptionNamed (options, "tool", "line").c_str ();

        cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
        cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);

        fillColor = optionGetFillColor ();
        fillColor = CompOption::getColorOptionNamed (options, "fill_color",
				         fillColor);

        strokeColor = optionGetStrokeColor ();
        strokeColor = CompOption::getColorOptionNamed (options,
				           "stroke_color", strokeColor);

        strokeWidth = optionGetStrokeWidth ();
        strokeWidth = CompOption::getFloatOptionNamed (options, "stroke_width",
				           strokeWidth);

        if (strcasecmp (tool, "rectangle") == 0)
        {
	    double x, y, w, h;

	    x = CompOption::getFloatOptionNamed (options, "x", 0);
	    y = CompOption::getFloatOptionNamed (options, "y", 0);
	    w = CompOption::getFloatOptionNamed (options, "w", 100);
	    h = CompOption::getFloatOptionNamed (options, "h", 100);

	    drawRectangle (x, y, w, h, fillColor, strokeColor,
			       strokeWidth);
        }
        else if (strcasecmp (tool, "ellipse") == 0)
        {
	    double xc, yc, xr, yr;

	    xc = CompOption::getFloatOptionNamed (options, "xc", 0);
	    yc = CompOption::getFloatOptionNamed (options, "yc", 0);
	    xr = CompOption::getFloatOptionNamed (options, "radiusX", 100);
	    yr = CompOption::getFloatOptionNamed (options, "radiusY", 100);

	    drawEllipse (xc, yc, xr, yr, fillColor, strokeColor,
			    strokeWidth);
        }
        else if (strcasecmp (tool, "line") == 0)
        {
	    double x1, y1, x2, y2;

	    x1 = CompOption::getFloatOptionNamed (options, "x1", 0);
	    y1 = CompOption::getFloatOptionNamed (options, "y1", 0);
	    x2 = CompOption::getFloatOptionNamed (options, "x2", 100);
	    y2 = CompOption::getFloatOptionNamed (options, "y2", 100);

	    drawLine (x1, y1, x2, y2, strokeWidth, fillColor);
        }
        else if (strcasecmp (tool, "text") == 0)
        {
	    double	     x, y, size;
	    const char	     *text, *family;
	    cairo_font_slant_t slant;
	    cairo_font_weight_t weight;
	    const char	     *str;

	    str =
	       CompOption::getStringOptionNamed (options, "slant", "").c_str ();
	    if (strcasecmp (str, "oblique") == 0)
	        slant = CAIRO_FONT_SLANT_OBLIQUE;
	    else if (strcasecmp (str, "italic") == 0)
	        slant = CAIRO_FONT_SLANT_ITALIC;
	    else
	        slant = CAIRO_FONT_SLANT_NORMAL;

	    str = 
	      CompOption::getStringOptionNamed (options, "weight", "").c_str ();
	    if (strcasecmp (str, "bold") == 0)
	        weight = CAIRO_FONT_WEIGHT_BOLD;
	    else
	        weight = CAIRO_FONT_WEIGHT_NORMAL;

	    x      = CompOption::getFloatOptionNamed (options, "x", 0);
	    y      = CompOption::getFloatOptionNamed (options, "y", 0);
	    text   = 
		CompOption::getStringOptionNamed (options, "text", "").c_str ();
	    family = CompOption::getStringOptionNamed (options, "family",
				           "Sans").c_str ();
	    size   = CompOption::getFloatOptionNamed (options, "size", 36.0);

	    drawText (x, y, text, family, size, slant, weight,
		          fillColor, strokeColor, strokeWidth);
        }
    }

    return true;
}

bool
AnnoScreen::terminate (CompAction         *action,
		       CompAction::State  state,
		       CompOption::Vector& options)
{
    if (grabIndex)
    {
        screen->removeGrab (grabIndex, NULL);
        grabIndex = 0;
    }

    action->setState (action->state () & ~(CompAction::StateTermKey |
					   CompAction::StateTermButton));

    switch (drawMode)
    {
    case LineMode:
	drawLine (initialPointerX, initialPointerY,
		  lineVector.x (), lineVector.y (),
		  optionGetStrokeWidth (),
		  optionGetStrokeColor ());
	break;

    case RectangleMode:
	drawRectangle (rectangle.x (),
		       rectangle.y (),
		       rectangle.width (),
		       rectangle.height (),
		       optionGetFillColor (),
		       optionGetStrokeColor (),
		       optionGetStrokeWidth ());
	break;

    case EllipseMode:
	drawEllipse (ellipse.center.x (),
		     ellipse.center.y (),
		     ellipse.radiusX,
		     ellipse.radiusY,
		     optionGetFillColor (),
		     optionGetStrokeColor (),
		     optionGetStrokeWidth ());
	break;

    default:
	break;
    }

    drawMode = NoMode;

    return false;
}

bool
AnnoScreen::initiateErase (CompAction         *action,
		           CompAction::State  state,
		           CompOption::Vector& options)
{
    if (screen->otherGrabExist (NULL))
        return false;

    if (!grabIndex)
        grabIndex = screen->pushGrab (None, "annotate");

    if (state & CompAction::StateInitButton)
        action->setState (action->state () | CompAction::StateTermButton);

    if (state & CompAction::StateInitKey)
        action->setState (action->state () | CompAction::StateTermKey);

    annoLastPointerX = pointerX;
    annoLastPointerY = pointerY;

    drawMode = EraseMode;

    screen->handleEventSetEnabled (this, true);

    return false;
}

bool
AnnoScreen::initiateFreeDraw (CompAction         *action,
			      CompAction::State  state,
			      CompOption::Vector& options)
{
    if (screen->otherGrabExist (NULL))
        return false;

    if (!grabIndex)
        grabIndex = screen->pushGrab (None, "annotate");

    if (state & CompAction::StateInitButton)
        action->setState (action->state () | CompAction::StateTermButton);

    if (state & CompAction::StateInitKey)
        action->setState (action->state () | CompAction::StateTermKey);

    annoLastPointerX = pointerX;
    annoLastPointerY = pointerY;

    drawMode = FreeDrawMode;

    screen->handleEventSetEnabled (this, true);

    return true;
}

bool
AnnoScreen::initiateLine (CompAction         *action,
			  CompAction::State  state,
			  CompOption::Vector& options)
{
    if (screen->otherGrabExist (NULL))
        return false;

    if (!grabIndex)
        grabIndex = screen->pushGrab (None, "annotate");

    if (state & CompAction::StateInitButton)
        action->setState (action->state () | CompAction::StateTermButton);

    if (state & CompAction::StateInitKey)
        action->setState (action->state () | CompAction::StateTermKey);

    initialPointerX = pointerX;
    initialPointerY = pointerY;

    drawMode = LineMode;

    screen->handleEventSetEnabled (this, true);

    return true;
}

bool
AnnoScreen::initiateRectangle (CompAction         *action,
			       CompAction::State  state,
			       CompOption::Vector& options)
{
    if (screen->otherGrabExist (NULL))
        return false;

    if (!grabIndex)
        grabIndex = screen->pushGrab (None, "annotate");

    if (state & CompAction::StateInitButton)
        action->setState (action->state () | CompAction::StateTermButton);

    if (state & CompAction::StateInitKey)
        action->setState (action->state () | CompAction::StateTermKey);

    drawMode = RectangleMode;

    initialPointerX = pointerX;
    initialPointerY = pointerY;
    rectangle.setGeometry (initialPointerX, initialPointerY, 0, 0);
    lastRect = rectangle;

    screen->handleEventSetEnabled (this, true);

    return true;
}

bool
AnnoScreen::initiateEllipse (CompAction         *action,
			     CompAction::State  state,
			     CompOption::Vector& options)
{
    if (screen->otherGrabExist (NULL))
        return false;

    if (!grabIndex)
        grabIndex = screen->pushGrab (None, "annotate");

    if (state & CompAction::StateInitButton)
        action->setState (action->state () | CompAction::StateTermButton);

    if (state & CompAction::StateInitKey)
        action->setState (action->state () | CompAction::StateTermKey);

    drawMode = EllipseMode;

    initialPointerX = pointerX;
    initialPointerY = pointerY;
    ellipse.radiusX = 0;
    ellipse.radiusY = 0;
    lastRect.setGeometry (initialPointerX, initialPointerY, 0, 0);

    screen->handleEventSetEnabled (this, true);

    return true;
}

bool
AnnoScreen::clear (CompAction         *action,
		   CompAction::State  state,
		   CompOption::Vector& options)
{
    if (content)
    {
        cairo_t *cr;

        cr = cairoContext ();
        if (cr)
	    cairoClear (cairo);

        cScreen->damageScreen ();

	/* We don't need to refresh the screen or handle events anymore */
	screen->handleEventSetEnabled (this, false);
	gScreen->glPaintOutputSetEnabled (this, false);
    }

    return true;
}

bool
AnnoScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
			   const GLMatrix	     &transform,
			   const CompRegion	     &region,
			   CompOutput 		     *output,
			   unsigned int		     mask)
{
    bool status;

    status = gScreen->glPaintOutput (attrib, transform, region, output, mask);

    if (status)
    {
	GLVertexBuffer *streamingBuffer = GLVertexBuffer::streamingBuffer ();
	GLfloat         vertexData[18];
	GLfloat         textureData[12];
	CompRect	rect;
	GLMatrix	sTransform = transform;
	int		numRect;
	int		pos = 0;
	float		offset;
	int		angle;

	offset = optionGetStrokeWidth () / 2;

	/* This replaced prepareXCoords (s, output, -DEFAULT_Z_CAMERA) */
	sTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);

	glEnable (GL_BLEND);

	if (content && !region.isEmpty ())
	{
	    foreach (GLTexture *tex, texture)
	    {
	        CompRect::vector rect = region.rects ();
	        numRect = region.rects ().size ();

	        tex->enable (GLTexture::Fast);

	        streamingBuffer->begin (GL_TRIANGLES);

	        while (numRect--)
	        {
		    GLfloat tx1 = COMP_TEX_COORD_X (tex->matrix (),
		                                       rect.at (pos).x1 ());
		    GLfloat tx2 = COMP_TEX_COORD_X (tex->matrix (),
		                                       rect.at (pos).x2 ());
		    GLfloat ty1 = COMP_TEX_COORD_Y (tex->matrix (),
		                                       rect.at (pos).y1 ());
		    GLfloat ty2 = COMP_TEX_COORD_Y (tex->matrix (),
		                                       rect.at (pos).y2 ());

		    vertexData[0]  = rect.at (pos).x1 ();
		    vertexData[1]  = rect.at (pos).y1 ();
		    vertexData[2]  = 0.0f;
		    vertexData[3]  = rect.at (pos).x1 ();
		    vertexData[4]  = rect.at (pos).y2 ();
		    vertexData[5]  = 0.0f;
		    vertexData[6]  = rect.at (pos).x2 ();
		    vertexData[7]  = rect.at (pos).y1 ();
		    vertexData[8]  = 0.0f;
		    vertexData[9]  = rect.at (pos).x1 ();
		    vertexData[10] = rect.at (pos).y2 ();
		    vertexData[11] = 0.0f;

		    vertexData[12] = rect.at (pos).x2 ();
		    vertexData[13] = rect.at (pos).y2 ();
		    vertexData[14] = 0.0f;

		    vertexData[15] = rect.at (pos).x2 ();
		    vertexData[16] = rect.at (pos).y1 ();
		    vertexData[17] = 0.0f;

		    textureData[0]  = tx1;
		    textureData[1]  = ty1;

		    textureData[2]  = tx1;
		    textureData[3]  = ty2;

		    textureData[4]  = tx2;
		    textureData[5]  = ty1;

		    textureData[6]  = tx1;
		    textureData[7]  = ty2;

		    textureData[8]  = tx2;
		    textureData[9]  = ty2;

		    textureData[10] = tx2;
		    textureData[11] = ty1;

		    streamingBuffer->addVertices (6, vertexData);
		    streamingBuffer->addTexCoords (0, 6, textureData);
	            pos++;
	        }

		streamingBuffer->end ();
		streamingBuffer->render (sTransform);

	        tex->disable ();
	    }
	}

	switch (drawMode)
	{
	case LineMode:
	    glLineWidth (optionGetStrokeWidth ());

	    streamingBuffer->begin (GL_LINES);

	    streamingBuffer->addColors (1, optionGetStrokeColor ());

	    vertexData[0] = initialPointerX;
	    vertexData[1] = initialPointerY;
	    vertexData[2] = 0.0f;
	    vertexData[3] = lineVector.x ();
	    vertexData[4] = lineVector.y ();
	    vertexData[5] = 0.0f;
	    streamingBuffer->addVertices (2, vertexData);

	    streamingBuffer->end ();
	    streamingBuffer->render (sTransform);
	    break;

	case RectangleMode:
	    vertexData[0]  = rectangle.x1 ();
	    vertexData[1]  = rectangle.y1 ();
	    vertexData[2]  = 0.0f;
	    vertexData[3]  = rectangle.x1 ();
	    vertexData[4]  = rectangle.y2 ();
	    vertexData[5]  = 0.0f;
	    vertexData[6]  = rectangle.x2 ();
	    vertexData[7]  = rectangle.y1 ();
	    vertexData[8]  = 0.0f;
	    vertexData[9]  = rectangle.x2 ();
	    vertexData[10] = rectangle.y2 ();
	    vertexData[11] = 0.0f;

	    /* fill rectangle */
	    streamingBuffer->begin (GL_TRIANGLE_STRIP);

	    streamingBuffer->addColors (1, optionGetFillColor ());
	    streamingBuffer->addVertices (4, vertexData);

	    streamingBuffer->end ();
	    streamingBuffer->render (sTransform);

	    /* draw rectangle outline */
/*	    streamingBuffer->begin ();

	    streamingBuffer->addColors (1, optionGetStrokeColor ());

	    vertexData[0] = rectangle.x1 () - offset;
	    vertexData[3] = rectangle.x1 () - offset;
	    streamingBuffer->addVertices (4, vertexData);

	    glRecti (rectangle.x2 () - offset, rectangle.y2 (),
		     rectangle.x2 () + offset, rectangle.y1 ());
	    glRecti (rectangle.x1 () - offset, rectangle.y1 () + offset,
		     rectangle.x2 () + offset, rectangle.y1 () - offset);
	    glRecti (rectangle.x1 () - offset, rectangle.y2 () + offset,
		     rectangle.x2 () + offset, rectangle.y2 () - offset);*/
	    break;

	case EllipseMode:
	    /* fill ellipse */
	    streamingBuffer->begin (GL_TRIANGLE_FAN);

	    streamingBuffer->addColors (1, optionGetFillColor ());

	    vertexData[0] = ellipse.center.x ();
	    vertexData[1] = ellipse.center.y ();
	    vertexData[2] = 0.0f;
	    streamingBuffer->addVertices (1, vertexData);

	    for (angle = 0; angle <= 360; angle += 1)
	    {
		vertexData[0] = ellipse.center.x () +
			 (ellipse.radiusX * sinf (angle * DEG2RAD));
		vertexData[1] = ellipse.center.y () +
			 (ellipse.radiusY * cosf (angle * DEG2RAD));
		streamingBuffer->addVertices (1, vertexData);
	    }

	    vertexData[0] = ellipse.center.x ();
	    vertexData[1] = ellipse.center.y () + ellipse.radiusY;
	    streamingBuffer->addVertices (1, vertexData);

	    streamingBuffer->end ();
	    streamingBuffer->render (sTransform);

	    /* draw ellipse outline */
	    glLineWidth (optionGetStrokeWidth ());

	    streamingBuffer->begin (GL_TRIANGLE_STRIP);

	    streamingBuffer->addColors (1, optionGetStrokeColor ());


	    vertexData[0] = ellipse.center.x ();
	    vertexData[1] = ellipse.center.y () + ellipse.radiusY - offset;
	    vertexData[2] = 0.0f;
	    streamingBuffer->addVertices (1, vertexData);

	    for (angle = 360; angle >= 0; angle -= 1)
	    {
		vertexData[0] = ellipse.center.x () + ((ellipse.radiusX -
			  offset) * sinf (angle * DEG2RAD));
		vertexData[1] = ellipse.center.y () + ((ellipse.radiusY -
			  offset) * cosf (angle * DEG2RAD));
		vertexData[2] = 0.0f;
		vertexData[3] = ellipse.center.x () + ((ellipse.radiusX +
			  offset) * sinf (angle * DEG2RAD));
		vertexData[4] = ellipse.center.y () + ((ellipse.radiusY +
			  offset) * cosf (angle * DEG2RAD));
		vertexData[5] = 0.0f;
		streamingBuffer->addVertices (2, vertexData);
	    }

	    vertexData[0] = ellipse.center.x ();
	    vertexData[1] = ellipse.center.y () + ellipse.radiusY + offset;
	    streamingBuffer->addVertices (1, vertexData);

	    streamingBuffer->end ();
	    streamingBuffer->render (sTransform);
	    break;

	default:
	    break;
	}

	glDisable (GL_BLEND);
    }

    return status;
}

void
AnnoScreen::handleMotionEvent (int	  xRoot,
		       	       int	  yRoot)
{
    CompRect damageRect;

    if (grabIndex)
    {
	static unsigned short clearColor[] = { 0, 0, 0, 0 };
	switch (drawMode)
	{
	case EraseMode:
	    drawLine (annoLastPointerX, annoLastPointerY,
		      xRoot, yRoot,
		      optionGetEraseWidth (), clearColor);
	    break;

	case FreeDrawMode:
	    drawLine (annoLastPointerX, annoLastPointerY,
		      xRoot, yRoot,
		      optionGetStrokeWidth (),
		      optionGetStrokeColor ());
	    break;

	case LineMode:
	    lineVector.setX (xRoot);
	    lineVector.setY (yRoot);

	    damageRect.setGeometry (MIN(initialPointerX, lineVector.x ()),
				    MIN(initialPointerY, lineVector.y ()),
				    abs (lineVector.x () - initialPointerX),
				    abs (lineVector.y () - initialPointerY));
	    break;

	case RectangleMode:
	    if (optionGetDrawShapesFromCenter ())
		rectangle.setGeometry (initialPointerX -
				       abs (xRoot - initialPointerX),
				       initialPointerY -
				       abs (yRoot - initialPointerY),
				      (abs (xRoot - initialPointerX)) * 2,
				      (abs (yRoot - initialPointerY)) * 2);
	    else
		rectangle.setGeometry (MIN(initialPointerX, xRoot),
				       MIN(initialPointerY, yRoot),
				       abs (xRoot - initialPointerX),
				       abs (yRoot - initialPointerY));

	    damageRect = rectangle;
	    break;

	case EllipseMode:
	    if (optionGetDrawShapesFromCenter ())
	    {
		ellipse.center.setX (initialPointerX);
		ellipse.center.setY (initialPointerY);
	    }
	    else
	    {
		ellipse.center.setX (initialPointerX +
				    (xRoot - initialPointerX) / 2);
		ellipse.center.setY (initialPointerY +
				    (yRoot - initialPointerY) / 2);
	    }

	    ellipse.radiusX = abs (xRoot - ellipse.center.x ());
	    ellipse.radiusY = abs (yRoot - ellipse.center.y ());

	    damageRect = CompRect (ellipse.center.x () - ellipse.radiusX,
				   ellipse.center.y () - ellipse.radiusY,
				   ellipse.radiusX * 2,
				   ellipse.radiusY * 2);
	    break;

	default:
	    break;
	}

	if (cScreen && (drawMode == LineMode ||
			drawMode == RectangleMode ||
			drawMode == EllipseMode))
	{
	    /* Add border width to the damage region */
	    damageRect.setGeometry (damageRect.x () -
				    (optionGetStrokeWidth () / 2),
				    damageRect.y () -
				    (optionGetStrokeWidth () / 2),
				    damageRect.width () +
				    optionGetStrokeWidth () + 1,
				    damageRect.height () +
				    optionGetStrokeWidth () + 1);

	    cScreen->damageRegion (damageRect);
	    cScreen->damageRegion (lastRect);

	    lastRect = damageRect;
	}

	annoLastPointerX = xRoot;
	annoLastPointerY = yRoot;

	gScreen->glPaintOutputSetEnabled (this, true);
    }
}

void
AnnoScreen::handleEvent (XEvent      *event)
{
    switch (event->type) {
    case MotionNotify:
	handleMotionEvent (pointerX, pointerY);
    case EnterNotify:
    case LeaveNotify:
	handleMotionEvent (pointerX, pointerY);
    default:
	if (event->type == cScreen->damageEvent () + XDamageNotify)
	{
	    XDamageNotifyEvent *de = (XDamageNotifyEvent *) event;
	    if (pixmap == de->drawable)
		cScreen->damageRegion (CompRegion (CompRect (de->area)));
	}
	break;
    }

    screen->handleEvent (event);
}

AnnoScreen::AnnoScreen (CompScreen *screen) :
    PluginClassHandler <AnnoScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    grabIndex (0),
    pixmap (None),
    surface (NULL),
    cairo (NULL),
    content (false),
    damage (None)
{
    ScreenInterface::setHandler (screen, false);
    GLScreenInterface::setHandler (gScreen, false);

    optionSetDrawInitiate
	(boost::bind (&AnnoScreen::draw, this, _1, _2, _3));
    optionSetEraseButtonInitiate
	(boost::bind (&AnnoScreen::initiateErase, this, _1, _2, _3));
    optionSetEraseButtonTerminate
	(boost::bind (&AnnoScreen::terminate, this, _1, _2, _3));
    optionSetInitiateFreeDrawButtonInitiate
	(boost::bind (&AnnoScreen::initiateFreeDraw, this, _1, _2, _3));
    optionSetInitiateFreeDrawButtonTerminate
	(boost::bind (&AnnoScreen::terminate, this, _1, _2, _3));
    optionSetInitiateLineButtonInitiate
	(boost::bind (&AnnoScreen::initiateLine, this, _1, _2, _3));
    optionSetInitiateLineButtonTerminate
	(boost::bind (&AnnoScreen::terminate, this, _1, _2, _3));
    optionSetInitiateRectangleButtonInitiate
	(boost::bind (&AnnoScreen::initiateRectangle, this, _1, _2, _3));
    optionSetInitiateRectangleButtonTerminate
	(boost::bind (&AnnoScreen::terminate, this, _1, _2, _3));
    optionSetInitiateEllipseButtonInitiate
	(boost::bind (&AnnoScreen::initiateEllipse, this, _1, _2, _3));
    optionSetInitiateEllipseButtonTerminate
	(boost::bind (&AnnoScreen::terminate, this, _1, _2, _3));
    optionSetClearKeyInitiate
	(boost::bind (&AnnoScreen::clear, this, _1, _2, _3));
    drawMode = NoMode;
}

AnnoScreen::~AnnoScreen ()
{
    if (cairo)
	cairo_destroy (cairo);
    if (surface)
	cairo_surface_destroy (surface);
    if (pixmap)
	XFreePixmap (screen->dpy (), pixmap);
    if (damage)
	XDamageDestroy (screen->dpy (), damage);
}

bool
AnnoPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
	!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

    return true;
}
