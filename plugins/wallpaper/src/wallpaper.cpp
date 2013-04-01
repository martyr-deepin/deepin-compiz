/*
 * Compiz wallpaper plugin
 *
 * wallpaper.cpp
 *
 * Copyright (c) 2008 Dennis Kasprzyk <onestone@opencompositing.org>
 *
 * Rewrite of wallpaper.c
 * Copyright (c) 2007 Robert Carr <racarr@opencompositing.org>
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

#include "wallpaper.h"

COMPIZ_PLUGIN_20090315 (wallpaper, WallpaperPluginVTable);

static Visual *
findArgbVisual (Display *dpy,
		int     screen)
{
    XVisualInfo		*xvi;
    XVisualInfo		temp;
    int			nvi;
    int			i;
    XRenderPictFormat	*format;
    Visual		*visual;

    temp.screen  = screen;
    temp.depth   = 32;
    temp.c_class = TrueColor;

    xvi = XGetVisualInfo (dpy,
			  VisualScreenMask |
			  VisualDepthMask  |
			  VisualClassMask,
			  &temp,
			  &nvi);
    if (!xvi)
	return 0;

    visual = 0;
    for (i = 0; i < nvi; i++)
    {
	format = XRenderFindVisualFormat (dpy, xvi[i].visual);
	if (format->type == PictTypeDirect && format->direct.alphaMask)
	{
	    visual = xvi[i].visual;
	    break;
	}
    }

    XFree (xvi);

    return visual;
}

void
WallpaperScreen::createFakeDesktopWindow ()
{
    Display              *dpy = screen->dpy ();
    XSizeHints           xsh;
    XWMHints             xwmh;
    XSetWindowAttributes attr;
    Visual               *visual;
    XserverRegion        region;

    visual = findArgbVisual (dpy, screen->screenNum ());
    if (!visual)
	return;

    xsh.flags       = PSize | PPosition | PWinGravity;
    xsh.width       = 1;
    xsh.height      = 1;
    xsh.win_gravity = StaticGravity;

    xwmh.flags = InputHint;
    xwmh.input = 0;

    attr.background_pixel = 0;
    attr.border_pixel     = 0;
    attr.colormap	  = XCreateColormap (dpy, screen->root (),
    					     visual, AllocNone);

    fakeDesktop = XCreateWindow (dpy, screen->root (), -1, -1, 1, 1, 0, 32,
				     InputOutput, visual,
				     CWBackPixel | CWBorderPixel | CWColormap,
				     &attr);

    XSetWMProperties (dpy, fakeDesktop, NULL, NULL,
		      programArgv, programArgc, &xsh, &xwmh, NULL);

    XChangeProperty (dpy, fakeDesktop, Atoms::winStateSkipPager,
		     XA_ATOM, 32, PropModeReplace,
		     (unsigned char *) &Atoms::winStateSkipPager, 1);

    XChangeProperty (dpy, fakeDesktop, Atoms::winType,
		     XA_ATOM, 32, PropModeReplace,
		     (unsigned char *) &Atoms::winTypeDesktop, 1);

    region = XFixesCreateRegion (dpy, NULL, 0);

    XFixesSetWindowShapeRegion (dpy, fakeDesktop, ShapeInput, 0, 0, region);

    XFixesDestroyRegion (dpy, region);

    XMapWindow (dpy, fakeDesktop);
    XLowerWindow (dpy, fakeDesktop);
}

void
WallpaperScreen::destroyFakeDesktopWindow ()
{
    if (fakeDesktop != None)
	XDestroyWindow (screen->dpy (), fakeDesktop);

    fakeDesktop = None;
}

void
WallpaperScreen::updateProperty ()
{
    if (backgroundsPrimary.empty())
    {
	if (propSet)
	    XDeleteProperty (screen->dpy (), screen->root (),
			     compizWallpaperAtom);
	propSet = false;
    }
    else if (!propSet)
    {
	unsigned char sd = 1;

	XChangeProperty (screen->dpy (), screen->root (),
			 compizWallpaperAtom, XA_CARDINAL,
			 8, PropModeReplace, &sd, 1);
	propSet = true;
    }
}

static void
initBackground (WallpaperBackground *back)
{
    unsigned int        c[2];
    unsigned short      *color;

    if (!back->image.empty ())
    {
	CompString pname ("wallpaper");
	back->imgTex = GLTexture::readImageToTexture (back->image, pname,
						      back->imgSize);
	if (back->imgTex.empty ())
	{
	    compLogMessage ("wallpaper", CompLogLevelWarn,
			    "Failed to load image: %s", back->image.c_str ());

	    back->imgSize.setWidth (0);
	    back->imgSize.setHeight (0);
	}
    }

    color = back->color1;
    c[0] = ((color[3] << 16) & 0xff000000) |
	    ((color[0] * color[3] >> 8) & 0xff0000) |
	    ((color[1] * color[3] >> 16) & 0xff00) |
	    ((color[2] * color[3] >> 24) & 0xff);

    color = back->color2;
    c[1] = ((color[3] << 16) & 0xff000000) |
	    ((color[0] * color[3] >> 8) & 0xff0000) |
	    ((color[1] * color[3] >> 16) & 0xff00) |
	    ((color[2] * color[3] >> 24) & 0xff);

    if (back->fillType == WallpaperOptions::BgFillTypeVerticalGradient)
    {
	back->fillTex = GLTexture::imageBufferToTexture ((char *) &c, CompSize (1, 2));
	back->fillTexMatrix.push_back (back->fillTex[0]->matrix());
	back->fillTexMatrix[0].xx = 0.0;
    }
    else if (back->fillType == WallpaperOptions::BgFillTypeHorizontalGradient)
    {
	back->fillTex = GLTexture::imageBufferToTexture ((char *) &c, CompSize (2, 1));
	back->fillTexMatrix.push_back (back->fillTex[0]->matrix());
	back->fillTexMatrix[0].yy = 0.0;
    }
    else
    {
	back->fillTex = GLTexture::imageBufferToTexture ((char *) &c, CompSize (1, 1));
	back->fillTexMatrix.push_back (back->fillTex[0]->matrix());
	back->fillTexMatrix[0].xx = 0.0;
	back->fillTexMatrix[0].yy = 0.0;
    }
}

void
WallpaperScreen::blackenSecondary ()
{
    unsigned short black [] = {1, 0, 0, 0};

    backgroundsSecondary.clear ();

    for (int i = 0; i < numBackgrounds; i++)
    {
	backgroundsSecondary.push_back (WallpaperBackground ());

	backgroundsSecondary[i].image = "";
	backgroundsSecondary[i].imagePos = 0;
	backgroundsSecondary[i].fillType = 0;
	memcpy (backgroundsSecondary[i].color1, black,
		4 * sizeof(unsigned short));
	memcpy (backgroundsSecondary[i].color2, black,
		4 * sizeof(unsigned short));

	initBackground (&backgroundsSecondary[i]);
    }
}

void
WallpaperScreen::updateBackgrounds ()
{
#define GET_OPTION(opt) CompOption::Value::Vector c##opt = optionGet##opt ();
    GET_OPTION (BgImage);
    GET_OPTION (BgImagePos);
    GET_OPTION (BgFillType);
    GET_OPTION (BgColor1);
    GET_OPTION (BgColor2);
#undef GET_OPTION

    if (!((cBgImagePos.size ()  == cBgImage.size ()) &&
	  (cBgFillType.size ()  == cBgImage.size ()) &&
	  (cBgColor1.size ()    == cBgImage.size ()) &&
	  (cBgColor2.size ()    == cBgImage.size ())))
    {
	compLogMessage ("wallpaper", CompLogLevelWarn, "Malformed option");
	return;
    }

    numBackgrounds = cBgImage.size ();

    backgroundsPrimary.clear ();

    for (unsigned int i = 0; i < cBgImage.size (); i++)
    {
     	backgroundsPrimary.push_back (WallpaperBackground ());

	backgroundsPrimary[i].image    = cBgImage[i].s ();
	backgroundsPrimary[i].imagePos = cBgImagePos[i].i ();
	backgroundsPrimary[i].fillType = cBgFillType[i].i ();
	memcpy (backgroundsPrimary[i].color1, cBgColor1[i].c (),
		4 * sizeof(unsigned short));
	memcpy (backgroundsPrimary[i].color2, cBgColor2[i].c (),
		4 * sizeof(unsigned short));

	initBackground (&backgroundsPrimary[i]);
    }

    blackenSecondary ();

    fadeDuration = optionGetCycleTimeout ();
    fadeTimer = optionGetFadeDuration ();
}

void
WallpaperScreen::rotateBackgrounds ()
{
    if (numBackgrounds)
    {
	WallpaperBackground item = backgroundsPrimary.front();

	backgroundsSecondary = backgroundsPrimary;
	backgroundsPrimary.erase (backgroundsPrimary.begin ());
	backgroundsPrimary.push_back (item);
    }

    fadeTimer = fadeDuration;
}

void
WallpaperScreen::updateTimers ()
{
    fadeTimeout = (optionGetCycleTimeout () * 1000 * 60);
    fadeDuration = (optionGetFadeDuration () * 1000);
    fadeTimer = fadeDuration;
    if (optionGetCycleWallpapers ())
	rotateTimer.start (fadeTimeout, fadeTimeout * 1.2);
    else
	rotateTimer.stop ();
}

bool
WallpaperScreen::rotateTimeout ()
{
    rotateBackgrounds ();
    updateProperty ();

    cScreen->preparePaintSetEnabled (this, true);
    cScreen->donePaintSetEnabled (this, true);

    cScreen->damageScreen ();

    return true;
}

/* Installed as a handler for the images setting changing through bcop */
void
WallpaperScreen::wallpaperBackgroundsChanged (CompOption *o,
			                      Options    num)
{
    updateBackgrounds ();
    updateProperty ();
    updateTimers ();

    cScreen->damageScreen ();
}

void
WallpaperScreen::wallpaperCycleOptionChanged (CompOption *o,
			                      Options    num)
{
    blackenSecondary ();
    updateTimers ();
}

void
WallpaperScreen::wallpaperToggleCycle (CompOption *o,
				       Options    num)
{
    if (optionGetCycleWallpapers ())
	rotateTimer.start (fadeTimeout, fadeTimeout * 1.2);
    else
	rotateTimer.stop ();
}

WallpaperBackground *
WallpaperScreen::getBackgroundForViewport (WallpaperBackgrounds &bg)
{
    CompPoint offset = cScreen->windowPaintOffset ();
    CompPoint vp = screen->vp ();
    CompSize  vpSize = screen->vpSize ();
    CompRect  workarea = screen->workArea ();
    int x, y;

    if (bg.empty())
	return NULL;

    x = vp.x () - (offset.x () / (int) workarea.width ());
    x %= vpSize.width ();
    if (x < 0)
	x += vpSize.width ();

    y = vp.y () - (offset.y () / (int) workarea.height ());
    y %= vpSize.height ();
    if (y < 0)
	y += vpSize.height ();

    return &bg[(x + (y * vpSize.width ())) % bg.size()];
}

void
WallpaperScreen::handleEvent (XEvent *event)
{
    screen->handleEvent (event);

    if (!screen->desktopWindowCount () && fakeDesktop == None && !backgroundsPrimary.empty())
	createFakeDesktopWindow ();

    if ((screen->desktopWindowCount () > 1 || backgroundsPrimary.empty())
	&& fakeDesktop != None)
	destroyFakeDesktopWindow ();
}

void
WallpaperScreen::preparePaint (int msSinceLastPaint)
{
    fadeTimer -= msSinceLastPaint;
    if (fadeTimer < 0)
	fadeTimer = 0;
    alpha = (fadeDuration - fadeTimer) / fadeDuration;

    cScreen->preparePaint (msSinceLastPaint);
}

void
WallpaperScreen::donePaint ()
{
    if (fadeTimer > 0)
	cScreen->damageScreen ();
    else
    {
	cScreen->preparePaintSetEnabled (this, false);
	cScreen->donePaintSetEnabled (this, false);
    }

    cScreen->donePaint ();
}

bool
WallpaperScreen::glPaintOutput (const GLScreenPaintAttrib &sAttrib,
				const GLMatrix &transform,
				const CompRegion &region,
				CompOutput *output,
				unsigned int mask)
{
    desktop = NULL;

    return gScreen->glPaintOutput (sAttrib, transform, region, output, mask);
}

void
WallpaperWindow::drawBackgrounds (GLFragment::Attrib &attrib,
				  const CompRegion &region,
				  unsigned int mask,
				  WallpaperBackgrounds& bg,
				  bool fadingIn)
{
    WALLPAPER_SCREEN (screen);

    CompRect            tmpRect;
    GLTexture::Matrix   matrix;
    GLTexture::MatrixList tmpMatrixList;
    WallpaperBackground *back = ws->getBackgroundForViewport (bg);
	GLFragment::Attrib tmpAttrib = attrib;

    tmpMatrixList.push_back (matrix);

    gWindow->geometry().reset();

    tmpMatrixList[0] = back->fillTexMatrix[0];

    if (back->fillType == WallpaperOptions::BgFillTypeVerticalGradient)
    {
	tmpMatrixList[0].yy /= (float) screen->height () / 2.0;
    }
    else if (back->fillType == WallpaperOptions::BgFillTypeHorizontalGradient)
    {
	tmpMatrixList[0].xx /= (float) screen->width () / 2.0;
    }

    gWindow->glAddGeometry (tmpMatrixList, screen->region (),
			    (mask & PAINT_WINDOW_TRANSFORMED_MASK) ?
			    infiniteRegion : region);

    if (ws->optionGetCycleWallpapers ())
    {
	if (fadingIn)
	    tmpAttrib.setOpacity ((OPAQUE * (1.0f - ws->alpha)) * (attrib.getOpacity () / (float)OPAQUE));
	else
	    tmpAttrib.setOpacity ((OPAQUE * ws->alpha) * (attrib.getOpacity () / (float)OPAQUE));
    }

    if (tmpAttrib.getOpacity () != OPAQUE)
	mask |= PAINT_WINDOW_BLEND_MASK;

    if (gWindow->geometry ().vCount)
	gWindow->glDrawTexture(back->fillTex[0], tmpAttrib, mask);

    if (back->imgSize.width () && back->imgSize.height ())
    {
	CompRegion reg = screen->region ();
	float  s1, s2;
	int    x, y;

	gWindow->geometry ().vCount = gWindow->geometry ().indexCount = 0;
	tmpMatrixList[0] = back->imgTex[0]->matrix ();

	if (back->imagePos == WallpaperOptions::BgImagePosScaleAndCrop)
	{
	    s1 = (float) screen->width () / back->imgSize.width ();
	    s2 = (float) screen->height () / back->imgSize.height ();

	    s1 = MAX (s1, s2);

	    tmpMatrixList[0].xx /= s1;
	    tmpMatrixList[0].yy /= s1;

	    x = (screen->width () - ((int)back->imgSize.width () * s1)) / 2.0;
	    tmpMatrixList[0].x0 -= x * tmpMatrixList[0].xx;
	    y = (screen->height () - ((int)back->imgSize.height () * s1)) / 2.0;
	    tmpMatrixList[0].y0 -= y * tmpMatrixList[0].yy;
	}
	else if (back->imagePos == WallpaperOptions::BgImagePosScaled)
	{
	    s1 = (float) screen->width () / back->imgSize.width ();
	    s2 = (float) screen->height () / back->imgSize.height ();
	    tmpMatrixList[0].xx /= s1;
	    tmpMatrixList[0].yy /= s2;
	}
	else if (back->imagePos == WallpaperOptions::BgImagePosCentered)
	{
	    x = (screen->width () - (int)back->imgSize.width ()) / 2;
	    y = (screen->height () - (int)back->imgSize.height ()) / 2;
	    tmpMatrixList[0].x0 -= x * tmpMatrixList[0].xx;
	    tmpMatrixList[0].y0 -= y * tmpMatrixList[0].yy;

	    tmpRect.setLeft (MAX (0, x));
	    tmpRect.setTop (MAX (0, y));
	    tmpRect.setRight (MIN (screen->width (), x + back->imgSize.width ()));
	    tmpRect.setBottom (MIN (screen->height (), y + back->imgSize.height ()));

	    reg = CompRegion (tmpRect);
	}

	if (back->imagePos == WallpaperOptions::BgImagePosTiled ||
	    back->imagePos == WallpaperOptions::BgImagePosCenterTiled)
	{
	    if (back->imagePos == WallpaperOptions::BgImagePosCenterTiled)
	    {
		x = (screen->width () - (int)back->imgSize.width ()) / 2;
		y = (screen->height () - (int)back->imgSize.height ()) / 2;

		if (x > 0)
		    x = (x % (int)back->imgSize.width ()) - (int)back->imgSize.width ();
		if (y > 0)
		    y = (y % (int)back->imgSize.height ()) - (int)back->imgSize.height ();
	    }
	    else
	    {
		x = 0;
		y = 0;
	    }

	    while (y < (int) screen->height ())
	    {
		int xi = x;
		while (xi < (int) screen->width ())
		{
		    tmpMatrixList[0] = back->imgTex[0]->matrix ();

		    tmpMatrixList[0].x0 -= xi * tmpMatrixList[0].xx;
		    tmpMatrixList[0].y0 -= y * tmpMatrixList[0].yy;

		    tmpRect.setLeft (MAX (0, xi));
		    tmpRect.setTop (MAX (0, y));
		    tmpRect.setRight (MIN (screen->width (), xi + back->imgSize.width ()));
		    tmpRect.setBottom (MIN (screen->height (),
						y + back->imgSize.height ()));

		    reg = CompRegion (tmpRect);

		    gWindow->glAddGeometry (tmpMatrixList, reg, region);

		    xi += (int)back->imgSize.width ();
		}
		y += (int)back->imgSize.height ();
	    }
	}
	else
	{
	    gWindow->glAddGeometry (tmpMatrixList, reg, region);
	}

	if (gWindow->geometry ().vCount)
	    gWindow->glDrawTexture (back->imgTex[0], tmpAttrib, mask | PAINT_WINDOW_BLEND_MASK);
    }
}

bool
WallpaperWindow::glDraw (const GLMatrix &transform,
			 GLFragment::Attrib &attrib,
			 const CompRegion &region,
			 unsigned int mask)
{
    WALLPAPER_SCREEN (screen);

    if ((!ws->desktop || ws->desktop == window) && !ws->backgroundsPrimary.empty() &&
	window->alpha () && window->type () & CompWindowTypeDesktopMask)
    {
	int filterIdx;
	GLTexture::Filter   saveFilter;

	if (mask & PAINT_WINDOW_ON_TRANSFORMED_SCREEN_MASK)
	    filterIdx = SCREEN_TRANS_FILTER;
	else if (mask & PAINT_WINDOW_TRANSFORMED_MASK)
	    filterIdx = WINDOW_TRANS_FILTER;
	else
	    filterIdx = NOTHING_TRANS_FILTER;

	saveFilter = ws->gScreen->filter (filterIdx);
	ws->gScreen->setFilter (filterIdx, GLTexture::Good);

	if (ws->optionGetCycleWallpapers () && ws->rotateTimer.active ())
	    drawBackgrounds (attrib, region, mask,
			     ws->backgroundsSecondary, true);
	drawBackgrounds (attrib, region, mask,
			 ws->backgroundsPrimary, false);

	ws->gScreen->setFilter (filterIdx, saveFilter);

	ws->desktop = window;
	attrib.setOpacity (OPAQUE);
    }

    return gWindow->glDraw (transform, attrib, region, mask);
}

bool
WallpaperWindow::damageRect (bool            initial,
			     const CompRect& rect)
{
    WALLPAPER_SCREEN (screen);

    if (window->id () == ws->fakeDesktop){
	ws->cScreen->damageScreen ();}

    return cWindow->damageRect (initial, rect);
}


WallpaperScreen::WallpaperScreen (CompScreen *screen) :
    PluginClassHandler<WallpaperScreen,CompScreen> (screen),
    WallpaperOptions (),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    backgroundsPrimary (),
    backgroundsSecondary ()
{
    ScreenInterface::setHandler (screen, true);
    CompositeScreenInterface::setHandler (cScreen, true);
    GLScreenInterface::setHandler (gScreen, true);

    compizWallpaperAtom = XInternAtom (screen->dpy (),
				       "_COMPIZ_WALLPAPER_SUPPORTED", 0);

    propSet = false;
    fakeDesktop = None;
    desktop = NULL;
    fadeTimer = 0.0f;
    fadeTimeout = 0.0f;
    fadeDuration = 0.0f;
    alpha = 0.0f;

    optionSetBgImageNotify    (boost::bind (&WallpaperScreen::
				wallpaperBackgroundsChanged, this, _1, _2));
    optionSetBgImagePosNotify (boost::bind (&WallpaperScreen::
				wallpaperBackgroundsChanged, this, _1, _2));
    optionSetBgFillTypeNotify (boost::bind (&WallpaperScreen::
				wallpaperBackgroundsChanged, this, _1, _2));
    optionSetBgColor1Notify   (boost::bind (&WallpaperScreen::
				wallpaperBackgroundsChanged, this, _1, _2));
    optionSetBgColor2Notify   (boost::bind (&WallpaperScreen::
				wallpaperBackgroundsChanged, this, _1, _2));

    optionSetCycleWallpapersNotify	(boost::bind (&WallpaperScreen::
				    wallpaperToggleCycle, this, _1, _2));
    optionSetCycleTimeoutNotify		(boost::bind (&WallpaperScreen::
				    wallpaperCycleOptionChanged, this, _1, _2));
    optionSetFadeDurationNotify		(boost::bind (&WallpaperScreen::
				    wallpaperCycleOptionChanged, this, _1, _2));

    rotateTimer.setCallback (boost::bind (&WallpaperScreen::rotateTimeout, this));

    updateBackgrounds ();
    updateProperty ();
    cScreen->damageScreen ();

    if (!screen->desktopWindowCount () && backgroundsPrimary.size())
	createFakeDesktopWindow ();
}

WallpaperScreen::~WallpaperScreen ()
{
    if (propSet)
	XDeleteProperty (screen->dpy (), screen->root (), compizWallpaperAtom);

    if (fakeDesktop != None)
	destroyFakeDesktopWindow ();
}

WallpaperWindow::WallpaperWindow (CompWindow *window) :
    PluginClassHandler <WallpaperWindow, CompWindow> (window),
    window (window),
    cWindow (CompositeWindow::get (window)),
    gWindow (GLWindow::get (window))
{
    CompositeWindowInterface::setHandler (cWindow, true);
    GLWindowInterface::setHandler (gWindow, true);
}

bool
WallpaperPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
    {
	 return false;
    }

    return true;
}
