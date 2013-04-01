/*
 *
 * Compiz magnifier plugin
 *
 * mag.h
 *
 * Copyright : (C) 2008 by Dennis Kasprzyk
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
 */

#include <cmath>

#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <mousepoll/mousepoll.h>

#include "mag_options.h"

#define MAG_SCREEN(s)                                                      \
    MagScreen *ms = MagScreen::get (s)

class MagScreen :
    public PluginClassHandler <MagScreen, CompScreen>,
    public MagOptions,
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface
{
    public:
	MagScreen (CompScreen *screen);
	~MagScreen ();

	CompositeScreen *cScreen;
	GLScreen	    *gScreen;

	int posX;
	int posY;

	bool adjust;

	GLfloat zVelocity;
	GLfloat zTarget;
	GLfloat zoom;

	enum MagOptions::Mode mode;

	GLuint texture;
	GLenum target;

	int width;
	int height;

	GLTexture::List overlay;
	GLTexture::List mask;
	CompSize	overlaySize, maskSize;

	GLuint program;

	MousePoller poller;
	
	bool
	checkStateTimeout ();

	void
	preparePaint (int ms);

	bool
	glPaintOutput (const GLScreenPaintAttrib &attrib,
		       const GLMatrix	         &transform,
		       const CompRegion	         &region,
		       CompOutput	         *output,
		       unsigned int              mask);

	void
	donePaint ();

	void
	cleanup ();

	bool
	loadFragmentProgram ();

	bool
	loadImages ();

	void
	optionChanged (CompOption	      *opt,
			   	  MagOptions::Options num);

	void
	doDamageRegion ();

	void
	positionUpdate (const CompPoint &pos);

	int
	adjustZoom (float chunk);

	void
	paintSimple ();

	void
	paintImage ();

	void
	paintFisheye ();

	bool
	terminate (CompAction	  *action,
	      	      CompAction::State   state,
	      	      CompOption::Vector options);

	bool
	initiate (CompAction	  *action,
		  CompAction::State   state,
		  CompOption::Vector options);

	bool
	zoomIn (CompAction	  *action,
		CompAction::State   state,
		CompOption::Vector options);

	bool
	zoomOut (CompAction	  *action,
		 CompAction::State   state,
		 CompOption::Vector options);

};

class MagPluginVTable :
    public CompPlugin::VTableForScreen <MagScreen>
{
    public:
	bool init ();
};

#if 0
static const char *fisheyeFpString =
    "!!ARBfp1.0"

    "PARAM p0 = program.env[0];"
    "PARAM p1 = program.env[1];"
    "PARAM p2 = program.env[2];"

    "TEMP t0, t1, t2, t3;"

    "SUB t1, p0.xyww, fragment.texcoord[0];"
    "DP3 t2, t1, t1;"
    "RSQ t2, t2.x;"
    "SUB t0, t2, p0;"

    "RCP t3, t2.x;"
    "MAD t3, t3, p1.z, p2.z;"
    "COS t3, t3.x;"

    "MUL t3, t3, p1.w;"

    "MUL t1, t2, t1;"
    "MAD t1, t1, t3, fragment.texcoord[0];"

    "CMP t1, t0.z, fragment.texcoord[0], t1;"
		
    "MAD t1, t1, p1, p2;"
    "TEX result.color, t1, texture[0], %s;"

    "END";
#endif
