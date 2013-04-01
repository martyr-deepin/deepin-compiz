/*
 * Compiz splash plugin
 *
 * splash.h
 *
 * Copyright : (C) 2006 by Dennis Kasprzyk
 * E-mail    : onestone@beryl-project.org
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

#include <core/core.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>

#include <cmath>

#include <X11/Xatom.h>

#include "splash_options.h"

#define MESH_W 16
#define MESH_H 16

extern const std::string SPLASH_BACKGROUND_DEFAULT;
extern const std::string SPLASH_LOGO_DEFAULT;

class SplashScreen :
    public PluginClassHandler <SplashScreen, CompScreen>,
    public SplashOptions,
    public CompositeScreenInterface,
    public GLScreenInterface
{
    public:

	SplashScreen (CompScreen *);

	CompositeScreen *cScreen;
	GLScreen	*gScreen;

	Atom splashAtom;

	int fade_in;
	int fade_out;
	int time;

	GLTexture::List back_img, logo_img;

	CompSize backSize, logoSize;
	bool     hasInit, hasLogo, hasBack;

	float mesh[MESH_W][MESH_H][2];

	float mMove;

	float brightness;
	float saturation;

	bool  initiate;
	bool  active;

	void
	preparePaint (int);

	bool
	glPaintOutput (const GLScreenPaintAttrib &attrib,
		       const GLMatrix &transform,
		       const CompRegion &region,
		       CompOutput *output,
		       unsigned int mask);

	void
	donePaint ();

	bool
	initiateSplash (CompAction *,
		        CompAction::State,
		        CompOption::Vector);

};

#define SPLASH_SCREEN(s)				  		       \
    SplashScreen *ss = SplashScreen::get (s)

class SplashWindow :
    public PluginClassHandler <SplashWindow, CompWindow>,
    public GLWindowInterface
{
    public:

	SplashWindow (CompWindow *);

	CompWindow *window;
	GLWindow   *gWindow;

	bool glPaint (const GLWindowPaintAttrib &,
		      const GLMatrix &,
		      const CompRegion &,
		      unsigned int);
};

#define SPLASH_WINDOW(w)						       \
    SplashWindow *sw = SplashWindow::get (w)

class SplashPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <SplashScreen, SplashWindow>
{
    public:

	bool init ();
};
