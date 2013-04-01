/*
 *
 * Compiz KDE compatibility plugin
 *
 * kdecompat.cpp
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

#include "kdecompat.h"
#include <sstream>

COMPIZ_PLUGIN_20090315 (kdecompat, KDECompatPluginVTable);

inline void
KDECompatScreen::checkPaintFunctions ()
{
    bool enabled = false;

    foreach (CompWindow *w, screen->windows ())
    {
	KDECompatWindow *kcw = KDECompatWindow::get (w);
	bool	wEnabled = (kcw->mPreviews.size () || kcw->mIsPreview ||
			     (kcw->mSlideData &&
			      kcw->mSlideData->remaining > 0.0));
	enabled |= wEnabled;

	kcw->gWindow->glPaintSetEnabled (kcw, wEnabled);
	kcw->cWindow->damageRectSetEnabled (kcw, wEnabled);
    }	
  
    KDECOMPAT_SCREEN (screen);
  
    gScreen->glPaintOutputSetEnabled (ks, enabled);
    cScreen->donePaintSetEnabled (ks, enabled);
    cScreen->preparePaintSetEnabled (ks, enabled);
}

void
KDECompatWindow::stopCloseAnimation ()
{
    while (mUnmapCnt)
    {
	window->unmap ();
	mUnmapCnt--;
    }

    while (mDestroyCnt)
    {
	window->destroy ();
	mDestroyCnt--;
    }
}

void
KDECompatWindow::sendSlideEvent (bool start)
{
    CompOption::Vector o (2);

    o[0] = CompOption ("window", CompOption::TypeInt);
    o[0].value ().set ((int) window->id ());

    o[1] = CompOption ("active", CompOption::TypeBool);
    o[1].value ().set (start);

    screen->handleCompizEvent ("kdecompat", "slide", o);
}

void
KDECompatWindow::startSlideAnimation (bool appearing)
{
    if (!mSlideData)
	return;

    KDECOMPAT_SCREEN (screen);
    
    if (appearing)
	mSlideData->duration = ks->optionGetSlideInDuration ();
    else
	mSlideData->duration = ks->optionGetSlideOutDuration ();

    if (mSlideData->remaining > mSlideData->duration)
	mSlideData->remaining = mSlideData->duration;
    else
	mSlideData->remaining = mSlideData->duration - mSlideData->remaining;

    mSlideData->appearing = appearing;
    ks->mHasSlidingPopups = true;

    ks->checkPaintFunctions ();
    
    cWindow->addDamage ();
    sendSlideEvent (true);
}

void
KDECompatWindow::endSlideAnimation ()
{
    if (mSlideData)
    {
	mSlideData->remaining = 0;
	stopCloseAnimation ();
	sendSlideEvent (false);
    }

   KDECompatScreen::get (screen)->checkPaintFunctions ();
}

void
KDECompatScreen::preparePaint (int msSinceLastPaint)
{
    if (mHasSlidingPopups)
    {
	foreach (CompWindow *w, screen->windows ())
	{
	    KDECOMPAT_WINDOW (w);

	    if (!kw->mSlideData)
		continue;

	    kw->mSlideData->remaining -= msSinceLastPaint;

	    if (kw->mSlideData->remaining <= 0)
		kw->endSlideAnimation ();
	}
    }

    cScreen->preparePaint (msSinceLastPaint);
}

bool
KDECompatScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
				const GLMatrix		  &transform,
				const CompRegion	  &region,
				CompOutput		  *output,
				unsigned int		  mask)
{
    bool status;

    if (mHasSlidingPopups)
	mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK;

    status = gScreen->glPaintOutput (attrib, transform, region, output, mask);

    return status;
}

void
KDECompatScreen::donePaint ()
{
    if (mHasSlidingPopups)
    {
	mHasSlidingPopups = false;

	foreach (CompWindow *w, screen->windows ())
	{
	    KDECOMPAT_WINDOW (w);

	    if (kw->mSlideData && kw->mSlideData->remaining)
	    {
		kw->cWindow->addDamage ();
		mHasSlidingPopups = true;
	    }
	}
    }

    cScreen->donePaint ();
}

bool
KDECompatWindow::glPaint (const GLWindowPaintAttrib &attrib,
			  const GLMatrix	    &transform,
			  const CompRegion	    &region,
			  unsigned int		    mask)
{
    bool status = false;

    KDECOMPAT_SCREEN (screen);

    if ((!(ks->optionGetPlasmaThumbnails () || mPreviews.empty ()) &&
        !(mSlideData || mSlideData->remaining)) ||
	!window->mapNum ()                ||
	(mask & PAINT_WINDOW_OCCLUSION_DETECTION_MASK))
    {
	status = gWindow->glPaint (attrib, transform, region, mask);
	return status;
    }

    if (mSlideData && mSlideData->remaining)
    {
	GLMatrix           wTransform = transform;
	SlideData          *data = mSlideData;
	float              xTranslate = 0, yTranslate = 0, remainder;
	CompRect           clipBox (window->x (), window->y (),
				    window->width (), window->height ());

	if (mask & PAINT_WINDOW_OCCLUSION_DETECTION_MASK)
	    return false;

	remainder = (float) data->remaining / data->duration;
	if (!data->appearing)
	    remainder = 1.0 - remainder;

	switch (data->position) {
	case East:
	    xTranslate = (data->start - window->x ()) * remainder;
	    clipBox.setWidth (data->start - clipBox.x ());
	    break;
	case West:
	    xTranslate = (data->start - window->width ()) * remainder;
	    clipBox.setX (data->start);
	    break;
	case North:
	    yTranslate = (data->start - window->height ()) * remainder;
	    clipBox.setY (data->start);
	    break;
	case South:
	default:
	    yTranslate = (data->start - window->y ()) * remainder;
	    clipBox.setHeight (data->start - clipBox.y1 ());
	    break;
	}

	status = gWindow->glPaint (attrib, transform, region, mask |
				   PAINT_WINDOW_NO_CORE_INSTANCE_MASK);

	if (window->alpha () || attrib.opacity != OPAQUE)
	    mask |= PAINT_WINDOW_TRANSLUCENT_MASK;

	wTransform.translate (xTranslate, yTranslate, 0.0f);

	glEnable (GL_SCISSOR_TEST);

	glScissor (clipBox.x1 (), screen->height () - clipBox.y2 (),
		   clipBox.width (), clipBox.height ());

	status = gWindow->glDraw (wTransform, attrib, region,
			 mask | PAINT_WINDOW_TRANSFORMED_MASK);

	glDisable (GL_SCISSOR_TEST);
    }

    foreach (const Thumb& thumb, mPreviews)
    {
	CompWindow     *tw = screen->findWindow (thumb.id);
	GLWindow       *gtw;
	const CompRect &rect = thumb.thumb;
	unsigned int   paintMask = mask | PAINT_WINDOW_TRANSFORMED_MASK;
	float          xScale = 1.0f, yScale = 1.0f, xTranslate, yTranslate;
	GLTexture      *icon = NULL;

	if (!tw)
	    continue;

	gtw = GLWindow::get (tw);

	xTranslate = rect.x () + window->x () - tw->x ();
	yTranslate = rect.y () + window->y () - tw->y ();

	if (!gtw->textures ().empty ())
	{
	    unsigned int width, height;
	    
	    width = tw->width () - tw->input ().left + tw->input ().right;
	    height = tw->height () - tw->input ().top + tw->input ().bottom;
	    
	    xScale = (float) rect.width () / width;
	    yScale = (float) rect.height () / height;

	    xTranslate += tw->input ().left * xScale;
	    yTranslate += tw->input ().top * yScale;
	}
	else
	{
	    icon = gWindow->getIcon (256, 256);
	    if (!icon)
		icon = ks->gScreen->defaultIcon ();

	    if (icon && !icon->name ())
		icon = NULL;

	    if (icon)
	    {
		GLTexture::MatrixList matrices (1);

		paintMask |= PAINT_WINDOW_BLEND_MASK;

		if (icon->width () >= rect.width () ||
		    icon->height () >= rect.height ())
		{
		    xScale = (float) rect.width () / icon->width ();
		    yScale = (float) rect.height () / icon->height ();

		    if (xScale < yScale)
			yScale = xScale;
		    else
			xScale = yScale;
		}

		xTranslate += rect.width () / 2 -
			      (icon->width () * xScale / 2);
		yTranslate += rect.height () / 2 -
			      (icon->height () * yScale / 2);

		matrices[0] = icon->matrix ();
		matrices[0].x0 -= (tw->x () * icon->matrix ().xx);
		matrices[0].y0 -= (tw->y () * icon->matrix ().yy);

		gtw->vertexBuffer ()->begin ();
		gtw->glAddGeometry (matrices, tw->geometry (), infiniteRegion);
		gtw->vertexBuffer ()->end ();
	    }
	}

	if (!gtw->textures ().empty () || icon)
	{
	    GLMatrix           wTransform (transform);

	    if (tw->alpha () || attrib.opacity != OPAQUE)
		paintMask |= PAINT_WINDOW_TRANSLUCENT_MASK;

	    wTransform.translate (tw->x (), tw->y (), 0.0f);
	    wTransform.scale (xScale, yScale, 1.0f);
	    wTransform.translate (xTranslate / xScale - tw->x (),
				  yTranslate / yScale - tw->y (),
				  0.0f);

	    if (!gtw->textures ().empty ())
		gtw->glDraw (wTransform, attrib,
				 infiniteRegion, paintMask);
	    else if (icon)
		gtw->glDrawTexture (icon, wTransform, attrib, paintMask);
	}
    }
    
    if (!status)
	status = gWindow->glPaint (attrib, transform, region, mask);

    return status;
}

void
KDECompatWindow::updatePreviews ()
{
    Atom	    actual;
    int		    result, format;
    unsigned long   n, left;
    unsigned char   *propData;
    unsigned int    oldPreviewsSize;

    KDECOMPAT_SCREEN (screen);

    oldPreviewsSize = mPreviews.size ();

    mPreviews.clear ();

    result = XGetWindowProperty (screen->dpy (), window->id (),
				 ks->mKdePreviewAtom, 0,
				 32768, false, AnyPropertyType, &actual,
				 &format, &n, &left, &propData);

    if (result == Success && propData)
    {
	if (format == 32 && actual == ks->mKdePreviewAtom)
	{
	    long         *data    = (long *) propData;
	    unsigned int nPreview = *data++;

	    if (n == (6 * nPreview + 1))
	    {
		while (mPreviews.size () < nPreview)
		{
		    Thumb t;

		    if (*data++ != 5)
			break;

		    t.id = *data++;
		    t.thumb.setX (*data++);
		    t.thumb.setY (*data++);
		    t.thumb.setWidth (*data++);
		    t.thumb.setHeight (*data++);

		    mPreviews.push_back (t);
		}
	    }
	}

	XFree (propData);
    }
    
    if (oldPreviewsSize != mPreviews.size ())
	cWindow->damageOutputExtents ();

    foreach (CompWindow *cw, screen->windows ())
    {
	CompWindow      *rw;
	KDECompatWindow *kcw = KDECompatWindow::get (cw);

	kcw->mIsPreview = false;

	foreach (rw, screen->windows ())
	{
	    KDECompatWindow *krw = KDECompatWindow::get (rw);

	    foreach (const Thumb& t, krw->mPreviews)
	    {
		if (t.id == cw->id ())
		{
		    kcw->mIsPreview = true;
		    break;
		}
	    }

	    if (kcw->mIsPreview)
		break;
	}
	
	ks->checkPaintFunctions ();
    }
}

void
KDECompatWindow::updateSlidePosition ()
{
    Atom	  actual;
    int		  result, format;
    unsigned long n, left;
    unsigned char *propData;

    KDECOMPAT_SCREEN (screen);

    if (mSlideData)
    {
	delete mSlideData;
	mSlideData = NULL;
    }

    result = XGetWindowProperty (screen->dpy (), window->id (),
				 ks->mKdeSlideAtom, 0, 32768, false,
				 AnyPropertyType, &actual, &format, &n,
				 &left, &propData);

    if (result == Success && propData)
    {
	if (format == 32 && actual == ks->mKdeSlideAtom && n == 2)
	{
	    long *data = (long *) propData;

	    mSlideData = new SlideData;
	    if (mSlideData)
	    {
		mSlideData->remaining = 0;
		mSlideData->start     = data[0];
		mSlideData->position  = (KDECompatWindow::SlidePosition) data[1];
	    }
	}
	
	window->windowNotifySetEnabled (this, true);

	XFree (propData);
    }
    else
	window->windowNotifySetEnabled (this, false);

    ks->checkPaintFunctions ();
}

void
KDECompatWindow::handleClose (bool destroy)
{
    KDECOMPAT_SCREEN (screen);

    if (mSlideData && ks->optionGetSlidingPopups ())
    {
	if (destroy)
	{
	    mDestroyCnt++;
	    window->incrementDestroyReference ();
	}
	else
	{
	    mUnmapCnt++;
	    window->incrementUnmapReference ();
	}

	if (mSlideData->appearing || !mSlideData->remaining)
	    startSlideAnimation (false);
    }
}

CompAction *
KDECompatScreen::getScaleAction (const char *name)
{
    CompPlugin *p = mScaleHandle;

    if (!p)
	return NULL;

    foreach (CompOption &option, p->vTable->getOptions ())
    {
	if (option.type () == CompOption::TypeAction ||
	    option.type () == CompOption::TypeButton ||
	    option.type () == CompOption::TypeKey)
	{
	    if (option.name () == name)
		return &option.value ().action ();
	}
    }

    return NULL;
}

bool
KDECompatScreen::scaleActivate ()
{
    if (mPresentWindow && !mScaleActive)
    {
	CompOption::Vector options (2);
	CompAction         *action;

	options[0] = CompOption ("root", CompOption::TypeInt);
	options[0].value ().set ((int) screen->root ());

	options[1] = CompOption ("match", CompOption::TypeMatch);
	options[1].value ().set (CompMatch ());

	CompMatch& windowMatch = options[1].value ().match ();

	foreach (Window win, mPresentWindowList)
	{
	    std::ostringstream exp;

	    exp << "xid=" << win;
	    windowMatch |= exp.str ();
	}

	windowMatch.update ();

	action = getScaleAction ("initiate_all_key");
	if (action && action->initiate ())
	    action->initiate () (action, 0, options);
    }

    return false;
}

void
KDECompatWindow::presentGroup ()
{
    Atom          actual;
    int           result, format;
    unsigned long n, left;
    unsigned char *propData;


    KDECOMPAT_SCREEN (screen);

    if (!ks->optionGetPresentWindows ())
	return;

    if (!ks->mScaleHandle)
    {
	compLogMessage ("kdecompat", CompLogLevelWarn,
			"Scale plugin not loaded, present windows "
			"effect not available!");
	return;
    }

    result = XGetWindowProperty (screen->dpy (), window->id (),
				 ks->mKdePresentGroupAtom, 0,
				 32768, false, AnyPropertyType, &actual,
				 &format, &n, &left, &propData);

    if (result == Success && propData)
    {
	if (format == 32 && actual == ks->mKdePresentGroupAtom)
	{
	    long *property = (long *) propData;

	    if (!n || !property[0])
	    {
		CompOption::Vector o (1);
		CompAction *action;

		/* end scale */
		o[0] = CompOption ("root", CompOption::TypeInt);
		o[0].value ().set ((int) screen->root ());

		action = ks->getScaleAction ("initiate_all_key");
		if (action && action->terminate ())
		    action->terminate () (action, CompAction::StateCancel, o);

		ks->mPresentWindow = NULL;
	    }
	    else
	    {
		/* Activate scale using a timeout - Rationale:
		 * At the time we get the property notify event, Plasma
		 * most likely holds a pointer grab due to the action being
		 * initiated by a button click. As scale also wants to get
		 * a pointer grab, we need to delay the activation a bit so
		 * Plasma can release its grab.
		 */

		ks->mPresentWindow = window;
		ks->mPresentWindowList.clear ();

		for (unsigned int i = 0; i < n; i++)
		    ks->mPresentWindowList.push_back (property[i]);

		ks->mScaleTimeout.setCallback (
		    boost::bind (&KDECompatScreen::scaleActivate, ks));
		ks->mScaleTimeout.start ();
	    }
	}

	XFree (propData);
    }
}

void
KDECompatScreen::handleCompizEvent (const char  *pluginName,
				    const char  *eventName,
				    CompOption::Vector &options)
{

    screen->handleCompizEvent (pluginName, eventName, options);

    if (mScaleHandle                      &&
	strcmp (pluginName, "scale") == 0 &&
	strcmp (eventName, "activate") == 0)
    {
	mScaleActive = CompOption::getBoolOptionNamed (options, "active", false);
	if (!mScaleActive && mPresentWindow)
	    XDeleteProperty (screen->dpy (), mPresentWindow->id (),
			     mKdePresentGroupAtom);
    }
}

void
KDECompatWindow::updateBlurProperty (bool enabled)
{
    Atom actual;
    int	 result, format;
    unsigned long n, left;
    unsigned char *propData;
    bool	  validProperty = false;

    KDECOMPAT_SCREEN (screen);

    if (!ks->mBlurLoaded || !ks->optionGetWindowBlur ())
	return;

    if (!enabled)
    {
	if (mBlurPropertySet)
	    XDeleteProperty (screen->dpy (), window->id (),
			     KDECompatScreen::get (screen)->mCompizWindowBlurAtom);
	return;
    }

    
    if (!mBlurPropertySet)
    {
	result = XGetWindowProperty (screen->dpy (), window->id (),
				     ks->mCompizWindowBlurAtom, 0, 32768,
				     false, AnyPropertyType, &actual,
				     &format, &n, &left, &propData);

	if (result == Success && propData)
	{
	    /* somebody else besides us already set a property,
	     * don't touch that property
	     */

	    XFree (propData);
	    return;
	}
    }

    result = XGetWindowProperty (screen->dpy (), window->id (),
				 ks->mKdeBlurBehindRegionAtom, 0, 32768,
				 false, AnyPropertyType, &actual, &format,
				 &n, &left, &propData);

    if (result == Success && propData)
    {
	if (format == 32 && actual == XA_CARDINAL &&
	    n > 0 && (n % 4 == 0))
	{
	    long	 *data = (long *) propData;
	    unsigned int nBox = n / 4;
	    long	 compizProp[nBox * 6 + 2];
	    unsigned int i = 2;

	    compizProp[0] = 2; /* threshold */
	    compizProp[1] = 0; /* filter */

	    while (nBox--)
	    {
		int x, y, w, h;
		x = *data++;
		y = *data++;
		w = *data++;
		h = *data++;

		compizProp[i++] = GRAVITY_NORTH | GRAVITY_WEST; /* P1 gravity */
		compizProp[i++] = x;                            /* P1 X */
		compizProp[i++] = y;                            /* P1 Y */
		compizProp[i++] = GRAVITY_NORTH | GRAVITY_WEST; /* P2 gravity */
		compizProp[i++] = x + w;                        /* P2 X */
		compizProp[i++] = y + h;                        /* P2 Y */
	    }

	    XChangeProperty (screen->dpy (), window->id (), ks->mCompizWindowBlurAtom,
			     XA_INTEGER, 32, PropModeReplace,
			     (unsigned char *) compizProp, i);

	    mBlurPropertySet = true;
	    validProperty       = TRUE;
	}

	XFree (propData);
    }

    if (mBlurPropertySet && !validProperty)
    {
	mBlurPropertySet = FALSE;
	XDeleteProperty (screen->dpy (), window->id (), ks->mKdeBlurBehindRegionAtom);
    }
}

void
KDECompatWindow::windowNotify (CompWindowNotify n)
{
    if (!KDECompatScreen::get (screen)->optionGetSlidingPopups ())
	return window->windowNotify (n);

    switch (n)
    {
	case CompWindowNotifyClose:
	    handleClose (false);
	    break;
	case CompWindowNotifyBeforeDestroy:
	    handleClose (true);
	    break;
	case CompWindowNotifyBeforeMap:
	    startSlideAnimation (true);
	    break;
	default:
	    break;
    }
}


void
KDECompatScreen::handleEvent (XEvent *event)
{
    CompWindow *w;

    screen->handleEvent (event);

    switch (event->type) {
    case PropertyNotify:
	if (event->xproperty.atom == mKdePreviewAtom)
	{
	    w = screen->findWindow (event->xproperty.window);
	    if (w)
		KDECompatWindow::get (w)->updatePreviews ();
	}
	else if (event->xproperty.atom == mKdeSlideAtom)
	{
	    w = screen->findWindow (event->xproperty.window);
	    if (w)
		KDECompatWindow::get (w)->updateSlidePosition ();
	}
	else if (event->xproperty.atom == mKdePresentGroupAtom)
	{
	    w = screen->findWindow (event->xproperty.window);
	    if (w)
		KDECompatWindow::get (w)->presentGroup ();
	}
	else if (event->xproperty.atom == mKdeBlurBehindRegionAtom)
	{
	    w = screen->findWindow (event->xproperty.window);
	    if (w)
		KDECompatWindow::get (w)->updateBlurProperty (true);
	}
	break;
    }
}

bool
KDECompatWindow::damageRect (bool           initial,
			     const CompRect &rect)
{
    bool       status;

    KDECOMPAT_SCREEN (screen);

    if (mIsPreview && ks->optionGetPlasmaThumbnails ())
    {
	foreach (CompWindow *cw, screen->windows ())
	{
	    KDECompatWindow *kdw = KDECompatWindow::get (cw);

	    foreach (const Thumb& thumb, kdw->mPreviews)
	    {
	        if (thumb.id != window->id ())
		    continue;

		CompRect rect (thumb.thumb.x () + cw->x (),
			       thumb.thumb.y () + cw->y (),
			       thumb.thumb.width (),
			       thumb.thumb.height ());

		ks->cScreen->damageRegion (rect);
	    }
        }
    }

    status = cWindow->damageRect (initial, rect);

    return status;
}

void
KDECompatScreen::advertiseSupport (Atom atom,
				   bool enable)
{
    if (enable)
    {
	unsigned char value = 0;

	XChangeProperty (screen->dpy (), screen->root (), atom,
			 mKdePreviewAtom, 8, PropModeReplace, &value, 1);
    }
    else
    {
	XDeleteProperty (screen->dpy (), screen->root (), atom);
    }
}

void
KDECompatScreen::optionChanged (CompOption                *option,
				KdecompatOptions::Options num)
{
    if (num == KdecompatOptions::PlasmaThumbnails)
	advertiseSupport (mKdePreviewAtom, option->value ().b ());
    else if (num == KdecompatOptions::SlidingPopups)
	advertiseSupport (mKdeSlideAtom, option->value (). b ());
    else if (num == KdecompatOptions::PresentWindows)
	advertiseSupport (mKdePresentGroupAtom,
			  option->value ().b () && mScaleHandle);
    else if (num == KdecompatOptions::WindowBlur)
    {
	advertiseSupport (mKdeBlurBehindRegionAtom,
			  option->value ().b () && mBlurLoaded);
	foreach (CompWindow *w, screen->windows ())
	    KDECompatWindow::get (w)->updateBlurProperty (option->value ().b ());
    }
}


KDECompatScreen::KDECompatScreen (CompScreen *screen) :
    PluginClassHandler<KDECompatScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    mKdePreviewAtom (XInternAtom (screen->dpy (), "_KDE_WINDOW_PREVIEW", 0)),
    mKdeSlideAtom (XInternAtom (screen->dpy (), "_KDE_SLIDE", 0)),
    mKdePresentGroupAtom (XInternAtom (screen->dpy (),
			  "_KDE_PRESENT_WINDOWS_GROUP", 0)),
    mKdeBlurBehindRegionAtom (XInternAtom (screen->dpy (),
					   "_KDE_NET_WM_BLUR_BEHIND_REGION",
					   0)),
    mCompizWindowBlurAtom (XInternAtom (screen->dpy (),
					      "_COMPIZ_WM_WINDOW_BLUR", 0)),
    mHasSlidingPopups (false),
    mDestroyCnt (0),
    mUnmapCnt (0),
    mScaleHandle (CompPlugin::find ("scale")),
    mScaleActive (false),
    mBlurLoaded ((CompPlugin::find ("blur") != NULL)),
    mPresentWindow (NULL)
{
    ScreenInterface::setHandler (screen);
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);

    mScaleTimeout.setTimes (100, 200);

    advertiseSupport (mKdePreviewAtom, optionGetPlasmaThumbnails ());
    advertiseSupport (mKdeSlideAtom, optionGetSlidingPopups ());
    advertiseSupport (mKdePresentGroupAtom, optionGetPresentWindows () &&
					    mScaleHandle);
    optionSetPlasmaThumbnailsNotify (
	boost::bind (&KDECompatScreen::optionChanged, this, _1, _2));
}

KDECompatScreen::~KDECompatScreen ()
{
    advertiseSupport (mKdePreviewAtom, false);
    advertiseSupport (mKdeSlideAtom, false);
    advertiseSupport (mKdePresentGroupAtom, false);
}

KDECompatWindow::KDECompatWindow (CompWindow *window) :
    PluginClassHandler <KDECompatWindow, CompWindow> (window),
    window (window),
    cWindow (CompositeWindow::get (window)),
    gWindow (GLWindow::get (window)),
    mIsPreview (false),
    mSlideData (NULL),
    mDestroyCnt (0),
    mUnmapCnt (0),
    mBlurPropertySet (false)
{
    WindowInterface::setHandler (window, false);
    CompositeWindowInterface::setHandler (cWindow, false);
    GLWindowInterface::setHandler (gWindow, false);
    
    updateBlurProperty (KDECompatScreen::get (screen)->optionGetWindowBlur ());
}

KDECompatWindow::~KDECompatWindow ()
{
    stopCloseAnimation ();

    if (mSlideData)
	delete mSlideData;

    if (KDECompatScreen::get (screen)->mPresentWindow == window)
	KDECompatScreen::get (screen)->mPresentWindow = NULL;

    updateBlurProperty (false);
}

bool
KDECompatPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
        return false;

    return true;
}
