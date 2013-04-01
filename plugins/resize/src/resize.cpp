/*
 * Copyright © 2005 Novell, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include <core/atoms.h>
#include "resize.h"

#include "window-impl.h"
#include "property-writer-impl.h"
#include "resize-window-impl.h"
#include "screen-impl.h"
#include "gl-screen-impl.h"
#include "composite-screen-impl.h"

COMPIZ_PLUGIN_20090315 (resize, ResizePluginVTable)

void ResizeScreen::handleEvent (XEvent *event)
{
    logic.handleEvent(event);
}

void
ResizeWindow::getStretchScale (BoxPtr pBox, float *xScale, float *yScale)
{
    CompRect rect (window->borderRect ());

    *xScale = (rect.width ())  ? (pBox->x2 - pBox->x1) /
				 (float) rect.width () : 1.0f;
    *yScale = (rect.height ()) ? (pBox->y2 - pBox->y1) /
				 (float) rect.height () : 1.0f;
}

static bool
resizeInitiateDefaultMode (CompAction	      *action,
			   CompAction::State  state,
			   CompOption::Vector &options)
{
    RESIZE_SCREEN (screen);
    return rs->logic.initiateResizeDefaultMode(action, state, options);
}

static bool
resizeTerminate (CompAction         *action,
	         CompAction::State  state,
	         CompOption::Vector &options)
{
    RESIZE_SCREEN (screen);
    return rs->logic.terminateResize(action, state, options);
}

void
ResizeScreen::glPaintRectangle (const GLScreenPaintAttrib &sAttrib,
				const GLMatrix            &transform,
				CompOutput                *output,
				unsigned short            *borderColor,
				unsigned short            *fillColor)
{
    GLVertexBuffer *streamingBuffer = GLVertexBuffer::streamingBuffer ();

    BoxRec   	   box;
    GLMatrix 	   sTransform (transform);
    GLfloat         vertexData [12];
    GLfloat         vertexData2[24];
    GLint    	   origSrc, origDst;
    GLushort	    fc[4], bc[4];

#ifdef USE_GLES
    GLint           origSrcAlpha, origDstAlpha;
    glGetIntegerv (GL_BLEND_SRC_RGB, &origSrc);
    glGetIntegerv (GL_BLEND_DST_RGB, &origDst);
    glGetIntegerv (GL_BLEND_SRC_ALPHA, &origSrcAlpha);
    glGetIntegerv (GL_BLEND_DST_ALPHA, &origDstAlpha);
#else
    glGetIntegerv (GL_BLEND_SRC, &origSrc);
    glGetIntegerv (GL_BLEND_DST, &origDst);
#endif

    /* Premultiply the alpha values */
    bc[3] = (float) borderColor[3] / (float) 65535.0f;
    bc[0] = ((float) borderColor[0] / 65535.0f) * bc[3];
    bc[1] = ((float) borderColor[1] / 65535.0f) * bc[3];
    bc[2] = ((float) borderColor[2] / 65535.0f) * bc[3];

    logic.getPaintRectangle (&box);

    vertexData[0]  = box.x1;
    vertexData[1]  = box.y1;
    vertexData[2]  = 0.0f;
    vertexData[3]  = box.x1;
    vertexData[4]  = box.y2;
    vertexData[5]  = 0.0f;
    vertexData[6]  = box.x2;
    vertexData[7]  = box.y1;
    vertexData[8]  = 0.0f;
    vertexData[9]  = box.x2;
    vertexData[10] = box.y2;
    vertexData[11] = 0.0f;

    // FIXME: this is a quick work-around.
    // GL_LINE_LOOP and GL_LINE_STRIP primitive types in the SGX Pvr X11 driver
    // take special number of vertices (and reorder them). Thus, usage of
    // those line primitive is currently not supported by our GLVertexBuffer
    // implementation. This is a quick workaround to make it all work until
    // we come up with a better GLVertexBuffer::render(...) function.

    vertexData2[0]  = box.x1;
    vertexData2[1]  = box.y1;
    vertexData2[2]  = 0.0f;
    vertexData2[3]  = box.x1;
    vertexData2[4]  = box.y2;
    vertexData2[5]  = 0.0f;
    vertexData2[6]  = box.x1;
    vertexData2[7]  = box.y2;
    vertexData2[8]  = 0.0f;
    vertexData2[9]  = box.x2;
    vertexData2[10] = box.y2;
    vertexData2[11] = 0.0f;
    vertexData2[12] = box.x2;
    vertexData2[13] = box.y2;
    vertexData2[14] = 0.0f;
    vertexData2[15] = box.x2;
    vertexData2[16] = box.y1;
    vertexData2[17] = 0.0f;
    vertexData2[18] = box.x2;
    vertexData2[19] = box.y1;
    vertexData2[20] = 0.0f;
    vertexData2[21] = box.x1;
    vertexData2[22] = box.y1;
    vertexData2[23] = 0.0f;

    sTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);

    glEnable (GL_BLEND);
    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    /* fill rectangle */
    if (fillColor)
    {
	fc[3] = fillColor[3];
	fc[0] = fillColor[0] * (unsigned long)fc[3] / 65535;
	fc[1] = fillColor[1] * (unsigned long)fc[3] / 65535;
	fc[2] = fillColor[2] * (unsigned long)fc[3] / 65535;

	streamingBuffer->begin (GL_TRIANGLE_STRIP);
	streamingBuffer->addColors (1, fc);
	streamingBuffer->addVertices (4, &vertexData[0]);
	streamingBuffer->end ();
	streamingBuffer->render (sTransform);
    }

    /* draw outline */
    static const int borderWidth = 2;
    glLineWidth (borderWidth);
    streamingBuffer->begin (GL_LINES);
    streamingBuffer->addColors (1, borderColor);
    streamingBuffer->addVertices (8, &vertexData2[0]);
    streamingBuffer->end ();
    streamingBuffer->render (sTransform);

    glDisable (GL_BLEND);
#ifdef USE_GLES
    glBlendFuncSeparate (origSrc, origDst,
                         origSrcAlpha, origDstAlpha);
#else
    glBlendFunc (origSrc, origDst);
#endif

    CompositeScreen *cScreen = CompositeScreen::get (screen);
    CompRect damage (box.x1 - borderWidth,
                     box.y1 - borderWidth,
                     box.x2 - box.x1 + 2 * borderWidth,
                     box.y2 - box.y1 + 2 * borderWidth);
    cScreen->damageRegion (damage);
}

bool
ResizeScreen::glPaintOutput (const GLScreenPaintAttrib &sAttrib,
			     const GLMatrix            &transform,
			     const CompRegion          &region,
			     CompOutput                *output,
			     unsigned int              mask)
{
    bool status;

    if (logic.w)
    {
	if (logic.mode == ResizeOptions::ModeStretch)
	    mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK;
    }

    status = gScreen->glPaintOutput (sAttrib, transform, region, output, mask);

    if (status && logic.w)
    {
	unsigned short *border, *fill;

	border = optionGetBorderColor ();
	fill   = optionGetFillColor ();

	switch (logic.mode) {
	    case ResizeOptions::ModeOutline:
		glPaintRectangle (sAttrib, transform, output, border, NULL);
		break;
	    case ResizeOptions::ModeRectangle:
		glPaintRectangle (sAttrib, transform, output, border, fill);
	    default:
		break;
	}
    }

    return status;
}

bool
ResizeWindow::glPaint (const GLWindowPaintAttrib &attrib,
		       const GLMatrix            &transform,
		       const CompRegion          &region,
		       unsigned int              mask)
{
    bool       status;

    if (window == static_cast<resize::CompWindowImpl*>(rScreen->logic.w)->impl ()
	&& rScreen->logic.mode == ResizeOptions::ModeStretch)
    {
	GLMatrix       wTransform (transform);
	BoxRec	       box;
	float	       xOrigin, yOrigin;
	float	       xScale, yScale;
	int            x, y;

	if (mask & PAINT_WINDOW_OCCLUSION_DETECTION_MASK)
	    return false;

	status = gWindow->glPaint (attrib, transform, region,
				   mask | PAINT_WINDOW_NO_CORE_INSTANCE_MASK);

	GLWindowPaintAttrib lastAttrib (gWindow->lastPaintAttrib ());

	if (window->alpha () || lastAttrib.opacity != OPAQUE)
	    mask |= PAINT_WINDOW_TRANSLUCENT_MASK;

	rScreen->logic.getPaintRectangle (&box);
	getStretchScale (&box, &xScale, &yScale);

	x = window->geometry (). x ();
	y = window->geometry (). y ();

	xOrigin = x - window->border ().left;
	yOrigin = y - window->border ().top;

	wTransform.translate (xOrigin, yOrigin, 0.0f);
	wTransform.scale (xScale, yScale, 1.0f);
	wTransform.translate ((rScreen->logic.geometry.x - x) / xScale - xOrigin,
			      (rScreen->logic.geometry.y - y) / yScale - yOrigin,
			      0.0f);

	gWindow->glDraw (wTransform, lastAttrib, region,
			 mask | PAINT_WINDOW_TRANSFORMED_MASK);
    }
    else
    {
	status = gWindow->glPaint (attrib, transform, region, mask);
    }

    return status;
}

bool
ResizeWindow::damageRect (bool initial, const CompRect &rect)
{
    bool status = false;

    if (window == static_cast<resize::CompWindowImpl*>(rScreen->logic.w)->impl ()
	&& rScreen->logic.mode == ResizeOptions::ModeStretch)
    {
	BoxRec box;

	rScreen->logic.getStretchRectangle (&box);
	rScreen->logic.damageRectangle (&box);

	status = true;
    }

    status |= cWindow->damageRect (initial, rect);

    return status;
}

/* We have to make some assumptions here in order to do this neatly,
 * see build/generated/resize_options.h for more info */

#define ResizeModeShiftMask (1 << 0)
#define ResizeModeAltMask (1 << 1)
#define ResizeModeControlMask (1 << 2)
#define ResizeModeMetaMask (1 << 3)

void
ResizeScreen::resizeMaskValueToKeyMask (int	   valueMask,
					int	   *mask)
{
    if (valueMask & ResizeModeShiftMask)
	*mask |= ShiftMask;
    if (valueMask & ResizeModeAltMask)
	*mask |= CompAltMask;
    if (valueMask & ResizeModeControlMask)
	*mask |= ControlMask;
    if (valueMask & ResizeModeMetaMask)
	*mask |= CompMetaMask;
}

void
ResizeScreen::optionChanged (CompOption		    *option,
			     ResizeOptions::Options num)
{
    int *mask = NULL;
    int valueMask = 0;

    switch (num)
    {
	case ResizeOptions::OutlineModifier:
	    mask = &logic.outlineMask;
	    valueMask = optionGetOutlineModifierMask ();
	    break;
	case ResizeOptions::RectangleModifier:
	    mask = &logic.rectangleMask;
	    valueMask = optionGetRectangleModifierMask ();
	    break;
	case ResizeOptions::StretchModifier:
	    mask = &logic.stretchMask;
	    valueMask = optionGetStretchModifierMask ();
	    break;
        case ResizeOptions::CenteredModifier:
	    mask = &logic.centeredMask;
	    valueMask = optionGetCenteredModifierMask ();
	    break;
	default:
	    break;
    }

    if (mask)
	resizeMaskValueToKeyMask (valueMask, mask);
}

ResizeScreen::ResizeScreen (CompScreen *s) :
    PluginClassHandler<ResizeScreen,CompScreen> (s),
    gScreen (GLScreen::get (s))
{
    logic.mScreen = new resize::CompScreenImpl (screen);
    logic.cScreen = resize::CompositeScreenImpl::wrap (CompositeScreen::get (s));
    logic.gScreen = resize::GLScreenImpl::wrap (gScreen);
    logic.options = this;

    CompOption::Vector atomTemplate;
    Display *dpy = s->dpy ();
    ResizeOptions::ChangeNotify notify =
	       boost::bind (&ResizeScreen::optionChanged, this, _1, _2);

    atomTemplate.resize (4);

    for (int i = 0; i < 4; i++)
    {
	char buf[4];
	snprintf (buf, 4, "%i", i);
	CompString tmpName (buf);

	atomTemplate.at (i).setName (tmpName, CompOption::TypeInt);
    }

    logic.resizeNotifyAtom = XInternAtom (s->dpy (), "_COMPIZ_RESIZE_NOTIFY", 0);

    logic.resizeInformationAtom = new resize::PropertyWriterImpl (
	    new PropertyWriter ("_COMPIZ_RESIZE_INFORMATION",
				atomTemplate));

    for (unsigned int i = 0; i < NUM_KEYS; i++)
	logic.key[i] = XKeysymToKeycode (s->dpy (), XStringToKeysym (logic.rKeys[i].name));

    logic.leftCursor      = XCreateFontCursor (dpy, XC_left_side);
    logic.rightCursor     = XCreateFontCursor (dpy, XC_right_side);
    logic.upCursor        = XCreateFontCursor (dpy, XC_top_side);
    logic.upLeftCursor    = XCreateFontCursor (dpy, XC_top_left_corner);
    logic.upRightCursor   = XCreateFontCursor (dpy, XC_top_right_corner);
    logic.downCursor      = XCreateFontCursor (dpy, XC_bottom_side);
    logic.downLeftCursor  = XCreateFontCursor (dpy, XC_bottom_left_corner);
    logic.downRightCursor = XCreateFontCursor (dpy, XC_bottom_right_corner);
    logic.middleCursor    = XCreateFontCursor (dpy, XC_fleur);

    logic.cursor[0] = logic.leftCursor;
    logic.cursor[1] = logic.rightCursor;
    logic.cursor[2] = logic.upCursor;
    logic.cursor[3] = logic.downCursor;

    optionSetInitiateKeyInitiate (resizeInitiateDefaultMode);
    optionSetInitiateKeyTerminate (resizeTerminate);
    optionSetInitiateButtonInitiate (resizeInitiateDefaultMode);
    optionSetInitiateButtonTerminate (resizeTerminate);

    optionSetOutlineModifierNotify (notify);
    optionSetRectangleModifierNotify (notify);
    optionSetStretchModifierNotify (notify);
    optionSetCenteredModifierNotify (notify);

    resizeMaskValueToKeyMask (optionGetOutlineModifierMask (), &logic.outlineMask);
    resizeMaskValueToKeyMask (optionGetRectangleModifierMask (), &logic.rectangleMask);
    resizeMaskValueToKeyMask (optionGetStretchModifierMask (), &logic.stretchMask);
    resizeMaskValueToKeyMask (optionGetCenteredModifierMask (), &logic.centeredMask);

    ScreenInterface::setHandler (s);

    if (gScreen)
	GLScreenInterface::setHandler (gScreen, false);
}

ResizeScreen::~ResizeScreen ()
{
    Display *dpy = screen->dpy ();

    if (logic.leftCursor)
	XFreeCursor (dpy, logic.leftCursor);
    if (logic.rightCursor)
	XFreeCursor (dpy, logic.rightCursor);
    if (logic.upCursor)
	XFreeCursor (dpy, logic.upCursor);
    if (logic.downCursor)
	XFreeCursor (dpy, logic.downCursor);
    if (logic.middleCursor)
	XFreeCursor (dpy, logic.middleCursor);
    if (logic.upLeftCursor)
	XFreeCursor (dpy, logic.upLeftCursor);
    if (logic.upRightCursor)
	XFreeCursor (dpy, logic.upRightCursor);
    if (logic.downLeftCursor)
	XFreeCursor (dpy, logic.downLeftCursor);
    if (logic.downRightCursor)
	XFreeCursor (dpy, logic.downRightCursor);

    delete logic.mScreen;
    delete logic.cScreen;
    delete logic.gScreen;
    delete logic.resizeInformationAtom;
}

ResizeWindow::ResizeWindow (CompWindow *w) :
    PluginClassHandler<ResizeWindow,CompWindow> (w),
    window (w),
    gWindow (GLWindow::get (w)),
    cWindow (CompositeWindow::get (w)),
    rScreen (ResizeScreen::get (screen))
{
    WindowInterface::setHandler (window);

    if (cWindow)
	CompositeWindowInterface::setHandler (cWindow, false);

    if (gWindow)
	GLWindowInterface::setHandler (gWindow, false);
}


ResizeWindow::~ResizeWindow ()
{
}


bool
ResizePluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	 return false;

    return true;
}

