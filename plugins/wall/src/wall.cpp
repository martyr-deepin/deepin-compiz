/**
 *
 * Compiz wall plugin
 *
 * wall.cpp
 *
 * Copyright (c) 2006 Robert Carr <racarr@beryl-project.org>
 *               2011 Linaro Limited
 *
 * Authors:
 * Robert Carr <racarr@beryl-project.org>
 * Dennis Kasprzyk <onestone@opencompositing.org>
 * Travis Watkins <travis.watkins@linaro.org>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <dlfcn.h>

#include <core/core.h>
#include <core/atoms.h>
#include <opengl/opengl.h>

#include "wall.h"

static const double PI = 3.14159265359f;
static const unsigned short VIEWPORT_SWITCHER_SIZE = 100;
static const unsigned short ARROW_SIZE = 33;

#define getColorRGBA(name) \
    r = optionGet##name##Red() / 65535.0f;\
    g = optionGet##name##Green() / 65535.0f; \
    b = optionGet##name##Blue() / 65535.0f; \
    a = optionGet##name##Alpha() / 65535.0f

#define sigmoid(x) (1.0f / (1.0f + exp (-5.5f * 2 * ((x) - 0.5))))
#define sigmoidProgress(x) ((sigmoid (x) - sigmoid (0)) / \
			    (sigmoid (1) - sigmoid (0)))

COMPIZ_PLUGIN_20090315 (wall, WallPluginVTable);

void
WallScreen::clearCairoLayer (cairo_t *cr)
{
    cairo_save (cr);
    cairo_set_operator (cr, CAIRO_OPERATOR_CLEAR);
    cairo_paint (cr);
    cairo_restore (cr);
}

void
WallScreen::drawSwitcherBackground ()
{
    cairo_t         *cr;
    cairo_pattern_t *pattern;
    float           outline = 2.0f;
    int             width, height, radius;
    float           r, g, b, a;
    unsigned int    i, j;

    destroyCairoContext (switcherContext);
    setupCairoContext (switcherContext);

    cr = switcherContext.cr;
    clearCairoLayer (cr);

    width = switcherContext.width - outline;
    height = switcherContext.height - outline;

    cairo_save (cr);
    cairo_translate (cr, outline / 2.0f, outline / 2.0f);

    /* set the pattern for the switcher's background */
    pattern = cairo_pattern_create_linear (0, 0, width, height);
    getColorRGBA (BackgroundGradientBaseColor);
    cairo_pattern_add_color_stop_rgba (pattern, 0.00f, r, g, b, a);
    getColorRGBA (BackgroundGradientHighlightColor);
    cairo_pattern_add_color_stop_rgba (pattern, 0.65f, r, g, b, a);
    getColorRGBA (BackgroundGradientShadowColor);
    cairo_pattern_add_color_stop_rgba (pattern, 0.85f, r, g, b, a);
    cairo_set_source (cr, pattern);

    /* draw the border's shape */
    radius = optionGetEdgeRadius ();
    if (radius)
    {
	cairo_arc (cr, radius, radius, radius, PI, 1.5f * PI);
	cairo_arc (cr, radius + width - 2 * radius,
		   radius, radius, 1.5f * PI, 2.0 * PI);
	cairo_arc (cr, width - radius, height - radius, radius, 0,  PI / 2.0f);
	cairo_arc (cr, radius, height - radius, radius,  PI / 2.0f, PI);
    }
    else
    {
	cairo_rectangle (cr, 0, 0, width, height);
    }

    cairo_close_path (cr);

    /* apply pattern to background... */
    cairo_fill_preserve (cr);

    /* ... and draw an outline */
    cairo_set_line_width (cr, outline);
    getColorRGBA (OutlineColor);
    cairo_set_source_rgba (cr, r, g, b, a);
    cairo_stroke (cr);

    cairo_pattern_destroy (pattern);
    cairo_restore (cr);

    cairo_save (cr);
    for (i = 0; i < (unsigned int) screen->vpSize ().height (); i++)
    {
	cairo_translate (cr, 0.0, viewportBorder);
	cairo_save (cr);
	for (j = 0; j < (unsigned int) screen->vpSize ().width (); j++)
	{
	    cairo_translate (cr, viewportBorder, 0.0);

	    /* this cuts a hole into our background */
	    cairo_set_operator (cr, CAIRO_OPERATOR_OVER);
	    cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
	    cairo_rectangle (cr, 0, 0, viewportWidth, viewportHeight);

	    cairo_fill_preserve (cr);
	    cairo_set_operator (cr, CAIRO_OPERATOR_XOR);
	    cairo_fill (cr);

	    cairo_translate (cr, viewportWidth, 0.0);
	}
	cairo_restore(cr);

	cairo_translate (cr, 0.0, viewportHeight);
    }
    cairo_restore (cr);
}

void
WallScreen::drawThumb ()
{
    cairo_t         *cr;
    cairo_pattern_t *pattern;
    float           r, g, b, a;
    float           outline = 2.0f;
    int             width, height;

    destroyCairoContext (thumbContext);
    setupCairoContext (thumbContext);

    cr = thumbContext.cr;
    clearCairoLayer (cr);

    width  = thumbContext.width - outline;
    height = thumbContext.height - outline;

    cairo_translate (cr, outline / 2.0f, outline / 2.0f);

    pattern = cairo_pattern_create_linear (0, 0, width, height);
    getColorRGBA (ThumbGradientBaseColor);
    cairo_pattern_add_color_stop_rgba (pattern, 0.0f, r, g, b, a);
    getColorRGBA (ThumbGradientHighlightColor);
    cairo_pattern_add_color_stop_rgba (pattern, 1.0f, r, g, b, a);

    /* apply the pattern for thumb background */
    cairo_set_source (cr, pattern);
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_fill_preserve (cr);

    cairo_set_line_width (cr, outline);
    getColorRGBA (OutlineColor);
    cairo_set_source_rgba (cr, r, g, b, a);
    cairo_stroke (cr);

    cairo_pattern_destroy (pattern);

    cairo_restore (cr);
}

void
WallScreen::drawHighlight ()
{
    cairo_t         *cr;
    cairo_pattern_t *pattern;
    int             width, height;
    float           r, g, b, a;
    float           outline = 2.0f;

    destroyCairoContext (highlightContext);
    setupCairoContext (highlightContext);

    cr = highlightContext.cr;
    clearCairoLayer (cr);

    width  = highlightContext.width - outline;
    height = highlightContext.height - outline;

    cairo_translate (cr, outline / 2.0f, outline / 2.0f);

    pattern = cairo_pattern_create_linear (0, 0, width, height);
    getColorRGBA (ThumbHighlightGradientBaseColor);
    cairo_pattern_add_color_stop_rgba (pattern, 0.0f, r, g, b, a);
    getColorRGBA (ThumbHighlightGradientShadowColor);
    cairo_pattern_add_color_stop_rgba (pattern, 1.0f, r, g, b, a);

    /* apply the pattern for thumb background */
    cairo_set_source (cr, pattern);
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_fill_preserve (cr);

    cairo_set_line_width (cr, outline);
    getColorRGBA (OutlineColor);
    cairo_set_source_rgba (cr, r, g, b, a);
    cairo_stroke (cr);

    cairo_pattern_destroy (pattern);

    cairo_restore (cr);
}

void
WallScreen::drawArrow ()
{
    cairo_t *cr;
    float   outline = 2.0f;
    float   r, g, b, a;

    destroyCairoContext (arrowContext);
    setupCairoContext (arrowContext);

    cr = arrowContext.cr;
    clearCairoLayer (cr);

    cairo_translate (cr, outline / 2.0f, outline / 2.0f);

    /* apply the pattern for thumb background */
    cairo_set_line_width (cr, outline);

    /* draw top part of the arrow */
    getColorRGBA (ArrowBaseColor);
    cairo_set_source_rgba (cr, r, g, b, a);
    cairo_move_to (cr, 15, 0);
    cairo_line_to (cr, 30, 30);
    cairo_line_to (cr, 15, 24.5);
    cairo_line_to (cr, 15, 0);
    cairo_fill (cr);

    /* draw bottom part of the arrow */
    getColorRGBA (ArrowShadowColor);
    cairo_set_source_rgba (cr, r, g, b, a);
    cairo_move_to (cr, 15, 0);
    cairo_line_to (cr, 0, 30);
    cairo_line_to (cr, 15, 24.5);
    cairo_line_to (cr, 15, 0);
    cairo_fill (cr);

    /* draw the arrow outline */
    getColorRGBA (OutlineColor);
    cairo_set_source_rgba (cr, r, g, b, a);
    cairo_move_to (cr, 15, 0);
    cairo_line_to (cr, 30, 30);
    cairo_line_to (cr, 15, 24.5);
    cairo_line_to (cr, 0, 30);
    cairo_line_to (cr, 15, 0);
    cairo_stroke (cr);

    cairo_restore (cr);
}

void
WallScreen::setupCairoContext (WallCairoContext &context)
{
    XRenderPictFormat *format;
    Screen            *xScreen;
    int               width, height;

    xScreen = ScreenOfDisplay (screen->dpy (), screen->screenNum ());

    width = context.width;
    height = context.height;

    format = XRenderFindStandardFormat (screen->dpy (), PictStandardARGB32);

    context.pixmap = XCreatePixmap (screen->dpy (), screen->root (),
				    width, height, 32);

    context.texture = GLTexture::bindPixmapToTexture (context.pixmap,
						      width, height, 32);
    if (context.texture.empty ())
    {
	screen->logMessage ("wall", CompLogLevelError,
			    "Couldn't create cairo context for switcher");
    }

    context.surface =
	cairo_xlib_surface_create_with_xrender_format (screen->dpy (),
						       context.pixmap,
						       xScreen, format,
						       width, height);

    context.cr = cairo_create (context.surface);
    clearCairoLayer (context.cr);
}

void
WallScreen::destroyCairoContext (WallCairoContext &context)
{
    if (context.cr)
	cairo_destroy (context.cr);

    if (context.surface)
	cairo_surface_destroy (context.surface);

    context.texture.clear ();

    if (context.pixmap)
	XFreePixmap (screen->dpy (), context.pixmap);
}

bool
WallScreen::checkDestination (unsigned int destX,
			      unsigned int destY)
{
    CompPoint point;
    CompSize  size;

    point = screen->vp ();
    size = screen->vpSize ();

    if (point.x () - destX >= (unsigned int) size.width ())
	return false;

    if (point.y () - destY >= (unsigned int) size.height ())
	return false;

    return true;
}

void
WallScreen::releaseMoveWindow ()
{
    CompWindow *window;

    window = screen->findWindow (moveWindow);
    if (window)
	window->syncPosition ();

    moveWindow = 0;
}

void
WallScreen::computeTranslation (float &x,
				float &y)
{
    float elapsed, duration;

    duration = optionGetSlideDuration () * 1000.0;
    if (duration != 0.0)
	elapsed = 1.0 - (timer / duration);
    else
	elapsed = 1.0;

    if (elapsed < 0.0)
	elapsed = 0.0;
    if (elapsed > 1.0)
	elapsed = 1.0;

    /* Use temporary variables to you can pass in &ps->cur_x */
    x = (gotoX - curPosX) * elapsed + curPosX;
    y = (gotoY - curPosY) * elapsed + curPosY;
}

/* movement remainder that gets ignored for direction calculation */
static const float IGNORE_REMAINDER = 0.05f;

void
WallScreen::determineMovementAngle ()
{
    int   angle;
    float dx, dy;

    dx = gotoX - curPosX;
    dy = gotoY - curPosY;

    if (dy > IGNORE_REMAINDER)
	angle = (dx > IGNORE_REMAINDER) ? 135 :
		(dx < -IGNORE_REMAINDER) ? 225 : 180;
    else if (dy < -IGNORE_REMAINDER)
	angle = (dx > IGNORE_REMAINDER) ? 45 :
		(dx < -IGNORE_REMAINDER) ? 315 : 0;
    else
	angle = (dx > IGNORE_REMAINDER) ? 90 :
		(dx < -IGNORE_REMAINDER) ? 270 : -1;

    direction = angle;
}

bool
WallScreen::moveViewport (int    x,
			  int    y,
			  Window moveWin)
{
    CompOption::Vector o(0);

    if (!x && !y)
	return false;

    if (screen->otherGrabExist ("move", "switcher", "group-drag", "wall", 0))
	return false;

    if (!checkDestination (x, y))
	return false;

    if (moveWindow != moveWin)
    {
	CompWindow *w;

	releaseMoveWindow ();
	w = screen->findWindow (moveWin);
	if (w)
	{
	    if (!(w->type () & (CompWindowTypeDesktopMask |
				CompWindowTypeDockMask)))
	    {
		if (!(w->state () & CompWindowStateStickyMask))
		{
		    moveWindow = w->id ();
		    moveWindowX = w->x ();
		    moveWindowY = w->y ();
		    w->raise ();
		}
	    }
	}
    }

    if (!moving)
    {
	curPosX = screen->vp ().x ();
	curPosY = screen->vp ().y ();
    }
    gotoX = screen->vp ().x () - x;
    gotoY = screen->vp ().y () - y;

    determineMovementAngle ();

    screen->handleCompizEvent ("wall", "start_viewport_switch", o);

    if (!grabIndex)
	grabIndex = screen->pushGrab (screen->invisibleCursor (), "wall");

    screen->moveViewport (x, y, true);

    moving          = true;
    focusDefault    = true;
    boxOutputDevice = screen->outputDeviceForPoint (pointerX, pointerY);

    if (optionGetShowSwitcher ())
	boxTimeout = optionGetPreviewTimeout () * 1000;
    else
	boxTimeout = 0;

    timer = optionGetSlideDuration () * 1000;

    cScreen->damageScreen ();

    return true;
}

void
WallScreen::handleEvent (XEvent *event)
{
    switch (event->type) {
    case ClientMessage:
	if (event->xclient.message_type == Atoms::desktopViewport)
	{
	    int dx, dy;

	    if (screen->otherGrabExist ("switcher", "wall", 0))
		break;

	    dx  = event->xclient.data.l[0] / screen->width();
	    dx -= screen->vp ().x ();
	    dy  = event->xclient.data.l[1] / screen->height();
	    dy -= screen->vp ().y ();

	    if (!dx && !dy)
		break;

	    moveViewport (-dx, -dy, None);
	}
	if (event->xclient.message_type == Atoms::xdndEnter)
	{
	    toggleEdges (true);
	    edgeDrag = true;
	}
	else if (event->xclient.message_type == Atoms::xdndLeave)
	    edgeDrag = false;

	break;

	case FocusIn:
	case FocusOut:
	    if (event->xfocus.mode == NotifyGrab)
		poller.start ();
	    else if (event->xfocus.mode == NotifyUngrab)
		poller.stop ();
	break;

	case ConfigureNotify:

	     if (event->xconfigure.window == screen->root ())
		updateScreenEdgeRegions ();

	break;
    }

    screen->handleEvent (event);
}

/*
 * Borrowed this from PrivateScreen::updateScreenEdges
 *
 */

#define SCREEN_EDGE_NUM		8

void
WallScreen::updateScreenEdgeRegions ()
{
    edgeRegion = CompRegion (0, 0, screen->width (), screen->height ());
    noEdgeRegion = CompRegion (0, 0, screen->width (), screen->height ());

    struct screenEdgeGeometry {
	int xw, x0;
	int yh, y0;
	int ww, w0;
	int hh, h0;
    } geometry[SCREEN_EDGE_NUM] = {
	{ 0,  0,   0,  2,   0,  2,   1, -4 }, /* left */
	{ 1, -2,   0,  2,   0,  2,   1, -4 }, /* right */
	{ 0,  2,   0,  0,   1, -4,   0,  2 }, /* top */
	{ 0,  2,   1, -2,   1, -4,   0,  2 }, /* bottom */
	{ 0,  0,   0,  0,   0,  2,   0,  2 }, /* top-left */
	{ 1, -2,   0,  0,   0,  2,   0,  2 }, /* top-right */
	{ 0,  0,   1, -2,   0,  2,   0,  2 }, /* bottom-left */
	{ 1, -2,   1, -2,   0,  2,   0,  2 }  /* bottom-right */
    };

    for (unsigned int i = 0; i < SCREEN_EDGE_NUM; i++)
    {
	CompRegion edge (geometry[i].xw * screen->width () +
			 geometry[i].x0,
			 geometry[i].yh * screen->height () +
			 geometry[i].y0,
			 geometry[i].ww * screen->width () +
			 geometry[i].w0,
			 geometry[i].hh * screen->height () +
			 geometry[i].h0);

	noEdgeRegion -= edgeRegion;
    }

    edgeRegion -= noEdgeRegion;
}

#undef SCREEN_EDGE_NUM

void
WallScreen::positionUpdate (const CompPoint &pos)
{
    if (edgeDrag)
	return;

    if (edgeRegion.contains (pos))
	toggleEdges (false);
    else if (noEdgeRegion.contains (pos))
    {
	if (!screen->grabbed ())
	    poller.stop ();
	toggleEdges (true);
    }
}

void
WallWindow::activate ()
{
    WALL_SCREEN (screen);

    if (window->placed () && !screen->otherGrabExist ("wall", "switcher", 0))
    {
	int       dx, dy;
	CompPoint viewport;

	screen->viewportForGeometry (window->geometry (), viewport);
	dx       = viewport.x ();
	dy       = viewport.y ();

	dx -= screen->vp ().x ();
	dy -= screen->vp ().y ();

	if (dx || dy)
	{
	    XWindowChanges xwc;
	    unsigned int   mask = 0;

	    /* If changing viewports fails we should not
	     * move the client window */
	    if (!ws->moveViewport (-dx, -dy, false))
	    {
		window->activate ();
		return;
	    }

	    ws->focusDefault = false;

	    CompRegion screenRegion;

	    foreach (const CompOutput &o, screen->outputDevs ())
		screenRegion += o.workArea ();

	    CompPoint d = compiz::wall::movementWindowOnScreen (window->serverBorderRect (),
								screenRegion);

	    mask |= d.x () !=0 ? CWX : 0;
	    mask |= d.y () !=0 ? CWY : 0;

	    xwc.x = window->serverGeometry ().x () + dx;
	    xwc.y = window->serverGeometry ().y () + dy;

	    window->configureXWindow (mask, &xwc);
	}
    }

    window->activate ();
}

void
WallWindow::grabNotify (int          x,
			int          y,
			unsigned int width,
			unsigned int height)
{
    WallScreen::get (screen)->toggleEdges (true);
    WallScreen::get (screen)->edgeDrag = true;

    window->grabNotify (x, y, width, height);
}

void
WallWindow::ungrabNotify ()
{
    WallScreen::get (screen)->edgeDrag = false;

    window->ungrabNotify ();
}

void
WallScreen::checkAmount (int 	      dx,
			 int 	      dy,
			 int          &amountX,
			 int          &amountY)
{
    CompPoint point;
    CompSize  size;

    point = screen->vp ();
    size = screen->vpSize ();

    amountX = -dx;
    amountY = -dy;

    if (optionGetAllowWraparound ())
    {
	if ((point.x () + dx) < 0)
	    amountX = -(size.width () + dx);
	else if ((point.x () + dx) >= size.width ())
	    amountX = size.width () - dx;

	if ((point.y () + dy) < 0)
	    amountY = -(size.height () + dy);
	else if ((point.y () + dy) >= size.height ())
	    amountY = size.height () - dy;
    }
}

bool
WallScreen::initiate (CompAction         *action,
		      CompAction::State  state,
		      CompOption::Vector &options,
		      Direction          dir,
		      bool               withWin)
{
    int          dx = 0, dy = 0, amountX, amountY;
    unsigned int vpX, vpY;
    CompSize     size;
    Window       win = None;

    vpX  = screen->vp ().x ();
    vpY  = screen->vp ().y ();
    size = screen->vpSize ();

    switch (dir) {
	case Up:
	    dy = -1;
	    checkAmount (dx, dy, amountX, amountY);
	    break;
	case Down:
	    dy = 1;
	    checkAmount (dx, dy, amountX, amountY);
	    break;
	case Left:
	    dx = -1;
	    checkAmount (dx, dy, amountX, amountY);
	    break;
	case Right:
	    dx = 1;
	    checkAmount (dx, dy, amountX, amountY);
	    break;
	case Next:
	    if ((vpX == (unsigned int) size.width () - 1) &&
	    	(vpY == (unsigned int) size.height () - 1))
	    {
		amountX = size.width () - 1;
		amountY = size.height () - 1;
	    }
	    else if (vpX == (unsigned int) size.width () - 1)
	    {
		amountX = size.width () - 1;
		amountY = -1;
	    }
	    else
	    {
		amountX = -1;
		amountY = 0;
	    }

	    break;
	case Prev:
	    if (vpX == 0 && vpY == 0)
	    {
		amountX = -(size.width () - 1);
		amountY = -(size.height () - 1);
	    }
	    else if (vpX == 0)
	    {
		amountX = -(size.width () - 1);
		amountY = 1;
	    }
	    else
	    {
		amountX = 1;
		amountY = 0;
	    }
	    break;
    }

    if (withWin)
	win = CompOption::getIntOptionNamed (options, "window", 0);

    if (!moveViewport (amountX, amountY, win))
	return true;

    if (state & CompAction::StateInitKey)
	action->setState (action->state () | CompAction::StateTermKey);

    if (state & CompAction::StateInitButton)
	action->setState (action->state () | CompAction::StateTermButton);

    showPreview = optionGetShowSwitcher ();

    return true;
}

bool
WallScreen::terminate (CompAction         *action,
		       CompAction::State   state,
		       CompOption::Vector &options)
{
    if (showPreview)
    {
	showPreview = false;
	cScreen->damageScreen ();
    }

    if (action)
	action->setState (action->state () & ~(CompAction::StateTermKey |
					       CompAction::StateTermButton));

    return false;
}

bool
WallScreen::initiateFlip (Direction         direction,
			  CompAction::State state)
{
    int dx, dy;
    int amountX, amountY;

    if (screen->otherGrabExist ("wall", "move", "group-drag", 0))
	return false;

    if (state & CompAction::StateInitEdgeDnd)
    {
	if (!optionGetEdgeflipDnd ())
	    return false;

	if (screen->otherGrabExist ("wall", 0))
	    return false;
    }
    else if (screen->grabExist ("move"))
    {
	if (!optionGetEdgeflipMove ())
	    return false;
    }
    else if (screen->grabExist ("group-drag"))
    {
	if (!optionGetEdgeflipDnd ())
	    return false;
    }
    else if (!optionGetEdgeflipPointer ())
    {
	toggleEdges (false);
	poller.start ();
	return false;
    }

    switch (direction) {
    case Left:
	dx = -1; dy = 0;
	break;
    case Right:
	dx = 1; dy = 0;
	break;
    case Up:
	dx = 0; dy = -1;
	break;
    case Down:
	dx = 0; dy = 1;
	break;
    default:
	dx = 0; dy = 0;
	break;
    }

    checkAmount (dx, dy, amountX, amountY);
    if (moveViewport (amountX, amountY, None))
    {
	int offsetX, offsetY;
	int warpX, warpY;

	if (dx < 0)
	{
	    offsetX = screen->width () - 10;
	    warpX = pointerX + screen->width ();
	}
	else if (dx > 0)
	{
	    offsetX = 1- screen->width ();
	    warpX = pointerX - screen->width ();
	}
	else
	{
	    offsetX = 0;
	    warpX = lastPointerX;
	}

	if (dy < 0)
	{
	    offsetY = screen->height () - 10;
	    warpY = pointerY + screen->height ();
	}
	else if (dy > 0)
	{
	    offsetY = 1- screen->height ();
	    warpY = pointerY - screen->height ();
	}
	else
	{
	    offsetY = 0;
	    warpY = lastPointerY;
	}

	screen->warpPointer (offsetX, offsetY);
	lastPointerX = warpX;
	lastPointerY = warpY;
    }

    return true;
}

inline void
wallDrawQuad (const GLMatrix    &transform,
              GLTexture::Matrix *matrix,
	      BOX               *box)
{
    GLfloat textureData[8];
    GLfloat vertexData[12];
    GLVertexBuffer *streamingBuffer = GLVertexBuffer::streamingBuffer ();

    streamingBuffer->begin (GL_TRIANGLE_STRIP);

    textureData[0] = COMP_TEX_COORD_X (*matrix, box->x1);
    textureData[1] = COMP_TEX_COORD_Y (*matrix, box->y2);
    textureData[2] = COMP_TEX_COORD_X (*matrix, box->x2);
    textureData[3] = COMP_TEX_COORD_Y (*matrix, box->y2);
    textureData[4] = COMP_TEX_COORD_X (*matrix, box->x1);
    textureData[5] = COMP_TEX_COORD_Y (*matrix, box->y1);
    textureData[6] = COMP_TEX_COORD_X (*matrix, box->x2);
    textureData[7] = COMP_TEX_COORD_Y (*matrix, box->y1);

    vertexData[0]  = box->x1;
    vertexData[1]  = box->y2;
    vertexData[2]  = 0;
    vertexData[3]  = box->x2;
    vertexData[4]  = box->y2;
    vertexData[5]  = 0;
    vertexData[6]  = box->x1;
    vertexData[7]  = box->y1;
    vertexData[8]  = 0;
    vertexData[9]  = box->x2;
    vertexData[10] = box->y1;
    vertexData[11] = 0;

    streamingBuffer->addTexCoords (0, 4, textureData);
    streamingBuffer->addVertices (4, vertexData);

    streamingBuffer->end ();
    streamingBuffer->render (transform);
}

void
WallScreen::drawCairoTextureOnScreen (const GLMatrix &transform)
{
    float             centerX, centerY;
    float             width, height;
    float             topLeftX, topLeftY;
    float             border;
    unsigned int      i, j;
    GLTexture::Matrix matrix;
    BOX               box;
    GLMatrix          wTransform (transform);
    GLVertexBuffer    *gl = GLVertexBuffer::streamingBuffer ();

    CompOutput::vector &outputDevs = screen->outputDevs ();
    CompOutput         output = outputDevs[boxOutputDevice];

    glEnable (GL_BLEND);

    centerX = output.x1 () + (output.width () / 2.0f);
    centerY = output.y1 () + (output.height () / 2.0f);

    border = (float) viewportBorder;
    width  = (float) switcherContext.width;
    height = (float) switcherContext.height;

    topLeftX = centerX - floor (width / 2.0f);
    topLeftY = centerY - floor (height / 2.0f);

    firstViewportX = topLeftX + border;
    firstViewportY = topLeftY + border;

    if (!moving)
    {
	double left, timeout;

	timeout = optionGetPreviewTimeout () * 1000.0f;
	left    = (timeout > 0) ? (float) boxTimeout / timeout : 1.0f;

	if (left < 0)
	    left = 0.0f;
	else if (left > 0.5)
	    left = 1.0f;
	else
	    left = 2 * left;

#ifndef USE_GLES
	glScreen->setTexEnvMode (GL_MODULATE);
#endif
	gl->color4f (left, left, left, left);
	wTransform.translate (0.0f, 0.0f, -(1 - left));

	mSzCamera = -(1 - left);
    }
    else
    {
	mSzCamera = 0.0f;
    }

    /* draw background */

    matrix = switcherContext.texture[0]->matrix ();
    matrix.x0 -= topLeftX * matrix.xx;
    matrix.y0 -= topLeftY * matrix.yy;

    box.x1 = topLeftX;
    box.x2 = box.x1 + width;
    box.y1 = topLeftY;
    box.y2 = box.y1 + height;

    switcherContext.texture[0]->enable (GLTexture::Fast);
    wallDrawQuad (wTransform, &matrix, &box);
    switcherContext.texture[0]->disable ();

    /* draw thumb */
    width = (float) thumbContext.width;
    height = (float) thumbContext.height;

    thumbContext.texture[0]->enable (GLTexture::Fast);
    for (i = 0; i < (unsigned int) screen->vpSize ().width (); i++)
    {
	for (j = 0; j < (unsigned int) screen->vpSize ().height (); j++)
	{
	    if (i == gotoX && j == gotoY && moving)
		continue;

	    box.x1 = i * (width + border);
	    box.x1 += topLeftX + border;
	    box.x2 = box.x1 + width;
	    box.y1 = j * (height + border);
	    box.y1 += topLeftY + border;
	    box.y2 = box.y1 + height;

	    matrix = thumbContext.texture[0]->matrix ();
	    matrix.x0 -= box.x1 * matrix.xx;
	    matrix.y0 -= box.y1 * matrix.yy;

	    wallDrawQuad (wTransform, &matrix, &box);
	}
    }
    thumbContext.texture[0]->disable ();

    if (moving || showPreview)
    {
	/* draw highlight */

	box.x1 = screen->vp ().x () * (width + border) + topLeftX + border;
	box.x2 = box.x1 + width;
	box.y1 = screen->vp ().y () * (height + border) + topLeftY + border;
	box.y2 = box.y1 + height;

	matrix = highlightContext.texture[0]->matrix ();
	matrix.x0 -= box.x1 * matrix.xx;
	matrix.y0 -= box.y1 * matrix.yy;

	highlightContext.texture[0]->enable (GLTexture::Fast);
	wallDrawQuad (wTransform, &matrix, &box);
	highlightContext.texture[0]->disable ();

	/* draw arrow */
	if (direction >= 0)
	{
	    arrowContext.texture[0]->enable (GLTexture::Fast);
	    int aW = arrowContext.width;
	    int aH = arrowContext.height;

	    /* if we have a viewport preview we just paint the
	       arrow outside the switcher */
	    if (optionGetMiniscreen ())
	    {
		width  = (float) switcherContext.width;
		height = (float) switcherContext.height;

		switch (direction)
		{
		    /* top left */
		    case 315:
			box.x1 = topLeftX - aW - border;
			box.y1 = topLeftY - aH - border;
			break;
		    /* up */
		    case 0:
			box.x1 = topLeftX + width / 2.0f - aW / 2.0f;
			box.y1 = topLeftY - aH - border;
			break;
		    /* top right */
		    case 45:
			box.x1 = topLeftX + width + border;
			box.y1 = topLeftY - aH - border;
			break;
		    /* right */
		    case 90:
			box.x1 = topLeftX + width + border;
			box.y1 = topLeftY + height / 2.0f - aH / 2.0f;
			break;
		    /* bottom right */
		    case 135:
			box.x1 = topLeftX + width + border;
			box.y1 = topLeftY + height + border;
			break;
		    /* down */
		    case 180:
			box.x1 = topLeftX + width / 2.0f - aW / 2.0f;
			box.y1 = topLeftY + height + border;
			break;
		    /* bottom left */
		    case 225:
			box.x1 = topLeftX - aW - border;
			box.y1 = topLeftY + height + border;
			break;
		    /* left */
		    case 270:
			box.x1 = topLeftX - aW - border;
			box.y1 = topLeftY + height / 2.0f - aH / 2.0f;
			break;
		    default:
			break;
		}
	    }
	    else
	    {
		/* arrow is visible (no preview is painted over it) */
		box.x1  = screen->vp().x() * (width + border) +
			  topLeftX + border;
		box.x1 += width / 2 - aW / 2;
		box.y1  = screen->vp().y() * (height + border) +
		          topLeftY + border;
		box.y1 += height / 2 - aH / 2;
	    }

	    box.x2 = box.x1 + aW;
	    box.y2 = box.y1 + aH;

	    wTransform.translate (box.x1 + aW / 2, box.y1 + aH / 2, 0.0f);
	    wTransform.rotate (direction, 0.0f, 0.0f, 1.0f);
	    wTransform.translate (-box.x1 - aW / 2, -box.y1 - aH / 2, 0.0f);

	    matrix = arrowContext.texture[0]->matrix ();
	    matrix.x0 -= box.x1 * matrix.xx;
	    matrix.y0 -= box.y1 * matrix.yy;

	    wallDrawQuad (wTransform, &matrix, &box);
	    arrowContext.texture[0]->disable ();
	}
    }

    glDisable (GL_BLEND);
#ifndef USE_GLES
    glScreen->setTexEnvMode (GL_REPLACE);
#endif
    gl->colorDefault ();
}

void
WallScreen::paint (CompOutput::ptrList& outputs,
		   unsigned int         mask)
{
    if (moving && outputs.size () > 1 && optionGetMmmode() == MmmodeSwitchAll)
    {
	outputs.clear ();
	outputs.push_back (&screen->fullscreenOutput ());
    }

    cScreen->paint (outputs, mask);
}

bool
WallScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
			   const GLMatrix            &matrix,
			   const CompRegion          &region,
			   CompOutput                *output,
			   unsigned int              mask)
{
    bool status;

    transform = NoTransformation;

    if (moving)
	mask |= PAINT_SCREEN_TRANSFORMED_MASK |
		PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK;

    status = glScreen->glPaintOutput (attrib, matrix, region, output, mask);

    if (optionGetShowSwitcher () &&
	(moving || showPreview || boxTimeout) &&
	(output->id () == boxOutputDevice ||
	 output == &screen->fullscreenOutput ()))
    {
	GLMatrix sMatrix (matrix);

	sMatrix.toScreenSpace (output, -DEFAULT_Z_CAMERA);

	drawCairoTextureOnScreen (sMatrix);

	if (optionGetMiniscreen ())
        {
	    unsigned int i, j;
	    float        mw, mh;

	    mw = viewportWidth;
	    mh = viewportHeight;

	    transform = MiniScreen;
	    mSAttribs.xScale = mw / screen->width ();
	    mSAttribs.yScale = mh / screen->height ();
	    mSAttribs.opacity = OPAQUE * (1.0 + mSzCamera);
	    mSAttribs.saturation = COLOR;

	    for (j = 0; j < (unsigned int) screen->vpSize ().height (); j++)
	    {
		for (i = 0; i < (unsigned int) screen->vpSize ().width (); i++)
		{
		    float        mx, my;
		    unsigned int msMask;
		    CompPoint    vp (i, j);

		    mx = firstViewportX +
			 (i * (viewportWidth + viewportBorder));
		    my = firstViewportY +
			 (j * (viewportHeight + viewportBorder));

		    mSAttribs.xTranslate = mx / output->width ();
		    mSAttribs.yTranslate = -my / output->height ();

		    mSAttribs.brightness = 0.4f * BRIGHT;

		    if (vp == screen->vp () &&
			(moving || boxTimeout || showPreview))
		    {
			mSAttribs.brightness = BRIGHT;
		    }

		    cScreen->setWindowPaintOffset ((screen->vp ().x () - i) *
						   screen->width (),
						   (screen->vp ().y () - j) *
						   screen->height ());

		    msMask = mask | PAINT_SCREEN_TRANSFORMED_MASK;

		    glScreen->glPaintTransformedOutput (attrib, matrix,
							region, output, msMask);

		}
	    }
	    transform = NoTransformation;
	    cScreen->setWindowPaintOffset (0, 0);
	}
    }

    return status;
}

void
WallScreen::preparePaint (int msSinceLastPaint)
{
    if (!moving && !showPreview && boxTimeout)
	boxTimeout -= msSinceLastPaint;

    if (timer)
	timer -= msSinceLastPaint;

    if (moving)
    {
	computeTranslation (curPosX, curPosY);

	if (moveWindow)
	{
	    CompWindow *window;

	    window = screen->findWindow (moveWindow);
	    if (window)
	    {
		float dx, dy;

		dx = (gotoX - curPosX) * screen->width ();
		dy = (gotoY - curPosY) * screen->height ();

		window->moveToViewportPosition (moveWindowX - dx,
						moveWindowY - dy,
						true);
	    }
	}
    }

    if (moving && curPosX == gotoX && curPosY == gotoY)
    {
	CompOption::Vector o (0);
	moving = false;
	timer  = 0;

	if (moveWindow)
	    releaseMoveWindow ();
	else if (focusDefault)
	{
	    /* only focus default window if switcher is not active */
	    if (!screen->grabExist ("switcher"))
		screen->focusDefaultWindow ();
	}

	screen->handleCompizEvent ("wall", "end_viewport_switch", o);
    }

    cScreen->preparePaint (msSinceLastPaint);
}

void
WallScreen::glPaintTransformedOutput (const GLScreenPaintAttrib &attrib,
				      const GLMatrix            &matrix,
				      const CompRegion          &region,
				      CompOutput                *output,
				      unsigned int              mask)
{
    bool clear = (mask & PAINT_SCREEN_CLEAR_MASK);

    if (transform == MiniScreen)
    {
	GLMatrix sMatrix (matrix);

	mask &= ~PAINT_SCREEN_CLEAR_MASK;

        /* move each screen to the correct output position */
	sMatrix.translate (-(float) output->x1 () / (float) output->width (),
			   (float) output->y1 () / (float) output->height (),
			   0.0f);
	sMatrix.translate (0.0f, 0.0f, -DEFAULT_Z_CAMERA);

	sMatrix.translate (mSAttribs.xTranslate,
			   mSAttribs.yTranslate,
			   mSzCamera);

	/* move origin to top left */
	sMatrix.translate (-0.5f, 0.5f, 0.0f);
	sMatrix.scale (mSAttribs.xScale, mSAttribs.yScale, 1.0);

	/* revert prepareXCoords region shift.
	   Now all screens display the same */
	sMatrix.translate (0.5f, 0.5f, DEFAULT_Z_CAMERA);
	sMatrix.translate ((float) output->x1 () / (float) output->width (),
			   -(float) output->y2 () / (float) output->height (),
			   0.0f);

	glScreen->glPaintTransformedOutput (attrib, sMatrix,
					    screen->region (), output, mask);
	return;
    }

    if (!moving)
	glScreen->glPaintTransformedOutput (attrib, matrix,
					    region, output, mask);

    mask &= ~PAINT_SCREEN_CLEAR_MASK;

    if (moving)
    {
	ScreenTransformation oldTransform = transform;
	GLMatrix             sMatrix (matrix);
	float                xTranslate, yTranslate;
	float                px, py;
	bool                 movingX, movingY;
	CompPoint            point (screen->vp ());
	CompRegion           outputRegion (*output);

	if (clear)
	    glScreen->clearTargetOutput (GL_COLOR_BUFFER_BIT);

	transform  = Sliding;
	currOutput = output;

	px = curPosX;
	py = curPosY;

	movingX = ((int) floor (px)) != ((int) ceil (px));
	movingY = ((int) floor (py)) != ((int) ceil (py));

	if (movingY)
	{
	    yTranslate = fmod (py, 1) - 1;

	    sMatrix.translate (0.0f, yTranslate, 0.0f);

	    if (movingX)
	    {
		xTranslate = 1 - fmod (px, 1);

		cScreen->setWindowPaintOffset ((point.x () - ceil (px)) *
					       screen->width (),
					       (point.y () - ceil (py)) *
					       screen->height ());

		sMatrix.translate (xTranslate, 0.0f, 0.0f);

		glScreen->glPaintTransformedOutput (attrib, sMatrix,
						    outputRegion, output, mask);

		sMatrix.translate (-xTranslate, 0.0f, 0.0f);
	    }
	    xTranslate = -fmod (px, 1);

	    cScreen->setWindowPaintOffset ((point.x () - floor (px)) *
					   screen->width (),
					   (point.y () - ceil (py)) *
					   screen->height ());

	    sMatrix.translate (xTranslate, 0.0f, 0.0f);

	    glScreen->glPaintTransformedOutput (attrib, sMatrix,
						outputRegion, output, mask);
	    sMatrix.translate (-xTranslate, -yTranslate, 0.0f);
	}

	yTranslate = fmod (py, 1);

	sMatrix.translate (0.0f, yTranslate, 0.0f);

	if (movingX)
	{
	    xTranslate = 1 - fmod (px, 1);

	    cScreen->setWindowPaintOffset ((point.x () - ceil (px)) *
					   screen->width (),
					   (point.y () - floor (py)) *
					   screen->height ());

	    sMatrix.translate (xTranslate, 0.0f, 0.0f);

	    glScreen->glPaintTransformedOutput (attrib, sMatrix,
						outputRegion, output, mask);

	    sMatrix.translate (-xTranslate, 0.0f, 0.0f);
	}

	xTranslate = -fmod (px, 1);

	cScreen->setWindowPaintOffset ((point.x () - floor (px)) *
				       screen->width (),
				       (point.y () - floor (py)) *
				       screen->height ());

	sMatrix.translate (xTranslate, 0.0f, 0.0f);
	glScreen->glPaintTransformedOutput (attrib, sMatrix, outputRegion,
					    output, mask);

	cScreen->setWindowPaintOffset (0, 0);
	transform = oldTransform;
    }
}

bool
WallWindow::glPaint (const GLWindowPaintAttrib &attrib,
		     const GLMatrix            &matrix,
		     const CompRegion          &region,
		     unsigned int              mask)
{
    bool status;

    WALL_SCREEN (screen);

    if (ws->transform == MiniScreen)
    {
	GLWindowPaintAttrib pA (attrib);

	pA.opacity    = attrib.opacity *
			((float) ws->mSAttribs.opacity / OPAQUE);
	pA.brightness = attrib.brightness *
			((float) ws->mSAttribs.brightness / BRIGHT);
	pA.saturation = attrib.saturation *
			((float) ws->mSAttribs.saturation / COLOR);

	if (!pA.opacity || !pA.brightness)
	    mask |= PAINT_WINDOW_NO_CORE_INSTANCE_MASK;

	status = glWindow->glPaint (pA, matrix, region, mask);
    }
    else if (ws->transform == Sliding && !isSliding)
    {
	GLMatrix wMatrix;

	/* Don't paint nonsliding windows multiple times */

	wMatrix.toScreenSpace (ws->currOutput, -DEFAULT_Z_CAMERA);
	status = glWindow->glPaint (attrib, wMatrix, region, mask | PAINT_WINDOW_TRANSFORMED_MASK);
    }
    else
    {
	status = glWindow->glPaint (attrib, matrix, region, mask);
    }

    return status;
}

void
WallScreen::donePaint ()
{
    if (moving || showPreview || boxTimeout)
    {
	boxTimeout = MAX (0, boxTimeout);
	cScreen->damageScreen ();
    }

    if (!moving && !showPreview && grabIndex)
    {
	screen->removeGrab (static_cast <CompScreen::GrabHandle> (grabIndex), NULL);
	grabIndex = 0;
    }

    cScreen->donePaint ();
}

void
WallScreen::createCairoContexts (bool initial)
{
    int width, height;

    viewportWidth = VIEWPORT_SWITCHER_SIZE *
		    (float) optionGetPreviewScale () / 100.0f;
    viewportHeight = viewportWidth * (float) screen->height () /
		     (float) screen->width ();
    viewportBorder = optionGetBorderWidth ();

    width  = screen->vpSize ().width () * (viewportWidth + viewportBorder) +
	     viewportBorder;
    height = screen->vpSize ().height () * (viewportHeight + viewportBorder) +
	     viewportBorder;

    destroyCairoContext (switcherContext);
    switcherContext.width = width;
    switcherContext.height = height;
    setupCairoContext (switcherContext);
    drawSwitcherBackground ();

    destroyCairoContext (thumbContext);
    thumbContext.width = viewportWidth;
    thumbContext.height = viewportHeight;
    setupCairoContext (thumbContext);
    drawThumb ();

    destroyCairoContext (highlightContext);
    highlightContext.width = viewportWidth;
    highlightContext.height = viewportHeight;
    setupCairoContext (highlightContext);
    drawHighlight ();

    if (initial)
    {
        arrowContext.width = ARROW_SIZE;
        arrowContext.height = ARROW_SIZE;
        setupCairoContext (arrowContext);
        drawArrow ();
    }
}

void
WallScreen::toggleEdges (bool enabled)
{
    WALL_SCREEN (screen);

    if (!enabled)
    {
	screen->removeAction (&ws->optionGetFlipLeftEdge ());
	screen->removeAction (&ws->optionGetFlipUpEdge ());
	screen->removeAction (&ws->optionGetFlipRightEdge ());
	screen->removeAction (&ws->optionGetFlipDownEdge ());
    }
    else
    {
	screen->addAction (&ws->optionGetFlipLeftEdge ());
	screen->addAction (&ws->optionGetFlipUpEdge ());
	screen->addAction (&ws->optionGetFlipRightEdge ());
	screen->addAction (&ws->optionGetFlipDownEdge ());
    }
}

void
WallScreen::optionChanged (CompOption           *opt,
			   WallOptions::Options num)
{
    switch(num) {
    case WallOptions::OutlineColor:
	drawSwitcherBackground ();
	drawHighlight ();
	drawThumb ();
	break;

    case WallOptions::EdgeRadius:
    case WallOptions::BackgroundGradientBaseColor:
    case WallOptions::BackgroundGradientHighlightColor:
    case WallOptions::BackgroundGradientShadowColor:
	drawSwitcherBackground ();
	break;

    case WallOptions::BorderWidth:
    case WallOptions::PreviewScale:
	createCairoContexts (false);
	break;

    case WallOptions::ThumbGradientBaseColor:
    case WallOptions::ThumbGradientHighlightColor:
	drawThumb ();
	break;

    case WallOptions::ThumbHighlightGradientBaseColor:
    case WallOptions::ThumbHighlightGradientShadowColor:
	drawHighlight ();
	break;

    case WallOptions::ArrowBaseColor:
    case WallOptions::ArrowShadowColor:
	drawArrow ();
	break;

    case WallOptions::NoSlideMatch:
	foreach (CompWindow *w, screen->windows ())
	{
	    WALL_WINDOW (w);
	    ww->isSliding = !optionGetNoSlideMatch ().evaluate (w);
	}
	break;

    default:
	break;
    }
}

bool
WallScreen::setOptionForPlugin (const char        *plugin,
				const char        *name,
				CompOption::Value &value)
{
    bool status = screen->setOptionForPlugin (plugin, name, value);

    if (strcmp (plugin, "core") == 0)
    {
        if (strcmp (name, "hsize") == 0 || strcmp (name, "vsize") == 0)
        {
            createCairoContexts (false);
        }
    }

    return status;
}

void
WallScreen::matchExpHandlerChanged ()
{
    screen->matchExpHandlerChanged ();

    foreach (CompWindow *w, screen->windows ())
    {
	WALL_WINDOW (w);
	ww->isSliding = !optionGetNoSlideMatch ().evaluate (w);
    }
}

void
WallScreen::matchPropertyChanged (CompWindow *window)
{
    WALL_WINDOW (window);

    screen->matchPropertyChanged (window);

    ww->isSliding = !optionGetNoSlideMatch ().evaluate (window);
}

WallScreen::WallScreen (CompScreen *screen) :
    PluginClassHandler <WallScreen, CompScreen> (screen),
    WallOptions (),
    cScreen (CompositeScreen::get (screen)),
    glScreen (GLScreen::get (screen)),
    moving (false),
    showPreview (false),
    direction (-1),
    boxTimeout (0),
    grabIndex (0),
    timer (0),
    moveWindow (None),
    focusDefault (true),
    transform (NoTransformation),
    edgeDrag (false)
{
    ScreenInterface::setHandler (screen);
    CompositeScreenInterface::setHandler (cScreen);
    GLScreenInterface::setHandler (glScreen);

    // HACK: we have to keep libcairo loaded even if wall gets unloaded
    // to prevent crashes in XCloseDisplay
    dlopen ("libcairo.so.2", RTLD_LAZY);

    memset (&switcherContext, 0, sizeof (WallCairoContext));
    memset (&thumbContext, 0, sizeof (WallCairoContext));
    memset (&highlightContext, 0, sizeof (WallCairoContext));
    memset (&arrowContext, 0, sizeof (WallCairoContext));
    createCairoContexts (true);

#define setAction(action, dir, win) \
    optionSet##action##Initiate (boost::bind (&WallScreen::initiate, this,   \
					      _1, _2, _3, dir, win));        \
    optionSet##action##Terminate (boost::bind (&WallScreen::terminate, this, \
					       _1, _2, _3))

#define setFlipAction(action, dir) \
    optionSet##action##Initiate (boost::bind (&WallScreen::initiateFlip, \
					      this, dir, _2))

    setAction (LeftKey, Left, false);
    setAction (RightKey, Right, false);
    setAction (UpKey, Up, false);
    setAction (DownKey, Down, false);
    setAction (NextKey, Next, false);
    setAction (PrevKey, Prev, false);
    setAction (LeftButton, Left, false);
    setAction (RightButton, Right, false);
    setAction (UpButton, Up, false);
    setAction (DownButton, Down, false);
    setAction (NextButton, Next, false);
    setAction (PrevButton, Prev, false);
    setAction (LeftWindowKey, Left, true);
    setAction (RightWindowKey, Right, true);
    setAction (UpWindowKey, Up, true);
    setAction (DownWindowKey, Down, true);

    setFlipAction (FlipLeftEdge, Left);
    setFlipAction (FlipRightEdge, Right);
    setFlipAction (FlipUpEdge, Up);
    setFlipAction (FlipDownEdge, Down);

#define setNotify(func) \
    optionSet##func##Notify (boost::bind (&WallScreen::optionChanged, \
					  this, _1, _2))

    setNotify (EdgeRadius);
    setNotify (BorderWidth);
    setNotify (PreviewScale);
    setNotify (OutlineColor);
    setNotify (BackgroundGradientBaseColor);
    setNotify (BackgroundGradientHighlightColor);
    setNotify (BackgroundGradientShadowColor);
    setNotify (ThumbGradientBaseColor);
    setNotify (ThumbGradientHighlightColor);
    setNotify (ThumbHighlightGradientBaseColor);
    setNotify (ThumbHighlightGradientShadowColor);
    setNotify (ArrowBaseColor);
    setNotify (ArrowShadowColor);
    setNotify (NoSlideMatch);
    setNotify (EdgeflipPointer);

    updateScreenEdgeRegions ();

    poller.setCallback (boost::bind (&WallScreen::positionUpdate, this,
				     _1));
}

WallScreen::~WallScreen ()
{
    destroyCairoContext (switcherContext);
    destroyCairoContext (thumbContext);
    destroyCairoContext (highlightContext);
    destroyCairoContext (arrowContext);
}

WallWindow::WallWindow (CompWindow *window) :
    PluginClassHandler <WallWindow, CompWindow> (window),
    window (window),
    glWindow (GLWindow::get (window))
{
    WALL_SCREEN (screen);

    isSliding = !ws->optionGetNoSlideMatch ().evaluate (window);

    GLWindowInterface::setHandler (glWindow);
    WindowInterface::setHandler (window);
}

bool
WallPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
        return false;
    if (!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI))
	return false;
    if (!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;
    if (!CompPlugin::checkPluginABI ("mousepoll", COMPIZ_MOUSEPOLL_ABI))
	return false;

    return true;
}

