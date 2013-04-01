/*
 *
 * Compiz ring switcher plugin
 *
 * ring.cpp
 *
 * Copyright : (C) 2007 by Danny Baumann
 * E-mail    : maniac@opencompositing.org
 *
 * Based on scale.c and switcher.c:
 * Copyright : (C) 2007 David Reveman
 * E-mail    : davidr@novell.com
 *
 * Ported to Compiz 0.9 by:
 * Copyright : (C) 2009 Sam Spilsbury
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
 */

#include "ring.h"

COMPIZ_PLUGIN_20090315 (ring, RingPluginVTable);

const double PI = 3.14159265359f;

const unsigned short ICON_SIZE = 64;

bool textAvailable;

static void
toggleFunctions (bool enabled)
{
    RING_SCREEN (screen);

    rs->cScreen->preparePaintSetEnabled (rs, enabled);
    rs->cScreen->donePaintSetEnabled (rs, enabled);
    rs->gScreen->glPaintOutputSetEnabled (rs, enabled);
    screen->handleEventSetEnabled (rs, enabled);

    foreach (CompWindow *w, screen->windows ())
    {
	RING_WINDOW (w);
	rw->gWindow->glPaintSetEnabled (rw, enabled);
	rw->cWindow->damageRectSetEnabled (rw, enabled);
    }
}

void
RingScreen::switchActivateEvent (bool activating)
{
    CompOption::Vector o;

    CompOption o1 ("root", CompOption::TypeInt);
    o1.value ().set ((int) screen->root ());

    o.push_back (o1);

    CompOption o2 ("active", CompOption::TypeBool);
    o2.value ().set (activating);

    o.push_back (o2);

    screen->handleCompizEvent ("ring", "activate", o);
}

bool
RingWindow::is (bool removing)
{
    RING_SCREEN (screen);

    if (!removing && window->destroyed ())
	return false;

    if (window->overrideRedirect ())
	return false;

    if (window->wmType () & (CompWindowTypeDockMask | CompWindowTypeDesktopMask))
	return false;

    if (!removing && (!window->mapNum () || !window->isViewable ()))
    {
	if (rs->optionGetMinimized ())
	{
	    if (!window->minimized () && !window->inShowDesktopMode () &&
	        !window->shaded ())
		return false;
	}
	else
    	    return false;
    }

    if (!removing && rs->mType == RingScreen::RingTypeNormal)
    {
	if (!window->mapNum () || !window->isViewable ())
	{
	    if (window->serverX () + window->width ()  <= 0    ||
		window->serverY () + window->height () <= 0    ||
		window->serverX () >= screen->width () ||
		window->serverY () >= screen->height ())
		return false;
	}
	else
	{
	    if (!window->focus ())
		return false;
	}
    }
    else if (rs->mType == RingScreen::RingTypeGroup &&
	     rs->mClientLeader != window->clientLeader () &&
	     rs->mClientLeader != window->id ())
    {
	return false;
    }

    if (window->state () & CompWindowStateSkipTaskbarMask)
	return false;

    if (!rs->mCurrentMatch.evaluate (window))
	return false;

    return true;
}

void
RingScreen::freeWindowTitle ()
{
}

void
RingScreen::renderWindowTitle ()
{
    if (!textAvailable)
	return;

    CompText::Attrib attrib;
    CompRect         oe;

    freeWindowTitle ();

    if (!mSelectedWindow)
	return;

    if (!optionGetWindowTitle ())
	return;

    oe = screen->getCurrentOutputExtents ();

    /* 75% of the output device as maximum width */
    attrib.maxWidth = oe.width () * 3 / 4;
    attrib.maxHeight = 100;

    attrib.size = optionGetTitleFontSize ();
    attrib.color[0] = optionGetTitleFontColorRed ();
    attrib.color[1] = optionGetTitleFontColorGreen ();
    attrib.color[2] = optionGetTitleFontColorBlue ();
    attrib.color[3] = optionGetTitleFontColorAlpha ();
    attrib.flags = CompText::WithBackground | CompText::Ellipsized;
    if (optionGetTitleFontBold ())
	attrib.flags |= CompText::StyleBold;
    attrib.family = "Sans";
    attrib.bgHMargin = 15;
    attrib.bgVMargin = 15;
    attrib.bgColor[0] = optionGetTitleBackColorRed ();
    attrib.bgColor[1] = optionGetTitleBackColorGreen ();
    attrib.bgColor[2] = optionGetTitleBackColorBlue ();
    attrib.bgColor[3] = optionGetTitleBackColorAlpha ();

    mText.renderWindowTitle (mSelectedWindow->id (),
                            mType == RingScreen::RingTypeAll,
                            attrib);
}

void
RingScreen::drawWindowTitle (const GLMatrix &transform)
{
    if (!textAvailable)
	return;

    float      x, y;
    CompRect   oe;

    oe = screen->getCurrentOutputExtents ();

    x = oe.centerX () - mText.getWidth () / 2;

    /* assign y (for the lower corner!) according to the setting */
    switch (optionGetTitleTextPlacement ())
    {
	case RingOptions::TitleTextPlacementCenteredOnScreen:
	    y = oe.centerY () + mText.getHeight () / 2;
	    break;
	case RingOptions::TitleTextPlacementAboveRing:
	case RingOptions::TitleTextPlacementBelowRing:
	    {
		CompRect workArea = screen->currentOutputDev ().workArea ();

	    	if (optionGetTitleTextPlacement () ==
		    RingOptions::TitleTextPlacementAboveRing)
    		    y = oe.y1 () + workArea.y () + mText.getHeight ();
		else
		    y = oe.y1 () + workArea.y2 ();
	    }
	    break;
	default:
	    return;
	    break;
    }

    mText.draw (transform, floor (x), floor (y), 1.0f);
}

bool
RingWindow::glPaint (const GLWindowPaintAttrib &attrib,
		     const GLMatrix            &transform,
		     const CompRegion	       &region,
		     unsigned int	       mask)
{
    bool       status;

    RING_SCREEN (screen);

    if (rs->mState != RingScreen::RingStateNone)
    {
	GLWindowPaintAttrib sAttrib = attrib;
	bool scaled = false;
	bool pixmap = true;

    	if (window->mapNum ())
	{
	    if (gWindow->textures ().empty ())
		gWindow->bind ();
	}

	if (mAdjust || mSlot)
	{
	    scaled = mAdjust || (mSlot);
	    mask |= PAINT_WINDOW_NO_CORE_INSTANCE_MASK;
	}
	else if (rs->mState != RingScreen::RingStateIn)
	{
	    if (rs->optionGetDarkenBack ())
	    {
		/* modify brightness of the other windows */
		sAttrib.brightness = sAttrib.brightness / 2;
	    }
	}

	status = gWindow->glPaint (sAttrib, transform, region, mask);

	pixmap = !gWindow->textures ().empty ();

	if (scaled && pixmap)
	{
	    GLWindowPaintAttrib wAttrib (gWindow->lastPaintAttrib ());
	    GLMatrix           wTransform = transform;

	    if (mask & PAINT_WINDOW_OCCLUSION_DETECTION_MASK)
		return false;

	    if (mSlot)
	    {
    		wAttrib.brightness = (float)wAttrib.brightness *
		                                         mSlot->depthBrightness;

		if (window != rs->mSelectedWindow)
		    wAttrib.opacity = (float)wAttrib.opacity *
		                          rs->optionGetInactiveOpacity () / 100;
	    }

	    if (window->alpha () || wAttrib.opacity != OPAQUE)
		mask |= PAINT_WINDOW_TRANSLUCENT_MASK;

	    wTransform.translate (window->x (), window->y (), 0.0f);
	    wTransform.scale (mScale, mScale, 1.0f);
	    wTransform.translate (mTx / mScale - window->x (),
			          mTy / mScale - window->y (),
			          0.0f);

	    gWindow->glDraw (wTransform, wAttrib, region,
			     mask | PAINT_WINDOW_TRANSFORMED_MASK);
	}

	if (scaled && (rs->mState != RingScreen::RingStateIn) &&
	    ((rs->optionGetOverlayIcon () != RingOptions::OverlayIconNone) ||
	     !pixmap))
	{
	    GLTexture *icon;

	    icon = gWindow->getIcon (256, 256);
	    if (!icon)
		icon = rs->gScreen->defaultIcon ();

	    if (icon)
	    {
		GLTexture::Matrix        matrix;
		GLTexture::MatrixList	 matricies;
		float                    scale;
		float                    x, y;
		int                      width, height;
		int                      scaledWinWidth, scaledWinHeight;

		enum RingOptions::OverlayIcon  iconOverlay;

		scaledWinWidth  = window->width () * mScale;
		scaledWinHeight = window->height () * mScale;

		if (!pixmap)
		    iconOverlay = RingOptions::OverlayIconBig;
		else
		    iconOverlay = (enum RingOptions::OverlayIcon)
						    rs->optionGetOverlayIcon ();

	    	switch (iconOverlay) {
    		case RingOptions::OverlayIconNone:
		case RingOptions::OverlayIconEmblem:
		    scale = (mSlot) ? mSlot->depthScale : 1.0f;
		    if (icon->width () > ICON_SIZE ||
			 icon->height () > ICON_SIZE)
			scale = MIN ((scale * ICON_SIZE / icon->width ()),
				     (scale * ICON_SIZE / icon->height ()));
		    break;
		case RingOptions::OverlayIconBig:
		default:
		    /* only change opacity if not painting an
		       icon for a minimized window */
		    if (pixmap)
			sAttrib.opacity /= 3;
		    scale = MIN (((float) scaledWinWidth / icon->width ()),
				 ((float) scaledWinHeight / icon->height ()));
		    break;
		}

		width  = icon->width ()  * scale;
		height = icon->height () * scale;

	    	switch (iconOverlay) {
		case RingOptions::OverlayIconNone:
		case RingOptions::OverlayIconEmblem:
		    x = window->x () + scaledWinWidth - width;
		    y = window->y () + scaledWinHeight - height;
		    break;
		case RingOptions::OverlayIconBig:
		default:
		    x = window->x () + scaledWinWidth / 2 - width / 2;
		    y = window->y () + scaledWinHeight / 2 - height / 2;
		    break;
		}

		x += mTx;
		y += mTy;

		mask |= PAINT_WINDOW_BLEND_MASK;

		/* if we paint the icon for a minimized window, we need
		   to force the usage of a good texture filter */
		if (!pixmap)
		    mask |= PAINT_WINDOW_TRANSFORMED_MASK;

		CompRegion iconReg (window->x (), window->y (),
				    icon->width (), icon->height ());

		matrix = icon->matrix ();
		matrix.x0 -= (window->x () * matrix.xx);
		matrix.y0 -= (window->y () * matrix.yy);

		matricies.push_back (matrix);

		gWindow->vertexBuffer ()->begin ();
		gWindow->glAddGeometry (matricies, iconReg, iconReg);
		if (gWindow->vertexBuffer ()->end ())
		{
		    GLWindowPaintAttrib wAttrib (sAttrib);
		    GLMatrix	       wTransform = transform;

		    if (!pixmap)
			sAttrib.opacity = gWindow->paintAttrib ().opacity;

		    if (mSlot)
		        wAttrib.brightness = (float)wAttrib.brightness *
		                             mSlot->depthBrightness;

		    wTransform.translate (window->x (), window->y (), 0.0f);
		    wTransform.scale (scale, scale, 1.0f);
		    wTransform.translate ((x - window->x ()) / scale - window->x (),
		                          (y - window->y ()) / scale - window->y (),
				          0.0f);

		    gWindow->glDrawTexture (icon, wTransform, wAttrib, mask);
		}
	    }
	}
    }
    else
    {
	status = gWindow->glPaint (attrib, transform, region, mask);
    }

    return status;
}

static inline float
ringLinearInterpolation (float valX,
			 float minX, float maxX,
			 float minY, float maxY)
{
    double factor = (maxY - minY) / (maxX - minX);
    return (minY + (factor * (valX - minX)));
}

bool
RingWindow::compareWindows (CompWindow *w1,
			    CompWindow *w2)
{
    if (w1->mapNum () && !w2->mapNum ())
	return true;

    if (w2->mapNum () && !w1->mapNum ())
	return false;

    return (w2->activeNum () < w1->activeNum ());
}

bool
RingWindow::compareRingWindowDepth (RingScreen::RingDrawSlot e1,
				    RingScreen::RingDrawSlot e2)
{
    RingScreen::RingSlot *a1   = (*(e1.slot));
    RingScreen::RingSlot *a2   = (*(e2.slot));

    if (a1->y < a2->y)
	return true;
    else if (a1->y > a2->y)
	return false;
    else
	return false;
}

bool
RingScreen::layoutThumbs ()
{
    float      baseAngle, angle;
    int        index = 0;
    int        ww, wh;
    float      xScale, yScale;
    int        centerX, centerY;
    int        ellipseA, ellipseB;
    CompRect   oe;

    if ((mState == RingStateNone) || (mState == RingStateIn))
	return false;

    baseAngle = (2 * PI * mRotTarget) / 3600;

    oe = screen->getCurrentOutputExtents ();

    /* the center of the ellipse is in the middle
       of the used output device */
    centerX  = oe.centerX ();
    centerY  = oe.centerY ();
    ellipseA = oe.width () * optionGetRingWidth () / 200;
    ellipseB = oe.height () * optionGetRingHeight () / 200;

    mDrawSlots.resize (mWindows.size ());

    foreach (CompWindow *w, mWindows)
    {
	RING_WINDOW (w);

	if (!rw->mSlot)
	    rw->mSlot = new RingSlot ();

	if (!rw->mSlot)
	    return false;

	/* we subtract the angle from the base angle
	   to order the windows clockwise */
	angle = baseAngle - (index * (2 * PI / mWindows.size ()));

	rw->mSlot->x = centerX + (optionGetRingClockwise () ? -1 : 1) *
	                        ((float) ellipseA * sin (angle));
	rw->mSlot->y = centerY + ((float) ellipseB * cos (angle));

	ww = w->width ()  + w->input ().left + w->input ().right;
	wh = w->height () + w->input ().top  + w->input ().bottom;

	if (ww > optionGetThumbWidth ())
	    xScale = (float)(optionGetThumbWidth ()) / (float) ww;
	else
	    xScale = 1.0f;

	if (wh > optionGetThumbHeight ())
	    yScale = (float)(optionGetThumbHeight ()) / (float) wh;
	else
	    yScale = 1.0f;

	rw->mSlot->scale = MIN (xScale, yScale);

	/* scale and brightness are obtained by doing a linear inter-
	   polation - the y positions are the x values for the interpolation
	   (the larger Y is, the nearer is the window), and scale/brightness
	   are the y values for the interpolation */
	rw->mSlot->depthScale =
	    ringLinearInterpolation (rw->mSlot->y,
				     centerY - ellipseB, centerY + ellipseB,
				     optionGetMinScale (), 1.0f);

	rw->mSlot->depthBrightness =
	    ringLinearInterpolation (rw->mSlot->y,
				     centerY - ellipseB, centerY + ellipseB,
				     optionGetMinBrightness (), 1.0f);

	mDrawSlots.at (index).w    = w;
	mDrawSlots.at (index).slot = &rw->mSlot;

	index++;
    }

    /* sort the draw list so that the windows with the
       lowest Y value (the windows being farest away)
       are drawn first */

    sort (mDrawSlots.begin (), mDrawSlots.end (),
	  RingWindow::compareRingWindowDepth); // TODO

    return true;
}

void
RingScreen::addWindowToList (CompWindow *w)
{
    mWindows.push_back (w);
}

bool
RingScreen::updateWindowList ()
{
    sort (mWindows.begin (), mWindows.end (), RingWindow::compareWindows);

    mRotTarget = 0;
    foreach (CompWindow *w, mWindows)
    {
	if (w == mSelectedWindow)
	    break;

	mRotTarget += DIST_ROT;
    }

    return layoutThumbs ();
}

bool
RingScreen::createWindowList ()
{
    mWindows.clear ();

    foreach (CompWindow *w, screen->windows ())
    {
	RING_WINDOW (w);
	if (rw->is ())
	{
	    addWindowToList (w);
	    rw->mAdjust = true;
	}
    }

    return updateWindowList ();
}

void
RingScreen::switchToWindow (bool	   toNext)
{
    CompWindow   *w; // We need w to be in this scope
    unsigned int cur = 0;

    if (!mGrabIndex)
	return;

    foreach (w, mWindows)
    {
	if (w == mSelectedWindow)
	    break;
	cur++;
    }

    if (cur == mWindows.size ())
	return;

    if (toNext)
	w = mWindows.at ((cur + 1) % mWindows.size ());
    else
	w = mWindows.at ((cur + mWindows.size () - 1) % mWindows.size ());

    if (w)
    {
	CompWindow *old = mSelectedWindow;

	mSelectedWindow = w;
	if (old != w)
	{
	    if (toNext)
		mRotAdjust += DIST_ROT;
	    else
		mRotAdjust -= DIST_ROT;

	    mRotateAdjust = true;

	    cScreen->damageScreen ();
	    renderWindowTitle ();
	}
    }
}

int
RingScreen::countWindows ()
{
    int	       count = 0;

    foreach (CompWindow *w, screen->windows ())
    {
	RING_WINDOW (w);

	if (rw->is ())
	    count++;
    }

    return count;
}

int
RingScreen::adjustRingRotation (float      chunk)
{
    float dx, adjust, amount;
    int   change;

    dx = mRotAdjust;

    adjust = dx * 0.15f;
    amount = fabs (dx) * 1.5f;
    if (amount < 0.2f)
	amount = 0.2f;
    else if (amount > 2.0f)
	amount = 2.0f;

    mRVelocity = (amount * mRVelocity + adjust) / (amount + 1.0f);

    if (fabs (dx) < 0.1f && fabs (mRVelocity) < 0.2f)
    {
	mRVelocity = 0.0f;
	mRotTarget += mRotAdjust;
	mRotAdjust = 0;
	return 0;
    }

    change = mRVelocity * chunk;
    if (!change)
    {
	if (mRVelocity)
	    change = (mRotAdjust > 0) ? 1 : -1;
    }

    mRotAdjust -= change;
    mRotTarget += change;

    if (!layoutThumbs ())
	return false;

    return true;
}

int
RingWindow::adjustVelocity ()
{
    float dx, dy, ds, adjust, amount;
    float x1, y1, scale;

    if (mSlot)
    {
	scale = mSlot->scale * mSlot->depthScale;
	x1 = mSlot->x - (window->width () * scale) / 2;
	y1 = mSlot->y - (window->height () * scale) / 2;
    }
    else
    {
	scale = 1.0f;
	x1 = window->x ();
	y1 = window->y ();
    }

    dx = x1 - (window->x () + mTx);

    adjust = dx * 0.15f;
    amount = fabs (dx) * 1.5f;
    if (amount < 0.5f)
	amount = 0.5f;
    else if (amount > 5.0f)
	amount = 5.0f;

    mXVelocity = (amount * mXVelocity + adjust) / (amount + 1.0f);

    dy = y1 - (window->y () + mTy);

    adjust = dy * 0.15f;
    amount = fabs (dy) * 1.5f;
    if (amount < 0.5f)
	amount = 0.5f;
    else if (amount > 5.0f)
	amount = 5.0f;

    mYVelocity = (amount * mYVelocity + adjust) / (amount + 1.0f);

    ds = scale - mScale;
    adjust = ds * 0.1f;
    amount = fabs (ds) * 7.0f;
    if (amount < 0.01f)
	amount = 0.01f;
    else if (amount > 0.15f)
	amount = 0.15f;

    mScaleVelocity = (amount * mScaleVelocity + adjust) /
	(amount + 1.0f);

    if (fabs (dx) < 0.1f && fabs (mXVelocity) < 0.2f &&
	fabs (dy) < 0.1f && fabs (mYVelocity) < 0.2f &&
	fabs (ds) < 0.001f && fabs (mScaleVelocity) < 0.002f)
    {
	mXVelocity = mYVelocity = mScaleVelocity = 0.0f;
	mTx = x1 - window->x ();
	mTy = y1 - window->y ();
	mScale = scale;

	return 0;
    }

    return 1;
}

bool
RingScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
			   const GLMatrix	     &transform,
			   const CompRegion	     &region,
			   CompOutput		     *output,
			   unsigned int		     mask)
{
    bool status;

    if (mState != RingStateNone)
	mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK;

    //mask |= PAINT_SCREEN_NO_OCCLUSION_DETECTION_MASK;

    status = gScreen->glPaintOutput (attrib, transform, region, output, mask);

    if (mState != RingStateNone)
    {
	GLMatrix      sTransform = transform;

	sTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);

	/* TODO: This code here should be reworked */

	if (mState == RingScreen::RingStateSwitching ||
	    mState == RingScreen::RingStateOut)
	{
	    for (std::vector <RingDrawSlot>::iterator it = mDrawSlots.begin ();
		 it != mDrawSlots.end (); ++it)
	    {
		CompWindow *w = (*it).w;

		RING_WINDOW (w);

		status |= rw->gWindow->glPaint (rw->gWindow->paintAttrib (),
					   sTransform, infiniteRegion, 0);
	    }
	}

	if (mState != RingStateIn)
	    drawWindowTitle (sTransform);
    }

    return status;
}

void
RingScreen::preparePaint (int msSinceLastPaint)
{
    if (mState != RingStateNone && (mMoreAdjust || mRotateAdjust))
    {
	int        steps;
	float      amount, chunk;

	amount = msSinceLastPaint * 0.05f * optionGetSpeed ();
	steps  = amount / (0.5f * optionGetTimestep ());

	if (!steps)
	    steps = 1;
	chunk  = amount / (float) steps;

	while (steps--)
	{
	    mRotateAdjust = adjustRingRotation (chunk);
	    mMoreAdjust = false;

	    foreach (CompWindow *w, screen->windows ())
	    {
		RING_WINDOW (w);

		if (rw->mAdjust)
		{
		    rw->mAdjust = rw->adjustVelocity ();

		    mMoreAdjust |= rw->mAdjust;

		    rw->mTx += rw->mXVelocity * chunk;
		    rw->mTy += rw->mYVelocity * chunk;
		    rw->mScale += rw->mScaleVelocity * chunk;
		}
		else if (rw->mSlot)
		{
		    rw->mScale = rw->mSlot->scale * rw->mSlot->depthScale;
	    	    rw->mTx = rw->mSlot->x - w->x () -
			     (w->width () * rw->mScale) / 2;
	    	    rw->mTy = rw->mSlot->y - w->y () -
			     (w->height () * rw->mScale) / 2;
		}
	    }

	    if (!mMoreAdjust && !mRotateAdjust)
	    {
		switchActivateEvent (false);
		break;
	    }
	}
    }

    cScreen->preparePaint (msSinceLastPaint);
}

void
RingScreen::donePaint ()
{
    if (mState != RingStateNone)
    {
	if (mMoreAdjust)
	{
	    cScreen->damageScreen ();
	}
	else
	{
	    if (mRotateAdjust)
		cScreen->damageScreen ();

	    if (mState == RingStateIn)
	    {
		toggleFunctions (false);
		mState = RingStateNone;
	    }
	    else if (mState == RingStateOut)
		mState = RingStateSwitching;
	}
    }

    cScreen->donePaint ();
}

bool
RingScreen::terminate (CompAction         *action,
		       CompAction::State  state,
		       CompOption::Vector options)
{
    if (mGrabIndex)
    {
        screen->removeGrab (mGrabIndex, 0);
        mGrabIndex = 0;
    }

    if (mState != RingStateNone)
    {
        foreach (CompWindow *w, screen->windows ())
        {
	    RING_WINDOW (w);

	    if (rw->mSlot)
	    {
	        delete rw->mSlot;
	        rw->mSlot = NULL;

	        rw->mAdjust = true;
	    }
        }
        mMoreAdjust = true;
        mState = RingStateIn;
        cScreen->damageScreen ();

        if (!(state & CompAction::StateCancel) &&
            mSelectedWindow && !mSelectedWindow->destroyed ())
        {
            screen->sendWindowActivationRequest (mSelectedWindow->id ());
        }
    }

    if (action)
	action->setState ( ~(CompAction::StateTermKey |
			     CompAction::StateTermButton |
			     CompAction::StateTermEdge));

    return false;
}

bool
RingScreen::initiate (CompAction         *action,
		      CompAction::State  state,
		      CompOption::Vector options)
{
    int       count;

    if (screen->otherGrabExist ("ring", NULL))
	return false;

    mCurrentMatch = optionGetWindowMatch ();

    mMatch = CompOption::getMatchOptionNamed (options, "match", CompMatch ());
    if (!mMatch.isEmpty ())
    {
	mCurrentMatch = mMatch;
    }

    count = countWindows ();

    if (count < 1)
    {
	return false;
    }

    if (!mGrabIndex)
    {
	if (optionGetSelectWithMouse ())
	    mGrabIndex = screen->pushGrab (screen->normalCursor (), "ring");
	else
	    mGrabIndex = screen->pushGrab (screen->invisibleCursor (), "ring");
    }

    if (mGrabIndex)
    {
	mState = RingScreen::RingStateOut;

	if (!createWindowList ())
	    return false;

	mSelectedWindow = mWindows.front ();
	renderWindowTitle ();
	mRotTarget = 0;

    	mMoreAdjust = true;
	toggleFunctions (true);
	cScreen->damageScreen ();

	switchActivateEvent (true);
    }

    return true;
}

bool
RingScreen::doSwitch (CompAction         *action,
		      CompAction::State  state,
		      CompOption::Vector options,
		      bool		 nextWindow,
		      RingType		 type)
{
    bool       ret = true;

    if ((mState == RingStateNone) || (mState == RingStateIn))
    {
        if (type == RingTypeGroup)
        {
	    CompWindow *w;
	    w = screen->findWindow (CompOption::getIntOptionNamed (options,
							           "window",
							           0));
	    if (w)
	    {
	        mType = RingTypeGroup;
	        mClientLeader =
		    (w->clientLeader ()) ? w->clientLeader () : w->id ();
	        ret = initiate (action, state, options);
	    }
        }
        else
        {
	    mType = type;
	    ret = initiate (action, mState, options);
        }

        if (state & CompAction::StateInitKey)
	    action->setState (action->state () | CompAction::StateTermKey);

        if (state & CompAction::StateInitEdge)
	    action->setState (action->state () | CompAction::StateTermEdge);
        else if (mState & CompAction::StateInitButton)
	    action->setState (action->state () |
			      CompAction::StateTermButton);
    }

    if (ret)
        switchToWindow (nextWindow);


    return ret;
}

void
RingScreen::windowSelectAt (int  x,
			    int  y,
			    bool shouldTerminate)
{
    CompWindow *selected = NULL;

    if (!optionGetSelectWithMouse ())
	return;

    /* first find the top-most window the mouse
       pointer is over */
    foreach (CompWindow *w, mWindows)
    {
	RING_WINDOW (w);
    	if (rw->mSlot)
	{
    	    if ((x >= (rw->mTx + w->x ())) &&
		(x <= (rw->mTx + w->x () + (w->width () * rw->mScale))) &&
		(y >= (rw->mTy + w->y ())) &&
		(y <= (rw->mTy + w->y () + (w->height () * rw->mScale))))
	    {
		/* we have found one, select it */
		selected = w;
		break;
	    }
	}
    }

    if (selected && shouldTerminate)
    {
	CompOption o ("root", CompOption::TypeInt);
	CompOption::Vector opts;

	o.value ().set ((int) screen->root ());

	opts.push_back (o);

	mSelectedWindow = selected;

	terminate (NULL, 0, opts);
    }
    else if (!shouldTerminate && (selected != mSelectedWindow ))
    {
	if (!selected)
	{
	    freeWindowTitle ();
	}
	else
	{
	    mSelectedWindow = selected;
	    renderWindowTitle ();
	}
	cScreen->damageScreen ();
    }
}

void
RingScreen::windowRemove (CompWindow *w)
{
    if (w)
    {
	bool   inList = false;
	CompWindow *selected;
	CompWindowVector::iterator it = mWindows.begin ();

	RING_WINDOW (w);

	if (mState == RingStateNone)
	    return;

	if (!rw->is (true))
    	    return;

	selected = mSelectedWindow;

	while (it != mWindows.end ())
	{
	    if (*it == w)
	    {
		inList = true;

		if (w == selected)
		{
		    ++it;
		    if (it != mWindows.end ())
			selected = *it;
    		    else
			selected = mWindows.front ();
		    --it;

		    mSelectedWindow = selected;
		    renderWindowTitle ();
		}

		mWindows.erase (it);
		break;
	    }
	    ++it;
	}

	if (!inList)
	    return;

	/* Terminate if the window closed was the last window in the list */

	if (mWindows.empty ())
	{
	    CompOption o ("root", CompOption::TypeInt);
	    CompOption::Vector opts;

	    o.value ().set ((int) screen->root ());

	    opts.push_back (o);

	    terminate (NULL, 0, opts);
	    return;
	}

	// Let the window list be updated to avoid crash
	// when a window is closed while ending (RingStateIn).
	if (!mGrabIndex && mState != RingStateIn)
	    return;

	if (updateWindowList ())
	{
	    mMoreAdjust = true;
	    mState = RingStateOut;
	    cScreen->damageScreen ();
	}
    }
}

void
RingScreen::handleEvent (XEvent *event)
{
    CompWindow *w = NULL;

    switch (event->type) {
    case DestroyNotify:
	/* We need to get the CompWindow * for event->xdestroywindow.window
	   here because in the ::screen->handleEvent call below, that
	   CompWindow's id will become 1, so findWindow won't be
	   able to find the CompWindow after that. */
	   w = ::screen->findWindow (event->xdestroywindow.window);
	break;
    default:
	break;
    }

    screen->handleEvent (event);

    switch (event->type) {
    case PropertyNotify:
	if (event->xproperty.atom == XA_WM_NAME)
	{
	    w = screen->findWindow (event->xproperty.window);
	    if (w)
	    {
		if (mGrabIndex && (w == mSelectedWindow))
    		{
    		    renderWindowTitle ();
    		    cScreen->damageScreen ();
		}
	    }
	}
	break;
    case ButtonPress:
	if (event->xbutton.button == Button1)
	{
	    if (mGrabIndex)
	        windowSelectAt (event->xbutton.x_root,
				event->xbutton.y_root,
				true);
	}
	break;
    case MotionNotify:
        if (mGrabIndex)
	    windowSelectAt (event->xmotion.x_root,
			    event->xmotion.y_root,
			    false);
    case UnmapNotify:
	w = ::screen->findWindow (event->xunmap.window);
	windowRemove (w);
	break;
    case DestroyNotify:
	windowRemove (w);
	break;
    }
}

bool
RingWindow::damageRect (bool     initial,
			const CompRect &rect)
{
    bool       status = false;

    RING_SCREEN (screen);

    if (initial)
    {
	if (rs->mGrabIndex && is ())
	{
	    rs->addWindowToList (window);
	    if (rs->updateWindowList ())
	    {
		mAdjust = true;
		rs->mMoreAdjust = true;
		rs->mState = RingScreen::RingStateOut;
		rs->cScreen->damageScreen ();
	    }
	}
    }
    else if (rs->mState == RingScreen::RingStateSwitching)
    {

	if (mSlot)
	{
	    cWindow->damageTransformedRect (mScale, mScale,
					    mTx, mTy,
					    rect);
	    status = true;
	}

    }

    status |= cWindow->damageRect (initial, rect);

    return status;
}

RingScreen::RingScreen (CompScreen *screen) :
    PluginClassHandler <RingScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    mGrabIndex (0),
    mState (RingScreen::RingStateNone),
    mMoreAdjust (false),
    mRotateAdjust (false),
    mRotAdjust (0),
    mRVelocity (0.0f),
    mSelectedWindow (NULL)
{

    ScreenInterface::setHandler (screen, false);
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);

#define RINGTERMBIND(opt, func)                                \
    optionSet##opt##Terminate (boost::bind (&RingScreen::func, \
					    this, _1, _2, _3));

#define RINGSWITCHBIND(opt, func, next, type)                 \
    optionSet##opt##Initiate (boost::bind (&RingScreen::func, \
					    this, _1, _2, _3, \
					    next, type));

    RINGSWITCHBIND (NextKey, doSwitch, true, RingTypeNormal);
    RINGSWITCHBIND (PrevKey, doSwitch, false, RingTypeNormal);
    RINGSWITCHBIND (NextAllKey, doSwitch, true, RingTypeAll);
    RINGSWITCHBIND (PrevAllKey, doSwitch, false, RingTypeAll);
    RINGSWITCHBIND (NextGroupKey, doSwitch, true, RingTypeGroup);
    RINGSWITCHBIND (PrevGroupKey, doSwitch, false, RingTypeGroup);

    RINGTERMBIND (NextKey, terminate);
    RINGTERMBIND (PrevKey, terminate);
    RINGTERMBIND (NextAllKey, terminate);
    RINGTERMBIND (PrevAllKey, terminate);
    RINGTERMBIND (NextGroupKey, terminate);
    RINGTERMBIND (PrevGroupKey, terminate);

    RINGSWITCHBIND (NextButton, doSwitch, true, RingTypeNormal);
    RINGSWITCHBIND (PrevButton, doSwitch, false, RingTypeNormal);
    RINGSWITCHBIND (NextAllButton, doSwitch, true, RingTypeAll);
    RINGSWITCHBIND (PrevAllButton, doSwitch, false, RingTypeAll);
    RINGSWITCHBIND (NextGroupButton, doSwitch, true, RingTypeGroup);
    RINGSWITCHBIND (PrevGroupButton, doSwitch, false, RingTypeGroup);

    RINGTERMBIND (NextButton, terminate);
    RINGTERMBIND (PrevButton, terminate);
    RINGTERMBIND (NextAllButton, terminate);
    RINGTERMBIND (PrevAllButton, terminate);
    RINGTERMBIND (NextGroupButton, terminate);
    RINGTERMBIND (PrevGroupButton, terminate);
}


RingScreen::~RingScreen ()
{
    mWindows.clear ();
    mDrawSlots.clear ();
}

RingWindow::RingWindow (CompWindow *window) :
    PluginClassHandler <RingWindow, CompWindow> (window),
    window (window),
    cWindow (CompositeWindow::get (window)),
    gWindow (GLWindow::get (window)),
    mSlot (NULL),
    mXVelocity (0.0f),
    mYVelocity (0.0f),
    mScaleVelocity (0.0f),
    mTx (0.0f),
    mTy (0.0f),
    mScale (1.0f),
    mAdjust (false)
{
    CompositeWindowInterface::setHandler (cWindow, false);
    GLWindowInterface::setHandler (gWindow, false);
}

RingWindow::~RingWindow ()
{
    if (mSlot)
	delete mSlot;
}

bool
RingPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
    	return false;

    if (!CompPlugin::checkPluginABI ("text", COMPIZ_TEXT_ABI))
    {
	compLogMessage ("ring", CompLogLevelWarn, "No compatible text plugin"\
						  " loaded");
	textAvailable = false;
    }
    else
	textAvailable = true;

    return true;
}
