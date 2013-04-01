/*
 * Compiz cube gears plugin
 *
 * gears.h
 *
 * This is an example plugin to show how to render something inside
 * of the transparent cube
 *
 * Copyright : (C) 2007 by Dennis Kasprzyk
 * E-mail    : onestone@opencompositing.org
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
 * Based on glxgears.c:
 *    http://cvsweb.xfree86.org/cvsweb/xc/programs/glxgears/glxgears.c
 */

#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <signal.h>
#include <unistd.h>
#include <cmath>

#include <core/core.h>
#include <core/pluginclasshandler.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <cube/cube.h>
#include "gears_options.h"

class GearsScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public CubeScreenInterface,
    public PluginClassHandler <GearsScreen, CompScreen>,
    public GearsOptions
{
    public:

	GearsScreen (CompScreen *s);

	~GearsScreen ();

	CompScreen      *screen;
	CompositeScreen *cScreen;
        GLScreen        *gScreen;
	CubeScreen      *csScreen;

	void 
	cubeClearTargetOutput (float      xRotate,
			       float      vRotate);
	void 
	cubePaintInside (const GLScreenPaintAttrib &sAttrib,
			 const GLMatrix            &transform,
			 CompOutput                *output,
			 int                       size);

	void
	preparePaint (int);

	void
	donePaint ();

    private:

	bool damage;

	float contentRotation;
	GLuint gear1, gear2, gear3;
	float angle;
	float a1, a2, a3;
};


#define GET_GEARS_SCREEN (screen) \
GearsScreen *es = GearsScreen::get (screen);


class GearsPluginVTable :
    public CompPlugin::VTableForScreen<GearsScreen>
{
    public:

	bool init ();
};


