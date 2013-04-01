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
#include <sys/time.h>

#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include <core/atoms.h>
#include <scale/scale.h>
#include "privates.h"

#define EDGE_STATE (CompAction::StateInitEdge)

class ScalePluginVTable :
    public CompPlugin::VTableForScreenAndWindow<ScaleScreen, ScaleWindow>
{
    public:

	bool init ();
	void fini ();
};

COMPIZ_PLUGIN_20090315 (scale, ScalePluginVTable)

bool
PrivateScaleWindow::isNeverScaleWin () const
{
    if (window->overrideRedirect ())
	return true;

    if (window->wmType () & (CompWindowTypeDockMask |
			     CompWindowTypeDesktopMask))
	return true;

    return false;
}

bool
PrivateScaleWindow::isScaleWin () const
{
    if (isNeverScaleWin ())
	return false;

    if (!spScreen->type || spScreen->type == ScaleTypeOutput)
    {
	if (!window->focus ())
	    return false;
    }

    if (window->state () & CompWindowStateSkipPagerMask)
	return false;

    if (window->state () & CompWindowStateShadedMask)
	return false;

    if (!window->mapNum () || !window->isViewable ())
	return false;

    switch (sScreen->priv->type) {
	case ScaleTypeGroup:
	    if (spScreen->clientLeader != window->clientLeader () &&
		spScreen->clientLeader != window->id ())
		return false;
	    break;
	case ScaleTypeOutput:
	    if ((unsigned int) window->outputDevice () !=
	    		       (unsigned int) screen->currentOutputDev ().id ())
		return false;
	default:
	    break;
    }

    if (!spScreen->currentMatch.evaluate (window))
	return false;

    return true;
}

void
PrivateScaleScreen::activateEvent (bool activating)
{
    CompOption::Vector o (0);

    o.push_back (CompOption ("root", CompOption::TypeInt));
    o.push_back (CompOption ("active", CompOption::TypeBool));

    o[0].value ().set ((int) screen->root ());
    o[1].value ().set (activating);

    screen->handleCompizEvent ("scale", "activate", o);
}

void
ScaleWindowInterface::scalePaintDecoration (const GLWindowPaintAttrib& attrib,
					    const GLMatrix&           transform,
					    const CompRegion&         region,
					    unsigned int              mask)
    WRAPABLE_DEF (scalePaintDecoration, attrib, transform, region, mask)

void
ScaleWindow::scalePaintDecoration (const GLWindowPaintAttrib& attrib,
				   const GLMatrix&            transform,
				   const CompRegion&          region,
				   unsigned int               mask)
{
    WRAPABLE_HND_FUNCTN (scalePaintDecoration, attrib, transform, region, mask)

    if (priv->spScreen->optionGetOverlayIcon () != ScaleOptions::OverlayIconNone)
    {
	GLWindowPaintAttrib sAttrib (attrib);
	GLTexture           *icon;

	icon = priv->gWindow->getIcon (96, 96);
	if (!icon)
	    icon = priv->spScreen->gScreen->defaultIcon ();

	if (icon)
	{
	    float  scale;
	    float  x, y;
	    int    width, height;
	    int    scaledWinWidth, scaledWinHeight;

	    scaledWinWidth  = priv->window->width () * priv->scale;
	    scaledWinHeight = priv->window->height () * priv->scale;

	    switch (priv->spScreen->optionGetOverlayIcon ()) {
		case ScaleOptions::OverlayIconNone:
		case ScaleOptions::OverlayIconEmblem:
		    scale = 1.0f;
		    break;
		case ScaleOptions::OverlayIconBig:
		default:
		    sAttrib.opacity /= 3;
		    scale = MIN (((float) scaledWinWidth / icon->width ()),
				 ((float) scaledWinHeight / icon->height ()));
		    break;
	    }

	    width  = icon->width () * scale;
	    height = icon->height () * scale;

	    switch (priv->spScreen->optionGetOverlayIcon ()) {
		case ScaleOptions::OverlayIconNone:
		case ScaleOptions::OverlayIconEmblem:
		    x = priv->window->x () + scaledWinWidth - icon->width ();
		    y = priv->window->y () + scaledWinHeight - icon->height ();
		    break;
		case ScaleOptions::OverlayIconBig:
		default:
		    x = priv->window->x () + scaledWinWidth / 2 - width / 2;
		    y = priv->window->y () + scaledWinHeight / 2 - height / 2;
		    break;
	    }

	    x += priv->tx;
	    y += priv->ty;

	    if (priv->slot)
	    {
		priv->delta = fabs (priv->slot->x1 () - priv->window->x ()) +
			      fabs (priv->slot->y1 () - priv->window->y ()) +
			      fabs (1.0f - priv->slot->scale) * 500.0f;
	    }

	    if (priv->delta)
	    {
		float o;
		float ds;

		ds = fabs (priv->tx) +
		     fabs (priv->ty) +
		     fabs (1.0f - priv->scale) * 500.0f;

		if (ds > priv->delta)
		    ds = priv->delta;

		o = ds / priv->delta;

		if (priv->slot)
		{
		    if (o < priv->lastThumbOpacity)
			o = priv->lastThumbOpacity;
		}
		else
		{
		    if (o > priv->lastThumbOpacity)
			o = 0.0f;
		}

		priv->lastThumbOpacity = o;

		sAttrib.opacity = sAttrib.opacity * o;
	    }

	    mask |= PAINT_WINDOW_BLEND_MASK;

	    CompRegion            iconReg (0, 0, width, height);
	    GLTexture::MatrixList ml (1);

	    ml[0] = icon->matrix ();
	    priv->gWindow->vertexBuffer ()->begin ();

	    if (width && height)
		priv->gWindow->glAddGeometry (ml, iconReg, iconReg);

	    if (priv->gWindow->vertexBuffer ()->end ())
	    {
		GLMatrix           wTransform (transform);

		wTransform.scale (scale, scale, 1.0f);
		wTransform.translate (x / scale, y / scale, 0.0f);

		priv->gWindow->glDrawTexture (icon, wTransform, sAttrib, mask);
	    }
	}
    }
}

bool
ScaleWindowInterface::setScaledPaintAttributes (GLWindowPaintAttrib& attrib)
    WRAPABLE_DEF (setScaledPaintAttributes, attrib)

bool
ScaleWindow::setScaledPaintAttributes (GLWindowPaintAttrib& attrib)
{
    WRAPABLE_HND_FUNCTN_RETURN (bool, setScaledPaintAttributes, attrib)

    bool drawScaled = false;

    /* Windows that wouldn't be visible before and after entering
     * scale mode (because some plugin modified CompWindow::focus)
     * should be faded in and out */
    if (window->state () & CompWindowStateHiddenMask)
    {
	GLfloat factor = 0;
	GLfloat targetX, targetY, targetScale;
	GLfloat scaleFactor, xFactor, yFactor, divFactor = 3.0f;

	if (priv->slot)
	{
	    targetX = priv->slot->x ();
	    targetY = priv->slot->y ();
	    targetScale = priv->slot->scale;
	}
	else
	{
	    targetX = priv->lastTargetX;
	    targetY = priv->lastTargetY;
	    targetScale = priv->lastTargetScale;
	}

	/* Don't FDIV0 */
	if (targetScale - priv->scale == 0.0f)
	{
	    divFactor -= 1.0f;
	    scaleFactor = 1.0f;
	}
	else
	    scaleFactor = (1.0f - priv->scale) / (1.0f - targetScale);

	if (targetX - ((float) window->x () + priv->tx) == 0.0f)
	{
	    divFactor -= 1.0f;
	    xFactor = 1.0f;
	}
	else
	{
	    float distActual = fabsf (window->x () - ((float) window->x () + priv->tx));
	    float distTarget = fabsf (window->x () - targetX);

	    xFactor = distActual / distTarget;
	}

	if (targetY - ((float) window->y () + priv->ty) == 0.0f)
	{
	    divFactor -= 1.0f;
	    yFactor = 1.0f;
	}
	else
	{
	    float distActual = fabsf (window->y () - ((float) window->y () + priv->ty));
	    float distTarget = fabsf (window->y () - targetY);

	    yFactor = distActual / distTarget;
	}

	if (divFactor)
	    factor = (scaleFactor + xFactor + yFactor) / divFactor;
	else
	    factor = 1.0f;

	attrib.opacity *= factor;
    }

    if (priv->adjust || priv->slot)
    {
	if (priv->window->id ()     != priv->spScreen->selectedWindow &&
	    priv->spScreen->opacity != OPAQUE                         &&
	    priv->spScreen->state   != ScaleScreen::In)
	{
	    /* modify opacity of windows that are not active */
	    attrib.opacity = (attrib.opacity * priv->spScreen->opacity) >> 16;
	}

	drawScaled = true;
    }
    else if (priv->spScreen->state != ScaleScreen::In)
    {
	if (priv->spScreen->optionGetDarkenBack ())
	{
	    /* modify brightness of the other windows */
	    attrib.brightness = attrib.brightness / 2;
	}

	/* hide windows on the outputs used for scaling
	   that are not in scale mode */
	if (!priv->isNeverScaleWin ())
	{
	    int moMode, output;

	    moMode = priv->spScreen->getMultioutputMode ();

	    switch (moMode) {
		case ScaleOptions::MultioutputModeOnCurrentOutputDevice:
		    output = screen->currentOutputDev ().id ();
		    if (priv->window->outputDevice () == output)
			attrib.opacity = 0;
		    break;
		default:
		    attrib.opacity = 0;
		    break;
	    }
	}
    }

    return drawScaled;
}

bool
PrivateScaleWindow::glPaint (const GLWindowPaintAttrib& attrib,
			     const GLMatrix&            transform,
			     const CompRegion&          region,
			     unsigned int               mask)
{
    bool status;

    if (spScreen->state != ScaleScreen::Idle)
    {
	GLWindowPaintAttrib sAttrib (attrib);
	bool                scaled;

	scaled = sWindow->setScaledPaintAttributes (sAttrib);

	if (adjust || slot)
	    mask |= PAINT_WINDOW_NO_CORE_INSTANCE_MASK;

	status = gWindow->glPaint (sAttrib, transform, region, mask);

	if (scaled)
	{
	    GLWindowPaintAttrib lastAttrib (gWindow->lastPaintAttrib ());
	    GLMatrix           wTransform (transform);

	    if (mask & PAINT_WINDOW_OCCLUSION_DETECTION_MASK)
		return false;

	    if (window->alpha () || lastAttrib.opacity != OPAQUE)
		mask |= PAINT_WINDOW_TRANSLUCENT_MASK;

	    wTransform.translate (window->x (), window->y (), 0.0f);
	    wTransform.scale (scale, scale, 1.0f);
	    wTransform.translate (tx / scale - window->x (),
				  ty / scale - window->y (), 0.0f);

	    gWindow->glDraw (wTransform, lastAttrib, region,
			     mask | PAINT_WINDOW_TRANSFORMED_MASK);

	    sWindow->scalePaintDecoration (sAttrib, transform, region, mask);
	}
    }
    else
    {
	status = gWindow->glPaint (attrib, transform, region, mask);
    }

    return status;
}

bool
PrivateScaleWindow::compareWindowsDistance (ScaleWindow *w1,
					    ScaleWindow *w2)
{
    return w1->priv->distance < w2->priv->distance;
}

void
PrivateScaleScreen::layoutSlotsForArea (const CompRect& workArea,
					int             nWindows)
{
    int i, j;
    int x, y, width, height;
    int lines, n, nSlots;
    int spacing;

    if (!nWindows)
	return;

    lines   = sqrt (nWindows + 1);
    spacing = optionGetSpacing ();
    nSlots  = 0;

    y      = workArea.y () + spacing;
    height = (workArea.height () - (lines + 1) * spacing) / lines;

    for (i = 0; i < lines; i++)
    {
	n = MIN (nWindows - nSlots, ceilf ((float) nWindows / lines));

	x     = workArea.x () + spacing;
	width = (workArea.width () - (n + 1) * spacing) / n;

	for (j = 0; j < n; j++)
	{
	    slots[this->nSlots].setGeometry (x, y, width, height);

	    slots[this->nSlots].filled = false;

	    x += width + spacing;

	    this->nSlots++;
	    nSlots++;
	}

	y += height + spacing;
    }
}

SlotArea::vector
PrivateScaleScreen::getSlotAreas ()
{
    SlotArea::vector slotAreas;

    slotAreas.resize (screen->outputDevs ().size ());

    unsigned int i = 0;
    foreach (CompOutput &o, screen->outputDevs ())
    {
	slotAreas[i].nWindows = 0;
	foreach (ScaleWindow *window, windows)
	{
	    CompWindow *cw = window->priv->window;
	    if (cw->outputDevice () == (int) o.id ())
		slotAreas[i].nWindows++;
	}

	slotAreas[i++].workArea = o.workArea ();
    }

    return slotAreas;
}

void
PrivateScaleScreen::layoutSlots ()
{
    int moMode;

    moMode  = getMultioutputMode ();

    /* if we have only one head, we don't need the
       additional effort of the all outputs mode */
    if (screen->outputDevs ().size () == 1)
	moMode = ScaleOptions::MultioutputModeOnCurrentOutputDevice;

    nSlots = 0;

    switch (moMode)
    {
	case ScaleOptions::MultioutputModeOnAllOutputDevices:
	    {
		SlotArea::vector slotAreas = getSlotAreas ();
		if (!slotAreas.empty ())
		{
		    foreach (SlotArea &sa, slotAreas)
			layoutSlotsForArea (sa.workArea, sa.nWindows);
		}
	    }
	    break;
	case ScaleOptions::MultioutputModeOnCurrentOutputDevice:
	default:
	    {
		CompRect workArea (screen->currentOutputDev ().workArea ());
		layoutSlotsForArea (workArea, windows.size ());
	    }
	    break;
    }
}

void
PrivateScaleScreen::findBestSlots ()
{
    CompWindow *w;
    int        i, d, d0 = 0;
    float      sx, sy, cx, cy;

    foreach (ScaleWindow *sw, windows)
    {
	w = sw->priv->window;

	if (sw->priv->slot)
	    continue;

	sw->priv->sid      = 0;
	sw->priv->distance = MAXSHORT;

	for (i = 0; i < nSlots; i++)
	{
	    if (!slots[i].filled)
	    {
		sx = (slots[i].x2 () + slots[i].x1 ()) / 2;
		sy = (slots[i].y2 () + slots[i].y1 ()) / 2;

		cx = (w->serverX () - (w->defaultViewport ().x () - screen->vp ().x ()) * screen->width ()) + w->width () / 2;
		cy = (w->serverY () - (w->defaultViewport ().y () - screen->vp ().y ()) * screen->height ()) + w->height () / 2;

		cx -= sx;
		cy -= sy;

		d = sqrt (cx * cx + cy * cy);
		if (d0 + d < sw->priv->distance)
		{
		    sw->priv->sid      = i;
		    sw->priv->distance = d0 + d;
		}
	    }
	}

	d0 += sw->priv->distance;
    }
}

bool
PrivateScaleScreen::fillInWindows ()
{
    CompWindow *w;
    int        width, height;
    float      sx, sy, cx, cy;

    foreach (ScaleWindow *sw, windows)
    {
	w = sw->priv->window;

	if (!sw->priv->slot)
	{
	    if (slots[sw->priv->sid].filled)
		return true;

	    sw->priv->slot = &slots[sw->priv->sid];

	    /* Auxilary items reparented into windows are clickable so we want to care about
	     * them when calculating the slot size */

	    width  = w->width ()  + w->input ().left + w->input ().right;
	    height = w->height () + w->input ().top  + w->input ().bottom;

	    sx = (float) (sw->priv->slot->x2 () - sw->priv->slot->x1 ()) / width;
	    sy = (float) (sw->priv->slot->y2 () - sw->priv->slot->y1 ()) / height;

	    sw->priv->slot->scale = MIN (MIN (sx, sy), 1.0f);

	    sx = width  * sw->priv->slot->scale;
	    sy = height * sw->priv->slot->scale;
	    cx = (sw->priv->slot->x1 () + sw->priv->slot->x2 ()) / 2;
	    cy = (sw->priv->slot->y1 () + sw->priv->slot->y2 ()) / 2;

	    cx += w->input ().left * sw->priv->slot->scale;
	    cy += w->input ().top  * sw->priv->slot->scale;

	    sw->priv->slot->setGeometry (cx - sx / 2, cy - sy / 2, sx, sy);

	    sw->priv->slot->filled = true;

	    sw->priv->lastThumbOpacity = 0.0f;

	    sw->priv->adjust = true;
	}
    }

    return false;
}

bool
ScaleScreenInterface::layoutSlotsAndAssignWindows ()
    WRAPABLE_DEF (layoutSlotsAndAssignWindows)

bool
ScaleScreen::layoutSlotsAndAssignWindows ()
{
    WRAPABLE_HND_FUNCTN_RETURN (bool, layoutSlotsAndAssignWindows)

    /* create a grid of slots */
    priv->layoutSlots ();

    do
    {
	/* find most appropriate slots for windows */
	priv->findBestSlots ();

	/* sort windows, window with closest distance to a slot first */
	priv->windows.sort (PrivateScaleWindow::compareWindowsDistance);
    } while (priv->fillInWindows ());

    return true;
}

bool
ScaleScreen::hasGrab () const
{
    return priv->grab;
}

ScaleScreen::State
ScaleScreen::getState () const
{
    return priv->state;
}

ScaleType
ScaleScreen::getType () const
{
    return priv->type;
}

const CompMatch&
ScaleScreen::getCustomMatch () const
{
    return priv->match;
}

const ScaleScreen::WindowList&
ScaleScreen::getWindows () const
{
    return priv->windows;
}

bool
PrivateScaleScreen::layoutThumbs ()
{
    switch (type) {
	case ScaleTypeAll:
	    return layoutThumbsAll ();
	case ScaleTypeNormal:
	default:
	    return layoutThumbsSingle ();
    }
}

bool
PrivateScaleScreen::layoutThumbsAll ()
{
    windows.clear ();

    /* add windows scale list, top most window first */
    foreach (CompWindow *w, screen->windows ())
    {
	SCALE_WINDOW (w);

	if (sw->priv->slot)
	    sw->priv->adjust = true;

	sw->priv->slot = NULL;

	if (!sw->priv->isScaleWin ())
            continue;

	windows.push_back (sw);
    }

    if (windows.empty ())
	return false;

    slots.resize (windows.size ());

    return ScaleScreen::get (screen)->layoutSlotsAndAssignWindows ();
}

bool
PrivateScaleScreen::layoutThumbsSingle ()
{
    bool ret = false;
    std::map <ScaleWindow *, ScaleSlot> slotWindows;
    CompWindowList          allWindows;

    for (int i = 0; i < screen->vpSize ().height (); i++)
    {
	for (int j = 0; j < screen->vpSize ().width (); j++)
	{
	    windows.clear ();
	    slots.clear ();

	    /* add windows scale list, top most window first */
	    foreach (CompWindow *w, screen->windows ())
	    {
		SCALE_WINDOW (w);

		if (w->defaultViewport () != CompPoint (j, i))
		    continue;

		if (sw->priv->slot)
		    sw->priv->adjust = true;

		sw->priv->slot = NULL;

		if (!sw->priv->isScaleWin ())
		    continue;

		windows.push_back (sw);
	    }

	    if (!windows.empty ())
	    {
		slots.resize (windows.size ());
		ret |= ScaleScreen::get (screen)->layoutSlotsAndAssignWindows ();

		foreach (ScaleWindow *sw, windows)
		    slotWindows[sw] = *sw->priv->slot;
	    }
	}
    }

    slots.clear ();
    windows.clear ();

    for (std::map<ScaleWindow *, ScaleSlot>::iterator it = slotWindows.begin ();
	 it != slotWindows.end (); ++it)
    {
	slots.push_back (it->second);
	windows.push_back (it->first);
	it->first->priv->slot = &slots.back ();
	it->first->priv->slot->setX (it->first->priv->slot->x () + (it->first->priv->window->defaultViewport ().x () - screen->vp ().x ()) * screen->width ());
	it->first->priv->slot->setY (it->first->priv->slot->y () + (it->first->priv->window->defaultViewport ().y () - screen->vp ().y ()) * screen->height ());
    }

    return ret;
}

bool
PrivateScaleWindow::adjustScaleVelocity ()
{
    float dx, dy, ds, adjust, amount;
    float x1, y1, scale;

    if (slot)
    {
	x1 = slot->x1 ();
	y1 = slot->y1 ();
	scale = slot->scale;
    }
    else
    {
	x1 = window->x ();
	y1 = window->y ();
	scale = 1.0f;
    }

    dx = x1 - (window->x () + tx);

    adjust = dx * 0.15f;
    amount = fabs (dx) * 1.5f;
    if (amount < 0.5f)
	amount = 0.5f;
    else if (amount > 5.0f)
	amount = 5.0f;

    xVelocity = (amount * xVelocity + adjust) / (amount + 1.0f);

    dy = y1 - (window->y () + ty);

    adjust = dy * 0.15f;
    amount = fabs (dy) * 1.5f;
    if (amount < 0.5f)
	amount = 0.5f;
    else if (amount > 5.0f)
	amount = 5.0f;

    yVelocity = (amount * yVelocity + adjust) / (amount + 1.0f);

    ds = scale - this->scale;

    adjust = ds * 0.1f;
    amount = fabs (ds) * 7.0f;
    if (amount < 0.01f)
	amount = 0.01f;
    else if (amount > 0.15f)
	amount = 0.15f;

    scaleVelocity = (amount * scaleVelocity + adjust) / (amount + 1.0f);

    if (fabs (dx) < 0.1f && fabs (xVelocity) < 0.2f &&
	fabs (dy) < 0.1f && fabs (yVelocity) < 0.2f &&
	fabs (ds) < 0.001f && fabs (scaleVelocity) < 0.002f)
    {
	xVelocity = yVelocity = scaleVelocity = 0.0f;
	tx = x1 - window->x ();
	ty = y1 - window->y ();
	this->scale = scale;

	return false;
    }

    return true;
}

bool
PrivateScaleScreen::glPaintOutput (const GLScreenPaintAttrib& sAttrib,
				   const GLMatrix&            transform,
				   const CompRegion&          region,
				   CompOutput                 *output,
				   unsigned int               mask)
{
    if (state != ScaleScreen::Idle)
	mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK;

    return gScreen->glPaintOutput (sAttrib, transform, region, output, mask);
}

void
PrivateScaleScreen::preparePaint (int msSinceLastPaint)
{
#ifndef LP1026986_FIXED_PROPERLY
    if (state != ScaleScreen::Idle)
	cScreen->damageScreen ();
#endif
    if (state != ScaleScreen::Idle && state != ScaleScreen::Wait)
    {
	int   steps;
	float amount, chunk;

	amount = msSinceLastPaint * 0.05f * optionGetSpeed ();
	steps  = amount / (0.5f * optionGetTimestep ());

	if (!steps)
	    steps = 1;
	chunk = amount / (float) steps;

	while (steps--)
	{
	    moreAdjust = 0;

	    foreach (CompWindow *w, screen->windows ())
	    {
		SCALE_WINDOW (w);

		if (sw->priv->adjust)
		{
		    sw->priv->adjust = sw->priv->adjustScaleVelocity ();

		    moreAdjust |= sw->priv->adjust;

		    sw->priv->tx += sw->priv->xVelocity * chunk;
		    sw->priv->ty += sw->priv->yVelocity * chunk;
		    sw->priv->scale += sw->priv->scaleVelocity * chunk;
		}
	    }

	    if (!moreAdjust)
		break;
	}
    }

    cScreen->preparePaint (msSinceLastPaint);
}

void
PrivateScaleScreen::donePaint ()
{
    if (state != ScaleScreen::Idle)
    {
	if (moreAdjust)
	{
	    cScreen->damageScreen ();
	}
	else
	{
	    if (state == ScaleScreen::In)
	    {
		/* The false activate event is sent when scale state
		   goes back to normal, to avoid animation conflicts
		   with other plugins. */
		activateEvent (false);
		state = ScaleScreen::Idle;

		cScreen->preparePaintSetEnabled (this, false);
		cScreen->donePaintSetEnabled (this, false);
		gScreen->glPaintOutputSetEnabled (this, false);

		foreach (CompWindow *w, screen->windows ())
		{
		    SCALE_WINDOW (w);
		    sw->priv->cWindow->damageRectSetEnabled (sw->priv, false);
		    sw->priv->gWindow->glPaintSetEnabled (sw->priv, false);
		}
	    }
	    else if (state == ScaleScreen::Out)
	    {
		state = ScaleScreen::Wait;

		// When the animation is completed, select the window under mouse
		selectWindowAt (pointerX, pointerY);
	    }
	}
    }

    cScreen->donePaint ();
}

ScaleWindow *
PrivateScaleScreen::checkForWindowAt (int x, int y)
{
    int                              x1, y1, x2, y2;
    CompWindowList::reverse_iterator rit = screen->windows ().rbegin ();

    for (; rit != screen->windows ().rend (); ++rit)
    {
	CompWindow *w = *rit;
	SCALE_WINDOW (w);

	if (sw->priv->slot)
	{
	    x1 = w->x () - w->input ().left * sw->priv->scale;
	    y1 = w->y () - w->input ().top  * sw->priv->scale;
	    x2 = w->x () +
		 (w->width () + w->input ().right) * sw->priv->scale;
	    y2 = w->y () +
		 (w->height () + w->input ().bottom) * sw->priv->scale;

	    x1 += sw->priv->tx;
	    y1 += sw->priv->ty;
	    x2 += sw->priv->tx;
	    y2 += sw->priv->ty;

	    if (x1 <= x && y1 <= y && x2 > x && y2 > y)
		return sw;
	}
    }

    return NULL;
}

void
PrivateScaleScreen::sendDndStatusMessage (Window source)
{
    XEvent xev;

    xev.xclient.type    = ClientMessage;
    xev.xclient.display = screen->dpy ();
    xev.xclient.format  = 32;

    xev.xclient.message_type = Atoms::xdndStatus;
    xev.xclient.window	     = source;

    xev.xclient.data.l[0] = dndTarget;
    xev.xclient.data.l[1] = 2;
    xev.xclient.data.l[2] = 0;
    xev.xclient.data.l[3] = 0;
    xev.xclient.data.l[4] = None;

    XSendEvent (screen->dpy (), source, false, 0, &xev);
}

bool
PrivateScaleScreen::scaleTerminate (CompAction         *action,
				    CompAction::State  state,
				    CompOption::Vector &options)
{
    SCALE_SCREEN (screen);

    Window xid;
    int    selectX = CompOption::getIntOptionNamed (options, "select_x", -1);
    int    selectY = CompOption::getIntOptionNamed (options, "select_y", -1);

    if (ss->priv->actionShouldToggle (action, state))
	return false;

    xid = CompOption::getIntOptionNamed (options, "root");
    if (xid && ::screen->root () != xid)
	return false;

    if (!ss->priv->grab)
	return false;

    if (selectX != -1 &&
	selectY != -1)
    {
	if (!ss->priv->selectWindowAt (selectX, selectY, true))
	    return false;
    }

    if (ss->priv->grabIndex)
    {
	::screen->removeGrab (ss->priv->grabIndex, 0);
	ss->priv->grabIndex = 0;
    }

    if (ss->priv->dndTarget)
	XUnmapWindow (::screen->dpy (), ss->priv->dndTarget);

    ss->priv->grab = false;

    if (ss->priv->state != ScaleScreen::Idle)
    {
	foreach (CompWindow *w, ::screen->windows ())
	{
	    SCALE_WINDOW (w);

	    if (sw->priv->slot)
	    {
		sw->priv->lastTargetScale = sw->priv->slot->scale;
		sw->priv->lastTargetX     = sw->priv->slot->x ();
		sw->priv->lastTargetY     = sw->priv->slot->y ();
		sw->priv->slot   = NULL;
		sw->priv->adjust = true;
	    }
	    else
	    {
		sw->priv->lastTargetScale = 1.0f;
		sw->priv->lastTargetX = w->x ();
		sw->priv->lastTargetY = w->y ();
	    }
	}

	if (state & CompAction::StateCancel)
	{
	    if (::screen->activeWindow () != ss->priv->previousActiveWindow)
	    {
		CompWindow *w;

		w = ::screen->findWindow (ss->priv->previousActiveWindow);
		if (w)
		    w->moveInputFocusTo ();
	    }
	}
	else if (ss->priv->state != ScaleScreen::In)
	{
	    CompWindow *w = ::screen->findWindow (ss->priv->selectedWindow);
	    if (w)
		w->activate ();
	}

	ss->priv->state = ScaleScreen::In;
	ss->priv->cScreen->damageScreen ();
    }

    if (state & CompAction::StateInitKey)
	action->setState (action->state () | CompAction::StateTermKey);

    ss->priv->lastActiveNum = 0;

    if (selectX != -1 &&
	selectY != -1)
	return true;

    return false;
}

bool
PrivateScaleScreen::ensureDndRedirectWindow ()
{
    if (!dndTarget)
    {
	XSetWindowAttributes attr;
	long		     xdndVersion = 3;

	attr.override_redirect = true;

	dndTarget = XCreateWindow (screen->dpy (), screen->root (),
				   0, 0, 1, 1, 0, CopyFromParent,
				   InputOnly, CopyFromParent,
				   CWOverrideRedirect, &attr);

	XChangeProperty (screen->dpy (), dndTarget,
			 Atoms::xdndAware,
			 XA_ATOM, 32, PropModeReplace,
			 (unsigned char *) &xdndVersion, 1);
    }

    XMoveResizeWindow (screen->dpy (), dndTarget,
		       0, 0, screen->width (), screen->height ());
    XMapRaised (screen->dpy (), dndTarget);

    return true;
}

bool
PrivateScaleScreen::actionShouldToggle (CompAction        *action,
			 		CompAction::State state)
{
    if (state & EDGE_STATE)
	return TRUE;

    if (state & (CompAction::StateInitKey | CompAction::StateTermKey))
    {
	if (optionGetKeyBindingsToggle ())
	    return TRUE;
	else if (!action->key ().modifiers ())
	    return TRUE;
    }

    if (state & (CompAction::StateInitButton | CompAction::StateTermButton))
	if (optionGetButtonBindingsToggle ())
	    return TRUE;

    return FALSE;
}

bool
PrivateScaleScreen::scaleInitiate (CompAction         *action,
				   CompAction::State  state,
				   CompOption::Vector &options,
				   ScaleType          type)
{
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "root");

    if (::screen->root () == xid)
    {
	SCALE_SCREEN (::screen);

	if (ss->priv->actionShouldToggle (action, state) &&
	    (ss->priv->state == ScaleScreen::Wait ||
	     ss->priv->state == ScaleScreen::Out))
	{
	    if (ss->priv->type == type)
		return scaleTerminate (action,
				       CompAction::StateCancel,
				       options);
	}
	else
	{
	    ss->priv->type = type;
	    return ss->priv->scaleInitiateCommon (action, state, options);
	}
    }

    return false;
}

bool
PrivateScaleScreen::scaleInitiateCommon (CompAction         *action,
					 CompAction::State  state,
					 CompOption::Vector &options)
{
    int noAutoGrab = CompOption::getIntOptionNamed (options, "no_auto_grab", 0);

    if (screen->otherGrabExist ("scale", NULL) && !noAutoGrab)
	return false;

    match = CompOption::getMatchOptionNamed (options, "match",
					     CompMatch::emptyMatch);
    if (match.isEmpty ())
	match = optionGetWindowMatch ();

    currentMatch = match;

    if (!layoutThumbs ())
	return false;

    /* Another plugin may be using us externally */
    grab = noAutoGrab;

    if (!grab)
    {
	if (state & CompAction::StateInitEdgeDnd)
	{
	    if (ensureDndRedirectWindow ())
		grab = true;
	}
	else if (!grabIndex)
	{
	    grabIndex = screen->pushGrab (cursor, "scale");
	    if (grabIndex)
		grab = true;
	}
    }

    if (grab)
    {
	if (!lastActiveNum)
	    lastActiveNum = screen->activeNum () - 1;

	previousActiveWindow = screen->activeWindow ();
	lastActiveWindow     = screen->activeWindow ();
	selectedWindow       = screen->activeWindow ();
	hoveredWindow        = None;

	this->state = ScaleScreen::Out;

	activateEvent (true);

	cScreen->damageScreen ();

	cScreen->preparePaintSetEnabled (this, true);
	cScreen->donePaintSetEnabled (this, true);
	gScreen->glPaintOutputSetEnabled (this, true);

	foreach (CompWindow *w, screen->windows ())
	{
	    SCALE_WINDOW (w);
	    sw->priv->cWindow->damageRectSetEnabled (sw->priv, true);
	    sw->priv->gWindow->glPaintSetEnabled (sw->priv, true);
	}
    }

    if ((state & (CompAction::StateInitButton | EDGE_STATE)) ==
	CompAction::StateInitButton)
    {
	action->setState (action->state () | CompAction::StateTermButton);
    }

    if (state & CompAction::StateInitKey)
	action->setState (action->state () | CompAction::StateTermKey);

    return false;
}

void
ScaleWindowInterface::scaleSelectWindow ()
    WRAPABLE_DEF (scaleSelectWindow)

void
ScaleWindow::scaleSelectWindow ()
{
    WRAPABLE_HND_FUNCTN (scaleSelectWindow)

    if (priv->spScreen->selectedWindow != priv->window->id ())
    {
	CompWindow *oldW, *newW;

	oldW = screen->findWindow (priv->spScreen->selectedWindow);
	newW = screen->findWindow (priv->window->id ());

	priv->spScreen->selectedWindow = priv->window->id ();

	if (oldW)
	    CompositeWindow::get (oldW)->addDamage ();

	if (newW)
	    CompositeWindow::get (newW)->addDamage ();
    }
}

bool
ScaleWindow::hasSlot () const
{
    return priv->slot != NULL;
}

ScaleSlot
ScaleWindow::getSlot () const
{
    if (!priv->slot)
    {
	ScaleSlot empty;
	return empty;
    }

    return *priv->slot;
}

void
ScaleWindow::setSlot (const ScaleSlot &newSlot)
{
    SCALE_SCREEN (screen);

    priv->adjust = true;

    if (!priv->slot)
	priv->slot = new ScaleSlot ();
    *priv->slot = newSlot;

    /* Trigger the animation to this point */

    if (ss->priv->state == ScaleScreen::Wait)
	ss->priv->state = ScaleScreen::Out;
    else if (ss->priv->state == ScaleScreen::Idle)
	ss->priv->state = ScaleScreen::In;

    priv->cWindow->addDamage ();
}

ScalePosition
ScaleWindow::getCurrentPosition () const
{
    ScalePosition pos;

    pos.setX (priv->tx);
    pos.setY (priv->ty);

    pos.scale = priv->scale;

    return pos;
}

void
ScaleWindow::setCurrentPosition (const ScalePosition &newPos)
{
    SCALE_SCREEN (screen);

    priv->tx = newPos.x ();
    priv->ty = newPos.y ();
    priv->scale = newPos.scale;

    /* Trigger the animation to this point */

    if (ss->priv->state == ScaleScreen::Wait)
	ss->priv->state = ScaleScreen::Out;
    else if (ss->priv->state == ScaleScreen::Idle)
	ss->priv->state = ScaleScreen::In;

    priv->cWindow->addDamage ();

    priv->adjust = true;
}

const Window &
ScaleScreen::getHoveredWindow () const
{
    return priv->hoveredWindow;
}

const Window &
ScaleScreen::getSelectedWindow () const
{
    return priv->selectedWindow;
}

bool
PrivateScaleScreen::selectWindowAt (int  x,
				    int  y,
				    bool moveInputFocus)
{
    ScaleWindow *w = checkForWindowAt (x, y);
    if (w && w->priv->isScaleWin ())
    {
	w->scaleSelectWindow ();

	if (moveInputFocus)
	{
	    lastActiveNum    = w->priv->window->activeNum ();
	    lastActiveWindow = w->priv->window->id ();

	    w->priv->window->moveInputFocusTo ();
	}

	hoveredWindow = w->priv->window->id ();

	return true;
    }

    hoveredWindow = None;

    return false;
}

bool
PrivateScaleScreen::selectWindowAt (int  x,
				    int  y)
{
    CompOption *o = screen->getOption ("click_to_focus");
    bool focus = (o && !o->value ().b ());

    return selectWindowAt (x, y, focus);
}

void
PrivateScaleScreen::moveFocusWindow (int dx,
				     int dy)
{
    CompWindow *active;
    CompWindow *focus = NULL;

    active = screen->findWindow (screen->activeWindow ());
    if (active)
    {
	SCALE_WINDOW (active);

	if (sw->priv->slot)
	{
	    ScaleSlot  *slot;
	    int	       x, y, cx, cy, d, min = MAXSHORT;

	    cx = (sw->priv->slot->x1 () + sw->priv->slot->x2 ()) / 2;
	    cy = (sw->priv->slot->y1 () + sw->priv->slot->y2 ()) / 2;

	    foreach (CompWindow *w, screen->windows ())
	    {
		slot = ScaleWindow::get (w)->priv->slot;
		if (!slot)
		    continue;

		x = (slot->x1 () + slot->x2 ()) / 2;
		y = (slot->y1 () + slot->y2 ()) / 2;

		d = abs (x - cx) + abs (y - cy);
		if (d < min)
		{
		    if ((dx > 0 && slot->x1 () < sw->priv->slot->x2 ()) ||
			(dx < 0 && slot->x2 () > sw->priv->slot->x1 ()) ||
			(dy > 0 && slot->y1 () < sw->priv->slot->y2 ()) ||
			(dy < 0 && slot->y2 () > sw->priv->slot->y1 ()))
			continue;

		    min   = d;
		    focus = w;
		}
	    }
	}
    }

    /* move focus to the last focused window if no slot window is currently
       focused */
    if (!focus)
    {
	foreach (CompWindow *w, screen->windows ())
	{
	    if (!ScaleWindow::get (w)->priv->slot)
		continue;

	    if (!focus || focus->activeNum () < w->activeNum ())
		focus = w;
	}
    }

    if (focus)
    {
	ScaleWindow::get (focus)->scaleSelectWindow ();

	lastActiveNum    = focus->activeNum ();
	lastActiveWindow = focus->id ();

	focus->moveInputFocusTo ();
    }
}

void
ScaleScreen::relayoutSlots (const CompMatch& match)
{
    if (match.isEmpty ())
	priv->currentMatch = priv->match;
    else
	priv->currentMatch = match;

    if (priv->state == ScaleScreen::Idle || priv->state == ScaleScreen::In)
	return;

    if (priv->layoutThumbs ())
    {
	priv->state = ScaleScreen::Out;
	priv->moveFocusWindow (0, 0);
    }

    priv->cScreen->damageScreen ();
}

void
PrivateScaleScreen::windowRemove (CompWindow *w)
{
    if (!w)
	return;

    if (state == ScaleScreen::Idle || state == ScaleScreen::In)
	return;

    foreach (ScaleWindow *lw, windows)
    {
	if (lw->priv->window == w)
	{
	    if (layoutThumbs ())
	    {
		state = ScaleScreen::Out;
		cScreen->damageScreen ();
		break;
	    }
	    else
	    {
		CompOption::Vector o (0);
		CompAction         *action;

		/* terminate scale mode if the recently closed
		 * window was the last scaled window */

		o.push_back (CompOption ("root", CompOption::TypeInt));
		o[0].value ().set ((int) screen->root ());

		action = &optionGetInitiateEdge ();
		scaleTerminate (action, CompAction::StateCancel, o);

		action = &optionGetInitiateKey ();
		scaleTerminate (action, CompAction::StateCancel, o);
		break;
	    }
	}
    }
}

bool
PrivateScaleScreen::hoverTimeout ()
{
    if (grab && state != ScaleScreen::In)
    {
	CompWindow         *w;
	CompOption::Vector o (0);

	w = screen->findWindow (selectedWindow);
	if (w)
	{
	    lastActiveNum    = w->activeNum ();
	    lastActiveWindow = w->id ();

	    w->moveInputFocusTo ();
	}

	o.push_back (CompOption ("root", CompOption::TypeInt));
	o[0].value ().set ((int) screen->root ());

	scaleTerminate (&optionGetInitiateEdge (), 0, o);
	scaleTerminate (&optionGetInitiateKey (), 0, o);
    }

    return false;
}

void
PrivateScaleScreen::handleEvent (XEvent *event)
{
    CompWindow *w = NULL;

    switch (event->type) {
	case KeyPress:
	    if (screen->root () == event->xkey.root)
	    {
		if (grabIndex)
		{
		    if (event->xkey.keycode == leftKeyCode)
			moveFocusWindow (-1, 0);
		    else if (event->xkey.keycode == rightKeyCode)
			moveFocusWindow (1, 0);
		    else if (event->xkey.keycode == upKeyCode)
			moveFocusWindow (0, -1);
		    else if (event->xkey.keycode == downKeyCode)
			moveFocusWindow (0, 1);
		}
	    }
	    break;
	case ButtonPress:
	    if (screen->root () == event->xbutton.root &&
		grabIndex                              &&
		state != ScaleScreen::In)
	    {
		XButtonEvent       *button = &event->xbutton;
		CompOption::Vector o (0);

		o.push_back (CompOption ("root", CompOption::TypeInt));
		o[0].value ().set ((int) screen->root ());

		/* Button1 terminates scale mode, other buttons can select
		 * windows */
		if (selectWindowAt (button->x_root, button->y_root, true) &&
		    event->xbutton.button == Button1)
		{
		    scaleTerminate (&optionGetInitiateEdge (), 0, o);
		    scaleTerminate (&optionGetInitiateKey (), 0, o);
		}
		else if (optionGetShowDesktop () &&
			 event->xbutton.button == Button1)
		{
		    CompPoint pointer (button->x_root, button->y_root);
		    CompRect  workArea (screen->workArea ());

		    if (workArea.contains (pointer))
		    {
			scaleTerminate (&optionGetInitiateEdge (), 0, o);
			scaleTerminate (&optionGetInitiateKey (), 0, o);
			screen->enterShowDesktopMode ();
		    }
		}
	    }
	    break;
	case MotionNotify:
	    if (screen->root () == event->xmotion.root &&
		grabIndex                              &&
		state != ScaleScreen::In)
	    {
		selectWindowAt (event->xmotion.x_root, event->xmotion.y_root);
	    }
	    break;
	case DestroyNotify:

	    /* We need to get the CompWindow * for event->xdestroywindow.window
	     * here because in the ::handleEvent call below that CompWindow's
	     * id will become "1" so CompScreen::findWindow won't
	     * be able to find teh window after that
	     */

	    w = screen->findWindow (event->xdestroywindow.window);
	    break;
	case UnmapNotify:

	     w = screen->findWindow (event->xunmap.window);
	     break;
	case ClientMessage:
	    if (event->xclient.message_type == Atoms::xdndPosition)
	    {
		w = screen->findWindow (event->xclient.window);
		if (w)
		{
		    if (w->id () == dndTarget)
			sendDndStatusMessage (event->xclient.data.l[0]);

		    if (grab			 &&
			state != ScaleScreen::In &&
			w->id () == dndTarget)
		    {
			ScaleWindow *sw = checkForWindowAt (pointerX, pointerY);
			if (sw && sw->priv->isScaleWin ())
			{
			    int time;

			    time = optionGetHoverTime ();

			    if (hover.active ())
			    {
				int lastMotion = sqrt (pow (pointerX - lastPointerX, 2) + pow (pointerY - lastPointerY, 2));
				
				if (sw->window->id () != selectedWindow || lastMotion > optionGetDndDistance ())
				    hover.stop ();
			    }

			    if (!hover.active ())
			    {
				hover.start (time, (float) time * 1.2);
			    }

			    selectWindowAt (pointerX, pointerY);
			}
			else
			{
			    if (hover.active ())
				hover.stop ();
			}
		    }
		}
	    }
	    else if (event->xclient.message_type == Atoms::xdndDrop ||
		     event->xclient.message_type == Atoms::xdndLeave)
	    {
		w = screen->findWindow (event->xclient.window);
		if (w)
		{
		    if (grab			 &&
			state != ScaleScreen::In &&
			w->id () == dndTarget)
		    {
			CompOption::Vector o (0);
			o.push_back (CompOption ("root", CompOption::TypeInt));
			o[0].value ().set ((int) screen->root ());

			scaleTerminate (&optionGetInitiateEdge (), 0, o);
			scaleTerminate (&optionGetInitiateKey (), 0, o);
		    }
		}
	    }
	default:
	    break;
    }

    screen->handleEvent (event);

    /* Only safe to remove the window after all events have been
     * handled, so that we don't get race conditions on calls
     * to scale functions */

    switch (event->type) {
	case UnmapNotify:
	    if (w)
		windowRemove (w);
	    break;
	case DestroyNotify:
	    if (w)
		windowRemove (w);
	    break;
    }
}

bool
PrivateScaleWindow::damageRect (bool            initial,
				const CompRect& rect)
{
    bool status = false;

    if (initial)
    {
	if (spScreen->grab && isScaleWin ())
	{
	    if (spScreen->layoutThumbs ())
	    {
		spScreen->state = ScaleScreen::Out;
		spScreen->cScreen->damageScreen ();
	    }
	}
    }
    else if (spScreen->state == ScaleScreen::Wait)
    {
	if (slot)
	{
	    cWindow->damageTransformedRect (scale, scale, tx, ty, rect);

	    status = true;
	}
    }

    status |= cWindow->damageRect (initial, rect);

    return status;
}

template class PluginClassHandler<ScaleScreen, CompScreen, COMPIZ_SCALE_ABI>;

ScaleScreen::ScaleScreen (CompScreen *s) :
    PluginClassHandler<ScaleScreen, CompScreen, COMPIZ_SCALE_ABI> (s),
    priv (new PrivateScaleScreen (s))
{
}

ScaleScreen::~ScaleScreen ()
{
    delete priv;
}

template class PluginClassHandler<ScaleWindow, CompWindow, COMPIZ_SCALE_ABI>;

ScaleWindow::ScaleWindow (CompWindow *w) :
    PluginClassHandler<ScaleWindow, CompWindow, COMPIZ_SCALE_ABI> (w),
    window (w),
    priv (new PrivateScaleWindow (w))
{
}

ScaleWindow::~ScaleWindow ()
{
    delete priv;
}

PrivateScaleScreen::PrivateScaleScreen (CompScreen *s) :
    cScreen (CompositeScreen::get (s)),
    gScreen (GLScreen::get (s)),

    lastActiveNum (0),
    lastActiveWindow (None),

    selectedWindow (None),
    hoveredWindow (None),
    previousActiveWindow (None),
    grab (false),
    grabIndex (0),
    dndTarget (None),
    state (ScaleScreen::Idle),
    moreAdjust (false),
    cursor (0),
    nSlots (0)
{
    leftKeyCode  = XKeysymToKeycode (screen->dpy (), XStringToKeysym ("Left"));
    rightKeyCode = XKeysymToKeycode (screen->dpy (), XStringToKeysym ("Right"));
    upKeyCode    = XKeysymToKeycode (screen->dpy (), XStringToKeysym ("Up"));
    downKeyCode  = XKeysymToKeycode (screen->dpy (), XStringToKeysym ("Down"));

    cursor = XCreateFontCursor (screen->dpy (), XC_left_ptr);

    opacity = (OPAQUE * optionGetOpacity ()) / 100;

    hover.setCallback (boost::bind (&PrivateScaleScreen::hoverTimeout, this));

    optionSetOpacityNotify (boost::bind (&PrivateScaleScreen::updateOpacity, this));

#define SCALEBIND(a)                                              \
    boost::bind (PrivateScaleScreen::scaleInitiate, _1, _2, _3, a)

    optionSetInitiateEdgeInitiate (SCALEBIND (ScaleTypeNormal));
    optionSetInitiateEdgeTerminate (PrivateScaleScreen::scaleTerminate);
    optionSetInitiateButtonInitiate (SCALEBIND (ScaleTypeNormal));
    optionSetInitiateButtonTerminate (PrivateScaleScreen::scaleTerminate);
    optionSetInitiateKeyInitiate (SCALEBIND (ScaleTypeNormal));
    optionSetInitiateKeyTerminate (PrivateScaleScreen::scaleTerminate);

    optionSetInitiateAllEdgeInitiate (SCALEBIND (ScaleTypeAll));
    optionSetInitiateAllEdgeTerminate (PrivateScaleScreen::scaleTerminate);
    optionSetInitiateAllButtonInitiate (SCALEBIND (ScaleTypeAll));
    optionSetInitiateAllButtonTerminate (PrivateScaleScreen::scaleTerminate);
    optionSetInitiateAllKeyInitiate (SCALEBIND (ScaleTypeAll));
    optionSetInitiateAllKeyTerminate (PrivateScaleScreen::scaleTerminate);

    optionSetInitiateGroupEdgeInitiate (SCALEBIND (ScaleTypeGroup));
    optionSetInitiateGroupEdgeTerminate (PrivateScaleScreen::scaleTerminate);
    optionSetInitiateGroupButtonInitiate (SCALEBIND (ScaleTypeGroup));
    optionSetInitiateGroupButtonTerminate (PrivateScaleScreen::scaleTerminate);
    optionSetInitiateGroupKeyInitiate (SCALEBIND (ScaleTypeGroup));
    optionSetInitiateGroupKeyTerminate (PrivateScaleScreen::scaleTerminate);

    optionSetInitiateOutputEdgeInitiate (SCALEBIND (ScaleTypeOutput));
    optionSetInitiateOutputEdgeTerminate (PrivateScaleScreen::scaleTerminate);
    optionSetInitiateOutputButtonInitiate (SCALEBIND (ScaleTypeOutput));
    optionSetInitiateOutputButtonTerminate (PrivateScaleScreen::scaleTerminate);
    optionSetInitiateOutputKeyInitiate (SCALEBIND (ScaleTypeOutput));
    optionSetInitiateOutputKeyTerminate (PrivateScaleScreen::scaleTerminate);

#undef SCALEBIND

    ScreenInterface::setHandler (s);
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);
}

PrivateScaleScreen::~PrivateScaleScreen ()
{
    if (cursor)
	XFreeCursor (screen->dpy (), cursor);
}

void
PrivateScaleScreen::updateOpacity ()
{
    opacity = (OPAQUE * optionGetOpacity ()) / 100;
}

/* When we are only scaling windows on the current output, over-ride the
 * multioutput mode so that windows will only be displayed on the current
 * output, regardless of the setting.
 */
int
PrivateScaleScreen::getMultioutputMode ()
{
    if (type == ScaleTypeOutput)
	return MultioutputModeOnCurrentOutputDevice;

    return optionGetMultioutputMode ();
}


PrivateScaleWindow::PrivateScaleWindow (CompWindow *w) :
    window (w),
    cWindow (CompositeWindow::get (w)),
    gWindow (GLWindow::get (w)),
    sWindow (ScaleWindow::get (w)),
    sScreen (ScaleScreen::get (screen)),
    spScreen (sScreen->priv),
    slot (NULL),
    sid (0),
    distance (0.0),
    xVelocity (0.0),
    yVelocity (0.0),
    scaleVelocity (0.0),
    scale (1.0),
    lastTargetScale (1.0f),
    lastTargetX (w->x ()),
    lastTargetY (w->y ()),
    tx (0.0),
    ty (0.0),
    delta (1.0),
    adjust (false),
    lastThumbOpacity (0.0)
{
    CompositeWindowInterface::setHandler (cWindow,
					  spScreen->state != ScaleScreen::Idle);
    GLWindowInterface::setHandler (gWindow,
				   spScreen->state != ScaleScreen::Idle);
}

PrivateScaleWindow::~PrivateScaleWindow ()
{
}

CompOption::Vector &
ScaleScreen::getOptions ()
{
    return priv->getOptions ();
}

bool
ScaleScreen::setOption (const CompString  &name,
			CompOption::Value &value)
{
    return priv->setOption (name, value);
}

bool
ScalePluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

    CompPrivate p;
    p.uval = COMPIZ_SCALE_ABI;
    screen->storeValue ("scale_ABI", p);

    return true;
}

void
ScalePluginVTable::fini ()
{
    screen->eraseValue ("scale_ABI");
}
