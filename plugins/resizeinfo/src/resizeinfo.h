/**
 *
 * Compiz metacity like info during resize
 *
 * resizeinfo.c
 *
 * Copyright (c) 2007 Robert Carr <racarr@opencompositing.org>
 *
 * Compiz resize atom usage and general cleanups by
 * Copyright (c) 2007 Danny Baumann <maniac@opencompositing.org>
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

#include <core/core.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>

#include <cairo-xlib-xrender.h>
#include <math.h>
#include <pango/pango.h>
#include <pango/pangocairo.h>

#include "resizeinfo_options.h"

extern const unsigned short RESIZE_POPUP_WIDTH;
extern const unsigned short RESIZE_POPUP_HEIGHT;

extern const double PI;

/* Cairo helper class */

class InfoLayer
{
    public:

	InfoLayer ();
	~InfoLayer ();

	bool valid;

	Screen		  *s;
	XRenderPictFormat *format;
	Pixmap            pixmap;
	cairo_surface_t   *surface;
	GLTexture::List   texture;
	cairo_t           *cr;

	void draw (const GLMatrix &transform,
	           int             x,
	 	   int y);

	void renderBackground ();
	void renderText ();
};

class InfoScreen :
    public PluginClassHandler <InfoScreen, CompScreen>,
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public ResizeinfoOptions
{
    public:

	InfoScreen (CompScreen *);

	GLScreen 	*gScreen;
	CompositeScreen *cScreen;

	Atom resizeInfoAtom;

	CompWindow *pWindow;

	bool drawing;
	int  fadeTime;

	InfoLayer backgroundLayer;
	InfoLayer textLayer;

	XRectangle resizeGeometry;

	void
	damagePaintRegion ();

	void
	preparePaint (int);

	bool
	glPaintOutput (const GLScreenPaintAttrib &,
		       const GLMatrix &,
		       const CompRegion &,
		       CompOutput *,
		       unsigned int);

	void
	donePaint ();

	void
	handleEvent (XEvent *event);
};

class InfoWindow :
    public PluginClassHandler <InfoWindow, CompWindow>,
    public WindowInterface
{
    public:

	InfoWindow (CompWindow *);

	CompWindow *window;

	void
	grabNotify (int,
	      	    int,
	      	    unsigned int,
	      	    unsigned int);

	void
	ungrabNotify ();
};

#define INFO_SCREEN(s)							       \
    InfoScreen *is = InfoScreen::get (s);

#define INFO_WINDOW(w)							       \
    InfoWindow *iw = InfoWindow::get (w);

class InfoPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <InfoScreen, InfoWindow>
{
    public:

	bool init ();
};
