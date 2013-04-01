/*
 *
 * Compiz stackswitch switcher plugin
 *
 * stackswitch.cpp
 *
 * Copyright : (C) 2007 by Danny Baumann
 * E-mail    : maniac@opencompositing.org
 *
 * Based on scale.c and switcher.c:
 * Copyright : (C) 2007 David Reveman
 * E-mail    : davidr@novell.com
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

#include "stackswitch.h"

COMPIZ_PLUGIN_20090315 (stackswitch, StackswitchPluginVTable)

bool textAvailable;

bool
StackswitchWindow::isStackswitchable ()
{
    STACKSWITCH_SCREEN (screen);

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

    if (ss->mType == StackswitchTypeNormal)
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
    else if (ss->mType == StackswitchTypeGroup &&
	     ss->mClientLeader != window->clientLeader () &&
	     ss->mClientLeader != window->id ())
    {
	return false;
    }

    if (window->state () & CompWindowStateSkipTaskbarMask)
	return false;

    if (!ss->mCurrentMatch.evaluate (window))
	return false;

    return true;
}

void
StackswitchScreen::renderWindowTitle ()
{
    CompText::Attrib tA;
    bool           showViewport;

    if (!textAvailable)
	return;

    if (!optionGetWindowTitle ())
	return;

    CompRect oe = screen->getCurrentOutputExtents ();

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

    showViewport = (mType == StackswitchTypeAll);
    mText.renderWindowTitle (mSelectedWindow, showViewport, tA);
}

void
StackswitchScreen::drawWindowTitle (GLMatrix &transform,
				    CompWindow *w)
{
    GLboolean     wasBlend;
    GLint         oldBlendSrc, oldBlendDst;
    GLMatrix      wTransform (transform), mvp;
    float         x, y, tx, width, height;
    GLTexture::Matrix    m;
    GLTexture     *icon;

    STACKSWITCH_WINDOW (w);

    CompRect oe = screen->getCurrentOutputExtents ();

    width = mText.getWidth ();
    height = mText.getHeight ();

    x = oe.centerX ();
    tx = x - width / 2;

    switch (optionGetTitleTextPlacement ())
    {
	case StackswitchOptions::TitleTextPlacementOnThumbnail:
	    {
	    GLVector v (w->x () + (w->width () / 2.0),
	    		w->y () + (w->width () / 2.0),
			0.0f,
			1.0f);
	    GLMatrix pm (gScreen->projectionMatrix ());

	    wTransform.scale (1.0, 1.0, 1.0 / screen->height ());
	    wTransform.translate (sw->mTx, sw->mTy, 0.0f);
	    wTransform.rotate (-mRotation, 1.0, 0.0, 0.0);
	    wTransform.scale (sw->mScale, sw->mScale, 1.0);
	    wTransform.translate (+w->border ().left, 0.0 - (w->height () +
							    w->border ().bottom),
							   0.0f);
	    wTransform.translate (-w->x (), -w->y (), 0.0f);

	    mvp = pm * wTransform;
	    v = mvp * v;
	    v.homogenize ();

	    x = (v[GLVector::x] + 1.0) * oe.width () * 0.5;
	    y = (v[GLVector::y] - 1.0) * oe.height () * -0.5;

	    x += oe.x1 ();
	    y += oe.y1 ();

	    tx = MAX (oe.x1 (), x - (width / 2.0));
	    if (tx + width > oe.x2 ())
		tx = oe.x2 () - width;
            }
	break;
	case StackswitchOptions::TitleTextPlacementCenteredOnScreen:
	    y = oe.centerY () + height / 2;
	    break;
	case StackswitchOptions::TitleTextPlacementAbove:
	case StackswitchOptions::TitleTextPlacementBelow:
	    {
		CompRect workArea = screen->currentOutputDev ().workArea ();

	    	if (optionGetTitleTextPlacement () ==
		    StackswitchOptions::TitleTextPlacementAbove)
    		    y = oe.y1 () + workArea.y () + height;
		else
		    y = oe.y1 () + workArea.y2 () - 96;
	    }
	    break;
	default:
	    return;
	    break;
    }

    tx = floor (tx);
    y  = floor (y);

    glGetIntegerv (GL_BLEND_SRC, &oldBlendSrc);
    glGetIntegerv (GL_BLEND_DST, &oldBlendDst);

    wasBlend = glIsEnabled (GL_BLEND);
    if (!wasBlend)
	glEnable (GL_BLEND);

    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor4f (1.0, 1.0, 1.0, 1.0);

    glPushMatrix ();
    glLoadMatrixf (wTransform.getMatrix ());

    icon = sw->gWindow->getIcon (96, 96);
    if (!icon)
	icon = gScreen->defaultIcon ();

    if (icon && (icon->name ()))
    {
	int                 off;
	float ix = floor (x - (icon->width () / 2.0));

	icon->enable (GLTexture::Good);

	m = icon->matrix ();

	glColor4f (0.0, 0.0, 0.0, 0.1);

	for (off = 0; off < 6; off++)
	{
	    glBegin (GL_QUADS);

	    glTexCoord2f (COMP_TEX_COORD_X (m, 0), COMP_TEX_COORD_Y (m ,0));
	    glVertex2f (ix - off, y - off);
	    glTexCoord2f (COMP_TEX_COORD_X (m, 0), COMP_TEX_COORD_Y (m, icon->height ()));
	    glVertex2f (ix - off, y + icon->height () + off);
	    glTexCoord2f (COMP_TEX_COORD_X (m, icon->width ()), COMP_TEX_COORD_Y (m, icon->height ()));
	    glVertex2f (ix + icon->width () + off, y + icon->height () + off);
	    glTexCoord2f (COMP_TEX_COORD_X (m, icon->width ()), COMP_TEX_COORD_Y (m, 0));
	    glVertex2f (ix + icon->width () + off, y - off);

	    glEnd ();
	}

	glColor4f (1.0, 1.0, 1.0, 1.0);

	glBegin (GL_QUADS);

	glTexCoord2f (COMP_TEX_COORD_X (m, 0), COMP_TEX_COORD_Y (m ,0));
	glVertex2f (ix, y);
	glTexCoord2f (COMP_TEX_COORD_X (m, 0), COMP_TEX_COORD_Y (m, icon->height ()));
	glVertex2f (ix, y + icon->height ());
	glTexCoord2f (COMP_TEX_COORD_X (m, icon->width ()), COMP_TEX_COORD_Y (m, icon->height ()));
	glVertex2f (ix + icon->width (), y + icon->height ());
	glTexCoord2f (COMP_TEX_COORD_X (m, icon->width ()), COMP_TEX_COORD_Y (m, 0));
	glVertex2f (ix + icon->width (), y);

	glEnd ();

	icon->disable ();
    }

    mText.draw (x - mText.getWidth () / 2, y, 1.0);

    glPopMatrix ();

    glColor4usv (defaultColor);

    if (!wasBlend)
	glDisable (GL_BLEND);
    glBlendFunc (oldBlendSrc, oldBlendDst);
}

bool
StackswitchWindow::glPaint (const GLWindowPaintAttrib &attrib,
			    const GLMatrix	      &transform,
			    const CompRegion	      &region,
			    unsigned int	      mask)
{
    bool       status;

    STACKSWITCH_SCREEN (screen);

    if (ss->mState != StackswitchStateNone)
    {
	GLWindowPaintAttrib sAttrib (attrib);
	bool		  scaled = false;
	float             rotation;

    	if (window->mapNum ())
	{
	    if (!gWindow->textures ().size ())
		gWindow->bind ();
	}

	if (mAdjust || mSlot)
	{
	    scaled = (mAdjust && ss->mState != StackswitchStateSwitching) ||
		     (mSlot && ss->mPaintingSwitcher);
	    mask |= PAINT_WINDOW_NO_CORE_INSTANCE_MASK;
	}
	else if (ss->mState != StackswitchStateIn)
	{
	    if (ss->optionGetDarkenBack ())
	    {
		/* modify brightness of the other windows */
		sAttrib.brightness = sAttrib.brightness / 2;
	    }
	}

	status = gWindow->glPaint (attrib, transform, region, mask);

	if (ss->optionGetInactiveRotate ())
	    rotation = MIN (mRotation, ss->mRotation);
	else
	    rotation = ss->mRotation;

	if (scaled && gWindow->textures ().size ())
	{
	    GLFragment::Attrib fragment (gWindow->lastPaintAttrib ());
	    GLMatrix           wTransform (transform);

	    if (mask & PAINT_WINDOW_OCCLUSION_DETECTION_MASK)
		return false;

	    if (mSlot)
	    {
		if (window->id () != ss->mSelectedWindow)
		    fragment.setOpacity ((float)fragment.getOpacity () *
			               ss->optionGetInactiveOpacity () / 100);
	    }

	    if (window->alpha () || fragment.getOpacity () != OPAQUE)
		mask |= PAINT_WINDOW_TRANSLUCENT_MASK;

	    wTransform.scale (1.0, 1.0, 1.0 / screen->height ());
	    wTransform.translate (mTx, mTy, 0.0f);

	    wTransform.rotate (-rotation, 1.0, 0.0, 0.0);
	    wTransform.scale (mScale, mScale, 1.0);

	    wTransform.translate (+window->border ().left,
				  0.0 -(window->height () +
				  window->border ().bottom), 0.0f);
	    wTransform.translate (-window->x (), -window->y (), 0.0f);
	    glPushMatrix ();
	    glLoadMatrixf (wTransform.getMatrix ());

	    gWindow->glDraw (wTransform, fragment, region,
			      mask | PAINT_WINDOW_TRANSFORMED_MASK);

	    glPopMatrix ();
	}

	if (scaled && !gWindow->textures ().size ())
	{
	    GLTexture *icon = gWindow->getIcon (96, 96);
	    if (!icon)
		icon = ss->gScreen->defaultIcon ();

	    if (icon && (icon->name ()))
	    {
		CompRegion          iconReg (0, 0,
					     icon->width (),
					     icon->height ());
		GLTexture::Matrix   matrix;
		float               scale;

		scale = MIN (window->width () / icon->width (),
			     window->height () / icon->height ());
		scale *= mScale;

		mask |= PAINT_WINDOW_BLEND_MASK;

		/* if we paint the icon for a minimized window, we need
		   to force the usage of a good texture filter */
		if (!gWindow->textures ().size ())
		    mask |= PAINT_WINDOW_TRANSFORMED_MASK;

		matrix = icon->matrix ();

		GLTexture::MatrixList matl;
		matl.push_back (matrix);

		gWindow->geometry ().reset ();
		gWindow->glAddGeometry (matl, iconReg, infiniteRegion);

		if (gWindow->geometry ().vCount)
		{
		    GLFragment::Attrib fragment (attrib);
		    GLMatrix           wTransform (transform);

		    if (!gWindow->textures ().size ())
			fragment.setOpacity (gWindow->paintAttrib ().opacity);

		    wTransform.scale (1.0, 1.0, 1.0 / screen->height ());
		    wTransform.translate (mTx, mTy, 0.0f);

		    wTransform.rotate (-rotation, 1.0, 0.0, 0.0);
		    wTransform.scale (scale, scale, 1.0);

		    wTransform.translate (0.0, -icon->height (), 0.0f);

		    glPushMatrix ();
		    glLoadMatrixf (wTransform.getMatrix ());

		    gWindow->glDrawTexture (icon, fragment, mask);

		    glPopMatrix ();
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

int
compareWindows (const void *elem1,
		const void *elem2)
{
    CompWindow *w1 = *((CompWindow **) elem1);
    CompWindow *w2 = *((CompWindow **) elem2);

    if (w1->mapNum () && !w2->mapNum ())
	return -1;

    if (w2->mapNum () && !w1->mapNum ())
	return 1;

    return w2->activeNum () - w1->activeNum ();
}

int
compareStackswitchWindowDepth (const void *elem1,
			       const void *elem2)
{
    StackswitchSlot *a1   = *(((StackswitchDrawSlot *) elem1)->slot);
    StackswitchSlot *a2   = *(((StackswitchDrawSlot *) elem2)->slot);

    if (a1->y < a2->y)
	return -1;
    else if (a1->y > a2->y)
	return 1;
    else
    {
	CompWindow *a1   = (((StackswitchDrawSlot *) elem1)->w);
        CompWindow *a2   = (((StackswitchDrawSlot *) elem2)->w);
	STACKSWITCH_SCREEN (screen);
	if (a1->id () == ss->mSelectedWindow)
	    return 1;
	else if (a2->id () == ss->mSelectedWindow)
	    return -1;
	else
	    return 0;
    }
}

bool
StackswitchScreen::layoutThumbs ()
{
    CompWindow *w;
    int        index;
    int        ww, wh;
    float      xScale, yScale;
    float      swi = 0.0, rh;
    int        cols, rows, col = 0, row = 0, r, c;
    int        cindex, ci, gap, hasActive = 0;
    bool       exit;

    if ((mState == StackswitchStateNone) || (mState == StackswitchStateIn))
	return false;

    CompRect oe = screen->getCurrentOutputExtents ();

    for (index = 0; index < mNWindows; index++)
    {
	w = mWindows[index];

	ww = w->width ()  + w->border ().left + w->border ().right;
	wh = w->height () + w->border ().top  + w->border ().bottom;

	swi += ((float) ww / (float) wh) * (oe.width () / (float) oe.height ());
    }

    cols = ceil (sqrtf (swi));

    swi = 0.0;
    for (index = 0; index < mNWindows; index++)
    {
	w = mWindows[index];

	ww = w->width ()  + w->border ().left + w->border ().right;
	wh = w->height () + w->border ().top  + w->border ().bottom;

	swi += (float)ww / (float)wh;

	if (swi > cols)
	{
	    row++;
	    swi = (float)ww / (float)wh;
	    col = 0;
	}

	col++;
    }
    rows = row + 1;

    oe.setHeight (oe.width () / cols);
    rh = ((float) oe.height () * 0.8) / rows;

    for (index = 0; index < mNWindows; index++)
    {
	w = mWindows[index];

	STACKSWITCH_WINDOW (w);

	if (!sw->mSlot)
	    sw->mSlot = (StackswitchSlot *) malloc (sizeof (StackswitchSlot));

	if (!sw->mSlot)
	    return false;

	mDrawSlots[index].w    = w;
	mDrawSlots[index].slot = &sw->mSlot;
    }

    index = 0;

    for (r = 0; r < rows && index < mNWindows; r++)
    {
	c = 0;
	swi = 0.0;
	cindex = index;
	exit = false;
	while (index < mNWindows && !exit)
	{
	    w = mWindows[index];

	    STACKSWITCH_WINDOW (w);
	    sw->mSlot->x = oe.x1 () + swi;
	    sw->mSlot->y = oe.y2 () - (rh * r) - (oe.height () * 0.1);

	    ww = w->width ()  + w->border ().left + w->border ().right;
	    wh = w->height () + w->border ().top  + w->border ().bottom;

	    if (ww > oe.width ())
		xScale = oe.width () / (float) ww;
	    else
		xScale = 1.0f;

	    if (wh > oe.height ())
		yScale = oe.height () / (float) wh;
	    else
		yScale = 1.0f;

	    sw->mSlot->scale = MIN (xScale, yScale);

	    if (swi + (ww * sw->mSlot->scale) > oe.width () && cindex != index)
	    {
		exit = true;
		continue;
	    }

	    if (w->id () == mSelectedWindow)
		hasActive = 1;
	    swi += ww * sw->mSlot->scale;

	    c++;
	    index++;
	}

	gap = oe.width () - swi;
	gap /= c + 1;

	index = cindex;
	ci = 1;
	while (ci <= c)
	{
	    w = mWindows[index];

	    STACKSWITCH_WINDOW (w);
	    sw->mSlot->x += ci * gap;

	    if (hasActive == 0)
		sw->mSlot->y += sqrt(2 * oe.height () * oe.height ()) - rh;

	    ci++;
	    index++;
	}

	if (hasActive == 1)
	    hasActive++;
    }

    /* sort the draw list so that the windows with the
       lowest Y value (the windows being farest away)
       are drawn first */
    qsort (mDrawSlots, mNWindows, sizeof (StackswitchDrawSlot),
	   compareStackswitchWindowDepth);

    return true;
}

void
StackswitchScreen::addWindowToList (CompWindow *w)
{
    if (mWindowsSize <= mNWindows)
    {
	mWindows = (CompWindow **) realloc (mWindows,
			       		   sizeof (CompWindow *) * (mNWindows + 32));
	if (!mWindows)
	    return;

	mDrawSlots = (StackswitchDrawSlot *) realloc (mDrawSlots,
				 		   sizeof (StackswitchDrawSlot) * (mNWindows + 32));

	if (!mDrawSlots)
	{
	    free (mDrawSlots);
	    return;
	}

	mWindowsSize = mNWindows + 32;
    }

    mWindows[mNWindows++] = w;
}

bool
StackswitchScreen::updateWindowList ()
{
    qsort (mWindows, mNWindows, sizeof (CompWindow *), compareWindows);

    return layoutThumbs ();
}

bool
StackswitchScreen::createWindowList ()
{
    mNWindows = 0;

    foreach (CompWindow *w, screen->windows ())
    {
	STACKSWITCH_WINDOW (w);

	if (sw->isStackswitchable ())
	{
	    STACKSWITCH_WINDOW (w);

	    addWindowToList (w);
	    sw->mAdjust = true;
	}
    }

    return updateWindowList ();
}

void
StackswitchScreen::switchToWindow (bool	   toNext)
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
	    mRotateAdjust = true;
	    mMoreAdjust = true;
	    foreach (CompWindow *w, screen->windows ())
	    {
		STACKSWITCH_WINDOW (w);
		sw->mAdjust = true;
	    }

	    cScreen->damageScreen ();
	    renderWindowTitle ();
	}
    }
}

int
StackswitchScreen::countWindows ()
{
    CompWindow *w;
    int	       count = 0;

    foreach (w, screen->windows ())
    {
	if (StackswitchWindow::get (w)->isStackswitchable ())
	    count++;
    }

    return count;
}

int
StackswitchScreen::adjustStackswitchRotation (float      chunk)
{
    float dx, adjust, amount, rot;

    if (mState != StackswitchStateNone && mState != StackswitchStateIn)
	rot = optionGetTilt ();
    else
	rot = 0.0;

    dx = rot - mRotation;

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
	mRotation = rot;
	return false;
    }

    mRotation += mRVelocity * chunk;
    return true;
}

int
StackswitchWindow::adjustVelocity ()
{
    float dx, dy, ds, dr, adjust, amount;
    float x1, y1, scale, rot;

    STACKSWITCH_SCREEN (screen);

    if (mSlot)
    {
	scale = mSlot->scale;
	x1 = mSlot->x;
	y1 = mSlot->y;
    }
    else
    {
	scale = 1.0f;
	x1 = window->x () - window->border ().left;
	y1 = window->y () + window->height () + window->border ().bottom;
    }

    if (window->id () == ss->mSelectedWindow)
	rot = ss->mRotation;
    else
	rot = 0.0;

    dx = x1 - mTx;

    adjust = dx * 0.15f;
    amount = fabs (dx) * 1.5f;
    if (amount < 0.5f)
	amount = 0.5f;
    else if (amount > 5.0f)
	amount = 5.0f;

    mXVelocity = (amount * mXVelocity + adjust) / (amount + 1.0f);

    dy = y1 - mTy;

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

    dr = rot - mRotation;
    adjust = dr * 0.15f;
    amount = fabs (dr) * 1.5f;
    if (amount < 0.2f)
	amount = 0.2f;
    else if (amount > 2.0f)
	amount = 2.0f;

    mRotVelocity = (amount * mRotVelocity + adjust) / (amount + 1.0f);

    if (fabs (dx) < 0.1f && fabs (mXVelocity) < 0.2f &&
	fabs (dy) < 0.1f && fabs (mYVelocity) < 0.2f &&
	fabs (ds) < 0.001f && fabs (mScaleVelocity) < 0.002f &&
        fabs (dr) < 0.1f && fabs (mRotVelocity) < 0.2f)
    {
	mXVelocity = mYVelocity = mScaleVelocity = 0.0f;
	mTx = x1;
	mTy = y1;
	mRotation = rot;
	mScale = scale;

	return 0;
    }

    return 1;
}

bool
StackswitchScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
				  const GLMatrix	    &transform,
				  const CompRegion	    &region,
				  CompOutput		    *output,
				  unsigned int		    mask)
{
    bool status;
    GLMatrix sTransform (transform);

    if (mState != StackswitchStateNone || mRotation != 0.0)
    {
	mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK;
	mask |= PAINT_SCREEN_TRANSFORMED_MASK;
	mask |= PAINT_SCREEN_CLEAR_MASK;

	sTransform.translate (0.0, -0.5, -DEFAULT_Z_CAMERA);
	sTransform.rotate (-mRotation, 1.0, 0.0, 0.0);
	sTransform.translate (0.0, 0.5, DEFAULT_Z_CAMERA);
    }

    status = gScreen->glPaintOutput (attrib, sTransform, region, output, mask);

    if (mState != StackswitchStateNone &&
    	 ((unsigned int) output->id () == (unsigned int) ~0 ||
	screen->outputDevs ().at (screen->currentOutputDev ().id ()) == *output))
    {
	int           i;
	CompWindow    *aw = NULL;

	sTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);
	glPushMatrix ();
	glLoadMatrixf (sTransform.getMatrix ());

	mPaintingSwitcher = true;

	for (i = 0; i < mNWindows; i++)
	{
	    if (mDrawSlots[i].slot && *(mDrawSlots[i].slot))
	    {
		CompWindow *w = mDrawSlots[i].w;
		if (w->id () == mSelectedWindow)
		    aw = w;

		STACKSWITCH_WINDOW (w);

		sw->gWindow->glPaint (sw->gWindow->paintAttrib (), sTransform, infiniteRegion, 0);
	    }
	}

	GLMatrix tTransform (transform);
	tTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);
	glLoadMatrixf (tTransform.getMatrix ());

	if (mText.getWidth () && (mState != StackswitchStateIn) && aw)
	    drawWindowTitle (sTransform, aw);

	mPaintingSwitcher = false;

	glPopMatrix ();
    }

    return status;
}

void
StackswitchScreen::preparePaint (int msSinceLastPaint)
{
    if (mState != StackswitchStateNone && (mMoreAdjust || mRotateAdjust))
    {
	CompWindow *w;
	int        steps;
	float      amount, chunk;

	amount = msSinceLastPaint * 0.05f * optionGetSpeed ();
	steps  = amount / (0.5f * optionGetTimestep ());

	if (!steps)
	    steps = 1;
	chunk  = amount / (float) steps;

	layoutThumbs ();
	while (steps--)
	{
	    mRotateAdjust = adjustStackswitchRotation (chunk);
	    mMoreAdjust = false;

	    foreach (w, screen->windows ())
	    {
		STACKSWITCH_WINDOW (w);

		if (sw->mAdjust)
		{
		    sw->mAdjust = sw->adjustVelocity ();

		    mMoreAdjust |= sw->mAdjust;

		    sw->mTx       += sw->mXVelocity * chunk;
		    sw->mTy       += sw->mYVelocity * chunk;
		    sw->mScale    += sw->mScaleVelocity * chunk;
		    sw->mRotation += sw->mRotVelocity * chunk;
		}
		else if (sw->mSlot)
		{
		    sw->mScale    = sw->mSlot->scale;
	    	    sw->mTx       = sw->mSlot->x;
	    	    sw->mTy       = sw->mSlot->y;
		    if (w->id () == mSelectedWindow)
			sw->mRotation = mRotation;
		    else
			sw->mRotation = 0.0;
		}
	    }

	    if (!mMoreAdjust && !mRotateAdjust)
		break;
	}
    }

    cScreen->preparePaint (msSinceLastPaint);
}

void
StackswitchScreen::donePaint ()
{
    if (mState != StackswitchStateNone)
    {
	if (mMoreAdjust)
	{
	    cScreen->damageScreen ();
	}
	else
	{
	    if (mRotateAdjust)
		cScreen->damageScreen ();

	    if (mState == StackswitchStateIn)
		mState = StackswitchStateNone;
	    else if (mState == StackswitchStateOut)
		mState = StackswitchStateSwitching;
	}
    }

    cScreen->donePaint ();
}

bool
StackswitchScreen::terminate (CompAction         *action,
			      CompAction::State  state,
			      CompOption::Vector options)
{
    if (mGrabIndex)
    {
        screen->removeGrab (mGrabIndex, 0);
        mGrabIndex = 0;
    }

    if (mState != StackswitchStateNone)
    {
        CompWindow *w;

        foreach (CompWindow *w, screen->windows ())
        {
	    STACKSWITCH_WINDOW (w);

	    if (sw->mSlot)
	    {
	        free (sw->mSlot);
	        sw->mSlot = NULL;

	        sw->mAdjust = true;
	    }
        }
        mMoreAdjust = true;
        mState = StackswitchStateIn;
        cScreen->damageScreen ();

        if (!(state & CompAction::StateCancel) && mSelectedWindow)
        {
	    w = screen->findWindow (mSelectedWindow);
	    if (w)
	        screen->sendWindowActivationRequest (w->id ());
        }
    }

    if (action)
	action->setState (action->state ()  & ~(CompAction::StateTermKey |
			   		        CompAction::StateTermButton |
			   		        CompAction::StateTermEdge));

    return false;
}

bool
StackswitchScreen::initiate (CompAction         *action,
			     CompAction::State  state,
		     	     CompOption::Vector options)
{
    CompMatch  match;
    int        count;

    if (screen->otherGrabExist ("stackswitch", 0))
    {
	return false;
    }

    mCurrentMatch = optionGetWindowMatch ();

    match = CompOption::getMatchOptionNamed (options, "match", CompMatch ());

    mMatch = match;

    count = countWindows ();

    if (count < 1)
    {
	return false;
    }

    if (!mGrabIndex)
    {
	mGrabIndex = screen->pushGrab (screen->invisibleCursor (), "stackswitch");
    }

    if (mGrabIndex)
    {
	mState = StackswitchStateOut;

	if (!createWindowList ())
	    return false;

    	mSelectedWindow = mWindows[0]->id ();
	renderWindowTitle ();

	foreach (CompWindow *w, screen->windows ())
	{
	    STACKSWITCH_WINDOW (w);

	    sw->mTx = w->x () - w->border ().left;
	    sw->mTy = w->y () + w->height () + w->border ().bottom;
	}
    	mMoreAdjust = true;
	cScreen->damageScreen ();
    }

    return true;
}

bool
StackswitchScreen::doSwitch (CompAction           *action,
			     CompAction::State    state,
		     	     CompOption::Vector   options,
		     	     bool                 nextWindow,
		     	     StackswitchType      type)
{
    bool       ret = true;

    if ((mState == StackswitchStateNone) || (mState == StackswitchStateIn))
    {
        if (mType == StackswitchTypeGroup)
        {
	    CompWindow *w;
	    w = screen->findWindow (CompOption::getIntOptionNamed (options, "window", 0));
	    if (w)
	    {
	        mType = StackswitchTypeGroup;
	        mClientLeader =
		    (w->clientLeader ()) ? w->clientLeader () : w->id ();
	        ret = initiate (action, state, options);
	    }
        }
        else
        {
	    mType = type;
	    ret = initiate (action, state, options);
        }

        if (state & CompAction::StateInitKey)
	    action->setState (action->state () | CompAction::StateTermKey);

        if (state & CompAction::StateInitEdge)
	    action->setState (action->state () | CompAction::StateTermEdge);
        else if (state & CompAction::StateInitButton)
	    action->setState (action->state () | CompAction::StateTermButton);
    }

    if (ret)
    switchToWindow (nextWindow);

    return ret;
}

void
StackswitchScreen::windowRemove (Window      id)
{
    CompWindow *w;

    w = screen->findWindow (id);
    if (w)
    {
	bool   inList = false;
	int    j, i = 0;
	Window selected;

	STACKSWITCH_WINDOW (w);

	if (mState == StackswitchStateNone)
	    return;

	if (sw->isStackswitchable ())
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
	    CompOption::Vector o;

	    terminate (NULL, 0, o);
	    return;
	}

	if (!mGrabIndex)
	    return;

	if (updateWindowList ())
	{
	    mMoreAdjust = true;
	    mState = StackswitchStateOut;
	    cScreen->damageScreen ();
	}
    }
}

void
StackswitchScreen::handleEvent (XEvent      *event)
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
    }
}

bool
StackswitchWindow::damageRect (bool initial,
			       const CompRect &rect)
{
    bool       status = false;

    STACKSWITCH_SCREEN (screen);

    if (initial)
    {
	if (ss->mGrabIndex && isStackswitchable ())
	{
	    ss->addWindowToList (window);
	    if (ss->updateWindowList ())
	    {
		mAdjust = true;
		ss->mMoreAdjust = true;
		ss->mState = StackswitchStateOut;
		ss->cScreen->damageScreen ();
	    }
	}
    }
    else if (ss->mState == StackswitchStateSwitching)
    {
	if (mSlot)
	{
	    ss->cScreen->damageScreen ();
	    status = true;
	}
    }

    status |= cWindow->damageRect (initial, rect);

    return status;
}


StackswitchScreen::StackswitchScreen (CompScreen *screen) :
    PluginClassHandler <StackswitchScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    mGrabIndex (0),
    mState (StackswitchStateNone),
    mMoreAdjust (false),
    mRotateAdjust (false),
    mPaintingSwitcher (false),
    mRVelocity (0.0f),
    mWindows (NULL),
    mDrawSlots (NULL),
    mWindowsSize (0),
    mNWindows (0),
    mClientLeader (None),
    mSelectedWindow (None)
{
    ScreenInterface::setHandler (screen);
    CompositeScreenInterface::setHandler (cScreen);
    GLScreenInterface::setHandler (gScreen);

#define STACKTERMBIND(opt, func)                                \
    optionSet##opt##Terminate (boost::bind (&StackswitchScreen::func, \
					    this, _1, _2, _3));

#define STACKSWITCHBIND(opt, func, next, type)                 \
    optionSet##opt##Initiate (boost::bind (&StackswitchScreen::func, \
					    this, _1, _2, _3, \
					    next, type));

    STACKSWITCHBIND (NextKey, doSwitch, true, StackswitchTypeNormal);
    STACKSWITCHBIND (PrevKey, doSwitch, false, StackswitchTypeNormal);
    STACKSWITCHBIND (NextAllKey, doSwitch, true, StackswitchTypeAll);
    STACKSWITCHBIND (PrevAllKey, doSwitch, false, StackswitchTypeAll);
    STACKSWITCHBIND (NextGroupKey, doSwitch, true, StackswitchTypeGroup);
    STACKSWITCHBIND (PrevGroupKey, doSwitch, false, StackswitchTypeGroup);

    STACKTERMBIND (NextKey, terminate);
    STACKTERMBIND (PrevKey, terminate);
    STACKTERMBIND (NextAllKey, terminate);
    STACKTERMBIND (PrevAllKey, terminate);
    STACKTERMBIND (NextGroupKey, terminate);
    STACKTERMBIND (PrevGroupKey, terminate);

    STACKSWITCHBIND (NextButton, doSwitch, true, StackswitchTypeNormal);
    STACKSWITCHBIND (PrevButton, doSwitch, false, StackswitchTypeNormal);
    STACKSWITCHBIND (NextAllButton, doSwitch, true, StackswitchTypeAll);
    STACKSWITCHBIND (PrevAllButton, doSwitch, false, StackswitchTypeAll);
    STACKSWITCHBIND (NextGroupButton, doSwitch, true, StackswitchTypeGroup);
    STACKSWITCHBIND (PrevGroupButton, doSwitch, false, StackswitchTypeGroup);

    STACKTERMBIND (NextButton, terminate);
    STACKTERMBIND (PrevButton, terminate);
    STACKTERMBIND (NextAllButton, terminate);
    STACKTERMBIND (PrevAllButton, terminate);
    STACKTERMBIND (NextGroupButton, terminate);
    STACKTERMBIND (PrevGroupButton, terminate);
}

StackswitchScreen::~StackswitchScreen ()
{
    if (mWindows)
	free (mWindows);

    if (mDrawSlots)
	free (mDrawSlots);
}

StackswitchWindow::StackswitchWindow (CompWindow *window) :
    PluginClassHandler <StackswitchWindow, CompWindow> (window),
    window (window),
    cWindow (CompositeWindow::get (window)),
    gWindow (GLWindow::get (window)),
    mSlot (NULL),
    mXVelocity (0.0f),
    mYVelocity (0.0f),
    mScaleVelocity (0.0f),
    mRotVelocity (0.0f),
    mTx (0.0f),
    mTy (0.0f),
    mScale (1.0f),
    mRotation (0.0f),
    mAdjust (false)
{
    CompositeWindowInterface::setHandler (cWindow);
    GLWindowInterface::setHandler (gWindow);
}

StackswitchWindow::~StackswitchWindow ()
{
    if (mSlot)
	free (mSlot);
}

bool
StackswitchPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
    	return false;

    if (!CompPlugin::checkPluginABI ("text", COMPIZ_TEXT_ABI))
    {
	compLogMessage ("stackswitch", CompLogLevelWarn, "No compatible text plugin"\
						          " loaded");
	textAvailable = false;
    }
    else
	textAvailable = true;

    return true;
}
