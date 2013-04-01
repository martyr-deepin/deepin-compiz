/*
 *
 * Compiz bicubic filter plugin
 *
 * bicubic.h
 *
 * Copyright : (C) 2008 by Dennis Kasprzyk
 * E-mail    : onestone@opencompositing.org
 *
 * Ported to Compiz 0.9 by:
 * Copyright : (C) 2008 by Sam Spilsbury
 * E-mail    : smspillaz@gmail.com
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
#include <composite/composite.h>
#include <opengl/opengl.h>

#include "bicubic_options.h"

class BicubicFunction
{
    public:
	int handle;
	int target;
	int param;
	int unit;
};

class BicubicScreen :
    public PluginClassHandler <BicubicScreen, CompScreen>,
    public BicubicOptions
{
    public:
	BicubicScreen (CompScreen *screen);
	~BicubicScreen ();

	GLScreen        *gScreen;
	CompositeScreen *cScreen;

	std::list <BicubicFunction *> func;

	GLenum lTexture;

	int
	getBicubicFragmentFunction (GLTexture   *texture,
				    int         param,
				    int         unit);

	void
	generateLookupTexture (GLenum format);
};

class BicubicWindow :
    public PluginClassHandler <BicubicWindow, CompWindow>,
    public GLWindowInterface
{
    public:
	BicubicWindow (CompWindow *);

	CompositeWindow *cWindow;
	GLWindow	*gWindow;

	void 
	glDrawTexture (GLTexture *texture,
		       GLFragment::Attrib &,
		       unsigned int);
};

#define BICUBIC_SCREEN(s)						       \
     BicubicScreen *bs = BicubicScreen::get (s)

#define BICUBIC_WINDOW(w)						       \
     BicubicWindow *bw = BicubicWindow::get (w)

class BicubicPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <BicubicScreen, BicubicWindow>
{
    public:

	bool init ();
};
