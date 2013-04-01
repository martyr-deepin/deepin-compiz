/*
 *
 * Compiz shift switcher plugin
 *
 * shift.cpp
 *
 * Copyright : (C) 2007 by Dennis Kasprzyk
 * E-mail    : onestone@opencompositing.org
 *
 *
 * Based on ring.c:
 * Copyright : (C) 2007 by Danny Baumann
 * E-mail    : maniac@opencompositing.org
 *
 * Based on scale.c and switcher.c:
 * Copyright : (C) 2007 David Reveman
 * E-mail    : davidr@novell.com
 *
 * Rounded corner drawing taken from wall.c:
 * Copyright : (C) 2007 Robert Carr
 * E-mail    : racarr@beryl-project.org
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
 */


#include "shift.h"

COMPIZ_PLUGIN_20090315 (shift, ShiftPluginVTable);

const double PI = 3.14159265359f;

bool textAvailable = false;

void
setFunctions (bool enabled)
{
    SHIFT_SCREEN (screen);

    screen->handleEventSetEnabled (ss, enabled);
    ss->cScreen->preparePaintSetEnabled (ss, enabled);
    ss->cScreen->paintSetEnabled (ss, enabled);
    ss->gScreen->glPaintOutputSetEnabled (ss, enabled);
    ss->cScreen->donePaintSetEnabled (ss, enabled);

    foreach (CompWindow *w, screen->windows ())
    {
	SHIFT_WINDOW (w);

	sw->gWindow->glPaintSetEnabled (sw, enabled);
	sw->cWindow->damageRectSetEnabled (sw, enabled);
    }
}

void
ShiftScreen::activateEvent (bool	activating)
{
    CompOption::Vector o;

    o.resize (2);

    o[0] = CompOption ("root", CompOption::TypeInt);
    o[0].value ().set ((int) screen->root ());

    o[1] = CompOption ("active", CompOption::TypeBool);
    o[1].value ().set (activating);

    screen->handleCompizEvent ("shift", "activate", o);
}

bool
ShiftWindow::isShiftable ()
{
    SHIFT_SCREEN (screen);

    if (window->overrideRedirect ())
	return false;

    if (window->wmType () & (CompWindowTypeDockMask | CompWindowTypeDesktopMask))
	return false;

    if (!window->mapNum () || !window->isViewable ())
    {
	if (ss->optionGetMinimized ())
	{
	    if (!window->minimized () && !window->inShowDesktopMode () && !window->shaded ())
		return false;
	}
	else
    	    return false;
    }

    if (ss->mType == ShiftTypeNormal)
    {
	if (!window->mapNum () || !window->isViewable ())
	{
	    if (window->serverGeometry ().x () + window->serverGeometry ().width () <= 0    ||
		window->serverGeometry ().y () + window->serverGeometry ().height () <= 0    ||
		window->serverGeometry ().x () >= screen->width () ||
		window->serverGeometry ().y () >= screen->height ())
		return false;
	}
	else
	{
	    if (!window->focus ())
		return false;
	}
    }
    else if (ss->mType == ShiftTypeGroup &&
	     ss->mClientLeader != window->clientLeader () &&
	     ss->mClientLeader != window->id ())
    {
	return false;
    }

    if (window->state () & CompWindowStateSkipTaskbarMask)
	return false;

    if (ss->mCurrentMatch && !ss->mCurrentMatch->evaluate (window))
	return false;

    return true;
}

void
ShiftScreen::freeWindowTitle ()
{
}

void
ShiftScreen::renderWindowTitle ()
{
    CompText::Attrib tA;
    CompRect	     oe;

    freeWindowTitle ();

    if (!textAvailable)
        return;

    if (!optionGetWindowTitle ())
        return;

    if (optionGetMultioutputMode () ==
                                    ShiftOptions::MultioutputModeOneBigSwitcher)
    {
        oe.setGeometry (0, 0, screen->width (), screen->height ());
    }
    else
        oe = screen->getCurrentOutputExtents ();

    /* 75% of the output device as maximum width */
    tA.maxWidth = oe.width () * 3 / 4;
    tA.maxHeight = 100;

    tA.family = "Sans";
    tA.size = optionGetTitleFontSize ();
    tA.color[0] = optionGetTitleFontColorRed ();
    tA.color[1] = optionGetTitleFontColorGreen ();
    tA.color[2] = optionGetTitleFontColorBlue ();
    tA.color[3] = optionGetTitleFontColorAlpha ();

    tA.flags = CompText::WithBackground | CompText::Ellipsized;
    if (optionGetTitleFontBold ())
        tA.flags |= CompText::StyleBold;

    tA.bgHMargin = 15;
    tA.bgVMargin = 15;
    tA.bgColor[0] = optionGetTitleBackColorRed ();
    tA.bgColor[1] = optionGetTitleBackColorGreen ();
    tA.bgColor[2] = optionGetTitleBackColorBlue ();
    tA.bgColor[3] = optionGetTitleBackColorAlpha ();

    text.renderWindowTitle (mSelectedWindow ? mSelectedWindow : None,
                            mType == ShiftTypeAll, tA);
}

void
ShiftScreen::drawWindowTitle (const GLMatrix &transform)
{
    float width, height, border = 10.0f;
    CompRect oe;

    width = text.getWidth ();
    height = text.getHeight ();

    if (optionGetMultioutputMode () == MultioutputModeOneBigSwitcher)
    {
        oe.setGeometry (0, 0, screen->width (), screen->height ());
    }
    else
    {
    	oe = (CompRect) screen->outputDevs ()[mUsedOutput];
    }

    float x = oe.centerX () - width / 2;
    float y;

    /* assign y (for the lower corner!) according to the setting */
    switch (optionGetTitleTextPlacement ())
    {
    case TitleTextPlacementCenteredOnScreen:
        y = oe.centerY () + height / 2;
        break;
    case TitleTextPlacementAbove:
    case TitleTextPlacementBelow:
        {
            CompRect workArea = screen->currentOutputDev ().workArea ();

            if (optionGetTitleTextPlacement () ==
                TitleTextPlacementAbove)
                y = oe.y1 () + workArea.y1 () + 2 * border + height;
            else
                y = oe.y1 () + workArea.y2 () - 2 * border;
        }
        break;
    default:
        return;
    }

    text.draw (transform, floor (x), floor (y), 1.0f);
}

bool
ShiftWindow::glPaint (const GLWindowPaintAttrib	&attrib,
		      const GLMatrix		&transform,
		      const CompRegion		&region,
		      unsigned int		mask)
{
    bool       status;

    SHIFT_SCREEN (screen);
    if (ss->mState != ShiftStateNone && !ss->mPaintingAbove &&
	!(window->wmType () & (CompWindowTypeDesktopMask |
			       CompWindowTypeDockMask)))
    {
	GLWindowPaintAttrib sAttrib = attrib;
	bool		  scaled = false;

    	if (window->mapNum ())
	{
	    if (gWindow->textures ().empty ())
		gWindow->bind ();
	}


	if (mActive)
	    scaled = (ss->mActiveSlot != NULL);

	if (mOpacity > 0.01 && (ss->mActiveSlot == NULL))
	{
	    sAttrib.brightness = sAttrib.brightness * mBrightness;
	    sAttrib.opacity = sAttrib.opacity * mOpacity;
	}
	else
	    mask |= PAINT_WINDOW_NO_CORE_INSTANCE_MASK;

	if (mActive &&
	    ((unsigned int) ss->mOutput->id () == (unsigned int) ss->mUsedOutput ||
	     (unsigned int) ss->mOutput->id () == (unsigned int) ~0))
	    mask |= PAINT_WINDOW_NO_CORE_INSTANCE_MASK;


	status = gWindow->glPaint (sAttrib, transform, region, mask);

	if (scaled && !gWindow->textures ().empty ())
	{
	    GLWindowPaintAttrib wAttrib (attrib);
	    GLMatrix wTransform = transform;
	    ShiftSlot      *slot = ss->mActiveSlot->slot;

	    float sx     = ss->mAnim * slot->tx;
	    float sy     = ss->mAnim * slot->ty;
	    float sz     = ss->mAnim * slot->z;
	    float srot   = (ss->mAnim * slot->rotation);
	    float anim   = MIN (1.0, MAX (0.0, ss->mAnim));

	    float sscale;
	    float sopacity;


	    if (slot->primary)
		sscale = (ss->mAnim * slot->scale) + (1 - ss->mAnim);
	    else
		sscale = ss->mAnim * slot->scale;

	    if (slot->primary && !ss->mReflectActive)
		sopacity = (ss->mAnim * slot->opacity) + (1 - ss->mAnim);
	    else
		sopacity = anim * anim * slot->opacity;

	    if (sopacity <= 0.05)
		return status;

	    /*if (mask & PAINT_WINDOW_OCCLUSION_DETECTION_MASK)
		return false;*/

	    wAttrib.opacity = (float)wAttrib.opacity * sopacity;
	    wAttrib.brightness = (float)wAttrib.brightness *
				    ss->mReflectBrightness;

	    if (window->alpha () || wAttrib.opacity != OPAQUE)
		mask |= PAINT_WINDOW_TRANSLUCENT_MASK;

	    wTransform.translate (sx, sy, sz);

	    wTransform.translate (window->x () + (window->width ()  * sscale / 2),
			          window->y () + (window->height ()  * sscale / 2.0),
			          0.0f);

	    wTransform.scale (ss->mOutput->width (), -ss->mOutput->height (),
                	 1.0f);

	    wTransform.rotate (srot, 0.0, 1.0, 0.0);

	    wTransform.scale (1.0f  / ss->mOutput->width (),
                	 -1.0f / ss->mOutput->height (), 1.0f);

	    wTransform.scale (sscale, sscale, 1.0f);
	    wTransform.translate (-window->x () - (window->width () / 2),
				  -window->y () - (window->height () / 2), 0.0f);

	    gWindow->glDraw (wTransform, wAttrib, region,
			      mask | PAINT_WINDOW_TRANSFORMED_MASK);
	}

	if (scaled && ((ss->optionGetOverlayIcon () != ShiftOptions::OverlayIconNone) ||
	     gWindow->textures ().empty ()))
	{
	    GLTexture *icon;

	    icon = gWindow->getIcon (96, 96);
	    if (!icon)
		icon = ss->gScreen->defaultIcon ();

	    if (icon && (icon->name ()))
	    {
		CompRegion iconReg;
		float  scale;
		float  x, y;
		int    width, height;
		int    scaledWinWidth, scaledWinHeight;
		int iconOverlay = ss->optionGetOverlayIcon ();
		ShiftSlot      *slot = ss->mActiveSlot->slot;
		GLTexture::MatrixList matl;

		float sx       = ss->mAnim * slot->tx;
		float sy       = ss->mAnim * slot->ty;
		float sz       = ss->mAnim * slot->z;
		float srot     = (ss->mAnim * slot->rotation);
		float sopacity = ss->mAnim * slot->opacity;

		float sscale;

		if (slot->primary)
		    sscale = (ss->mAnim * slot->scale) + (1 - ss->mAnim);
		else
		    sscale = ss->mAnim * ss->mAnim * slot->scale;

		scaledWinWidth  = window->width ()  * sscale;
		scaledWinHeight = window->height () * sscale;

		if (gWindow->textures ().empty ())
		    iconOverlay = ShiftOptions::OverlayIconBig;

	    	switch (iconOverlay)
		{
		    case ShiftOptions::OverlayIconNone:
		    case ShiftOptions::OverlayIconEmblem:
			scale = 1.0f;
			break;
		    case ShiftOptions::OverlayIconBig:
		    default:
			/* only change opacity if not painting an
			icon for a minimized window */
			if (!gWindow->textures ().empty ())
			    sAttrib.opacity /= 3;
			scale = MIN (((float) scaledWinWidth / icon->width ()),
				    ((float) scaledWinHeight / icon->height ()));
			break;
		}

		width  = icon->width ()  * scale;
		height = icon->height () * scale;

		switch (iconOverlay)
		{
		    case ShiftOptions::OverlayIconNone:
		    case ShiftOptions::OverlayIconEmblem:
			x = scaledWinWidth - width;
			y = scaledWinHeight - height;
		    break;
		    case ShiftOptions::OverlayIconBig:
		    default:
			x = scaledWinWidth / 2 - width / 2;
			y = scaledWinHeight / 2 - height / 2;
			break;
		}

		mask |= PAINT_WINDOW_BLEND_MASK;

		/* if we paint the icon for a minimized window, we need
		   to force the usage of a good texture filter */
		if (gWindow->textures ().empty ())
		    mask |= PAINT_WINDOW_TRANSFORMED_MASK;

		iconReg = CompRegion (0, 0, icon->width (), icon->height ());

		gWindow->vertexBuffer ()->begin ();

		matl.push_back (icon->matrix ());

		gWindow->glAddGeometry (matl, iconReg, iconReg);

		if (gWindow->vertexBuffer ()->end ())
		{
		    GLWindowPaintAttrib wAttrib (sAttrib);
		    GLMatrix		wTransform (transform);

		    if (gWindow->textures ().empty ())
			sAttrib.opacity = gWindow->paintAttrib ().opacity;

		    wAttrib = GLWindowPaintAttrib (sAttrib);

		    wAttrib.opacity = (float)wAttrib.opacity * sopacity;
		    wAttrib.brightness = (float)wAttrib.brightness *
		                         ss->mReflectBrightness;

		    wTransform.translate (sx, sy, sz);

		    wTransform.translate (window->x () +
				     (window->width ()  * sscale / 2),
				     window->y () +
		                            (window->height ()  * sscale / 2.0),
		                          0.0f);

		    wTransform.scale (ss->mOutput->width (),
		                      -ss->mOutput->height (),
		                      1.0f);

		    wTransform.rotate (srot, 0.0, 1.0, 0.0);

		    wTransform.scale (1.0f  / ss->mOutput->width (),
		                      -1.0f / ss->mOutput->height (),
		                      1.0f);

		    wTransform.translate (x - (window->width () * sscale / 2),
		                          y - (window->height () * sscale / 2.0),
		                          0.0f);
		    wTransform.scale (scale, scale, 1.0f);

		    gWindow->glDrawTexture (icon, wTransform, wAttrib, mask);
		}
	    }
	}
    }
    else
    {
	GLWindowPaintAttrib sAttrib = attrib;

	if (ss->mPaintingAbove)
	{
	    sAttrib.opacity = sAttrib.opacity * (1.0 - ss->mAnim);

	    if (ss->mAnim > 0.99)
		mask |= PAINT_WINDOW_NO_CORE_INSTANCE_MASK;
	}

	status = gWindow->glPaint (sAttrib, transform, region, mask);
    }

    return status;
}

static int
compareWindows (const void *elem1,
		const void *elem2)
{
    CompWindow *w1 = *((CompWindow **) elem1);
    CompWindow *w2 = *((CompWindow **) elem2);
    CompWindow *w  = w1;

    if (w1 == w2)
	return 0;

    if (!w1->shaded () && !w1->isViewable () &&
        (w2->shaded () || w2->isViewable ()))
    {
	return 1;
    }

    if (!w2->shaded () && !w2->isViewable () &&
        (w1->shaded () || w1->isViewable ()))
    {
	return -1;
    }

    while (w)
    {
	if (w == w2)
	    return 1;
	w = w->next;
    }
    return -1;
}

static int
compareShiftWindowDistance (const void *elem1,
			    const void *elem2)
{
    float a1   = ((ShiftDrawSlot *) elem1)->distance;
    float a2   = ((ShiftDrawSlot *) elem2)->distance;
    float ab   = fabs (a1 - a2);

    if (ab > 0.3 && a1 > a2)
	return -1;
    else if (ab > 0.3 && a1 < a2)
	return 1;
    else
	return compareWindows (&((ShiftDrawSlot *) elem2)->w,
			       &((ShiftDrawSlot *) elem1)->w);
}

bool
ShiftScreen::layoutThumbsCover ()
{
    CompWindow *w;
    int index;
    int ww, wh;
    float xScale, yScale;
    float distance;
    int i;

    CompRect oe;

    if (optionGetMultioutputMode () ==
	ShiftScreen::MultioutputModeOneBigSwitcher)
    {
	oe.setGeometry (0, 0, screen->width (), screen->height ());
    }
    else
    {
	oe = screen->outputDevs ()[mUsedOutput];
    }

    /* the center of the ellipse is in the middle
       of the used output device */
    int centerX = oe.centerX ();
    int centerY = oe.centerY ();

    int maxThumbWidth  = oe.width () * optionGetSize () / 100;
    int maxThumbHeight = oe.height () * optionGetSize () / 100;

    for (index = 0; index < mNWindows; index++)
    {
	w = mWindows[index];
	SHIFT_WINDOW (w);

	ww = w->width ()  + w->border ().left + w->border ().right;
	wh = w->height () + w->border ().top  + w->border ().bottom;

	if (ww > maxThumbWidth)
	    xScale = (float)(maxThumbWidth) / (float)ww;
	else
	    xScale = 1.0f;

	if (wh > maxThumbHeight)
	    yScale = (float)(maxThumbHeight) / (float)wh;
	else
	    yScale = 1.0f;


	float val1 = floor((float) MIN (mNWindows,
					optionGetCoverMaxVisibleWindows ()) / 2.0);

	float pos;
	float space = (maxThumbWidth / 2);
	space *= cos (sin (PI / 4) * PI / 3);
	space *= 2;
	//space += (space / sin (PI / 4)) - space;

	for (i = 0; i < 2; i++)
	{
	    if (mInvert ^ (i == 0))
	    {
		distance = mMvTarget - index;
		distance += optionGetCoverOffset ();
	    }
	    else
	    {
		distance = mMvTarget - index + mNWindows;
		distance += optionGetCoverOffset ();
		if (distance > mNWindows)
		    distance -= mNWindows * 2;
	    }


	    pos = MIN (1.0, MAX (-1.0, distance));

	    sw->mSlots[i].opacity = 1.0 - MIN (1.0, MAX (0.0, fabs(distance) - val1));
	    sw->mSlots[i].scale   = MIN (xScale, yScale);

	    sw->mSlots[i].y = centerY + (maxThumbHeight / 2.0) -
				(((w->height () / 2.0) + w->border ().bottom) *
				sw->mSlots[i].scale);

	    if (fabs(distance) < 1.0)
	    {
		sw->mSlots[i].x  = centerX + (sin(pos * PI * 0.5) * space * optionGetCoverExtraSpace ());
		sw->mSlots[i].z  = fabs (distance);
		sw->mSlots[i].z *= -(maxThumbWidth / (2.0 * oe.width ()));

		sw->mSlots[i].rotation = sin(pos * PI * 0.5) * -optionGetCoverAngle ();
	    }
	    else
	    {
		float rad = (space / oe.width ()) / sin(PI / 6.0);

		float ang = (PI / MAX(72.0, mNWindows * 2)) *
			    (distance - pos) + (pos * (PI / 6.0));

		sw->mSlots[i].x  = centerX;
		sw->mSlots[i].x += sin(ang) * rad * oe.width () * optionGetCoverExtraSpace ();

		sw->mSlots[i].rotation  = optionGetCoverAngle () + 30;
		sw->mSlots[i].rotation -= fabs(ang) * 180.0 / PI;
		sw->mSlots[i].rotation *= -pos;

		sw->mSlots[i].z  = -(maxThumbWidth / (2.0 * oe.width ()));
		sw->mSlots[i].z += -(cos(PI / 6.0) * rad);
		sw->mSlots[i].z += (cos(ang) * rad);
	    }

	    mDrawSlots[index * 2 + i].w     = w;
	    mDrawSlots[index * 2 + i].slot  = &sw->mSlots[i];
	    mDrawSlots[index * 2 + i].distance = fabs(distance);

	}

	if (mDrawSlots[index * 2].distance >
	    mDrawSlots[index * 2 + 1].distance)
	{
	    mDrawSlots[index * 2].slot->primary     = false;
	    mDrawSlots[index * 2 + 1].slot->primary = true;
	}
	else
	{
	    mDrawSlots[index * 2].slot->primary     = true;
	    mDrawSlots[index * 2 + 1].slot->primary = false;
	}

    }

    mNSlots = mNWindows * 2;

    qsort (mDrawSlots, mNSlots, sizeof (ShiftDrawSlot),
	   compareShiftWindowDistance);

    return true;
}

bool
ShiftScreen::layoutThumbsFlip ()
{
    CompWindow *w;
    int index;
    int ww, wh;
    float xScale, yScale;
    float distance;
    int i;
    float angle;
    int slotNum;

    CompRect oe;

    if (optionGetMultioutputMode () ==
	ShiftOptions::MultioutputModeOneBigSwitcher)
    {
	oe.setGeometry (0, 0, screen->width (), screen->height ());
    }
    else
    {
	oe = screen->outputDevs ()[mUsedOutput];
    }

    /* the center of the ellipse is in the middle
       of the used output device */
    int centerX = oe.centerX ();
    int centerY = oe.centerY ();

    int maxThumbWidth  = oe.width () * optionGetSize () / 100;
    int maxThumbHeight = oe.height () * optionGetSize () / 100;

    slotNum = 0;

    for (index = 0; index < mNWindows; index++)
    {
	w = mWindows[index];
	SHIFT_WINDOW (w);

	ww = w->width ()  + w->border ().left + w->border ().right;
	wh = w->height () + w->border ().top  + w->border ().bottom;

	if (ww > maxThumbWidth)
	    xScale = (float)(maxThumbWidth) / (float)ww;
	else
	    xScale = 1.0f;

	if (wh > maxThumbHeight)
	    yScale = (float)(maxThumbHeight) / (float)wh;
	else
	    yScale = 1.0f;

	angle = optionGetFlipRotation () * PI / 180.0;

	for (i = 0; i < 2; i++)
	{
	    if (mInvert ^ (i == 0))
		distance = mMvTarget - index;
	    else
	    {
		distance = mMvTarget - index + mNWindows;
		if (distance > 1.0)
		    distance -= mNWindows * 2;
	    }

	    if (distance > 0.0)
		sw->mSlots[i].opacity = MAX (0.0, 1.0 - (distance * 1.0));
	    else
	    {
		if (distance < -(mNWindows - 1))
		    sw->mSlots[i].opacity = MAX (0.0, mNWindows +
						distance);
		else
		    sw->mSlots[i].opacity = 1.0;
	    }

	    if (distance > 0.0 && w->id () != mSelectedWindow)
		sw->mSlots[i].primary = false;
	    else
		sw->mSlots[i].primary = true;


	    sw->mSlots[i].scale   = MIN (xScale, yScale);

	    sw->mSlots[i].y = centerY + (maxThumbHeight / 2.0) -
				(((w->height () / 2.0) + w->border ().bottom) *
				sw->mSlots[i].scale);

	    sw->mSlots[i].x  = sin(angle) * distance * (maxThumbWidth / 2);
	    if (distance > 0 && false)
		sw->mSlots[i].x *= 1.5;
	    sw->mSlots[i].x += centerX;

	    sw->mSlots[i].z  = cos(angle) * distance;
	    if (distance > 0)
		sw->mSlots[i].z *= 1.5;
	    sw->mSlots[i].z *= (maxThumbWidth / (2.0 * oe.width ()));

	    sw->mSlots[i].rotation = optionGetFlipRotation ();

	    if (sw->mSlots[i].opacity > 0.0)
	    {
		mDrawSlots[slotNum].w     = w;
		mDrawSlots[slotNum].slot  = &sw->mSlots[i];
		mDrawSlots[slotNum].distance = -distance;
		slotNum++;
	    }
	}
    }

    mNSlots = slotNum;

    qsort (mDrawSlots, mNSlots, sizeof (ShiftDrawSlot),
	   compareShiftWindowDistance);

    return true;
}


bool
ShiftScreen::layoutThumbs ()
{
    bool result = false;

    if (mState == ShiftStateNone)
	return false;

    switch (optionGetMode ())
    {
	case ShiftScreen::ModeCover:
	    result = layoutThumbsCover ();
	    break;
	case ShiftScreen::ModeFlip:
	    result = layoutThumbsFlip ();
	    break;
    }

    if (mState == ShiftStateIn)
    	return false;

    return result;
}


void
ShiftScreen::addWindowToList (CompWindow *w)
{
    if (mWindowsSize <= mNWindows)
    {
	mWindows = (CompWindow **) realloc (mWindows,
			       sizeof (CompWindow *) * (mNWindows + 32));
	if (!mWindows)
	    return;

	mWindowsSize = mNWindows + 32;
    }

    if (mSlotsSize <= mNWindows * 2)
    {
	mDrawSlots = (ShiftDrawSlot *) realloc (mDrawSlots,
				 sizeof (ShiftDrawSlot) *
				 ((mNWindows * 2) + 64));

	if (!mDrawSlots)
	{
	    free (mDrawSlots);
	    return;
	}

	mSlotsSize = (mNWindows * 2) + 64;
    }

    mWindows[mNWindows++] = w;
}

bool
ShiftScreen::updateWindowList ()
{
    int        i, idx;
    CompWindow **wins;

    qsort (mWindows, mNWindows, sizeof (CompWindow *), compareWindows);

    mMvTarget = 0;
    mMvAdjust = 0;
    mMvVelocity = 0;
    for (i = 0; i < mNWindows; i++)
    {
	if (mWindows[i]->id () == mSelectedWindow)
	    break;

	mMvTarget++;
    }
    if (mMvTarget == mNWindows)
	mMvTarget = 0;

    /* create spetial window order to create a good animation
       A,B,C,D,E --> A,B,D,E,C to get B,D,E,C,(A),B,D,E,C as initial state */
    if (optionGetMode () == ShiftScreen::ModeCover)
    {
	wins = (CompWindow **) malloc(mNWindows * sizeof (CompWindow *));
	if (!wins)
	    return false;

	memcpy(wins, mWindows, mNWindows * sizeof (CompWindow *));
	for (i = 0; i < mNWindows; i++)
	{
	    idx = ceil (i * 0.5);
	    idx *= (i & 1) ? 1 : -1;
	    if (idx < 0)
		idx += mNWindows;
	    mWindows[idx] = wins[i];
	}
	free (wins);
    }

    return layoutThumbs ();
}

bool
ShiftScreen::createWindowList ()
{
    mNWindows = 0;

    foreach (CompWindow *w, screen->windows ())
    {
	SHIFT_WINDOW (w);
	if (sw->isShiftable ())
	{
	    addWindowToList (w);
	    sw->mActive = true;
	}
    }

    return updateWindowList ();
}

void
ShiftScreen::switchToWindow (bool	   toNext)
{
    CompWindow *w;
    int	       cur;

    if (!mGrabIndex)
	return;

    for (cur = 0; cur < mNWindows; cur++)
    {
	if (mWindows[cur]->id () == mSelectedWindow)
	    break;
    }

    if (cur == mNWindows)
	return;

    if (toNext)
	w = mWindows[(cur + 1) % mNWindows];
    else
	w = mWindows[(cur + mNWindows - 1) % mNWindows];

    if (w)
    {
	Window old = mSelectedWindow;
	mSelectedWindow = w->id ();

	if (old != w->id ())
	{
	    if (toNext)
		mMvAdjust += 1;
	    else
		mMvAdjust -= 1;

	    mMoveAdjust = true;
	    cScreen->damageScreen ();
	    renderWindowTitle ();
	}
    }
}

int
ShiftScreen::countWindows ()
{
    int	       count = 0;

    foreach (CompWindow *w, screen->windows ())
    {
	if (ShiftWindow::get (w)->isShiftable ())
	    count++;
    }

    return count;
}

int
ShiftScreen::adjustShiftMovement (float chunk)
{
    float dx, adjust, amount;
    float change;

    dx = mMvAdjust;

    adjust = dx * 0.15f;
    amount = fabs(dx) * 1.5f;
    if (amount < 0.2f)
	amount = 0.2f;
    else if (amount > 2.0f)
	amount = 2.0f;

    mMvVelocity = (amount * mMvVelocity + adjust) / (amount + 1.0f);

    if (fabs (dx) < 0.002f && fabs (mMvVelocity) < 0.004f)
    {
	mMvVelocity = 0.0f;
	mMvTarget = mMvTarget + mMvAdjust;
	mMvAdjust = 0;
	layoutThumbs ();
	return false;
    }

    change = mMvVelocity * chunk;
    if (!change)
    {
	if (mMvVelocity)
	    change = (mMvAdjust > 0) ? 0.01 : -0.01;
    }

    mMvAdjust -= change;
    mMvTarget += change;

    while (mMvTarget >= mNWindows)
    {
	mMvTarget -= mNWindows;
	mInvert = !mInvert;
    }

    while (mMvTarget < 0)
    {
	mMvTarget += mNWindows;
	mInvert = !mInvert;
    }

    if (!layoutThumbs ())
	return false;

    return true;
}

bool
ShiftWindow::adjustShiftAttribs (float chunk)
{
    float dp, db, adjust, amount;
    float opacity, brightness;

    SHIFT_SCREEN (screen);

    if ((mActive && ss->mState != ShiftStateIn &&
	ss->mState != ShiftStateNone) ||
	(ss->optionGetHideAll () && !(window->type () & CompWindowTypeDesktopMask) &&
	(ss->mState == ShiftStateOut || ss->mState == ShiftStateSwitching ||
	 ss->mState == ShiftStateFinish)))
	opacity = 0.0;
    else
	opacity = 1.0;

    if (ss->mState == ShiftStateIn || ss->mState == ShiftStateNone)
	brightness = 1.0;
    else
	brightness = ss->optionGetBackgroundIntensity ();

    dp = opacity - mOpacity;
    adjust = dp * 0.1f;
    amount = fabs (dp) * 7.0f;
    if (amount < 0.01f)
	amount = 0.01f;
    else if (amount > 0.15f)
	amount = 0.15f;

    mOpacityVelocity = (amount * mOpacityVelocity + adjust) /
	(amount + 1.0f);

    db = brightness - mBrightness;
    adjust = db * 0.1f;
    amount = fabs (db) * 7.0f;
    if (amount < 0.01f)
	amount = 0.01f;
    else if (amount > 0.15f)
	amount = 0.15f;

    mBrightnessVelocity = (amount * mBrightnessVelocity + adjust) /
	(amount + 1.0f);

    /* FIXME: There is a possible floating point overflow here,
     * can be worked-around but not particularly nice */

    if ((fabs (dp) < 0.1f && fabs (mOpacityVelocity) < 0.2f &&
	fabs (db) < 0.1f && fabs (mBrightnessVelocity) < 0.2f) ||
	(fabs(db) != fabs (db) || fabs (mOpacityVelocity) != fabs (mOpacityVelocity) ||
	fabs (dp) != fabs (dp) || fabs (mBrightnessVelocity) != fabs (mBrightnessVelocity)))
    {
	mBrightness = brightness;
	mOpacity = opacity;
	return false;
    }

    mBrightness += mBrightnessVelocity * chunk;
    mOpacity += mOpacityVelocity * chunk;
    return true;
}

bool
ShiftScreen::adjustShiftAnimationAttribs (float chunk)
{
    float dr, adjust, amount;
    float anim;

    if (mState != ShiftStateIn && mState != ShiftStateNone)
	anim = 1.0;
    else
	anim = 0.0;

    dr = anim - mAnim;
    adjust = dr * 0.1f;
    amount = fabs (dr) * 7.0f;
    if (amount < 0.002f)
	amount = 0.002f;
    else if (amount > 0.15f)
	amount = 0.15f;

    mAnimVelocity = (amount * mAnimVelocity + adjust) /
	(amount + 1.0f);

    if (fabs (dr) < 0.002f && fabs (mAnimVelocity) < 0.004f)
    {
	mAnim = anim;
	return false;
    }

    mAnim += mAnimVelocity * chunk;
    return true;
}

bool
ShiftScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
			    const GLMatrix	      &transform,
			    const CompRegion	      &region,
			    CompOutput		      *output,
			    unsigned int	      mask)
{
    bool status;

    if (mState != ShiftStateNone)
	mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK;

    mPaintingAbove = false;

    mOutput = output;

    status = gScreen->glPaintOutput (attrib, transform, region, output, mask);

    if (mState != ShiftStateNone &&
	((unsigned int) output->id () == (unsigned int) mUsedOutput ||
	((unsigned int) output->id () == (unsigned int) ~0)))
    {
	CompWindow    *w;
	GLMatrix      sTransform (transform);
	int           i;
	int           oy1 = screen->outputDevs ()[mUsedOutput].region ()->extents.y1;
	int           oy2 = screen->outputDevs ()[mUsedOutput].region ()->extents.y2;
	int           maxThumbHeight = (oy2 - oy1) * optionGetSize () / 100;
	int           oldFilter = gScreen->textureFilter ();

	if (optionGetMultioutputMode () == ShiftOptions::MultioutputModeOneBigSwitcher)
	{
	    oy1 = 0;
	    oy2 = screen->height ();
	}

	sTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);

	/* Reflection drawing */

	if (optionGetReflection ())
	{
	    GLMatrix	   rTransform = sTransform;
	    GLMatrix        r2Transform;
	    GLushort        colorData[4];
	    GLfloat         vertexData[12];
	    int            cull, cullInv;
	    GLVertexBuffer *streamingBuffer = GLVertexBuffer::streamingBuffer ();

	    glGetIntegerv (GL_CULL_FACE_MODE, &cull);
	    cullInv = (cull == GL_BACK)? GL_FRONT : GL_BACK;

	    rTransform.translate (0.0, oy1 + oy2 + maxThumbHeight, 0.0);
	    rTransform.scale (1.0, -1.0, 1.0);

	    glCullFace (cullInv);

	    if (optionGetMipmaps ())
		gScreen->setTextureFilter (GL_LINEAR_MIPMAP_LINEAR);

	    mReflectActive = true;
	    mReflectBrightness = optionGetIntensity ();
	    for (i = 0; i < mNSlots; i++)
	    {
		w = mDrawSlots[i].w;

		SHIFT_WINDOW (w);

		mActiveSlot = &mDrawSlots[i];
		{
		    sw->gWindow->glPaint (sw->gWindow->paintAttrib (), rTransform,
					  infiniteRegion, 0);
		}
	    }

	    glCullFace (cull);
	    glEnable(GL_BLEND);
	    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	    r2Transform.translate (0.0, 0.0, -DEFAULT_Z_CAMERA);

	    streamingBuffer->begin (GL_TRIANGLE_STRIP);

	    colorData[0] = 0;
	    colorData[1] = 0;
	    colorData[2] = 0;
	    colorData[3] = 0;
	    streamingBuffer->addColors (1, colorData);
	    streamingBuffer->addColors (1, colorData);

	    colorData[3] = MIN (1.0,
	                        1.0 - optionGetIntensity ()) * 2.0 * mAnim;
	    streamingBuffer->addColors (1, colorData);
	    streamingBuffer->addColors (1, colorData);

	    vertexData[0]  = 0.5;
	    vertexData[1]  = 0;
	    vertexData[2]  = 0;
	    vertexData[3]  = -0.5;
	    vertexData[4]  = 0;
	    vertexData[5]  = 0;
	    vertexData[6]  = 0.5;
	    vertexData[7]  = -0.5;
	    vertexData[8]  = 0;
	    vertexData[9]  = -0.5;
	    vertexData[10] = -0.5;
	    vertexData[11] = 0;

	    streamingBuffer->addVertices (4, vertexData);

	    streamingBuffer->end ();
	    streamingBuffer->render (r2Transform);

	    if (optionGetGroundSize () > 0.0)
	    {
		streamingBuffer->begin (GL_TRIANGLE_STRIP);

		colorData[0] = optionGetGroundColor1 ()[0];
		colorData[1] = optionGetGroundColor1 ()[1];
		colorData[2] = optionGetGroundColor1 ()[2];
		colorData[3] = (float)optionGetGroundColor1 ()[3] * mAnim;
		streamingBuffer->addColors (1, colorData);
		streamingBuffer->addColors (1, colorData);

		colorData[3] = (float)optionGetGroundColor2 ()[3] * mAnim;
		streamingBuffer->addColors (1, colorData);
		streamingBuffer->addColors (1, colorData);

		vertexData[0]  = -0.5;
		vertexData[1]  = -0.5;
		vertexData[2]  = 0;
		vertexData[3]  = 0.5;
		vertexData[4]  = -0.5;
		vertexData[5]  = 0;
		vertexData[6]  = -0.5;
		vertexData[7]  = -0.5 + optionGetGroundSize ();
		vertexData[8]  = 0;
		vertexData[9]  = 0.5;
		vertexData[10] = -0.5 + optionGetGroundSize ();
		vertexData[11] = 0;

		streamingBuffer->addVertices (4, vertexData);

		streamingBuffer->end ();
		streamingBuffer->render (r2Transform);
	    }

	    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	    glDisable(GL_BLEND);
	}

	/* Drawing normal windows */

	mReflectBrightness = 1.0;
	mReflectActive     = false;

	for (i = 0; i < mNSlots; i++)
	{
	    w = mDrawSlots[i].w;

	    SHIFT_WINDOW (w);

	    mActiveSlot = &mDrawSlots[i];
	    {
		sw->gWindow->glPaint (sw->gWindow->paintAttrib (), sTransform,
				      infiniteRegion, 0);
	    }
	}

	mActiveSlot = NULL;

	gScreen->setTextureFilter (oldFilter);

	if (textAvailable && (mState != ShiftStateIn))
	    drawWindowTitle (sTransform);

	if (mState == ShiftStateIn || mState == ShiftStateOut)
	{
	    bool found;
	    mPaintingAbove = true;

	    w = screen->findWindow (mSelectedWindow);

	    for (; w; w = w->next)
	    {
		if (w->destroyed ())
		    continue;

		if (!w->shaded ())
		{
		    if (!w->isViewable () || !CompositeWindow::get (w)->damaged ())
			continue;
		}

		found = false;
		for (i = 0; i < mNWindows; i++)
		    if (mWindows[i] == w)
			found = true;
		if (found)
		    continue;

		SHIFT_WINDOW (w);

		sw->gWindow->glPaint (sw->gWindow->paintAttrib (), sTransform,
				      infiniteRegion, 0);
	    }

	    mPaintingAbove = false;
	}
    }

    return status;
}

void
ShiftScreen::paint (CompOutput::ptrList &outputs,
		    unsigned int       mask)
{

    if (mState != ShiftStateNone && outputs.size () > 0 &&
        optionGetMultioutputMode () == ShiftOptions::MultioutputModeOneBigSwitcher)
    {
	CompOutput::ptrList newOutputs;
	newOutputs.push_back (&screen->fullscreenOutput ());

	cScreen->paint (newOutputs, mask);
	return;
    }

    cScreen->paint (outputs, mask);
    return;
}

void
ShiftScreen::preparePaint (int	msSinceLastPaint)
{
    if (mState != ShiftStateNone &&
	(mMoreAdjust || mMoveAdjust))
    {
	int        steps;
	float      amount, chunk;
	int        i;

	amount = msSinceLastPaint * 0.05f * optionGetShiftSpeed ();
	steps  = amount / (0.5f * optionGetTimestep ());

	if (!steps)
	    steps = 1;
	chunk  = amount / (float) steps;


	while (steps--)
	{
	    mMoveAdjust = adjustShiftMovement (chunk);
	    if (!mMoveAdjust)
		break;
	}

	amount = msSinceLastPaint * 0.05f * optionGetSpeed ();
	steps  = amount / (0.5f * optionGetTimestep ());

	if (!steps)
	    steps = 1;
	chunk  = amount / (float) steps;

	while (steps--)
	{
	    mMoreAdjust = adjustShiftAnimationAttribs (chunk);

	    foreach (CompWindow *w, screen->windows ())
	    {
		SHIFT_WINDOW (w);

		mMoreAdjust |= sw->adjustShiftAttribs (chunk);
		for (i = 0; i < 2; i++)
		{
		    ShiftSlot *slot = &sw->mSlots[i];
		    slot->tx = slot->x - w->x () -
			(w->width () * slot->scale) / 2;
		    slot->ty = slot->y - w->y () -
			(w->height () * slot->scale) / 2;
		}
	    }

	    if (!mMoreAdjust)
		break;
	}
    }

    cScreen->preparePaint (msSinceLastPaint);
}

bool
ShiftWindow::canStackRelativeTo ()
{
    if (window->overrideRedirect ())
        return false;

    if (!window->shaded () && !window->pendingMaps ())
    {
        if (!window->isViewable () || window->mapNum () == 0)
            return false;
    }

    return true;
}

void
ShiftScreen::donePaint ()
{
    if (mState != ShiftStateNone)
    {
	if (mMoreAdjust)
	{
	    cScreen->damageScreen ();
	}
	else
	{
	    if (mState == ShiftStateIn)
	    {
		mState = ShiftStateNone;
		activateEvent (false);
		foreach (CompWindow *w, screen->windows ())
		{
		    SHIFT_WINDOW (w);
		    sw->mActive = false;
		}
		setFunctions (false);
		cScreen->damageScreen ();
	    }
	    else if (mState == ShiftStateOut)
		     mState = ShiftStateSwitching;

	    if (mMoveAdjust)
	    {
		cScreen->damageScreen ();
	    }
	    else if (mState == ShiftStateFinish)
	    {

		CompWindow *w;

		CompWindow *pw = NULL;

		mState = ShiftStateIn;
		mMoreAdjust = true;
		cScreen->damageScreen ();

		if (!mCancelled && mMvTarget != 0)
		{
		    int i;
		    for (i = 0; i < mNSlots; i++)
		    {
			w = mDrawSlots[i].w;

			SHIFT_WINDOW (w);

			if (mDrawSlots[i].slot->primary && sw->canStackRelativeTo ())
			{
			    if (pw)
				w->restackAbove (pw);
			    pw = w;
			}
		    }
		}

		if (!mCancelled && mSelectedWindow)
		{
		    w = screen->findWindow (mSelectedWindow);
		    if (w)
			screen->sendWindowActivationRequest (mSelectedWindow);

		}
	    }

	    cScreen->damageScreen ();
	}
    }

    cScreen->donePaint ();
}

void
ShiftScreen::term (bool cancel)
{
    if (mGrabIndex)
    {
        screen->removeGrab (mGrabIndex, 0);
        mGrabIndex = 0;
    }

    if (mState != ShiftStateNone)
    {
	if (cancel && mMvTarget != 0)
	{
	    if (mNWindows - mMvTarget > mMvTarget)
		mMvAdjust = -mMvTarget;
	    else
		mMvAdjust = mNWindows - mMvTarget;
	    mMoveAdjust = true;
	}

	mMoreAdjust = false;
	mMoveAdjust = false;
	mState = ShiftStateFinish;
	mCancelled = cancel;
	cScreen->damageScreen ();
    }
}

bool
ShiftScreen::terminate (CompAction         *action,
			CompAction::State	   state,
			CompOption::Vector &options)
{
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "root", 0);

    if (!(xid && screen->root () != xid))
    {
	term ((state & CompAction::StateCancel));

	if (action->state () & CompAction::StateTermButton)
	    action->setState (action->state () & ~CompAction::StateTermButton);

	if (action->state () & CompAction::StateTermKey)
	    action->setState (action->state () & ~CompAction::StateTermKey);
    }

    return false;
}

bool
ShiftScreen::initiateScreen (CompAction         *action,
			     CompAction::State    state,
			     CompOption::Vector &options)
{
    CompMatch match;
    int       count;

    if (screen->otherGrabExist ("shift", NULL))
	return false;

    mCurrentMatch = &(optionGetWindowMatch ());

    match = CompOption::getMatchOptionNamed (options, "match", CompMatch::emptyMatch);
    if (match != CompMatch::emptyMatch)
    {
	mMatch = match;
	mMatch.update ();
	mCurrentMatch = &mMatch;
    }

    count = countWindows ();

    if (count < 1)
	return false;

    if (!mGrabIndex)
	mGrabIndex = screen->pushGrab (screen->invisibleCursor (), "shift");


    if (mGrabIndex)
    {
	mState = ShiftStateOut;
	activateEvent (true);

	if (!createWindowList ())
	    return false;

    	mSelectedWindow = mWindows[0]->id ();
	renderWindowTitle ();
	mMvTarget = 0;
	mMvAdjust = 0;
	mMvVelocity = 0;

    	mMoreAdjust = true;
	cScreen->damageScreen ();
    }

    mUsedOutput = screen->currentOutputDev ().id ();

    setFunctions (true);

    return true;
}

bool
ShiftScreen::doSwitch (CompAction         *action,
		       CompAction::State    state,
		       CompOption::Vector &options,
		       bool		  nextWindow,
		       ShiftType	  type)
{
    bool       ret = true;
    bool       initial = false;

    if ((mState == ShiftStateNone) || (mState == ShiftStateIn))
    {
	if (type == ShiftTypeGroup)
	{
	    CompWindow *w;
	    w = screen->findWindow (CompOption::getIntOptionNamed (options,
								   "window",
								   0));
	    if (w)
	    {
		mType = ShiftTypeGroup;
		mClientLeader =
		    (w->clientLeader ()) ? w->clientLeader () : w->id ();
		ret = initiateScreen (action, state, options);
	    }
	}
	else
	{
	    mType = type;
	    ret = initiateScreen (action, state, options);
	}

	if (state & CompAction::StateInitKey)
	    action->setState (state | CompAction::StateTermKey);

	if (state & CompAction::StateInitButton)
	    action->setState (state | CompAction::StateTermButton);

	if (state & CompAction::StateInitEdge)
	    action->setState (state | CompAction::StateTermEdge);

	initial = true;
    }

    if (ret)
    {
	switchToWindow (nextWindow);
	if (initial && false)
	{
	    mMvTarget += mMvAdjust;
	    mMvAdjust  = 0.0;
	}
    }


    return ret;
}

bool
ShiftScreen::initiate (CompAction         *action,
		       CompAction::State    state,
		       CompOption::Vector &options)
{
    bool       ret = true;

    mType = ShiftTypeNormal;

    if ((mState == ShiftStateNone) || (mState == ShiftStateIn) ||
	(mState == ShiftStateFinish))
	ret = initiateScreen (action, state, options);
    else
	ret = terminate (action, state, options);

    if (state & CompAction::StateTermButton)
	action->setState (state & ~CompAction::StateTermButton);

    if (state & CompAction::StateTermKey)
	action->setState (state & ~CompAction::StateTermKey);

    return ret;
}

bool
ShiftScreen::initiateAll (CompAction         *action,
			  CompAction::State    state,
			  CompOption::Vector &options)
{
    bool       ret = true;

    mType = ShiftTypeAll;

    if ((mState == ShiftStateNone) || (mState == ShiftStateIn) ||
	(mState == ShiftStateFinish))
	ret = initiateScreen (action, state, options);
    else
	ret = terminate (action, state, options);

    if (state & CompAction::StateTermButton)
	action->setState (state & ~CompAction::StateTermButton);

    if (state & CompAction::StateTermKey)
	action->setState (state & ~CompAction::StateTermKey);


    return ret;
}
void
ShiftScreen::windowRemove (Window id)
{
    CompWindow *w = screen->findWindow (id);
    if (w)
    {
	bool inList = false;
	int j, i = 0;
	Window selected;

	SHIFT_WINDOW (w);

	if (mState == ShiftStateNone)
	    return;

	if (sw->isShiftable ())
    	    return;

	selected = mSelectedWindow;

	while (i < mNWindows)
	{
    	    if (w->id () == mWindows[i]->id ())
	    {
		inList = true;

		if (w->id () == selected)
		{
		    if (i < (mNWindows - 1))
			selected = mWindows[i + 1]->id ();
    		    else
			selected = mWindows[0]->id ();

		    mSelectedWindow = selected;
		}

		mNWindows--;
		for (j = i; j < mNWindows; j++)
		    mWindows[j] = mWindows[j + 1];
	    }
	    else
	    {
		i++;
	    }
	}

	if (!inList)
	    return;

	if (mNWindows == 0)
	{
	    CompOption o;
	    CompOption::Vector opts;

	    o = CompOption ("root", CompOption::TypeInt);
	    o.value ().set ((int) screen->root ());

	    opts.push_back (o);

	    terminate (NULL, 0, opts);
	    return;
	}

	// Let the window list be updated to avoid crash
	// when a window is closed while ending shift (ShiftStateIn).
	if (!mGrabIndex && mState != ShiftStateIn)
	    return;

	if (updateWindowList ())
	{
	    mMoreAdjust = true;
	    mState = ShiftStateOut;
	    cScreen->damageScreen ();
	}
    }
}

void
ShiftScreen::handleEvent (XEvent      *event)
{
    screen->handleEvent (event);

    switch (event->type) {
    case PropertyNotify:
	if (event->xproperty.atom == XA_WM_NAME)
	{
	    CompWindow *w;
	    w = screen->findWindow (event->xproperty.window);
	    if (w)
	    {
    		if (mGrabIndex && (w->id () == mSelectedWindow))
    		{
    		    renderWindowTitle ();
    		    cScreen->damageScreen ();
		}
	    }
	}
	break;
    case UnmapNotify:
	windowRemove (event->xunmap.window);
	break;
    case DestroyNotify:
	windowRemove (event->xdestroywindow.window);
	break;
    case KeyPress:
	if (mState == ShiftStateSwitching)
	{
	    if (event->xkey.keycode == mLeftKey)
		switchToWindow (false);
	    else if (event->xkey.keycode == mRightKey)
		switchToWindow (true);
	    else if (event->xkey.keycode == mUpKey)
		switchToWindow (false);
	    else if (event->xkey.keycode == mDownKey)
		switchToWindow (true);
	}

	break;
    case ButtonPress:
	if (mState == ShiftStateSwitching || mState == ShiftStateOut)
	{
	    if (event->xbutton.button == Button5)
		switchToWindow (false);
	    else if (event->xbutton.button == Button4)
		switchToWindow (true);
	    if (event->xbutton.button == Button1)
	    {
		mButtonPressTime = event->xbutton.time;
		mButtonPressed   = true;
		mStartX          = event->xbutton.x_root;
		mStartY          = event->xbutton.y_root;
		mStartTarget     = mMvTarget + mMvAdjust;
	    }
	}

	break;
    case ButtonRelease:
	if (mState == ShiftStateSwitching || mState == ShiftStateOut)
	{
	    if (event->xbutton.button == Button1 && mButtonPressed)
	    {
		int iNew;
		if ((int)(event->xbutton.time - mButtonPressTime) <
		    optionGetClickDuration ())
		    term (false);

		mButtonPressTime = 0;
		mButtonPressed   = false;

		if (mMvTarget - floor (mMvTarget) >= 0.5)
		{
		    mMvAdjust = ceil(mMvTarget) - mMvTarget;
		    iNew = ceil(mMvTarget);
		}
		else
		{
		    mMvAdjust = floor(mMvTarget) - mMvTarget;
		    iNew = floor(mMvTarget);
		}

		while (iNew < 0)
		    iNew += mNWindows;
		iNew = iNew % mNWindows;

		mSelectedWindow = mWindows[iNew]->id ();

		renderWindowTitle ();
		mMoveAdjust = true;
		cScreen->damageScreen ();
	    }

	}
	break;
    case MotionNotify:
	if (mState == ShiftStateSwitching || mState == ShiftStateOut)
	{
	    if (mButtonPressed)
	    {
		CompRect oe = screen->outputDevs ()[mUsedOutput];
		float div = 0;
		int   wx  = 0;
		int   wy  = 0;
		int   iNew;

		switch (optionGetMode ())
		{
		    case ShiftOptions::ModeCover:
			div = event->xmotion.x_root - mStartX;
			div /= oe.width () / optionGetMouseSpeed ();
			break;
		    case ShiftOptions::ModeFlip:
			div = event->xmotion.y_root - mStartY;
			div /= oe.height () / optionGetMouseSpeed ();
			break;
		}

		mMvTarget = mStartTarget + div - mMvAdjust;
		mMoveAdjust = true;
		while (mMvTarget >= mNWindows)
		{
		    mMvTarget -= mNWindows;
		    mInvert = !mInvert;
		}

		while (mMvTarget < 0)
		{
		    mMvTarget += mNWindows;
		    mInvert = !mInvert;
		}

		if (mMvTarget - floor (mMvTarget) >= 0.5)
		    iNew = ceil(mMvTarget);
		else
		    iNew = floor(mMvTarget);

		while (iNew < 0)
		    iNew += mNWindows;
		iNew = iNew % mNWindows;

		if (mSelectedWindow != mWindows[iNew]->id ())
		{
		    mSelectedWindow = mWindows[iNew]->id ();
		    renderWindowTitle ();
		}

		if (event->xmotion.x_root < 50)
		    wx = 50;
		if (screen->width () - event->xmotion.x_root < 50)
		    wx = -50;
		if (event->xmotion.y_root < 50)
		    wy = 50;
		if (screen->height () - event->xmotion.y_root < 50)
		    wy = -50;
		if (wx != 0 || wy != 0)
		{
		    screen->warpPointer (wx, wy);
		    mStartX += wx;
		    mStartY += wy;
		}

		cScreen->damageScreen ();
	    }

	}
	break;
    }
}

bool
ShiftWindow::damageRect (bool initial,
			 const CompRect &rect)
{
    bool status = false;

    SHIFT_SCREEN (screen);

    if (initial)
    {
	if (ss->mGrabIndex && isShiftable ())
	{
	    ss->addWindowToList (window);
	    if (ss->updateWindowList ())
	    {
    		mActive = true;
		ss->mMoreAdjust = true;
		ss->mState = ShiftStateOut;
		ss->cScreen->damageScreen ();
	    }
	}
    }
    else if (ss->mState == ShiftStateSwitching)
    {
	if (mActive)
	{
	    ss->cScreen->damageScreen ();
	    status = true;
	}
    }

    status |= cWindow->damageRect (initial, rect);

    return status;
}

ShiftScreen::ShiftScreen (CompScreen *screen) :
    PluginClassHandler <ShiftScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    mLeftKey (XKeysymToKeycode (screen->dpy (), XStringToKeysym ("Left"))),
    mRightKey (XKeysymToKeycode (screen->dpy (), XStringToKeysym ("Right"))),
    mUpKey (XKeysymToKeycode (screen->dpy (), XStringToKeysym ("Up"))),
    mDownKey (XKeysymToKeycode (screen->dpy (), XStringToKeysym ("Down"))),
    mGrabIndex (0),
    mState (ShiftStateNone),
    mMoreAdjust (false),
    mMvTarget (0),
    mMvVelocity (0),
    mInvert (false),
    mCursor (XCreateFontCursor (screen->dpy (), XC_left_ptr)),
    mWindows (NULL),
    mNWindows (0),
    mWindowsSize (0),
    mDrawSlots (NULL),
    mNSlots (0),
    mSlotsSize (0),
    mActiveSlot (NULL),
    mSelectedWindow (0),
    mCurrentMatch (NULL),
    mUsedOutput (0),
    mAnim (0.0),
    mAnimVelocity (0.0),
    mButtonPressTime (0),
    mButtonPressed (false),
    mStartX (0),
    mStartY (0),
    mStartTarget (0.0f)
{
    ScreenInterface::setHandler (screen, false);
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);

#define SHIFTINITBIND(opt, func)                                \
    optionSet##opt##Initiate (boost::bind (&ShiftScreen::func, \
					    this, _1, _2, _3));

#define SHIFTTERMBIND(opt, func)                                \
    optionSet##opt##Terminate (boost::bind (&ShiftScreen::func, \
					    this, _1, _2, _3));

#define SHIFTSWITCHBIND(opt, func, next, type)                 \
    optionSet##opt##Initiate (boost::bind (&ShiftScreen::func, \
					    this, _1, _2, _3, \
					    next, type));

    /* Key actions */

    SHIFTINITBIND (InitiateKey, initiate);
    SHIFTINITBIND (InitiateAllKey, initiateAll);

    SHIFTSWITCHBIND (NextKey, doSwitch, true, ShiftTypeNormal);
    SHIFTSWITCHBIND (PrevKey, doSwitch, false, ShiftTypeNormal);
    SHIFTSWITCHBIND (NextAllKey, doSwitch, true, ShiftTypeAll);
    SHIFTSWITCHBIND (PrevAllKey, doSwitch, false, ShiftTypeAll);
    SHIFTSWITCHBIND (NextGroupKey, doSwitch, true, ShiftTypeGroup);
    SHIFTSWITCHBIND (PrevGroupKey, doSwitch, false, ShiftTypeGroup);

    SHIFTTERMBIND (NextKey, terminate);
    SHIFTTERMBIND (PrevKey, terminate);
    SHIFTTERMBIND (NextAllKey, terminate);
    SHIFTTERMBIND (PrevAllKey, terminate);
    SHIFTTERMBIND (NextGroupKey, terminate);
    SHIFTTERMBIND (PrevGroupKey, terminate);

    SHIFTTERMBIND (InitiateKey, terminate);
    SHIFTTERMBIND (InitiateAllKey, terminate);

    /* Button Actions */

    SHIFTINITBIND (InitiateButton, initiate);
    SHIFTINITBIND (InitiateAllButton, initiateAll);

    SHIFTSWITCHBIND (NextButton, doSwitch, true, ShiftTypeNormal);
    SHIFTSWITCHBIND (PrevButton, doSwitch, false, ShiftTypeNormal);
    SHIFTSWITCHBIND (NextAllButton, doSwitch, true, ShiftTypeAll);
    SHIFTSWITCHBIND (PrevAllButton, doSwitch, false, ShiftTypeAll);
    SHIFTSWITCHBIND (NextGroupButton, doSwitch, true, ShiftTypeGroup);
    SHIFTSWITCHBIND (PrevGroupButton, doSwitch, false, ShiftTypeGroup);

    SHIFTTERMBIND (NextButton, terminate);
    SHIFTTERMBIND (PrevButton, terminate);
    SHIFTTERMBIND (NextAllButton, terminate);
    SHIFTTERMBIND (PrevAllButton, terminate);
    SHIFTTERMBIND (NextGroupButton, terminate);
    SHIFTTERMBIND (PrevGroupButton, terminate);

    SHIFTTERMBIND (InitiateButton, terminate);
    SHIFTTERMBIND (InitiateAllButton, terminate);

    /* Edge Actions */

    SHIFTINITBIND (InitiateEdge, initiate);
    SHIFTINITBIND (InitiateAllEdge, initiateAll);
    SHIFTTERMBIND (InitiateEdge, terminate);
    SHIFTTERMBIND (InitiateAllEdge, terminate);
}

ShiftScreen::~ShiftScreen ()
{
    freeWindowTitle ();

    XFreeCursor (screen->dpy (), mCursor);

    if (mWindows)
        free (mWindows);

    if (mDrawSlots)
        free (mDrawSlots);

}

ShiftWindow::ShiftWindow (CompWindow *window) :
    PluginClassHandler <ShiftWindow, CompWindow> (window),
    window (window),
    cWindow (CompositeWindow::get (window)),
    gWindow (GLWindow::get (window)),
    mOpacity (1.0),
    mBrightness (1.0),
    mOpacityVelocity (0.0f),
    mBrightnessVelocity (0.0f),
    mActive (false)
{
    CompositeWindowInterface::setHandler (cWindow, false);
    GLWindowInterface::setHandler (gWindow, false);

    mSlots[0].scale = 1.0;
    mSlots[1].scale = 1.0;
}

ShiftWindow::~ShiftWindow ()
{
}

bool
ShiftPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
        return false;

    if (!CompPlugin::checkPluginABI ("text", COMPIZ_TEXT_ABI))
    {
        compLogMessage ("shift", CompLogLevelWarn, "No compatible text plugin"\
                                                   " loaded");
        textAvailable = false;
    }
    else
        textAvailable = true;

    return true;
}
