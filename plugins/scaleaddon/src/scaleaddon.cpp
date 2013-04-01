/*
 *
 * Compiz scale plugin addon plugin
 *
 * scaleaddon.cpp
 *
 * Copyright : (C) 2007 by Danny Baumann
 * E-mail    : maniac@opencompositing.org
 *
 * Organic scale mode taken from Beryl's scale.c, written by
 * Copyright : (C) 2006 Diogo Ferreira
 * E-mail    : diogo@underdev.org
 *
 * Ported to Compiz 0.9 by:
 * Copyright : (C) 2009 by Sam Spilsbury
 * E-mail    : smspillaz@gmail.com
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
 *
 */

#include "scaleaddon.h"
#include <iostream>

COMPIZ_PLUGIN_20090315 (scaleaddon, ScaleAddonPluginVTable);

bool textAvailable;

void
ScaleAddonWindow::renderTitle ()
{
    CompText::Attrib attrib;
    float            scale;
    int              titleOpt;

    ADDON_SCREEN (screen);

    if (!textAvailable)
	return;

    text.clear ();

    if (!sWindow->hasSlot ())
	return;

    titleOpt = as->optionGetWindowTitle ();

    if (titleOpt == ScaleaddonOptions::WindowTitleNoDisplay)
	return;

    if (titleOpt == ScaleaddonOptions::WindowTitleHighlightedWindowOnly &&
	as->highlightedWindow != window->id ())
    {
	return;
    }

    scale = sWindow->getSlot ().scale;
    attrib.maxWidth = window->width () * scale;
    attrib.maxHeight = window->height () * scale;

    attrib.family = "Sans";
    attrib.size = as->optionGetTitleSize ();
    attrib.color[0] = as->optionGetFontColorRed ();
    attrib.color[1] = as->optionGetFontColorGreen ();
    attrib.color[2] = as->optionGetFontColorBlue ();
    attrib.color[3] = as->optionGetFontColorAlpha ();

    attrib.flags = CompText::WithBackground | CompText::Ellipsized;
    if (as->optionGetTitleBold ())
	attrib.flags |= CompText::StyleBold;

    attrib.bgHMargin = as->optionGetBorderSize ();
    attrib.bgVMargin = as->optionGetBorderSize ();
    attrib.bgColor[0] = as->optionGetBackColorRed ();
    attrib.bgColor[1] = as->optionGetBackColorGreen ();
    attrib.bgColor[2] = as->optionGetBackColorBlue ();
    attrib.bgColor[3] = as->optionGetBackColorAlpha ();

    text.renderWindowTitle (window->id (),
			    as->sScreen->getType () == ScaleTypeAll,
			    attrib);
}

void
ScaleAddonWindow::drawTitle (const GLMatrix &transform)
{
    float         x, y, width, height;
    ScalePosition pos = sWindow->getCurrentPosition ();
    CompRect      geom = window->borderRect ();

    width  = text.getWidth ();
    height = text.getHeight ();

    x = pos.x () + window->x () + geom.width () * pos.scale / 2 - width / 2;
    y = pos.y () + window->y () + geom.height () * pos.scale / 2 - height / 2;

    text.draw (transform, floor (x), floor (y), 1.0f);
}

void
ScaleAddonWindow::drawHighlight (const GLMatrix &transform)
{
    GLint         oldBlendSrc, oldBlendDst;
    GLushort colorData[4];
    GLfloat  vertexData[12];
    GLVertexBuffer *streamingBuffer = GLVertexBuffer::streamingBuffer ();
    float         x, y, width, height;
    ScalePosition pos = sWindow->getCurrentPosition ();
    CompRect      geom = window->borderRect ();

    ADDON_SCREEN (screen);

#ifdef USE_GLES
    GLint oldBlendSrcAlpha, oldBlendDstAlpha;
#endif

    if (rescaled)
	return;

    x      = pos.x () + window->x () - (window->border ().left * pos.scale);
    y      = pos.y () + window->y () - (window->border ().top * pos.scale);
    width  = geom.width () * pos.scale;
    height = geom.height () * pos.scale;

    /* we use a poor replacement for roundf()
     * (available in C99 only) here */
    x = floor (x + 0.5f);
    y = floor (y + 0.5f);

#ifdef USE_GLES
    glGetIntegerv (GL_BLEND_SRC_RGB, &oldBlendSrc);
    glGetIntegerv (GL_BLEND_DST_RGB, &oldBlendDst);
    glGetIntegerv (GL_BLEND_SRC_ALPHA, &oldBlendSrcAlpha);
    glGetIntegerv (GL_BLEND_DST_ALPHA, &oldBlendDstAlpha);
#else
    glGetIntegerv (GL_BLEND_SRC, &oldBlendSrc);
    glGetIntegerv (GL_BLEND_DST, &oldBlendDst);
#endif

    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    streamingBuffer->begin (GL_TRIANGLE_STRIP);

    colorData[0] = as->optionGetHighlightColorRed ();
    colorData[1] = as->optionGetHighlightColorGreen ();
    colorData[2] = as->optionGetHighlightColorBlue ();
    colorData[3] = as->optionGetHighlightColorAlpha ();

    streamingBuffer->addColors (1, colorData);

    vertexData[0]  = x;
    vertexData[1]  = y;
    vertexData[2]  = 0.0f;
    vertexData[3]  = x;
    vertexData[4]  = y + height;
    vertexData[5]  = 0.0f;
    vertexData[6]  = x + width;
    vertexData[7]  = y;
    vertexData[8]  = 0.0f;
    vertexData[9]  = x + width;
    vertexData[10] = y + height;
    vertexData[11] = 0.0f;

    streamingBuffer->addVertices (4, vertexData);

    streamingBuffer->end ();
    streamingBuffer->render (transform);

#ifdef USE_GLES
    glBlendFuncSeparate (oldBlendSrc, oldBlendDst,
                         oldBlendSrcAlpha, oldBlendDstAlpha);
#else
    glBlendFunc (oldBlendSrc, oldBlendDst);
#endif
}

void
ScaleAddonScreen::checkWindowHighlight ()
{
    if (highlightedWindow != lastHighlightedWindow)
    {
	CompWindow *w;

	w = screen->findWindow (highlightedWindow);
	if (w)
	{
	    ADDON_WINDOW (w);
	    aw->renderTitle ();
	    aw->cWindow->addDamage ();
	}

	w = screen->findWindow (lastHighlightedWindow);
	if (w)
	{
	    ADDON_WINDOW (w);
	    aw->renderTitle ();
	    aw->cWindow->addDamage (w);
	}

	lastHighlightedWindow = highlightedWindow;
    }
}

bool
ScaleAddonScreen::closeWindow (CompAction         *action,
			       CompAction::State  state,
			       CompOption::Vector options)
{
    CompWindow *w;

    if (!sScreen->hasGrab ())
	return false;

    w = screen->findWindow (highlightedWindow);
    if (w)
	w->close (screen->getCurrentTime ());

    return true;
}

bool
ScaleAddonScreen::pullWindow (CompAction         *action,
			      CompAction::State  state,
			      CompOption::Vector options)
{
    CompWindow *w;

    if (!sScreen->hasGrab ())
	return false;

    w = screen->findWindow (highlightedWindow);
    if (w)
    {
	int       x, y, xOffset, yOffset;
	CompPoint vp;

	vp = w->defaultViewport ();

	xOffset = (screen->vp ().x () - vp.x ()) * screen->width ();
	yOffset = (screen->vp ().y () - vp.y ()) * screen->height ();

	x = w->x () + xOffset;
	y = w->y () + yOffset;

	if (optionGetConstrainPullToScreen ())
	{
	    CompRect workArea, extents;

	    workArea = screen->outputDevs ()[w->outputDevice ()].workArea ();
	    extents  = w->borderRect ();

	    extents.setX (extents.x () + xOffset);
	    extents.setY (extents.y () + yOffset);

	    if (extents.x1 () < workArea.x1 ())
	        x += workArea.x1 () - extents.x1 ();
	    else if (extents.x2 () > workArea.x2 ())
	        x += workArea.x2 () - extents.x2 ();

	    if (extents.y1 () < workArea.y1 ())
	        y += workArea.y1 () - extents.y1 ();
	    else if (extents.y2 () > workArea.y2 ())
	        y += workArea.y2 () - extents.y2 ();
	}

	if (x != w->x () || y != w->y ())
	{
	    ScalePosition pos, oldPos;
	    ADDON_WINDOW (w);

	    oldPos = aw->sWindow->getCurrentPosition ();

	    w->moveToViewportPosition (x, y, true);

	    /* Select this window when ending scale */
	    aw->sWindow->scaleSelectWindow ();

	    /* stop scaled window dissapearing */
	    pos.setX (oldPos.x () - xOffset);
	    pos.setY (oldPos.y () - yOffset);

	    if (optionGetExitAfterPull ())
	    {
		CompAction         *action;
		CompOption::Vector o;
		CompOption         *opt;

		o.push_back (CompOption ("root", CompOption::TypeInt));
		o[0].value ().set ((int) screen->root ());

		opt = CompOption::findOption (sScreen->getOptions (),
					      "initiate_key", 0);
		action = &opt->value ().action ();

		if (action->terminate ())
		    action->terminate () (action, 0, o);
	    }
	    else
	    {
		ScaleSlot slot = aw->sWindow->getSlot ();

		/* provide a simple animation */
		aw->cWindow->addDamage ();

		pos.setX (oldPos.x () -  slot.width () / 20);
		pos.setY (oldPos.y () - slot.height () / 20);
		pos.scale = oldPos.scale * 1.1f;

		aw->sWindow->setCurrentPosition (pos);

		aw->cWindow->addDamage ();
	    }
	}
    }

    return true;
}

bool
ScaleAddonScreen::zoomWindow (CompAction         *action,
			      CompAction::State  state,
			      CompOption::Vector options)
{
    CompWindow *w;

    if (!sScreen->hasGrab ())
	return false;

    w = screen->findWindow (highlightedWindow);
    if (w)
    {
	CompRect output;
	int      head;

	ADDON_WINDOW (w);

	if (!aw->sWindow->hasSlot ())
	    return false;

	head   = screen->outputDeviceForPoint (aw->sWindow->getSlot ().pos ());
	output = screen->outputDevs ()[head];

	/* damage old rect */
	aw->cWindow->addDamage ();

	if (!aw->rescaled)
	{
	    ScaleSlot slot = aw->sWindow->getSlot ();
	    int       x1, x2, y1, y2;
	    CompRect  geom = w->borderRect ();

	    aw->oldAbove = w->next;
	    w->raise ();

	    /* backup old values */
	    aw->origSlot = slot;
	    aw->rescaled = true;

	    x1 = output.centerX () - geom.width () / 2 + w->border ().left;
	    y1 = output.centerY () - geom.height () / 2 + w->border ().top;
	    x2 = slot.x () + geom.width ();
	    y2 = slot.y () + geom.height ();

	    slot.scale = 1.0f;
	    slot.setGeometry (x1, y1, x2 - x1, y2 - y1);

	    aw->sWindow->setSlot (slot);
	}
	else
	{
	    if (aw->oldAbove)
	        w->restackBelow (aw->oldAbove);

	    aw->rescaled = false;
	    aw->sWindow->setSlot (aw->origSlot);
	}

	/* slot size may have changed, so
	 * update window title */
	aw->renderTitle ();

	aw->cWindow->addDamage ();
    }

    return true;
}

void
ScaleAddonScreen::handleEvent (XEvent *event)
{
    screen->handleEvent (event);

    switch (event->type)
    {
    case PropertyNotify:
	if (event->xproperty.atom == XA_WM_NAME && sScreen->hasGrab ())
	{
	    CompWindow *w;

	    w = screen->findWindow (event->xproperty.window);
	    if (w)
	    {
		ADDON_WINDOW (w);
		aw->renderTitle ();
		aw->cWindow->addDamage ();
	    }
	}
	break;
    case MotionNotify:
	if (sScreen->hasGrab ())
	{
	    highlightedWindow = sScreen->getHoveredWindow ();
	    checkWindowHighlight ();
	}
	break;
    default:
	break;
    }
}

void
ScaleAddonWindow::scalePaintDecoration (const GLWindowPaintAttrib &attrib,
				        const GLMatrix		  &transform,
					const CompRegion          &region,
					unsigned int		  mask)
{
    ScaleScreen::State state;

    ADDON_SCREEN (screen);

    state = as->sScreen->getState ();
    sWindow->scalePaintDecoration (attrib, transform, region, mask);

    if (state == ScaleScreen::Wait || state == ScaleScreen::Out)
    {
	if (as->optionGetWindowHighlight ())
	{
	    if (window->id () == as->highlightedWindow)
		drawHighlight (transform);
	}

	if (textAvailable)
	    drawTitle (transform);
    }
}

void
ScaleAddonWindow::scaleSelectWindow ()
{
    ADDON_SCREEN (screen);

    as->highlightedWindow = window->id ();
    as->checkWindowHighlight ();

    sWindow->scaleSelectWindow ();
}

void
ScaleAddonScreen::donePaint ()
{
    ScaleScreen::State state = sScreen->getState ();

    if (state != ScaleScreen::Idle && lastState == ScaleScreen::Idle)
    {
	foreach (CompWindow *w, screen->windows ())
	    ScaleAddonWindow::get (w)->renderTitle ();
    }
    else if (state == ScaleScreen::Idle && lastState != ScaleScreen::Idle)
    {
	foreach (CompWindow *w, screen->windows ())
	    ScaleAddonWindow::get (w)->text.clear ();
    }

    if (state == ScaleScreen::Out && lastState != ScaleScreen::Out)
    {
	lastHighlightedWindow = None;
	checkWindowHighlight ();
    }

    lastState = state;

    cScreen->donePaint ();
}

void
ScaleAddonScreen::handleCompizEvent (const char         *pluginName,
				     const char         *eventName,
				     CompOption::Vector &options)
{
    screen->handleCompizEvent (pluginName, eventName, options);

    if ((strcmp (pluginName, "scale") == 0) &&
	(strcmp (eventName, "activate") == 0))
    {
	bool activated =
	    CompOption::getBoolOptionNamed (options, "active", false);

	if (activated)
	{
	    screen->addAction (&optionGetCloseKey ());
	    screen->addAction (&optionGetZoomKey ());
	    screen->addAction (&optionGetPullKey ());
	    screen->addAction (&optionGetCloseButton ());
	    screen->addAction (&optionGetZoomButton ());
	    screen->addAction (&optionGetPullButton ());

	    /* TODO: or better
	       ad->highlightedWindow     = sd->selectedWindow;
	       here? do we want to show up the highlight without
	       mouse move initially? */

	    highlightedWindow     = None;
	    lastHighlightedWindow = None;
	    checkWindowHighlight ();
	}
	else
	{
	    foreach (CompWindow *w, screen->windows ())
	    {
		ADDON_WINDOW (w);
		aw->rescaled = false;
	    }

	    screen->removeAction (&optionGetCloseKey ());
	    screen->removeAction (&optionGetZoomKey ());
	    screen->removeAction (&optionGetPullKey ());
	    screen->removeAction (&optionGetCloseButton ());
	    screen->removeAction (&optionGetZoomButton ());
	    screen->removeAction (&optionGetPullButton ());
	}
    }
}

/**
 * experimental organic layout method
 * inspired by smallwindows (smallwindows.sf.net) by Jens Egeblad
 * FIXME: broken.
 * */
#if 0
static const double ORGANIC_STEP = 0.05f;
static int
organicCompareWindows (const void *elem1,
		       const void *elem2)
{
    CompWindow *w1 = *((CompWindow **) elem1);
    CompWindow *w2 = *((CompWindow **) elem2);

    return (WIN_X (w1) + WIN_Y (w1)) - (WIN_X (w2) + WIN_Y (w2));
}

static double
layoutOrganicCalculateOverlap (CompScreen *s,
			       int        win,
			       int        x,
			       int        y)
{
    int    i;
    int    x1, y1, x2, y2;
    int    overlapX, overlapY;
    int    xMin, xMax, yMin, yMax;
    double result = -0.01;

    SCALE_SCREEN ();
    ADDON_SCREEN ();

    x1 = x;
    y1 = y;
    x2 = x1 + WIN_W (ss->windows[win]) * as->scale;
    y2 = y1 + WIN_H (ss->windows[win]) * as->scale;

    for (i = 0; i < ss->nWindows; i++)
    {
	if (i == win)
	    continue;

	overlapX = overlapY = 0;
	xMax = MAX (ss->slots[i].x1, x1);
	xMin = MIN (ss->slots[i].x1 + WIN_W (ss->windows[i]) * as->scale, x2);
	if (xMax <= xMin)
	    overlapX = xMin - xMax;

	yMax = MAX (ss->slots[i].y1, y1);
	yMin = MIN (ss->slots[i].y1 + WIN_H (ss->windows[i]) * as->scale, y2);

	if (yMax <= yMin)
	    overlapY = yMin - yMax;

	result += (double)overlapX * overlapY;
    }

    return result;
}

static double
layoutOrganicFindBestHorizontalPosition (CompScreen *s,
					 int        win,
					 int        *bestX,
					 int        areaWidth)
{
    int    i, y1, y2, w;
    double bestOverlap = 1e31, overlap;

    SCALE_SCREEN ();
    ADDON_SCREEN ();

    y1 = ss->slots[win].y1;
    y2 = ss->slots[win].y1 + WIN_H (ss->windows[win]) * as->scale;

    w = WIN_W (ss->windows[win]) * as->scale;
    *bestX = ss->slots[win].x1;

    for (i = 0; i < ss->nWindows; i++)
    {
	CompWindow *lw = ss->windows[i];
	if (i == win)
	    continue;

	if (ss->slots[i].y1 < y2 &&
	    ss->slots[i].y1 + WIN_H (lw) * as->scale > y1)
	{
	    if (ss->slots[i].x1 - w >= 0)
	    {
		double overlap;
		
		overlap = layoutOrganicCalculateOverlap (s, win,
		 					 ss->slots[i].x1 - w,
							 y1);

		if (overlap < bestOverlap)
		{
		    *bestX = ss->slots[i].x1 - w;
		    bestOverlap = overlap;
		}
	    }
	    if (WIN_W (lw) * as->scale + ss->slots[i].x1 + w < areaWidth)
	    {
		double overlap;
		
		overlap = layoutOrganicCalculateOverlap (s, win,
		 					 ss->slots[i].x1 +
		 					 WIN_W (lw) * as->scale,
		 					 y1);

		if (overlap < bestOverlap)
		{
		    *bestX = ss->slots[i].x1 + WIN_W (lw) * as->scale;
		    bestOverlap = overlap;
		}
	    }
	}
    }

    overlap = layoutOrganicCalculateOverlap (s, win, 0, y1);
    if (overlap < bestOverlap)
    {
	*bestX = 0;
	bestOverlap = overlap;
    }

    overlap = layoutOrganicCalculateOverlap (s, win, areaWidth - w, y1);
    if (overlap < bestOverlap)
    {
	*bestX = areaWidth - w;
	bestOverlap = overlap;
    }

    return bestOverlap;
}

static double
layoutOrganicFindBestVerticalPosition (CompScreen *s,
				       int        win,
				       int        *bestY,
				       int        areaHeight)
{
    int    i, x1, x2, h;
    double bestOverlap = 1e31, overlap;

    SCALE_SCREEN ();
    ADDON_SCREEN ();

    x1 = ss->slots[win].x1;
    x2 = ss->slots[win].x1 + WIN_W (ss->windows[win]) * as->scale;
    h = WIN_H (ss->windows[win]) * as->scale;
    *bestY = ss->slots[win].y1;

    for (i = 0; i < ss->nWindows; i++)
    {
	CompWindow *w = ss->windows[i];

	if (i == win)
	    continue;

	if (ss->slots[i].x1 < x2 &&
	    ss->slots[i].x1 + WIN_W (w) * as->scale > x1)
	{
	    if (ss->slots[i].y1 - h >= 0 && ss->slots[i].y1 < areaHeight)
	    {
		double overlap;
		overlap = layoutOrganicCalculateOverlap (s, win, x1,
	 						 ss->slots[i].y1 - h);
		if (overlap < bestOverlap)
		{
		    *bestY = ss->slots[i].y1 - h;
		    bestOverlap = overlap;
		}
	    }
	    if (WIN_H (w) * as->scale + ss->slots[i].y1 > 0 &&
		WIN_H (w) * as->scale + h + ss->slots[i].y1 < areaHeight)
	    {
		double overlap;
		
		overlap = layoutOrganicCalculateOverlap (s, win, x1,
		 					 WIN_H (w) * as->scale +
							 ss->slots[i].y1);

		if (overlap < bestOverlap)
		{
		    *bestY = ss->slots[i].y1 + WIN_H(w) * as->scale;
		    bestOverlap = overlap;
		}
	    }
	}
    }

    overlap = layoutOrganicCalculateOverlap (s, win, x1, 0);
    if (overlap < bestOverlap)
    {
	*bestY = 0;
	bestOverlap = overlap;
    }

    overlap = layoutOrganicCalculateOverlap (s, win, x1, areaHeight - h);
    if (overlap < bestOverlap)
    {
	*bestY = areaHeight - h;
	bestOverlap = overlap;
    }

    return bestOverlap;
}

static bool
layoutOrganicLocalSearch (CompScreen *s,
			  int        areaWidth,
			  int        areaHeight)
{
    bool   improvement;
    int    i;
    double totalOverlap;

    SCALE_SCREEN ();

    do
    {
	improvement = false;
	for (i = 0; i < ss->nWindows; i++)
	{
	    bool improved;

	    do
	    {
		int    newX, newY;
		double oldOverlap, overlapH, overlapV;

		improved = false;
		oldOverlap = layoutOrganicCalculateOverlap (s, i,
 							    ss->slots[i].x1,
							    ss->slots[i].y1);

		overlapH = layoutOrganicFindBestHorizontalPosition (s, i,
								    &newX,
								    areaWidth);
		overlapV = layoutOrganicFindBestVerticalPosition (s, i,
								  &newY,
								  areaHeight);

		if (overlapH < oldOverlap - 0.1 ||
		    overlapV < oldOverlap - 0.1)
		{
		    improved = true;
		    improvement = true;
		    if (overlapV > overlapH)
			ss->slots[i].x1 = newX;
		    else
			ss->slots[i].y1 = newY;
		}
    	    }
	    while (improved);
	}
    }
    while (improvement);

    totalOverlap = 0.0;
    for (i = 0; i < ss->nWindows; i++)
    {
	totalOverlap += layoutOrganicCalculateOverlap (s, i,
						       ss->slots[i].x1,
						       ss->slots[i].y1);
    }

    return (totalOverlap > 0.1);
}

static void
layoutOrganicRemoveOverlap (CompScreen *s,
			    int        areaWidth,
			    int        areaHeight)
{
    int        i, spacing;
    CompWindow *w;

    SCALE_SCREEN ();
    ADDON_SCREEN ();

    spacing = ss->opt[SCALE_SCREEN_OPTION_SPACING].value.i;

    while (layoutOrganicLocalSearch (s, areaWidth, areaHeight))
    {
	for (i = 0; i < ss->nWindows; i++)
	{
	    int centerX, centerY;
	    int newX, newY, newWidth, newHeight;

	    w = ss->windows[i];

	    centerX = ss->slots[i].x1 + WIN_W (w) / 2;
	    centerY = ss->slots[i].y1 + WIN_H (w) / 2;

	    newWidth = (int)((1.0 - ORGANIC_STEP) *
			     (double)WIN_W (w)) - spacing / 2;
	    newHeight = (int)((1.0 - ORGANIC_STEP) *
			      (double)WIN_H (w)) - spacing / 2;
	    newX = centerX - (newWidth / 2);
	    newY = centerY - (newHeight / 2);

	    ss->slots[i].x1 = newX;
	    ss->slots[i].y1 = newY;
	    ss->slots[i].x2 = newX + WIN_W (w);
	    ss->slots[i].y2 = newY + WIN_H (w);
	}
	as->scale -= ORGANIC_STEP;
    }
}

static bool
layoutOrganicThumbs (CompScreen *s)
{
    CompWindow *w;
    int        i, moMode;
    XRectangle workArea;

    SCALE_SCREEN ();
    ADDON_SCREEN ();

    moMode = ss->opt[SCALE_SCREEN_OPTION_MULTIOUTPUT_MODE].value.i;

    switch (moMode) {
    case SCALE_MOMODE_ALL:
	workArea = s->workArea;
	break;
    case SCALE_MOMODE_CURRENT:
    default:
	workArea = s->outputDev[s->currentOutputDev].workArea;
	break;
    }

    as->scale = 1.0f;

    qsort (ss->windows, ss->nWindows, sizeof(CompWindow *),
	   organicCompareWindows);

    for (i = 0; i < ss->nWindows; i++)
    {
	w = ss->windows[i];
	SCALE_WINDOW (w);

	sWindow->slot = &ss->slots[i];
	ss->slots[i].x1 = WIN_X (w) - workArea.x;
	ss->slots[i].y1 = WIN_Y (w) - workArea.y;
	ss->slots[i].x2 = WIN_X (w) + WIN_W (w) - workArea.x;
	ss->slots[i].y2 = WIN_Y (w) + WIN_H (w) - workArea.y;

	if (ss->slots[i].x1 < 0)
	{
	    ss->slots[i].x2 += abs (ss->slots[i].x1);
	    ss->slots[i].x1 = 0;
	}
	if (ss->slots[i].x2 > workArea.width - workArea.x)
	{
	    ss->slots[i].x1 -= abs (ss->slots[i].x2 - workArea.width);
	    ss->slots[i].x2 = workArea.width - workArea.x;
	}

	if (ss->slots[i].y1 < 0)
	{
	    ss->slots[i].y2 += abs (ss->slots[i].y1);
	    ss->slots[i].y1 = 0;
	}
	if (ss->slots[i].y2 > workArea.height - workArea.y)
	{
	    ss->slots[i].y1 -= abs (ss->slots[i].y2 -
				    workArea.height - workArea.y);
	    ss->slots[i].y2 = workArea.height - workArea.y;
	}
    }

    ss->nSlots = ss->nWindows;

    layoutOrganicRemoveOverlap (s, workArea.width - workArea.x,
				workArea.height - workArea.y);
    for (i = 0; i < ss->nWindows; i++)
    {
	w = ss->windows[i];
	SCALE_WINDOW (w);

	if (ss->type == ScaleTypeGroup)
	    raiseWindow (ss->windows[i]);

	ss->slots[i].x1 += w->input.left + workArea.x;
	ss->slots[i].x2 += w->input.left + workArea.x;
	ss->slots[i].y1 += w->input.top + workArea.y;
	ss->slots[i].y2 += w->input.top + workArea.y;
	sWindow->adjust = true;
    }

    return true;
}

#endif

/*
 * Inspired by KWin - the KDE Window Manager
 * presentwindows.cpp
 * Copyright (C) 2007 Rivo Laks <rivolaks@hot.ee>
 * Copyright (C) 2008 Lucas Murray <lmurray@undefinedfire.com>
 * 
 */

bool
ScaleAddonScreen::isOverlappingAny (ScaleWindow *w,
				    std::map <ScaleWindow *, CompRegion> targets,
				    const CompRegion &border)
{
    if (border.intersects (targets[w]))
	return true;
    // Is there a better way to do this?
    std::map <ScaleWindow *, CompRegion>::const_iterator i;
    for (i = targets.begin (); i != targets.end (); ++i)
    {
	if (w == (*i).first)
	    continue;
	if (targets[w].intersects ((*i).second))
	    return true;
    }
    return false;
}

bool
ScaleAddonScreen::layoutNaturalThumbs ()
{
    ScaleScreen::WindowList windows = ScaleScreen::get (screen)->getWindows ();
    bool overlapping;
    CompRect area = screen->workArea ();
    CompRect bounds = area;
    std::map <ScaleWindow *, CompRegion> targets;
    std::map <ScaleWindow *, int> directions;
    int				  direction = 0;
    int				  iterCount = 0;

    if (windows.size () == 1)
    {
	// Just move the window to its original location to save time
	if (screen->fullscreenOutput ().workArea ().contains (windows.front ()->window->geometry ()))
	{
	    ScaleSlot slot ((CompRect &) windows.front ()->window->geometry ());
	    windows.front ()->setSlot (slot);
	    return true;
	}
    }

    foreach (ScaleWindow *w, windows)
    {
        bounds = CompRegion (bounds).united (w->window->outputRect ()).boundingRect ();
        targets[w] = CompRegion (w->window->outputRect ());
	// Reuse the unused "slot" as a preferred direction attribute. This is used when the window
	// is on the edge of the screen to try to use as much screen real estate as possible.
	directions[w] = direction;
	direction++;
	if (direction == 4)
	    direction = 0;
    }
        
    do
    {
	overlapping = false;
	foreach (ScaleWindow *w, windows)
	{
	    foreach (ScaleWindow *e, windows)
	    {
		if (e->window->id () != w->window->id () && targets[w].intersects (targets[e]))
		{
		    int moveX = targets[w].boundingRect ().centerX () - targets[e].boundingRect ().centerX ();
		    int moveY = targets[w].boundingRect ().centerY () - targets[e].boundingRect ().centerY ();
		    //int xSection, ySection;
		    // Overlap detected, determine direction to push
		    
		    overlapping = true;

		    moveX /= optionGetNaturalPrecision ();
		    moveY /= optionGetNaturalPrecision ();
		    
		    /* Force movement */
		    if (moveX == 0)
			moveX = optionGetNaturalPrecision ();
		    if (moveY == 0)
			moveY = optionGetNaturalPrecision ();
		    
		    targets[w] = targets[w].translated (moveX, moveY);
		    targets[e] = targets[e].translated (-moveX, -moveY);
		    
		    /* Try to keep the bounding rect the same aspect as the screen so that more
		     * screen real estate is utilised. We do this by splitting the screen into nine
		     * equal sections, if the window center is in any of the corner sections pull the
		     * window towards the outer corner. If it is in any of the other edge sections
		     * alternate between each corner on that edge. We don't want to determine it
		     * randomly as it will not produce consistant locations when using the filter.
		     * Only move one window so we don't cause large amounts of unnecessary zooming
		     * in some situations. We need to do this even when expanding later just in case
		     * all windows are the same size.
		     * (We are using an old bounding rect for this, hopefully it doesn't matter)
		     * FIXME: Disabled for now
		     *
		    xSection = (targets[w].boundingRect ().x () - bounds.x ()) / (bounds.width () / 3);
		    ySection = (targets[w].boundingRect ().y () - bounds.y ()) / (bounds.height () / 3);
		    moveX = 0;
		    moveY = 0;
		    if (xSection != 1 || ySection != 1) // Remove this if you want the center to pull as well
		    {
			if (xSection == 1)
			    xSection = (directions[w] / 2 ? 2 : 0);
			if (ySection == 1)
			    ySection = (directions[w] % 2 ? 2 : 0);
		    }
		    
                    if (xSection == 0 && ySection == 0)
		    {
			moveX = bounds.left () - targets[w].boundingRect ().centerX ();
			moveY = bounds.top () - targets[w].boundingRect ().centerY ();
		    }
                    if (xSection == 2 && ySection == 0)
		    {
			moveX = bounds.right () - targets[w].boundingRect ().centerX ();
			moveY = bounds.top () - targets[w].boundingRect ().centerY ();
		    }
                    if (xSection == 2 && ySection == 2)
		    {
			moveX = bounds.right () - targets[w].boundingRect ().centerX ();
			moveY = bounds.bottom () - targets[w].boundingRect ().centerY ();
		    }
                    if (xSection == 0 && ySection == 2)
		    {
			moveX = bounds.left () - targets[w].boundingRect ().centerX ();
			moveY = bounds.right () - targets[w].boundingRect ().centerY ();
		    }
                    if (moveX != 0 || moveY != 0)
                        targets[w].translate (moveX, moveY);
		    */
		}
		
		// Update bounding rect
		bounds = CompRegion (bounds).united (targets[w]).boundingRect ();
		bounds = CompRegion (bounds).united (targets[e]).boundingRect ();
	    }
	}
    }
    while (overlapping);

    // Work out scaling by getting the most top-left and most bottom-right window coords.
    // The 20's and 10's are so that the windows don't touch the edge of the screen.
    double scale;
    if (bounds == area)
        scale = 1.0; // Don't add borders to the screen
    else if (area.width () / double (bounds.width ()) < area.height () / double (bounds.height ()))
        scale = (area.width () - 20) / double (bounds.width ());
    else
        scale = (area.height () - 20) / double (bounds.height ());
    // Make bounding rect fill the screen size for later steps
    bounds = CompRect (
        bounds.x () - (area.width () - 20 - bounds.width () * scale ) / 2 - 10 / scale,
        bounds.y () - (area.height () - 20 - bounds.height () * scale ) / 2 - 10 / scale,
        area.width () / scale,
        area.height () / scale
        );
    
    // Move all windows back onto the screen and set their scale
    foreach (ScaleWindow *w, windows)
    {
        targets[w] = CompRect (
            (targets[w].boundingRect ().x () - bounds.x () ) * scale + area.x (),
	    (targets[w].boundingRect ().y () - bounds.y ()) * scale + area.y (),
	    targets[w].boundingRect ().width () * scale,
	    targets[w].boundingRect ().height () * scale
            );
	ScaleSlot slt (targets[w].boundingRect ());
	slt.scale = scale;
	slt.filled = true;
	
	w->setSlot (slt);
    }

    // Don't expand onto or over the border
    CompRegion borderRegion = CompRegion (area);
    CompRegion areaRegion = CompRegion (area);
    borderRegion.translate (-200, -200);
    borderRegion.shrink (-200, -200); // actually expands the region
    areaRegion.translate (10 / scale, 10 / scale);
    areaRegion.shrink (10 / scale, 10 / scale);
    
    borderRegion ^= areaRegion;

    bool moved = false;
    do
    {
	moved = false;
	foreach (ScaleWindow *w, windows)
	{
	    CompRegion oldRegion;
	    
	    // This may cause some slight distortion if the windows are enlarged a large amount
	    int widthDiff = optionGetNaturalPrecision ();
	    int heightDiff = ((w->window->height () / w->window->width ()) * 
	    (targets[w].boundingRect ().width() + widthDiff)) - targets[w].boundingRect ().height ();
	    int xDiff = widthDiff / 2;  // Also move a bit in the direction of the enlarge, allows the
	    int yDiff = heightDiff / 2; // center windows to be enlarged if there is gaps on the side.
	    
	    // Attempt enlarging to the top-right
	    oldRegion = targets[w];
	    targets[w] = CompRegion (
				     targets[w].boundingRect ().x () + xDiff,
				     targets[w].boundingRect ().y () - yDiff - heightDiff,
				     targets[w].boundingRect ().width () + widthDiff,
				     targets[w].boundingRect ().height () + heightDiff
					);
	    if (isOverlappingAny (w, targets, borderRegion))
		targets[w] = oldRegion;
	    else
		moved = true;
	    
	    // Attempt enlarging to the bottom-right
	    oldRegion = targets[w];
	    targets[w] = CompRegion(
				    targets[w].boundingRect ().x () + xDiff,
				    targets[w].boundingRect ().y () + yDiff,
				    targets[w].boundingRect ().width () + widthDiff,
				    targets[w].boundingRect ().height () + heightDiff
				    );
	    if (isOverlappingAny (w, targets, borderRegion))
		targets[w] = oldRegion;
	    else
		moved = true;
		
	    // Attempt enlarging to the bottom-left
	    oldRegion = targets[w];
	    targets[w] = CompRegion (
				    targets[w].boundingRect ().x() - xDiff - widthDiff,
				    targets[w].boundingRect ().y() + yDiff,
				    targets[w].boundingRect ().width() + widthDiff,
				    targets[w].boundingRect ().height() + heightDiff
				    );
	    if (isOverlappingAny (w, targets, borderRegion))
		targets[w] = oldRegion;
	    else
		moved = true;
		    
	    // Attempt enlarging to the top-left
	    oldRegion = targets[w];
	    targets[w] = CompRegion (
				    targets[w].boundingRect ().x() - xDiff - widthDiff,
				    targets[w].boundingRect ().y() - yDiff - heightDiff,
				    targets[w].boundingRect ().width() + widthDiff,
				    targets[w].boundingRect ().height() + heightDiff
				    );
	    if (isOverlappingAny (w, targets, borderRegion))
		targets[w] = oldRegion;
	    else
		moved = true;
	}
	
	iterCount++;
    }
    while (moved && iterCount < 100);

    // The expanding code above can actually enlarge windows over 1.0/2.0 scale, we don't like this
    // We can't add this to the loop above as it would cause a never-ending loop so we have to make
    // do with the less-than-optimal space usage with using this method.
    foreach (ScaleWindow *w, windows)
    {
	double scale = targets[w].boundingRect ().width() / double( w->window->width());
	if (scale > 2.0 || (scale > 1.0 && (w->window->width() > 300 || w->window->height() > 300)))
	{
	    scale = (w->window->width () > 300 || w->window->height () > 300) ? 1.0 : 2.0;
	    targets[w] = CompRegion (
				    targets[w].boundingRect ().center().x() - int (w->window->width() * scale) / 2,
				    targets[w].boundingRect ().center().y() - int (w->window->height () * scale) / 2,
				    w->window->width() * scale,
				    w->window->height() * scale
				    );
	}
    }

    return true;

}

bool
ScaleAddonScreen::layoutSlotsAndAssignWindows ()
{
    bool status;

    switch (optionGetLayoutMode ())
    {
    case LayoutModeNatural:
	status = layoutNaturalThumbs ();
	break;
    case LayoutModeNormal:
    default:
	status = sScreen->layoutSlotsAndAssignWindows ();
	break;
    }

    return status;
}

void
ScaleAddonScreen::optionChanged (CompOption                 *opt,
				 ScaleaddonOptions::Options num)
{
    switch (num)
    {
	case ScaleaddonOptions::WindowTitle:
	case ScaleaddonOptions::TitleBold:
	case ScaleaddonOptions::TitleSize:
	case ScaleaddonOptions::BorderSize:
	case ScaleaddonOptions::FontColor:
	case ScaleaddonOptions::BackColor:
	    if (textAvailable)
	    {
		foreach (CompWindow *w, screen->windows ())
		{
		    ADDON_WINDOW (w);
		    aw->renderTitle ();
		}
	    }
	    break;
	default:
	    break;
    }
}

ScaleAddonScreen::ScaleAddonScreen (CompScreen *) :
    PluginClassHandler <ScaleAddonScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    sScreen (ScaleScreen::get (screen)),
    highlightedWindow (0),
    lastHighlightedWindow (0),
    lastState (ScaleScreen::Idle),
    scale (1.0f)
{
    CompAction::CallBack cb;
    ChangeNotify         notify;

    ScreenInterface::setHandler (screen, true);
    CompositeScreenInterface::setHandler (cScreen, true);
    ScaleScreenInterface::setHandler (sScreen, true);

    cb = boost::bind (&ScaleAddonScreen::closeWindow, this, _1, _2, _3);
    optionSetCloseKeyInitiate (cb);
    optionSetCloseButtonInitiate (cb);

    cb = boost::bind (&ScaleAddonScreen::zoomWindow, this, _1, _2, _3);
    optionSetZoomKeyInitiate (cb);
    optionSetZoomButtonInitiate (cb);

    cb = boost::bind (&ScaleAddonScreen::pullWindow, this, _1, _2, _3);
    optionSetPullKeyInitiate (cb);
    optionSetPullButtonInitiate (cb);

    notify = boost::bind (&ScaleAddonScreen::optionChanged, this, _1, _2);
    optionSetWindowTitleNotify (notify);
    optionSetTitleBoldNotify (notify);
    optionSetTitleSizeNotify (notify);
    optionSetBorderSizeNotify (notify);
    optionSetFontColorNotify (notify);
    optionSetBackColorNotify (notify);
}

ScaleAddonWindow::ScaleAddonWindow (CompWindow *window) :
    PluginClassHandler <ScaleAddonWindow, CompWindow> (window),
    window (window),
    sWindow (ScaleWindow::get (window)),
    cWindow (CompositeWindow::get (window)),
    rescaled (false),
    oldAbove (NULL)
{
    ScaleWindowInterface::setHandler (sWindow);
}

bool
ScaleAddonPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
	!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI) ||
	!CompPlugin::checkPluginABI ("scale", COMPIZ_SCALE_ABI))
	return false;

    if (!CompPlugin::checkPluginABI ("text", COMPIZ_TEXT_ABI))
    {
	compLogMessage ("scaleaddon", CompLogLevelInfo,
			"Text Plugin not loaded, no text will be drawn.");
	textAvailable = false;
    }
    else
	textAvailable = true;

    return true;
}
