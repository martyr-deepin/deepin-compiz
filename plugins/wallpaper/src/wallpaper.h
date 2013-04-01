/*
 * Compiz wallpaper plugin
 *
 * wallpaper.h
 *
 * Copyright : (C) 2008 by Dennis Kasprzyk
 * E-mail    : onestone@compiz-fusion.org
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
 */

#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/shape.h>

#include <core/core.h>
#include <core/pluginclasshandler.h>
#include <core/atoms.h>

#include "wallpaper_options.h"

#include <composite/composite.h>
#include <opengl/opengl.h>

class WallpaperBackground
{
    public:
	CompString     image;
	int            imagePos;
	int            fillType;
	unsigned short color1[4];
	unsigned short color2[4];

	GLTexture::List imgTex;
	CompSize	imgSize;
	GLTexture::List fillTex;
	GLTexture::MatrixList fillTexMatrix;
};

typedef std::vector<WallpaperBackground> WallpaperBackgrounds;

class WallpaperScreen :
    public PluginClassHandler<WallpaperScreen,CompScreen>,
    public WallpaperOptions,
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface
{
    public:
	
	WallpaperScreen (CompScreen *screen);
	~WallpaperScreen ();

	CompositeScreen *cScreen;
        GLScreen        *gScreen;

	bool		propSet;
	Window		fakeDesktop;
	CompWindow	*desktop;
	int		numBackgrounds;

	CompTimer rotateTimer;
	float fadeTimer, fadeTimeout, fadeDuration, alpha;

	WallpaperBackgrounds backgroundsPrimary, backgroundsSecondary;

	void createFakeDesktopWindow ();
	void destroyFakeDesktopWindow ();

	void updateProperty();
	void blackenSecondary ();
	void updateBackgrounds ();
	void rotateBackgrounds ();
	void updateTimers ();

	bool rotateTimeout ();

	void wallpaperBackgroundsChanged (CompOption *opt,
					  Options    num);

	void wallpaperCycleOptionChanged (CompOption *opt,
					  Options    num);

	void wallpaperToggleCycle	 (CompOption *opt,
					  Options    num);

	WallpaperBackground *getBackgroundForViewport (WallpaperBackgrounds&);

	void
	handleEvent (XEvent *);

	void
	preparePaint (int msSinceLastPaint);

	void
	donePaint ();

	bool
	glPaintOutput (const GLScreenPaintAttrib &sAttrib,
		       const GLMatrix &transform,
		       const CompRegion &region,
		       CompOutput *output,
		       unsigned int mask);

	/* _COMPIZ_WALLPAPER_SUPPORTED atom is used to indicate that
	 * the wallpaper plugin or a plugin providing similar functionality is
	 * active so that desktop managers can respond appropriately */
	Atom compizWallpaperAtom;
};

class WallpaperWindow :
    public PluginClassHandler <WallpaperWindow, CompWindow>,
    public CompositeWindowInterface,
    public GLWindowInterface
{
    public:

	WallpaperWindow (CompWindow *);

	CompWindow      *window;
	CompositeWindow *cWindow;
        GLWindow        *gWindow;

	void
	drawBackgrounds (GLFragment::Attrib &,
			 const CompRegion &, unsigned int,
			 WallpaperBackgrounds&, bool);

	bool glDraw (const GLMatrix &, GLFragment::Attrib &,
		     const CompRegion &, unsigned int);

	bool damageRect (bool, const CompRect &);
};

#define WALLPAPER_SCREEN(s)				\
    WallpaperScreen *ws = WallpaperScreen::get (s);

#define WALLPAPER_WINDOW(w)				\
    WallpaperWindow *ww = WallpaperWindow::get (w);

class WallpaperPluginVTable :
    public CompPlugin::VTableForScreenAndWindow
    <WallpaperScreen, WallpaperWindow>
{
    public:

	bool init ();
};
