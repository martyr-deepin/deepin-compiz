/**
 *
 * Compiz group plugin
 *
 * paint.cpp
 *
 * Copyright : (C) 2006-2010 by Patrick Niklaus, Roi Cohen,
 * 				Danny Baumann, Sam Spilsbury
 * Authors: Patrick Niklaus <patrick.niklaus@googlemail.com>
 *          Roi Cohen       <roico.beryl@gmail.com>
 *          Danny Baumann   <maniac@opencompositing.org>
 * 	    Sam Spilsbury   <smspillaz@gmail.com>
 *
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

#include "group.h"

/*
 * GroupTabBarSlot::setTargetOpacity
 *
 * Convenience function to set the target opacity for the tab bar
 */

void
GroupTabBarSlot::setTargetOpacity (int tOpacity)
{
    mOpacity = tOpacity;
}

/*
 * TextureLayer::setPaintWindow
 *
 * Convenience function to set the window we are painting on top of
 * for a texture for the tab bar
 */

void
TextureLayer::setPaintWindow (CompWindow *w)
{
    mPaintWindow = w;
}

/*
 * GroupTabBarSlot::List::paint
 *
 * Paint all tabs in a list
 *
 */

void
GroupTabBarSlot::List::paint (const GLWindowPaintAttrib &attrib,
			      const GLMatrix	        &transform,
			      const CompRegion	        &region,
			      const CompRegion	        &clipRegion,
			      int			mask)
{
    GROUP_SCREEN (screen);

    foreach (GroupTabBarSlot *slot, *this)
    {
	if (slot != gs->mDraggedSlot || !gs->mDragged)
	{
	    slot->setTargetOpacity (attrib.opacity);
	    slot->paint (attrib, transform, clipRegion,
			  clipRegion, mask);
	}
    }
}

/*
 * GroupTabBarSlot::paint - taken from switcher and modified for tab bar
 *
 * We need to scale down the window to a small thumbnail, skip the
 * geometry modification stage (so we don't get wobbly uglyness)
 * and paint it directly to the screen for a second time.
 *
 * Also fade in and out.
 *
 */
void
GroupTabBarSlot::paint (const GLWindowPaintAttrib &attrib,
			const GLMatrix	          &transform,
			const CompRegion	  &region,
			const CompRegion	  &clipRegion,
			int			  mask)
{
    CompWindow            *w = mWindow;
    unsigned int	  oldGlAddGeometryIndex;
    GLWindowPaintAttrib   wAttrib (GLWindow::get (mWindow)->paintAttrib ());
    int                   tw, th;

    GROUP_WINDOW (w);
    GROUP_SCREEN (screen);

    tw = mRegion.boundingRect ().width ();
    th = mRegion.boundingRect ().height ();

    /* Wrap glDrawGeometry to make sure the general
       glDrawGeometry function is used */
    oldGlAddGeometryIndex = gw->gWindow->glAddGeometryGetCurrentIndex ();
    gw->gWindow->glAddGeometrySetCurrentIndex (MAXSHORT);

    /* animate fade */
    if (mTabBar->mState == PaintFadeIn)
    {
	wAttrib.opacity -= wAttrib.opacity * mTabBar->mAnimationTime /
	                   (gs->optionGetFadeTime () * 1000);
    }
    else if (mTabBar->mState == PaintFadeOut)
    {
	wAttrib.opacity = wAttrib.opacity * mTabBar->mAnimationTime /
	                  (gs->optionGetFadeTime () * 1000);
    }

    wAttrib.opacity = wAttrib.opacity * mOpacity / OPAQUE;

    if (w->mapNum ())
    {
	GLFragment::Attrib fragment (wAttrib);
	GLMatrix	   wTransform (transform);
	int           	   width, height;
	int         	   vx, vy;
	unsigned int	   oldGlDrawIndex;

	width = w->width () + w->output ().left + w->output ().right;
	height = w->height () + w->output ().top + w->output ().bottom;

	if (width > tw)
	    wAttrib.xScale = (float) tw / width;
	else
	    wAttrib.xScale = 1.0f;
	if (height > th)
	    wAttrib.yScale = (float) tw / height;
	else
	    wAttrib.yScale = 1.0f;

	if (wAttrib.xScale < wAttrib.yScale)
	    wAttrib.yScale = wAttrib.xScale;
	else
	    wAttrib.xScale = wAttrib.yScale;

	/* FIXME: do some more work on the highlight on hover feature
	// Highlight on hover
	if (group && group->mTabBar && group->mTabBar->hoveredSlot == slot) {
	wAttrib.saturation = 0;
	wAttrib.brightness /= 1.25f;
	}*/

	getDrawOffset (vx, vy);

	wAttrib.xTranslate = (mRegion.boundingRect ().x1 () +
			      mRegion.boundingRect ().x2 ()) / 2 + vx;
	wAttrib.yTranslate = mRegion.boundingRect ().y1 () + vy;

	/* Translate matrix to the first point in the drawn region
	 * and then scale (scales to the top right corner) to our
	 * desired size */
	wTransform.translate (wAttrib.xTranslate, wAttrib.yTranslate, 0.0f);
	wTransform.scale (wAttrib.xScale, wAttrib.yScale, 1.0f);
	wTransform.translate (-(WIN_X (w) + WIN_WIDTH (w) / 2),
			 -(WIN_Y (w) - w->output ().top), 0.0f);

	glPushMatrix ();
	glLoadMatrixf (wTransform.getMatrix ());

	/* Skip to the end of glDraw, so we don't end up with wobbly
	 * and all that (we also loaded with a simple matrix, so we
	 * miss the one in glPaint) */
	oldGlDrawIndex = gw->gWindow->glDrawGetCurrentIndex ();
	gw->gWindow->glDraw (wTransform, fragment, clipRegion,
			  mask | PAINT_WINDOW_TRANSFORMED_MASK |
			  PAINT_WINDOW_TRANSLUCENT_MASK);
	gw->gWindow->glDrawSetCurrentIndex (oldGlDrawIndex);

	glPopMatrix ();
    }

    gw->gWindow->glAddGeometrySetCurrentIndex (oldGlAddGeometryIndex);
}

/*
 * TextureLayer::paint
 *
 * Paint some texture on with a window's geometry. This involves
 * putting the texture in the right place, adding it's geometry to the
 * window geometry (so it can be modified by wobbly and friends),
 * adding it, setting the texture layer opacity and then painting it
 * with the window geometry
 *
 */

void
TextureLayer::paint (const GLWindowPaintAttrib &attrib,
		     const GLMatrix	       &transform,
		     const CompRegion	       &paintRegion,
		     const CompRegion	       &clipRegion,
		     int		       mask)
{
    GroupWindow *gwTopTab = GroupWindow::get (mPaintWindow);
    const CompRect &box = paintRegion.boundingRect ();

    /* Handle tiled textures */
    foreach (GLTexture *tex, mTexture)
    {
	GLTexture::Matrix matrix = tex->matrix ();
	GLTexture::MatrixList matl;
	CompRegion reg;

	int x1 = box.x1 ();
	int y1 = box.y1 ();
	int x2 = box.x2 ();
	int y2 = box.y2 ();

	/* remove the old x1 and y1 so we have a relative value */
	x2 -= x1;
	y2 -= y1;
	x1 = (x1 - mPaintWindow->x ()) / attrib.xScale +
		     mPaintWindow->x ();
	y1 = (y1 - mPaintWindow->y ()) / attrib.yScale +
		     mPaintWindow->y ();

	/* now add the new x1 and y1 so we have a absolute value again,
	also we don't want to stretch the texture... */
	if (x2 * attrib.xScale < width ())
	    x2 += x1;
	else
	    x2 = x1 + width ();

	if (y2 * attrib.yScale < height ())
	    y2 += y1;
	else
	    y2 = y1 + height ();

	/* Set the x-position to our x1 minus the scale factor */
	matrix.x0 -= x1 * matrix.xx;
	matrix.y0 -= y1 * matrix.yy;

	matl.push_back (matrix);
	reg = CompRegion (x1, y1,
			  x2 - x1,
			  y2 - y1);

	/* Reset current window geometry and re-add it with this
	 * new geometry for the tab bar */
	gwTopTab->gWindow->geometry ().reset ();

	gwTopTab->gWindow->glAddGeometry (matl, reg, clipRegion);

	if (gwTopTab->gWindow->geometry ().vertices)
	{
	    GLFragment::Attrib fragment (attrib);
	    GLMatrix	       wTransform (transform);

	    /* Translate to where we want to paint, and scale
	     * (via a 3x3 matrix) */
	    wTransform.translate (WIN_X (mPaintWindow),
				  WIN_Y (mPaintWindow), 0.0f);
	    wTransform.scale (attrib.xScale, attrib.yScale, 1.0f);
	    wTransform.translate (
			 attrib.xTranslate / attrib.xScale
						 - WIN_X (mPaintWindow),
			 attrib.yTranslate / attrib.yScale
						 - WIN_Y (mPaintWindow),
			 0.0f);

	    glPushMatrix ();
	    glLoadMatrixf (wTransform.getMatrix ());

	    fragment.setOpacity (attrib.opacity);

	    gwTopTab->glDrawTexture (tex, fragment, mask |
					PAINT_WINDOW_BLEND_MASK |
					PAINT_WINDOW_TRANSFORMED_MASK |
					PAINT_WINDOW_TRANSLUCENT_MASK);

	    glPopMatrix ();
	}
    }
}

/*
 * BackgroundLayer::paint
 *
 * Paint the backgroud layer. It might need to be scaled a bit
 * if it is expanding
 */

void
BackgroundLayer::paint (const GLWindowPaintAttrib &attrib,
			const GLMatrix	          &transform,
			const CompRegion	  &paintRegion,
			const CompRegion	  &clipRegion,
			int		          mask)
{
    int   newWidth;
    GLWindowPaintAttrib wAttrib (attrib);
    CompRect box = paintRegion.boundingRect ();

    /* handle the repaint of the background */
    newWidth = mGroup->mTabBar->mRegion.boundingRect ().width ();
    if (newWidth > width ())
	newWidth = width ();

    /* if the region expanded and we haven't re-rended ,just scale
     * the tab bar up slightly */
    wAttrib.xScale = (double) (mGroup->mTabBar->mRegion.boundingRect ().width () /
		       (double) newWidth);

    /* FIXME: maybe move this over to groupResizeTabBarRegion -
     * the only problem is that we would have 2 redraws if
     * here is an animation */
    if (newWidth != mGroup->mTabBar->mOldWidth ||
        mGroup->mTabBar->mBgLayer->mBgAnimation)
	render ();

    mGroup->mTabBar->mOldWidth = newWidth;
    box	  = mGroup->mTabBar->mRegion.boundingRect ();

    TextureLayer::paint (wAttrib, transform, box, clipRegion, mask);
}

/*
 * SelectionLayer::paint
 *
 * Paint the selection background behind the selection tab. This is
 * just a regular texture to paint that normally
 *
 */

void
SelectionLayer::paint (const GLWindowPaintAttrib &attrib,
		       const GLMatrix	         &transform,
		       const CompRegion	  	 &paintRegion,
		       const CompRegion	  	 &clipRegion,
		       int		         mask)
{
    TextureLayer::paint (attrib, transform,
			 mGroup->mTabBar->mTopTab->mRegion,
			 clipRegion, mask);
}

/*
 * TextLayer::paint
 *
 * Paint the text layer on top of the selection layer,
 *
 * We need to adjust the region here and paint faded in or out (then
 * just paint like a normal texture here)
 */

void
TextLayer::paint (const GLWindowPaintAttrib &attrib,
	          const GLMatrix	    &transform,
	          const CompRegion	    &paintRegion,
	          const CompRegion	    &clipRegion,
	          int		            mask)
{
    /* add a slight buffer around the clipping region
     * to account for the text
     */
    CompRect	        box;
    int			alpha = OPAQUE;
    GLWindowPaintAttrib wAttrib (attrib);

    GROUP_SCREEN (screen);

    int x1 = mGroup->mTabBar->mRegion.boundingRect ().x1 () + 5;
    int x2 = mGroup->mTabBar->mRegion.boundingRect ().x1 () +
	     width () + 5;
    int y1 = mGroup->mTabBar->mRegion.boundingRect ().y2 () -
	     height () - 5;
    int y2 = mGroup->mTabBar->mRegion.boundingRect ().y2 () - 5;

    if (x2 > mGroup->mTabBar->mRegion.boundingRect ().x2 ())
	x2 = mGroup->mTabBar->mRegion.boundingRect ().x2 ();

    box = CompRect (x1, y1, x2 - x1, y2 - y1);

    /* recalculate the alpha again for text fade... */
    if (mState == PaintFadeIn)
	alpha -= alpha * mAnimationTime /
	     (gs->optionGetFadeTextTime () * 1000);
    else if (mState == PaintFadeOut)
	alpha = alpha * mAnimationTime /
	    (gs->optionGetFadeTextTime () * 1000);

    wAttrib.opacity = alpha * ((float) wAttrib.opacity / OPAQUE);

    TextureLayer::paint (wAttrib, transform, box, clipRegion, mask);
}

/*
 * GroupTabBar::paint
 *
 * Paint the tab bar. This involves determining the window we want to
 * paint with geometry (the top tab usually, unless animating). All
 * of the other layers have paint functions, so add those to a list
 * and batch-paint them
 *
 */
void
GroupTabBar::paint (const GLWindowPaintAttrib    &attrib,
		    const GLMatrix		 &transform,
		    unsigned int		 mask,
		    CompRegion		 	 clipRegion)
{
    CompWindow      *topTab;
    std::vector <GLLayer *> paintList;
    CompRect        box;

    GROUP_SCREEN (screen);

    if (HAS_TOP_WIN (mGroup))
	topTab = TOP_TAB (mGroup);
    else
	topTab = PREV_TOP_TAB (mGroup);

    /* Set the windows we want to paint with */

    mBgLayer->setPaintWindow (topTab);
    mSelectionLayer->setPaintWindow (topTab);

    /* Paint background, then selection, then slots and then
     * if we can, the text */

    paintList.push_back (mBgLayer);
    paintList.push_back (mSelectionLayer);
    paintList.push_back (&mSlots);

    if (mTextLayer && (mTextLayer->mState != PaintOff))
    {
	mTextLayer->setPaintWindow (topTab);
	paintList.push_back (mTextLayer);
    }

    /* On each layer, set up texture filtering, fade in and out and
     * paint the layer */

    foreach (GLLayer *layer, paintList)
    {
	GLWindowPaintAttrib wAttrib (attrib);
	GLenum              oldTextureFilter;
	int            	    alpha = OPAQUE;

	wAttrib.xScale = 1.0f;
	wAttrib.yScale = 1.0f;

	oldTextureFilter = gs->gScreen->textureFilter ();

	if (gs->optionGetMipmaps ())
	    gs->gScreen->setTextureFilter (GL_LINEAR_MIPMAP_LINEAR);

	if (mState == PaintFadeIn)
	    alpha -= alpha * mAnimationTime /
				      (gs->optionGetFadeTime () * 1000);
	else if (mState == PaintFadeOut)
	    alpha = alpha * mAnimationTime /
				      (gs->optionGetFadeTime () * 1000);

	wAttrib.opacity = alpha * ((float) wAttrib.opacity / OPAQUE);
	layer->paint (wAttrib, transform, clipRegion, clipRegion, mask);

	gs->gScreen->setTextureFilter (oldTextureFilter);
    }
}

/*
 * Selection::paint
 *
 * Paint the selection outline here, basically just draw a basic outline
 * shape with opengl and paint with the right transformation matrix
 *
 */
void
Selection::paint (const GLScreenPaintAttrib sa,
		  const GLMatrix	    transform,
		  CompOutput                *output,
		  bool                      transformed)
{
    GROUP_SCREEN (screen);

    int x1, x2, y1, y2;

    x1 = MIN (mX1, mX2);
    y1 = MIN (mY1, mY2);
    x2 = MAX (mX1, mX2);
    y2 = MAX (mY1, mY2);

    if (gs->mGrabState == GroupScreen::ScreenGrabSelect)
    {
	GLMatrix sTransform (transform);

	if (transformed)
	{
	    gs->gScreen->glApplyTransform (sa, output, &sTransform);
	    sTransform.toScreenSpace (output, -sa.zTranslate);
	}
	else
	    sTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);

	glPushMatrix ();
	glLoadMatrixf (sTransform.getMatrix ());

	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glEnable (GL_BLEND);

	glColor4usv (gs->optionGetFillColor ());
	glRecti (x1, y2, x2, y1);

	glColor4usv (gs->optionGetLineColor ());
	glBegin (GL_LINE_LOOP);
	glVertex2i (x1, y1);
	glVertex2i (x2, y1);
	glVertex2i (x2, y2);
	glVertex2i (x1, y2);
	glEnd ();

	glColor4usv (defaultColor);
	glDisable (GL_BLEND);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glPopMatrix ();
    }
}

/*
 * GroupScreen::preparePaint
 *
 * Called before ::glPaint, this tells us how long it has been
 * since the last screen paint cycle, so we can handle animation.
 *
 * Go through the groups, handle the switch, tabbing, and fade
 * animations
 *
 */
void
GroupScreen::preparePaint (int msSinceLastPaint)
{
    GroupSelection *group;
    GroupSelection::List::iterator it = mGroups.begin ();
    bool	   keepPainting = false;

    cScreen->preparePaint (msSinceLastPaint);

    while (it != mGroups.end ())
    {
	group = *it;
	GroupTabBar *bar = group->mTabBar;

	if (bar)
	{
	    keepPainting |= bar->applyForces ((mDragged) ? mDraggedSlot : NULL);
	    bar->applySpeeds (msSinceLastPaint);

	    if (bar->mState == PaintFadeIn ||
	        bar->mState == PaintFadeOut)
		keepPainting |= bar->handleTabBarFade (msSinceLastPaint);

	    if (bar->mTextLayer)
		keepPainting |= bar->handleTextFade (msSinceLastPaint);

	    if (bar->mBgLayer && bar->mBgLayer->mBgAnimation)
		keepPainting |= bar->mBgLayer->handleAnimation (msSinceLastPaint);
	}

	if (group->mTabBar &&
	    group->mTabBar->mChangeState != GroupTabBar::NoTabChange)
	{
	    /* Only change to the new tab once the animation is done
	     */
	    group->mTabBar->mChangeAnimationTime -= msSinceLastPaint;
	    if (group->mTabBar->mChangeAnimationTime <= 0)
		keepPainting |= group->handleAnimation ();
	    else
		keepPainting = true;
	}

	/* groupDrawTabAnimation may delete the group, so better
	   save the pointer to the next chain element */

	++it;

	if (group->mTabbingState != GroupSelection::NoTabbing)
	    keepPainting |= group->drawTabAnimation (msSinceLastPaint);
    }

    /* We just need to disable preparePaint here directly, since
     * checkFunctions will enable it again if there are groups with
     * animations or groups with tab bars with a dragged slot */

    if (!keepPainting)
	cScreen->preparePaintSetEnabled (this, false);

    /* Always enable donePaint here (since there might be some
     * damage or whatever)
     */

    cScreen->donePaintSetEnabled (this, true);
}

/*
 * GroupScreen::glPaintOutput
 *
 * The base output-paint function. Here we just need to paint the
 * selection layer and the dragged slot on top of everything else
 *
 */
bool
GroupScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
			    const GLMatrix	      &transform,
			    const CompRegion	      &region,
			    CompOutput		      *output,
			    unsigned int	      mask)
{
    GroupSelection *group;
    bool           status;

    /* Keep track of viewports */
    mTmpSel.mPainted = false;
    mTmpSel.mVpX = screen->vp ().x ();
    mTmpSel.mVpY = screen->vp ().y ();

    /* Allow us to paint windows transformed
     */

    foreach (group, mGroups)
    {
	if (group->mResizeInfo ||
	    (group->mTabBar &&
	     (group->mTabBar->mChangeState != GroupTabBar::NoTabChange ||
	      group->mTabBar->mState != PaintOff)) ||
	    group->mTabbingState != GroupSelection::NoTabbing)
	{
	    mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK;
	    break;
	}
    }

    status = gScreen->glPaintOutput (attrib, transform, region, output,
				     mask);

    /* Just double check that we haven't painted our dragged tab
     * and selection rect on a transformed screen */
    if (status && !mTmpSel.mPainted)
    {
	if ((mGrabState == ScreenGrabTabDrag) && mDraggedSlot)
	{
	    GLMatrix wTransform (transform);
	    GLWindow *gWindow = GLWindow::get (mDraggedSlot->mWindow);
	    PaintState    state;

	    wTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);

	    glPushMatrix ();
	    glLoadMatrixf (wTransform.getMatrix ());

	    /* prevent tab bar drawing.. */
	    state = mDraggedSlot->mTabBar->mState;
	    mDraggedSlot->mTabBar->mState = PaintOff;
	    mDraggedSlot->setTargetOpacity (OPAQUE);
	    mDraggedSlot->paint (gWindow->paintAttrib (),
				 wTransform, region, region, 0);
	    mDraggedSlot->mTabBar->mState = state;

	    glPopMatrix ();
	}
	else if (mGrabState == ScreenGrabSelect)
	{
	    mTmpSel.paint (attrib, transform, output, false);
	}
    }

    return status;
}

/*
 * GroupScreen::glPaintTransformedOutput
 *
 * This gets called if the screen is transformed, since there are
 * are different conditions here, we want to ensure that our
 * tab drag animation is still painted correctly
 *
 */
void
GroupScreen::glPaintTransformedOutput (const GLScreenPaintAttrib &attrib,
				       const GLMatrix		&transform,
				       const CompRegion	        &region,
				       CompOutput		*output,
				       unsigned int		mask)
{
    gScreen->glPaintTransformedOutput (attrib, transform, region, output, mask);

    /* If we are on the same viewport here, then we are OK to paint */
    if ((mTmpSel.mVpX == screen->vp ().x ()) &&
         (mTmpSel.mVpY == screen->vp ().y ()))
    {
	mTmpSel.mPainted = true;

	if ((mGrabState == ScreenGrabTabDrag) &&
	    mDraggedSlot && mDragged)
	{
	    PaintState state;
	    GLMatrix wTransform (transform);
	    GLWindow *gWindow = GLWindow::get (mDraggedSlot->mWindow);

	    gScreen->glApplyTransform (attrib, output, &wTransform);
	    wTransform.toScreenSpace (output, -attrib.zTranslate);
	    glPushMatrix ();
	    glLoadMatrixf (wTransform.getMatrix ());

	    /* prevent tab bar drawing.. */
	    state = mDraggedSlot->mTabBar->mState;
	    mDraggedSlot->mTabBar->mState = PaintOff;
	    mDraggedSlot->setTargetOpacity (OPAQUE);
	    mDraggedSlot->paint (gWindow->paintAttrib (),
				 wTransform, region, region, 0);
	    mDraggedSlot->mTabBar->mState = state;

	    glPopMatrix ();
	}
	else if (mGrabState == ScreenGrabSelect)
	{
	    mTmpSel.paint (attrib, transform, output, true);
	}
    }
}

/*
 * GroupScreen::donePaint
 *
 * Damage everything that needs to be damaged (usually the screen
 * for animations [FIXME] or tab bar / text regions if they are
 * currently animating
 *
 */
void
GroupScreen::donePaint ()
{
    GroupSelection *group;
    bool	   damaged = false;

    cScreen->donePaint ();

    foreach (group, mGroups)
    {
	/* Animations are a special case, damage the whole screen */
	if (group->mTabbingState != GroupSelection::NoTabbing)
	{
	    cScreen->damageScreen ();
	    damaged = true;
	}
	else if (group->mTabBar &&
		 group->mTabBar->mChangeState !=
					      GroupTabBar::NoTabChange)
	{
	    cScreen->damageScreen ();
	    damaged = true;
	}
	else if (group->mTabBar)
	{
	    bool needDamage = false;

	    if ((group->mTabBar->mState == PaintFadeIn) ||
		(group->mTabBar->mState == PaintFadeOut))
	    {
		needDamage = true;
	    }

	    if (group->mTabBar->mTextLayer)
	    {
		if ((group->mTabBar->mTextLayer->mState == PaintFadeIn) ||
		    (group->mTabBar->mTextLayer->mState == PaintFadeOut))
		{
		    needDamage = true;
		}
	    }

	    if (group->mTabBar->mBgLayer &&
		group->mTabBar->mBgLayer->mBgAnimation)
		needDamage = true;

	    if (mDraggedSlot)
		needDamage = true;

	    /* If we needed damage, then damage the whole tab bar
	     * region */
	    if (needDamage)
		group->mTabBar->damageRegion ();

	    damaged |= needDamage;
	}
    }


    /* If nothing needed damaging we can disable donePaint for now:
     * it will come back again when we call preparePaint anyways
     */
    if (!damaged)
	cScreen->donePaintSetEnabled (this, false);
}

/*
 * GroupWindow::glDraw
 *
 * Our matrix is initialized here, so we can paint the glow here
 * (since we are free to paint with geometry)
 *
 */
bool
GroupWindow::glDraw (const GLMatrix           &transform,
		     GLFragment::Attrib       &attrib,
		     const CompRegion	      &region,
		     unsigned int	      mask)
{
    bool       status;
    CompRegion paintRegion (region);

    /* Don't bother if we don't need to paint glow */
    if (mGroup && (mGroup->mWindows.size () > 1) && mGlowQuads)
    {
	if (mask & PAINT_WINDOW_TRANSFORMED_MASK)
	    paintRegion = CompRegion (infiniteRegion);

	if (paintRegion.numRects ())
	{
	    /* reset geometry and paint */
	    gWindow->geometry ().reset ();

	    paintGlow (attrib, paintRegion, mask);
	}
    }

    status = gWindow->glDraw (transform, attrib, region, mask);

    return status;
}

/*
 * GroupWindow::getStretchRectangle
 *
 * Return how much to scale on the X and Y axis for some box
 * provided compared to the window geometry */

void
GroupWindow::getStretchRectangle (CompRect &box,
				       float    &xScaleRet,
				       float    &yScaleRet)
{
    int    x1, x2, y1, y2;
    int    width, height;
    float  xScale, yScale;

    x1 = mResizeGeometry.x () - window->border ().left;
    y1 = mResizeGeometry.y () - window->border ().top;
    x2 = mResizeGeometry.x () + mResizeGeometry.width () +
	     window->serverGeometry ().border () * 2 + window->border ().right;

    if (window->shaded ())
    {
	y2 = mResizeGeometry.y () + window->height () + window->border ().bottom;
    }
    else
    {
	y2 = mResizeGeometry.y () + mResizeGeometry.height () +
	     window->serverGeometry ().border () * 2 +
	     window->border ().bottom;
    }

    width  = window->width ()  + window->border ().left +
	     window->border ().right;
    height = window->height () + window->border ().top  +
	     window->border ().bottom;

    xScale = (width)  ? (x2 - x1) / (float) width  : 1.0f;
    yScale = (height) ? (y2 - y1) / (float) height : 1.0f;

    x1 = x1 - (window->output ().left - window->border ().left) * xScale;
    y1 = y1 - (window->output ().top - window->border ().top) * yScale;
    x2 = x2 + window->output ().right * xScale;
    y2 = y2 + window->output ().bottom * yScale;

    box = CompRect (x1, y1, x2 - x1, y2 - y1);

    xScaleRet = xScale;
    yScaleRet = yScale;
}

/*
 * GroupScreen::damagePaintRectangle
 *
 * Damage some region, with 1px padding
 */

void
GroupScreen::damagePaintRectangle (const CompRect &box)
{
    CompRegion reg (box);

    reg.translate (-1, -1);
    reg.shrink (1, 1);

    cScreen->damageRegion (reg);
}

/*
 * GroupWindow::checkTabbing
 *
 * Check if this window should be tabbing
 *
 */
bool
GroupWindow::checkTabbing ()
{
    /* Do the tabbing animation if we are currently in an animated
     * state and the following check fails
     * -> We have a tab bar AND
     * -> This is the top tab for the window AND
     * -> We are in a tabbing state
     *
     * In essence, we want to animate all windows in this group
     * in the tabbing animation, except where the current window
     * is the "prinicpal" window (ignoring a situation where
     * we are ungrouping a single window, in which case we animate
     * all windows)
     */

    if (!mGroup || !mGroup->mTabBar)
	return false;

    return  (mAnimateState & (IS_ANIMATED | FINISHED_ANIMATION)) &&
	     !(mGroup->mTabBar && IS_TOP_TAB (window, mGroup) &&
	      (mGroup->mTabbingState == GroupSelection::Tabbing));
}

/*
 * GroupWindow::checkRotating
 *
 * Check if this window should be rotating
 *
 */
bool
GroupWindow::checkRotating ()
{
    /* Rotate the window if we are changing tabs, and if the window
     * passes the following safety checks:
     * -> It has a top tab (and associated window) AND
     * -> It has a previous top tab (the window that we are
     * 				 switching from) AND
     * Either
     *  -> It is a top tab (and associated window) or OR
     *  -> It is a previous top tab (and associated window) OR
     *
     * In essense, we can only do the rotate animation if there is
     * a window we are switching to or from (since the animation
     * has a "from" stage and a "to" stage)
     */

    if (!mGroup)
	return false;

    return (mGroup->mTabBar &&
	    mGroup->mTabBar->mChangeState != GroupTabBar::NoTabChange) &&
	     HAS_TOP_WIN (mGroup) && HAS_PREV_TOP_WIN (mGroup) &&
	     (IS_TOP_TAB (window , mGroup) ||
	      IS_PREV_TOP_TAB (window, mGroup));
}

/*
 * GroupWindow::checkShowTabBar
 *
 * Check if this window should show it's tab bar
 *
 */
bool
GroupWindow::checkShowTabBar ()
{
    /* Show the tab bar if it exists, and is set to be painted and
     * the following checks pass:
     * Either:
     * -> This window is the top tab for the group AND
     *    Either:
     *    -> We aren't changing tabs OR
     *    -> We are changing to this tab
     * OR:
     * -> This window is the previous top tab AND
     * -> We are changing away from this window
     *
     * The tab bar should be visible during the rotate animation.
     * All other times it should be invisible, except when hovering
     * over it
     *
     */

    if (!mGroup)
	return false;

    return mGroup->mTabBar && (mGroup->mTabBar->mState != PaintOff) &&
	     (((IS_TOP_TAB (window, mGroup)) &&
	       ((mGroup->mTabBar->mChangeState == GroupTabBar::NoTabChange) ||
		(mGroup->mTabBar->mChangeState == GroupTabBar::TabChangeNewIn))) ||
	      (IS_PREV_TOP_TAB (window, mGroup) &&
	       (mGroup->mTabBar->mChangeState == GroupTabBar::TabChangeOldOut)));
}

inline void
perspectiveDistortAndResetZ (GLMatrix &transform)
{
    float v = -1.0 / screen->width ();
    /*
      This does
      transform = M * transform, where M is
      1, 0, 0, 0,
      0, 1, 0, 0,
      0, 0, 0, v,
      0, 0, 0, 1
    */

    transform[8] = v * transform[12];
    transform[9] = v * transform[13];
    transform[10] = v * transform[14];
    transform[11] = v * transform[15];
}

/*
 * GroupWindow::glPaint
 *
 * This is different to GLDraw, since we can still modify the compiz
 * matrix.
 *
 * In this function, we handle painting of the tabbing/untabbing
 * stretched windows for resize and rotation of windows when switching
 * tabs
 *
 */
bool
GroupWindow::glPaint (const GLWindowPaintAttrib &attrib,
		      const GLMatrix		&transform,
		      const CompRegion		&region,
		      unsigned int		mask)
{
    bool       status;
    bool       doRotate = checkRotating ();
    bool       doTabbing = checkTabbing ();
    bool       showTabbar = checkShowTabBar ();
    CompWindow *w = window;

    GROUP_SCREEN (screen);

    /* If this window is hidden, then don't draw it on screen */
    if (mWindowHideInfo)
	mask |= PAINT_WINDOW_NO_CORE_INSTANCE_MASK;

    /* If the window is being:
     * -> Selected
     * -> Group Resized
     * -> Rotated
     * -> Tabbed
     * -> Has a tab bar
     */

    if (mInSelection || !mResizeGeometry.isEmpty () || doRotate ||
	doTabbing || showTabbar)
    {
	GLWindowPaintAttrib wAttrib (attrib);
	GLMatrix            wTransform (transform);
	float               animProgress = 0.0f;
	int                 drawnPosX = 0, drawnPosY = 0;

	/* If it's selected, show that by changing it's paint
	 * attributes (such as brightness, opacity, saturation */
	if (mInSelection)
	{
	    wAttrib.opacity    = OPAQUE * gs->optionGetSelectOpacity () / 100;
	    wAttrib.saturation = COLOR * gs->optionGetSelectSaturation () / 100;
	    wAttrib.brightness = BRIGHT * gs->optionGetSelectBrightness () / 100;
	}

	if (doTabbing)
	{
	    /* fade the window out */
	    float progress;
	    int   distanceX, distanceY;
	    float origDistance, distance;

	    /* If we are finished the animation, draw in the destination
	     * not at the translation speed */
	    if (mAnimateState & FINISHED_ANIMATION)
	    {
		drawnPosX = mDestination.x ();
		drawnPosY = mDestination.y ();
	    }
	    else
	    {
		/* Add new translation points to drawn position */
		drawnPosX = mOrgPos.x () + mTx;
		drawnPosY = mOrgPos.y () + mTy;
	    }

	    /* Determine progress as distance towards the destination */
	    distanceX = drawnPosX - mDestination.x ();
	    distanceY = drawnPosY - mDestination.y ();
	    distance = sqrt (pow (distanceX, 2) + pow (distanceY, 2));

	    distanceX = (mOrgPos.x () - mDestination.x ());
	    distanceY = (mOrgPos.y () - mDestination.y ());
	    origDistance = sqrt (pow (distanceX, 2) + pow (distanceY, 2));

	    /* Avoid div0 */
	    if (!distanceX && !distanceY)
		progress = 1.0f;
	    else
		/* Fading progress is 1 - the distance on the ratio
		 * of current difference to original distance */
		progress = 1.0f - (distance / origDistance);

	    animProgress = progress;

	    /* If we are tabbing the group, invert that (since we are
	     * fading out here) */
	    progress = MAX (progress, 0.0f);
	    if (mGroup->mTabbingState == GroupSelection::Tabbing)
		progress = 1.0f - progress;

	    /* Paint with a progressional opacity */
	    wAttrib.opacity = (float)wAttrib.opacity * progress;
	}

	if (doRotate)
	{
	    /* Determine animation progress for rotation, here,
	     * "2" is the maximum point, so at "1" the window switching
	     * should have reached a half-way point, and we will no
	     * longer paint that window and instead paint the new
	     * incoming window
	     */
	    float timeLeft = mGroup->mTabBar->mChangeAnimationTime;
	    int   animTime = gs->optionGetChangeAnimationTime () * 500;

	    if (mGroup->mTabBar->mChangeState == GroupTabBar::TabChangeOldOut)
		timeLeft += animTime;

	    /* 0 at the beginning, 1 at the end */
	    animProgress = 1 - (timeLeft / (2 * animTime));
	}

	/* Determine resize geometry scale (window stretch on group
	 * resize) */
	if (!mResizeGeometry.isEmpty ())
	{
	    int    xOrigin, yOrigin;
	    float  xScale, yScale;
	    CompRect box;

	    /* Get the scale amount for the resize box */
	    getStretchRectangle (box, xScale, yScale);

	    xOrigin = window->x () - w->border ().left;
	    yOrigin = window->y () - w->border ().top;

	    wTransform.translate (xOrigin, yOrigin, 0.0f);
	    wTransform.scale (xScale, yScale, 1.0f);
	    wTransform.translate ((mResizeGeometry.x () - window->x ()) /
			     	   xScale - xOrigin,
			     	  (mResizeGeometry.y () - window->y ()) /
			     	   yScale - yOrigin,
			     	  0.0f);

	    mask |= PAINT_WINDOW_TRANSFORMED_MASK;
	}
	else if (doRotate || doTabbing)
	{
	    float      animWidth, animHeight;
	    float      animScaleX, animScaleY;
	    CompWindow *morphBase, *morphTarget;

	    /* morphBase and morphTarget here are for both animations,
	     * since during the course of the animation, they scale
	     * the window to the size of the new relevant window
	     * the user will see.
	     *
	     * In the tabbing animation, the windows morph into the
	     * size of the top tab of the window group, or if untabbing
	     * then they morph from the size of the top tab, and morph
	     * to their original size.
	     *
	     * In the rotate animation, the outgoing window morphs into
	     * the same size as the incoming window
	     */
	    if (doTabbing)
	    {
		if (mGroup->mTabbingState == GroupSelection::Tabbing)
		{
		    morphBase   = w;
		    morphTarget = TOP_TAB (mGroup);
		}
		else
		{
		    morphTarget = w;
		    if (HAS_TOP_WIN (mGroup))
			morphBase = TOP_TAB (mGroup);
		    else
			morphBase = mGroup->mTabBar->mLastTopTab;
		}
	    }
	    else /* doRotate */
	    {
		morphBase   = PREV_TOP_TAB (mGroup);
		morphTarget = TOP_TAB (mGroup);
	    }

	    /* Morph progressively based on the animation progress */
	    animWidth = (1 - animProgress) * WIN_REAL_WIDTH (morphBase) +
		        animProgress * WIN_REAL_WIDTH (morphTarget);
	    animHeight = (1 - animProgress) * WIN_REAL_HEIGHT (morphBase) +
		         animProgress * WIN_REAL_HEIGHT (morphTarget);

	    /* Don't allow absurdly small values or div0 */
	    animWidth = MAX (1.0f, animWidth);
	    animHeight = MAX (1.0f, animHeight);
	    animScaleX = animWidth / WIN_REAL_WIDTH (w);
	    animScaleY = animHeight / WIN_REAL_HEIGHT (w);

	    /* If we are rotating, we need to scale on z by 1 / z */
	    if (doRotate)
		wTransform.scale (1.0f, 1.0f, 1.0f / screen->width ());

	    /* Translate to the window center so we can paint windows
	     * translated and rotated */
	    wTransform.translate (WIN_REAL_X (w) + WIN_REAL_WIDTH (w) / 2.0f,
			          WIN_REAL_Y (w) + WIN_REAL_HEIGHT (w) / 2.0f,
			          0.0f);

	    /* Rotate the window based on the animation progress in the
	     * rotating case. If this is the top window, then invert
	     * the rotation start point. If the change direction
	     * is left, then invert the rotating direction */
	    if (doRotate)
	    {
		float rotateAngle = animProgress * 180.0f;
		if (IS_TOP_TAB (w, mGroup))
		    rotateAngle += 180.0f;

		if (mGroup->mTabBar->mChangeAnimationDirection < 0)
		    rotateAngle *= -1.0f;

		perspectiveDistortAndResetZ (wTransform);
		wTransform.rotate (rotateAngle, 0.0f, 1.0f, 0.0f);
	    }

	    /* Draw the window translated depending on position */
	    if (doTabbing)
		wTransform.translate (drawnPosX - WIN_X (w),
				      drawnPosY - WIN_Y (w), 0.0f);

	    /* Since we are still centered, we can scale directly to
	     * our morphing targets */
	    wTransform.scale (animScaleX, animScaleY, 1.0f);

	    /* Recorrect translation matrix for next plugin */
	    wTransform.translate (-(WIN_REAL_X (w) + WIN_REAL_WIDTH (w) / 2.0f),
			          -(WIN_REAL_Y (w) + WIN_REAL_HEIGHT (w) / 2.0f),
			         0.0f);

	    mask |= PAINT_WINDOW_TRANSFORMED_MASK;
	}

	status = gWindow->glPaint (wAttrib, wTransform, region, mask);

	if (showTabbar)
	{
	    /* Paint the tab bar (only gets the geometry it got from
	     * glPaint so far, so it doesn't wobbly or anything strange
	     * like that, though maybe FIXME this should be changed)
	     *
	     * Disable our glPaint function here to avoid recursive
	     * calls within GroupTabBar::paint, since we need to paint
	     * this window geometry a few more times
	     */
	    gWindow->glPaintSetEnabled (this, false);
	    mGroup->mTabBar->paint (wAttrib, wTransform, mask, region);
	    gWindow->glPaintSetEnabled (this, true);
	}
    }
    else
    {
	status = gWindow->glPaint (attrib, transform, region, mask);
    }

    return status;
}
