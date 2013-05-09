/*
 * Compiz Fusion Grid plugin
 *
 * Copyright (c) 2008 Stephen Kennedy <suasol@gmail.com>
 * Copyright (c) 2010 Scott Moreau <oreaus@gmail.com>
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
 * Description:
 *
 * Plugin to act like winsplit revolution (http://www.winsplit-revolution.com/)
 * use <Control><Alt>NUMPAD_KEY to move and tile your windows.
 *
 * Press the tiling keys several times to cycle through some tiling options.
 */

#include <boost/bind.hpp>
#include <cmath>
#include <cassert>
#include "grid.h"
#include "grabhandler.h"

using namespace GridWindowType;
namespace cgw = compiz::grid::window;

static std::map <unsigned int, GridProps> gridProps;

static int const CURVE_ANIMATION = 35;

void
GridScreen::handleCompizEvent(const char*    plugin,
			      const char*    event,
			      CompOption::Vector&  o)
{
    if (strcmp(event, "start_viewport_switch") == 0)
	mSwitchingVp = true;
    else if (strcmp(event, "end_viewport_switch") == 0)
	mSwitchingVp = false;

    screen->handleCompizEvent(plugin, event, o);
}

CompRect
GridScreen::slotToRect (CompWindow      *w,
			const CompRect& slot)
{
    return CompRect (slot.x () + w->border ().left,
		     slot.y () + w->border ().top,
		     slot.width () - (w->border ().left + w->border ().right),
		     slot.height () - (w->border ().top + w->border ().bottom));
}

CompRect
GridScreen::constrainSize (CompWindow      *w,
			   const CompRect& slot)
{
    int      cw, ch;
    CompRect result = slotToRect (w, slot);

    if (w->constrainNewWindowSize (result.width (), result.height (), &cw, &ch))
    {
	/* constrained size may put window offscreen, adjust for that case */
	int dx = result.x () + cw - workarea.right () + w->border ().right;
	int dy = result.y () + ch - workarea.bottom () + w->border ().bottom;

	if (dx > 0)
	    result.setX (result.x () - dx);
	if (dy > 0)
	    result.setY (result.y () - dy);

	result.setWidth (cw);
	result.setHeight (ch);
    }

    return result;
}

void
GridScreen::getPaintRectangle (CompRect &cRect)
{
    if (typeToMask (edgeToGridType ()) != GridUnknown && optionGetDrawIndicator ())
	cRect = desiredSlot;
    else
	cRect.setGeometry (0, 0, 0, 0);
}

int
applyProgress (int a, int b, float progress)
{
    return a < b ?
		b - (ABS (a - b) * progress) :
		b + (ABS (a - b) * progress);
}

void
GridScreen::setCurrentRect (Animation &anim)
{
    anim.currentRect.setLeft (applyProgress (anim.targetRect.x1 (),
					     anim.fromRect.x1 (),
					     anim.progress));
    anim.currentRect.setRight (applyProgress (anim.targetRect.x2 (),
					      anim.fromRect.x2 (),
					      anim.progress));
    anim.currentRect.setTop (applyProgress (anim.targetRect.y1 (),
					    anim.fromRect.y1 (),
					    anim.progress));
    anim.currentRect.setBottom (applyProgress (anim.targetRect.y2 (),
					       anim.fromRect.y2 (),
					       anim.progress));
}

bool
GridScreen::initiateCommon (CompAction		*action,
			    CompAction::State	state,
			    CompOption::Vector	&option,
			    unsigned int	where,
			    bool		resize,
			    bool		key)
{
    CompWindow *cw = 0;

    Window xid = CompOption::getIntOptionNamed (option, "window");
    cw  = screen->findWindow (xid);

    if (cw)
    {
	XWindowChanges xwc;
	bool maximizeH = where & (GridBottom | GridTop | GridMaximize);
	bool maximizeV = where & (GridLeft | GridRight | GridMaximize);

	bool horzMaximizedGridPosition = where & (GridTop | GridBottom);
	bool vertMaximizedGridPosition = where & (GridLeft | GridRight);
	bool  anyMaximizedGridPosition = horzMaximizedGridPosition ||
					 vertMaximizedGridPosition ||
					 where & GridMaximize;

	if (!(cw->actions () & CompWindowActionResizeMask) ||
	    (maximizeH && !(cw->actions () & CompWindowActionMaximizeHorzMask)) ||
	    (maximizeV && !(cw->actions () & CompWindowActionMaximizeVertMask)) ||
	    where & GridUnknown)
	    return false;

	GRID_WINDOW (cw);

	if (gw->lastTarget & ~(where))
	    gw->resizeCount = 0;
	else if (!key)
	    return false;

	props = gridProps[where];

	/* get current available area */
	if (cw == mGrabWindow)
	    workarea = screen->getWorkareaForOutput
		       (screen->outputDeviceForPoint (pointerX, pointerY));
	else
	{
	    workarea = screen->getWorkareaForOutput (cw->outputDevice ());

	    if (props.numCellsX == 1)
		centerCheck = true;

	    /* Do not overwrite the original size if we already have been gridded */
	    if (!gw->isGridResized && !gw->isGridHorzMaximized && !gw->isGridVertMaximized)
		/* Store size not including borders when using a keybinding */
		gw->originalSize = slotToRect(cw, cw->serverBorderRect ());
	}

	if ((cw->state () & MAXIMIZE_STATE) &&
	    (resize || optionGetSnapoffMaximized ()))
	    /* maximized state interferes with us, clear it */
	    cw->maximize (0);

	if ((where & GridMaximize) && resize)
	{
	    /* move the window to the correct output */
	    if (cw == mGrabWindow)
	    {
		/* TODO: Remove these magic numbers */
		xwc.x = workarea.x () + 50;
		xwc.y = workarea.y () + 50;
		xwc.width = workarea.width ();
		xwc.height = workarea.height ();
		cw->configureXWindow (CWX | CWY, &xwc);
	    }
	    cw->maximize (MAXIMIZE_STATE);
	    /* Core can handle fully maximized windows so we don't
	     * have to worry about them. Don't mark the window as a
	     * gridded one.
	     */
	    gw->isGridResized = false;
	    gw->isGridHorzMaximized = false;
	    gw->isGridVertMaximized = false;

	    for (unsigned int i = 0; i < animations.size (); ++i)
		animations.at (i).fadingOut = true;
	    return true;
	}

	/* Convention:
	 * xxxSlot include decorations (it's the screen area occupied)
	 * xxxRect are undecorated (it's the constrained position
				    of the contents)
	 */

	/* slice and dice to get desired slot - including decorations */
	desiredSlot.setY (workarea.y () + props.gravityDown *
			  (workarea.height () / props.numCellsY));
	desiredSlot.setHeight (workarea.height () / props.numCellsY);

	desiredSlot.setX (workarea.x () + props.gravityRight *
			  (workarea.width () / props.numCellsX));
	desiredSlot.setWidth (workarea.width () / props.numCellsX);

	if (!optionGetCycleSizes ())
	{
	    /* Adjust for constraints and decorations */
	    if (!anyMaximizedGridPosition)
		desiredRect = constrainSize (cw, desiredSlot);
	    else
		desiredRect = slotToRect (cw, desiredSlot);
	}
	else /* (optionGetCycleSizes ()) */
	{
	    /* Adjust for constraints and decorations */
	    if (where & ~GridMaximize)
		desiredRect = constrainSize (cw, desiredSlot);
	    else
		desiredRect = slotToRect (cw, desiredSlot);
	}

	/* Get current rect not including decorations */
	currentRect.setGeometry (cw->serverX (), cw->serverY (),
				 cw->serverWidth (),
				 cw->serverHeight ());

	/* We do not want to allow cycling through sizes,
	 * unless the user explicitely specified that in CCSM
	 */
	if (gw->lastTarget == where &&
	    gw->isGridResized &&
	    !optionGetCycleSizes ())
	    return false;

	/* !(Grid Left/Right/Top/Bottom) are only valid here, if
	 * cycling through sizes is disabled also
	 */
	if ((where & ~(GridMaximize) ||
	     ((!horzMaximizedGridPosition || !vertMaximizedGridPosition) &&
	      !optionGetCycleSizes ())) &&
	    gw->lastTarget & where)
	{
	    int slotWidth25  = workarea.width () / 4;
	    int slotWidth33  = (workarea.width () / 3) + cw->border ().left;
	    int slotWidth17  = slotWidth33 - slotWidth25;
	    int slotWidth66  = workarea.width () - slotWidth33;
	    int slotWidth75  = workarea.width () - slotWidth25;

	    if (props.numCellsX == 2) /* keys (1, 4, 7, 3, 6, 9) */
	    {
		if ((currentRect.width () == desiredRect.width () &&
		     currentRect.x () == desiredRect.x ()) ||
		    (gw->resizeCount < 1) || (gw->resizeCount > 5))
		    gw->resizeCount = 3;

		/* tricky, have to allow for window constraints when
		 * computing what the 33% and 66% offsets would be
		 */
		switch (gw->resizeCount)
		{
		    case 1:
			desiredSlot.setWidth (slotWidth66);
			desiredSlot.setX (workarea.x () +
					  props.gravityRight * slotWidth33);
			++gw->resizeCount;
			break;

		    case 2:
			++gw->resizeCount;
			break;

		    case 3:
			desiredSlot.setWidth (slotWidth33);
			desiredSlot.setX (workarea.x () +
					  props.gravityRight * slotWidth66);
			++gw->resizeCount;
			break;

		    case 4:
			desiredSlot.setWidth (slotWidth25);
			desiredSlot.setX (workarea.x () +
					  props.gravityRight * slotWidth75);
			++gw->resizeCount;
			break;

		    case 5:
			desiredSlot.setWidth (slotWidth75);
			desiredSlot.setX (workarea.x () +
					  props.gravityRight * slotWidth25);
			++gw->resizeCount;
			break;

		    default:
			break;
		}
	    }
	    else /* keys (2, 5, 8) */
	    {
		if ((currentRect.width () == desiredRect.width () &&
		     currentRect.x () == desiredRect.x ()) ||
		    (gw->resizeCount < 1) || (gw->resizeCount > 5))
		    gw->resizeCount = 1;

		switch (gw->resizeCount)
		{
		    case 1:
			desiredSlot.setWidth (workarea.width () -
					      (slotWidth17 * 2));
			desiredSlot.setX (workarea.x () + slotWidth17);
			++gw->resizeCount;
			break;

		    case 2:
			desiredSlot.setWidth ((slotWidth25 * 2) +
					      (slotWidth17 * 2));
			desiredSlot.setX (workarea.x () +
					  (slotWidth25 - slotWidth17));
			++gw->resizeCount;
			break;

		    case 3:
			desiredSlot.setWidth ((slotWidth25 * 2));
			desiredSlot.setX (workarea.x () + slotWidth25);
			++gw->resizeCount;
			break;

		    case 4:
			desiredSlot.setWidth (slotWidth33 -
					      (cw->border ().left +
					       cw->border ().right));
			desiredSlot.setX (workarea.x () + slotWidth33);
			++gw->resizeCount;
			break;

		    case 5:
			++gw->resizeCount;
			break;

		    default:
			break;
		}
	    }

	    if (gw->resizeCount == 6)
		gw->resizeCount = 1;

	    desiredRect = constrainSize (cw, desiredSlot);
	}

	xwc.x = desiredRect.x ();
	xwc.y = desiredRect.y ();
	xwc.width  = desiredRect.width ();
	xwc.height = desiredRect.height ();

	/* Store a copy of xwc since configureXWindow changes it's values */
	XWindowChanges wc = xwc;

	if (cw->mapNum ())
	    cw->sendSyncRequest ();

	/* TODO: animate move+resize */
	if (resize)
	{
	    unsigned int valueMask = CWX | CWY | CWWidth | CWHeight;
	    gw->lastTarget = where;
	    gw->currentSize = CompRect (wc.x, wc.y, wc.width, wc.height);
	    CompWindowExtents lastBorder = gw->window->border ();

	    gw->sizeHintsFlags = 0;

	    if (!optionGetCycleSizes ())
	    {
		/* Special cases for left/right and top/bottom gridded windows, where we
		 * actually vertically respective horizontally semi-maximize the window
		 */
		if (horzMaximizedGridPosition || vertMaximizedGridPosition)
		{
		    /* First restore the window to its original size */
		    XWindowChanges rwc;

		    rwc.x = gw->originalSize.x ();
		    rwc.y = gw->originalSize.y ();
		    rwc.width = gw->originalSize.width ();
		    rwc.height = gw->originalSize.height ();

		    cw->configureXWindow (CWX | CWY | CWWidth | CWHeight, &rwc);

		    /* GridLeft || GridRight */
		    if (vertMaximizedGridPosition)
		    {
			gw->isGridVertMaximized = true;
			gw->isGridHorzMaximized = false;
			gw->isGridResized = false;

			/* Semi-maximize the window vertically */
			cw->maximize (CompWindowStateMaximizedVertMask);
		    }
		    /* GridTop || GridBottom */
		    else /* (horzMaximizedGridPosition) */
		    {
			gw->isGridHorzMaximized = true;
			gw->isGridVertMaximized = false;
			gw->isGridResized = false;

			/* Semi-maximize the window horizontally */
			cw->maximize (CompWindowStateMaximizedHorzMask);
		    }

		    /* Be evil */
		    if (cw->sizeHints ().flags & PResizeInc)
		    {
			gw->sizeHintsFlags |= PResizeInc;
			gw->window->sizeHints ().flags &= ~(PResizeInc);
		    }
		}
		else /* GridCorners || GridCenter */
		{
		    gw->isGridResized = true;
		    gw->isGridHorzMaximized = false;
		    gw->isGridVertMaximized = false;
		}
	    }
	    else /* if (optionGetCycleSizes ()) */
	    {
		gw->isGridResized = true;
		gw->isGridHorzMaximized = false;
		gw->isGridVertMaximized = false;
	    }

	    int dw = (lastBorder.left + lastBorder.right) -
		     (gw->window->border ().left +
		      gw->window->border ().right);

	    int dh = (lastBorder.top + lastBorder.bottom) -
		     (gw->window->border ().top +
		      gw->window->border ().bottom);

	    xwc.width += dw;
	    xwc.height += dh;

	    /* Make window the size that we want */
	    cw->configureXWindow (valueMask, &xwc);

	    for (unsigned int i = 0; i < animations.size (); ++i)
		animations.at (i).fadingOut = true;
	}

	/* This centers a window if it could not be resized to the desired
	 * width. Without this, it can look buggy when desired width is
	 * beyond the minimum or maximum width of the window.
	 */
	if (centerCheck)
	{
	    if ((cw->serverBorderRect ().width () >
		 desiredSlot.width ()) ||
		cw->serverBorderRect ().width () <
		desiredSlot.width ())
	    {
		wc.x = (workarea.width () >> 1) -
		       ((cw->serverBorderRect ().width () >> 1) -
			cw->border ().left);
		cw->configureXWindow (CWX, &wc);
	    }

	    centerCheck = false;
	}
    }

    return true;
}

void
GridScreen::glPaintRectangle (const GLScreenPaintAttrib &sAttrib,
			      const GLMatrix            &transform,
			      CompOutput                *output)
{
    CompRect rect;
    GLMatrix sTransform (transform);
    std::vector<Animation>::iterator iter;
    GLVertexBuffer *streamingBuffer = GLVertexBuffer::streamingBuffer ();
    GLfloat         vertexData[12];
    GLushort        colorData[4];
    GLushort       *color;
    GLboolean       isBlendingEnabled;

    const float MaxUShortFloat = std::numeric_limits <unsigned short>::max ();

    getPaintRectangle (rect);

    for (unsigned int i = 0; i < animations.size (); ++i)
	setCurrentRect (animations.at (i));

    sTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);

    glGetBooleanv (GL_BLEND, &isBlendingEnabled);
    glEnable (GL_BLEND);

    for (iter = animations.begin (); iter != animations.end () && animating; ++iter)
    {
	Animation& anim = *iter;

	float curve = powf (CURVE_ANIMATION, -anim.progress);
	float alpha = (optionGetFillColorAlpha () / MaxUShortFloat) * anim.opacity;
	color = optionGetFillColor ();

	colorData[0] = alpha * color[0];
	colorData[1] = alpha * color[1];
	colorData[2] = alpha * color[2];
	colorData[3] = alpha * MaxUShortFloat;

	if (optionGetDrawStretchedWindow ())
	    colorData[3] *= (1.0 - curve);

	vertexData[0]  = anim.currentRect.x1 ();
	vertexData[1]  = anim.currentRect.y1 ();
	vertexData[2]  = 0.0f;
	vertexData[3]  = anim.currentRect.x1 ();
	vertexData[4]  = anim.currentRect.y2 ();
	vertexData[5]  = 0.0f;
	vertexData[6]  = anim.currentRect.x2 ();
	vertexData[7]  = anim.currentRect.y1 ();
	vertexData[8]  = 0.0f;
	vertexData[9]  = anim.currentRect.x2 ();
	vertexData[10] = anim.currentRect.y2 ();
	vertexData[11] = 0.0f;

	streamingBuffer->begin (GL_TRIANGLE_STRIP);
	streamingBuffer->addColors (1, colorData);
	streamingBuffer->addVertices (4, vertexData);

	streamingBuffer->end ();
	streamingBuffer->render (sTransform);

	/* Set outline rect smaller to avoid damage issues */
	anim.currentRect.setGeometry (anim.currentRect.x () + 1,
				      anim.currentRect.y () + 1,
				      anim.currentRect.width () - 2,
				      anim.currentRect.height () - 2);

	/* draw outline */
	alpha = (optionGetOutlineColorAlpha () / MaxUShortFloat) * anim.opacity;
	color = optionGetOutlineColor ();

	colorData[0] = alpha * color[0];
	colorData[1] = alpha * color[1];
	colorData[2] = alpha * color[2];
	colorData[3] = alpha * MaxUShortFloat;

	if (optionGetDrawStretchedWindow ())
	    colorData[3] *= (1.0 - curve);

	vertexData[0]  = anim.currentRect.x1 ();
	vertexData[1]  = anim.currentRect.y1 ();
	vertexData[3]  = anim.currentRect.x1 ();
	vertexData[4]  = anim.currentRect.y2 ();
	vertexData[6]  = anim.currentRect.x2 ();
	vertexData[7]  = anim.currentRect.y2 ();
	vertexData[9]  = anim.currentRect.x2 ();
	vertexData[10] = anim.currentRect.y1 ();

	glLineWidth (2.0);

	streamingBuffer->begin (GL_LINE_LOOP);
	streamingBuffer->addColors (1, colorData);
	streamingBuffer->addVertices (4, vertexData);

	streamingBuffer->end ();
	streamingBuffer->render (sTransform);
    }

    if (!animating)
    {
	/* draw filled rectangle */
	float alpha = optionGetFillColorAlpha () / MaxUShortFloat;
	color = optionGetFillColor ();

	colorData[0] = alpha * color[0];
	colorData[1] = alpha * color[1];
	colorData[2] = alpha * color[2];
	colorData[3] = alpha * MaxUShortFloat;

	vertexData[0]  = rect.x1 ();
	vertexData[1]  = rect.y1 ();
	vertexData[2]  = 0.0f;
	vertexData[3]  = rect.x1 ();
	vertexData[4]  = rect.y2 ();
	vertexData[5]  = 0.0f;
	vertexData[6]  = rect.x2 ();
	vertexData[7]  = rect.y1 ();
	vertexData[8]  = 0.0f;
	vertexData[9]  = rect.x2 ();
	vertexData[10] = rect.y2 ();
	vertexData[11] = 0.0f;

	streamingBuffer->begin (GL_TRIANGLE_STRIP);
	streamingBuffer->addColors (1, colorData);
	streamingBuffer->addVertices (4, vertexData);

	streamingBuffer->end ();
	streamingBuffer->render (sTransform);

	/* Set outline rect smaller to avoid damage issues */
	rect.setGeometry (rect.x () + 1,
			  rect.y () + 1,
			  rect.width () - 2,
			  rect.height () - 2);

	/* draw outline */
	alpha = optionGetOutlineColorAlpha () / MaxUShortFloat;
	color = optionGetOutlineColor ();

	colorData[0] = alpha * color[0];
	colorData[1] = alpha * color[1];
	colorData[2] = alpha * color[2];
	colorData[3] = alpha * MaxUShortFloat;

	vertexData[0]  = rect.x1 ();
	vertexData[1]  = rect.y1 ();
	vertexData[3]  = rect.x1 ();
	vertexData[4]  = rect.y2 ();
	vertexData[6]  = rect.x2 ();
	vertexData[7]  = rect.y2 ();
	vertexData[9]  = rect.x2 ();
	vertexData[10] = rect.y1 ();

	glLineWidth (2.0);

	streamingBuffer->begin (GL_LINE_LOOP);
	streamingBuffer->addColors (1, colorData);
	streamingBuffer->addVertices (4, vertexData);

	streamingBuffer->end ();
	streamingBuffer->render (sTransform);
    }

    if (!isBlendingEnabled)
	glDisable (GL_BLEND);
}

bool
GridScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
			   const GLMatrix            &matrix,
			   const CompRegion          &region,
			   CompOutput                *output,
			   unsigned int              mask)
{
    bool status = glScreen->glPaintOutput (attrib, matrix, region, output, mask);

    glPaintRectangle (attrib, matrix, output);

    return status;
}

namespace
{
class GridTypeMask
{
    public:

	GridTypeMask (unsigned int m, int t):
	    mask (m),
	    type (t)
	{
	}

	unsigned int mask;
	int type;
};
}

unsigned int
GridScreen::typeToMask (int t)
{
    std::vector <GridTypeMask> type;
    type.push_back (GridTypeMask (GridWindowType::GridUnknown, 0));
    type.push_back (GridTypeMask (GridWindowType::GridBottomLeft, 1));
    type.push_back (GridTypeMask (GridWindowType::GridBottom, 2));
    type.push_back (GridTypeMask (GridWindowType::GridBottomRight, 3));
    type.push_back (GridTypeMask (GridWindowType::GridLeft, 4));
    type.push_back (GridTypeMask (GridWindowType::GridCenter, 5));
    type.push_back (GridTypeMask (GridWindowType::GridRight, 6));
    type.push_back (GridTypeMask (GridWindowType::GridTopLeft, 7));
    type.push_back (GridTypeMask (GridWindowType::GridTop, 8));
    type.push_back (GridTypeMask (GridWindowType::GridTopRight, 9));
    type.push_back (GridTypeMask (GridWindowType::GridMaximize, 10));


    for (unsigned int i = 0; i < type.size (); ++i)
    {
	GridTypeMask &tm = type[i];
	if (tm.type == t)
	    return tm.mask;
    }

    return GridWindowType::GridUnknown;
}

int
GridScreen::edgeToGridType ()
{
    int ret;

    switch (edge)
    {
	case Left:
	    ret = (int) optionGetLeftEdgeAction ();
	    break;

	case Right:
	    ret = (int) optionGetRightEdgeAction ();
	    break;

	case Top:
	    ret = (int) optionGetTopEdgeAction ();
	    break;

	case Bottom:
	    ret = (int) optionGetBottomEdgeAction ();
	    break;

	case TopLeft:
	    ret = (int) optionGetTopLeftCornerAction ();
	    break;

	case TopRight:
	    ret = (int) optionGetTopRightCornerAction ();
	    break;

	case BottomLeft:
	    ret = (int) optionGetBottomLeftCornerAction ();
	    break;

	case BottomRight:
	    ret = (int) optionGetBottomRightCornerAction ();
	    break;

	case NoEdge:
	default:
	    ret = -1;
	    break;
    }

    return ret;
}

void
GridScreen::handleEvent (XEvent *event)
{
    CompWindow *w;

    screen->handleEvent (event);

    if (event->type != MotionNotify || !mGrabWindow)
	return;

    /* Detect when cursor enters another output */

    currentWorkarea = screen->getWorkareaForOutput
		      (screen->outputDeviceForPoint (pointerX, pointerY));

    if (lastWorkarea != currentWorkarea)
    {
	lastWorkarea = currentWorkarea;

	if (cScreen)
	    cScreen->damageRegion (desiredSlot);

	initiateCommon (0, 0, o, typeToMask (edgeToGridType ()), false, false);

	if (cScreen)
	    cScreen->damageRegion (desiredSlot);
    }

    CompOutput out = screen->outputDevs ().at (
	      screen->outputDeviceForPoint (CompPoint (pointerX, pointerY)));

    /* Detect corners first */

    /* Bottom Left */
    if (pointerY > (out.y () + out.height () - optionGetBottomEdgeThreshold ()) &&
	pointerX < (out.x () + optionGetLeftEdgeThreshold ()))
	edge = BottomLeft;

    /* Bottom Right */
    else if (pointerY > (out.y () + out.height () - optionGetBottomEdgeThreshold ()) &&
	     pointerX > (out.x () + out.width () - optionGetRightEdgeThreshold ()))
	edge = BottomRight;

    /* Top Left */
    else if (pointerY < (out.y () + optionGetTopEdgeThreshold ()) &&
	     pointerX < (out.x () + optionGetLeftEdgeThreshold ()))
	edge = TopLeft;

    /* Top Right */
    else if (pointerY < (out.y () + optionGetTopEdgeThreshold ()) &&
	     pointerX > (out.x () + out.width () - optionGetRightEdgeThreshold ()))
	edge = TopRight;

    /* Left */
    else if (pointerX < (out.x () + optionGetLeftEdgeThreshold ()))
	edge = Left;

    /* Right */
    else if (pointerX > (out.x () + out.width () - optionGetRightEdgeThreshold ()))
	edge = Right;

    /* Top */
    else if (pointerY < (out.y () + optionGetTopEdgeThreshold ()))
	edge = Top;

    /* Bottom */
    else if (pointerY > (out.y () + out.height () - optionGetBottomEdgeThreshold ()))
	edge = Bottom;

    /* No Edge */
    else
	edge = NoEdge;

    /* Detect edge region change */

    if (lastEdge != edge)
    {
	bool check = false;
	unsigned int target = typeToMask (edgeToGridType ());
	lastSlot = desiredSlot;

	if (edge == NoEdge || target == GridUnknown)
	    desiredSlot.setGeometry (0, 0, 0, 0);

	if (cScreen)
	    cScreen->damageRegion (desiredSlot);

	check = initiateCommon (NULL, 0, o, target, false, false);

	if (cScreen)
	    cScreen->damageRegion (desiredSlot);

	if (lastSlot != desiredSlot)
	{
	    if (!animations.empty ())
		/* Begin fading previous animation instance */
		animations.at (animations.size () - 1).fadingOut = true;

	    if (edge != NoEdge && check)
	    {
		CompWindow *cw = screen->findWindow (screen->activeWindow ());
		if (cw)
		{
		    animations.push_back (Animation ());
		    int current = animations.size () - 1;
		    animations.at (current).fromRect	= cw->serverBorderRect ();
		    animations.at (current).currentRect	= cw->serverBorderRect ();
		    animations.at (current).duration = optionGetAnimationDuration ();
		    animations.at (current).timer = animations.at (current).duration;
		    animations.at (current).targetRect = desiredSlot;
		    animations.at (current).window = cw->id();

		    if (lastEdge == NoEdge || !animating)
		    {
			/* Cursor has entered edge region from non-edge region */
			animating = true;
			glScreen->glPaintOutputSetEnabled (this, true);
			cScreen->preparePaintSetEnabled (this, true);
			cScreen->donePaintSetEnabled (this, true);
		    }
		}
	    }
	}

	lastEdge = edge;
    }

    w = screen->findWindow (CompOption::getIntOptionNamed (o, "window"));

    if (w)
    {
	GRID_WINDOW (w);

	if ((gw->pointerBufDx > SNAPOFF_THRESHOLD ||
	     gw->pointerBufDy > SNAPOFF_THRESHOLD ||
	     gw->pointerBufDx < -SNAPOFF_THRESHOLD ||
	     gw->pointerBufDy < -SNAPOFF_THRESHOLD) &&
	     gw->isGridResized &&
	     optionGetSnapbackWindows ())
	    restoreWindow (0, 0, o);
    }
}

void
GridWindow::validateResizeRequest (unsigned int &xwcm,
				   XWindowChanges *xwc,
				   unsigned int source)
{
    window->validateResizeRequest (xwcm, xwc, source);

    /* Don't allow non-pagers to change
     * the size of the window, the user
     * specified this size */
    if (isGridHorzMaximized || isGridVertMaximized)
	if (source != ClientTypePager)
	    xwcm = 0;
}

void
GridWindow::grabNotify (int          x,
			int          y,
			unsigned int state,
			unsigned int mask)
{
    static cgw::GrabActiveFunc grabActive (boost::bind (&CompScreen::grabExist,
							screen, _1));
    cgw::GrabWindowHandler gwHandler (mask, grabActive);

    if (gwHandler.track ())
    {
	gScreen->o[0].value ().set ((int) window->id ());

	screen->handleEventSetEnabled (gScreen, true);
	gScreen->mGrabWindow = window;
	pointerBufDx = pointerBufDy = 0;
	grabMask = mask;

	if (!isGridResized &&
	    !isGridHorzMaximized &&
	    !isGridVertMaximized &&
	    gScreen->optionGetSnapbackWindows ())
	    /* Store size not including borders when grabbing with cursor */
	    originalSize = gScreen->slotToRect(window,
					       window->serverBorderRect ());
    }
    else if (gwHandler.resetResize ())
    {
	isGridResized = false;
	resizeCount = 0;
    }

    window->grabNotify (x, y, state, mask);
}

void
GridWindow::ungrabNotify ()
{
    if (window == gScreen->mGrabWindow)
    {
	gScreen->initiateCommon
		(NULL, 0, gScreen->o, gScreen->typeToMask (gScreen->edgeToGridType ()), true,
		 gScreen->edge != gScreen->lastResizeEdge);

	screen->handleEventSetEnabled (gScreen, false);
	grabMask = 0;
	gScreen->mGrabWindow = NULL;
	gScreen->o[0].value ().set (0);
	gScreen->cScreen->damageRegion (gScreen->desiredSlot);
    }

    gScreen->lastResizeEdge = gScreen->edge;
    gScreen->edge = NoEdge;

    window->ungrabNotify ();
}

void
GridWindow::moveNotify (int dx, int dy, bool immediate)
{
    window->moveNotify (dx, dy, immediate);

    if (isGridResized &&
	!isGridHorzMaximized &&
	!isGridVertMaximized &&
	!GridScreen::get (screen)->mSwitchingVp)
    {
	if (window->grabbed () && screen->grabExist ("expo"))
	{
	    /* Window is being dragged in expo.
	     * Restore the original geometry right
	     * away to avoid any confusion.
	     */
	    gScreen->restoreWindow (0, 0, gScreen->o);
	    return;
	}
	if (window->grabbed () && (grabMask & CompWindowGrabMoveMask))
	{
	    pointerBufDx += dx;
	    pointerBufDy += dy;
	}

	/* Do not allow the window to be moved while it
	 * is resized */
	dx = currentSize.x () - window->geometry ().x ();
	dy = currentSize.y () - window->geometry ().y ();

	window->move (dx, dy);
    }
}

void
GridWindow::stateChangeNotify (unsigned int lastState)
{
    if (lastState & MAXIMIZE_STATE &&
	!(window->state () & MAXIMIZE_STATE))
    {
	lastTarget = GridUnknown;

	if ((isGridHorzMaximized &&
	     (lastState & MAXIMIZE_STATE) == CompWindowStateMaximizedHorzMask) ||
	    (isGridVertMaximized &&
	     (lastState & MAXIMIZE_STATE) == CompWindowStateMaximizedVertMask))
	    gScreen->restoreWindow(0, 0, gScreen->o);
    }
    else if (!(lastState & MAXIMIZE_STATE) &&
	     window->state () & MAXIMIZE_STATE)
    {
	/* Unset grid resize state */
	isGridResized = false;
	resizeCount = 0;

	if ((window->state () & MAXIMIZE_STATE) == MAXIMIZE_STATE)
	    lastTarget = GridMaximize;

	if (window->grabbed ())
	    originalSize = gScreen->slotToRect (window,
						window->serverBorderRect ());
    }

    window->stateChangeNotify (lastState);
}

bool
GridScreen::restoreWindow (CompAction         *action,
			   CompAction::State  state,
			   CompOption::Vector &option)
{
    XWindowChanges xwc;
    int xwcm = 0;
    CompWindow *cw = screen->findWindow (screen->activeWindow ());

    if (!cw)
	return false;

    GRID_WINDOW (cw);

    /* We have nothing to do here */
    if (!gw->isGridResized &&
	!gw->isGridVertMaximized &&
	!gw->isGridHorzMaximized)
	return false;

    else if (!gw->isGridResized &&
	     gw->isGridHorzMaximized &&
	     !gw->isGridVertMaximized)
    {
	/* Window has been horizontally maximized by grid. We only need
	 * to restore Y and height - core handles X and width. */
	if (gw->sizeHintsFlags)
	    gw->window->sizeHints ().flags |= gw->sizeHintsFlags;
	xwcm |=  CWY | CWHeight;
    }
    else if (!gw->isGridResized &&
	     !gw->isGridHorzMaximized &&
	     gw->isGridVertMaximized)
    {
	/* Window has been vertically maximized by grid. We only need
	 * to restore X and width - core handles Y and height. */
	if (gw->sizeHintsFlags)
	    gw->window->sizeHints ().flags |= gw->sizeHintsFlags;
	xwcm |= CWX | CWWidth;
    }
    else if (gw->isGridResized &&
	     !gw->isGridHorzMaximized &&
	     !gw->isGridVertMaximized)
	/* Window is just gridded (center, corners).
	 * We need to handle everything. */
	xwcm |= CWX | CWY | CWWidth | CWHeight;
    else
    {
	/* This should never happen. But if it does, just bail out
	 * gracefully. */
	assert (gw->isGridResized &&
		(gw->isGridHorzMaximized || gw->isGridVertMaximized));
	return false;
    }

    if (cw == mGrabWindow)
    {
	xwc.x = pointerX - (gw->originalSize.width () / 2);
	xwc.y = pointerY + (cw->border ().top / 2);
    }
    else if (cw->grabbed () && screen->grabExist ("expo"))
    {
	/* We're restoring a window inside expo by dragging. This is a bit
	 * tricky. Pointer location is absolute to the screen, not relative
	 * to expo viewport. So we can't use pointer location to calculate
	 * the position of the restore window.
	 *
	 * The best solution is to resize it in place. */
	xwcm = CWWidth | CWHeight;
    }
    else
    {
	xwc.x = gw->originalSize.x ();
	xwc.y = gw->originalSize.y ();
    }

    xwc.width  = gw->originalSize.width ();
    xwc.height = gw->originalSize.height ();

    if (cw->mapNum() && xwcm)
	cw->sendSyncRequest();

    cw->configureXWindow (xwcm, &xwc);
    gw->currentSize = CompRect ();
    gw->pointerBufDx = 0;
    gw->pointerBufDy = 0;
    gw->isGridHorzMaximized = false;
    gw->isGridVertMaximized = false;
    gw->isGridResized = false;

    if (cw->state () & MAXIMIZE_STATE)
	cw->maximize(0);

    gw->resizeCount = 0;
    gw->lastTarget = GridUnknown;

    return true;
}

void
GridScreen::snapbackOptionChanged (CompOption *option,
				   Options    num)
{
    GRID_WINDOW (screen->findWindow
		 (CompOption::getIntOptionNamed (o, "window")));
    gw->isGridResized = false;
    gw->isGridHorzMaximized = false;
    gw->isGridVertMaximized = false;
    gw->resizeCount = 0;
}

void
GridScreen::preparePaint (int msSinceLastPaint)
{
    std::vector<Animation>::iterator iter;

    for (iter = animations.begin (); iter != animations.end (); ++iter)
    {
	Animation& anim = *iter;
	anim.timer -= msSinceLastPaint;

	if (anim.timer < 0)
	    anim.timer = 0;

	if (anim.fadingOut)
	    anim.opacity -= msSinceLastPaint * 0.002;
	else
	    if (anim.opacity < 1.0f)
		anim.opacity = anim.progress * anim.progress;
	    else
		anim.opacity = 1.0f;

	if (anim.opacity < 0)
	{
	    anim.opacity = 0.0f;
	    anim.fadingOut = false;
	    anim.complete = true;
	}

	anim.progress =	(anim.duration - anim.timer) / anim.duration;
    }

    if (optionGetDrawStretchedWindow ())
    {
	CompWindow *cw = screen->findWindow (screen->activeWindow ());
	GRID_WINDOW (cw);

	gw->gWindow->glPaintSetEnabled (gw, true);
    }

    cScreen->preparePaint (msSinceLastPaint);
}

void
GridScreen::donePaint ()
{
    std::vector<Animation>::iterator iter;

    for (iter = animations.begin (); iter != animations.end ();)
    {
	Animation& anim = *iter;

	if (anim.complete)
	    iter = animations.erase(iter);
	else
	    ++iter;
    }

    if (animations.empty ())
    {
	cScreen->preparePaintSetEnabled (this, false);
	cScreen->donePaintSetEnabled (this, false);

	if (edge == NoEdge)
	    glScreen->glPaintOutputSetEnabled (this, false);

	animations.clear ();
	animating = false;
    }

    if (optionGetDrawStretchedWindow ())
    {
	CompWindow *cw = screen->findWindow (screen->activeWindow ());
	GRID_WINDOW (cw);

	gw->gWindow->glPaintSetEnabled (gw, false);
    }

    cScreen->damageScreen ();

    cScreen->donePaint ();
}

Animation::Animation ()
{
    progress = 0.0f;
    fromRect = CompRect (0, 0, 0, 0);
    targetRect = CompRect (0, 0, 0, 0);
    currentRect = CompRect (0, 0, 0, 0);
    opacity = 0.0f;
    timer = 0.0f;
    duration = 0;
    complete = false;
    fadingOut = false;
    window = 0;
}

GridScreen::GridScreen (CompScreen *screen) :
    PluginClassHandler<GridScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    glScreen (GLScreen::get (screen)),
    props (),
    centerCheck (false),
    mGrabWindow (NULL),
    animating (false),
    mSwitchingVp (false)
{
    o.push_back (CompOption ("window", CompOption::TypeInt));

    ScreenInterface::setHandler (screen, false);
    screen->handleCompizEventSetEnabled (this, true);
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (glScreen, false);

    edge = lastEdge = lastResizeEdge = NoEdge;
    currentWorkarea = lastWorkarea = screen->getWorkareaForOutput
				     (screen->outputDeviceForPoint (pointerX, pointerY));
    gridProps[GridUnknown] = GridProps (0,1, 1,1);
    gridProps[GridBottomLeft]  = GridProps (0,1, 2,2);
    gridProps[GridBottom]  = GridProps (0,1, 1,2);
    gridProps[GridBottomRight] = GridProps (1,1, 2,2);
    gridProps[GridLeft]  = GridProps (0,0, 2,1);
    gridProps[GridCenter]  = GridProps (0,0, 1,1);
    gridProps[GridRight]  = GridProps (1,0, 2,1);
    gridProps[GridTopLeft]  = GridProps (0,0, 2,2);
    gridProps[GridTop]  = GridProps (0,0, 1,2);
    gridProps[GridTopRight]  = GridProps (1,0, 2,2);
    gridProps[GridMaximize]  = GridProps (0,0, 1,1);

    animations.clear ();

#define GRIDSET(opt,where,resize,key)					       \
    optionSet##opt##Initiate (boost::bind (&GridScreen::initiateCommon, this,  \
    _1, _2, _3, where, resize, key))

    GRIDSET (PutCenterKey, GridWindowType::GridCenter, true, true);
    GRIDSET (PutLeftKey, GridWindowType::GridLeft, true, true);
    GRIDSET (PutRightKey, GridWindowType::GridRight, true, true);
    GRIDSET (PutTopKey, GridWindowType::GridTop, true, true);
    GRIDSET (PutBottomKey, GridWindowType::GridBottom, true, true);
    GRIDSET (PutTopleftKey, GridWindowType::GridTopLeft, true, true);
    GRIDSET (PutToprightKey, GridWindowType::GridTopRight, true, true);
    GRIDSET (PutBottomleftKey, GridWindowType::GridBottomLeft, true, true);
    GRIDSET (PutBottomrightKey, GridWindowType::GridBottomRight, true, true);
    GRIDSET (PutMaximizeKey, GridWindowType::GridMaximize, true, true);

#undef GRIDSET

    optionSetSnapbackWindowsNotify (boost::bind (&GridScreen::
						 snapbackOptionChanged, this, _1, _2));

    optionSetPutRestoreKeyInitiate (boost::bind (&GridScreen::
						 restoreWindow, this, _1, _2, _3));
}

GridWindow::GridWindow (CompWindow *window) :
    PluginClassHandler <GridWindow, CompWindow> (window),
    window (window),
    gWindow (GLWindow::get(window)),
    gScreen (GridScreen::get (screen)),
    isGridResized (false),
    isGridHorzMaximized (false),
    isGridVertMaximized (false),
    grabMask (0),
    pointerBufDx (0),
    pointerBufDy (0),
    resizeCount (0),
    lastTarget (GridUnknown),
    sizeHintsFlags (0)
{
    WindowInterface::setHandler (window);
    GLWindowInterface::setHandler (gWindow, false);
}

GridWindow::~GridWindow ()
{
    if (gScreen->mGrabWindow == window)
	gScreen->mGrabWindow = NULL;

    CompWindow *w = screen->findWindow (CompOption::getIntOptionNamed (gScreen->o, "window"));
    if (w == window)
	gScreen->o[0].value ().set (0);
}

bool
GridWindow::glPaint (const GLWindowPaintAttrib& attrib, const GLMatrix& matrix,
		     const CompRegion& region, const unsigned int mask)
{
    bool status = gWindow->glPaint (attrib, matrix, region, mask);

    std::vector<Animation>::iterator iter;

    for (iter = gScreen->animations.begin ();
	 iter != gScreen->animations.end () && gScreen->animating; ++iter)
    {
	Animation& anim = *iter;

	if (anim.timer > 0.0f && anim.window == window->id())
	{
	    GLWindowPaintAttrib wAttrib(attrib);
	    GLMatrix wTransform (matrix);
	    unsigned int wMask(mask);

	    float curve = powf (CURVE_ANIMATION, -anim.progress);
	    wAttrib.opacity *= curve;

	    wMask |= PAINT_WINDOW_TRANSFORMED_MASK;
	    wMask |= PAINT_WINDOW_TRANSLUCENT_MASK;
	    wMask |= PAINT_WINDOW_BLEND_MASK;

	    float scaleX = (anim.currentRect.x2 () - anim.currentRect.x1 ()) /
			   (float) window->borderRect ().width ();

	    float scaleY = (anim.currentRect.y2 () - anim.currentRect.y1 ()) /
			   (float) window->borderRect ().height ();

	    float translateX = (anim.currentRect.x1 () - window->x ()) +
			       window->border ().left * scaleX;

	    float translateY = (anim.currentRect.y1 () - window->y ()) +
			       window->border ().top * scaleY;

	    wTransform.translate (window->x (), window->y (), 0.0f);
	    wTransform.scale (scaleX, scaleY, 1.0f);
	    wTransform.translate (translateX / scaleX - window->x (),
				  translateY / scaleY - window->y (), 0.0f);


	    gWindow->glPaint (wAttrib, wTransform, region, wMask);
	}
    }

    return status;
}

/* Initial plugin init function called. Checks to see if we are ABI
 * compatible with core, otherwise unload */

bool
GridPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    return true;
}
