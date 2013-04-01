/*
 *
 * Compiz trip plugin
 *
 * trip.h
 *
 * Copyright : (C) 2010 by Scott Moreau
 * E-mail    : oreaus@gmail.com
 * 
 * Based off the mag plugin by :
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

#include "trip_options.h"

#define TRIP_SCREEN(s)                                                      \
    TripScreen *ts = TripScreen::get (s)

class Ripple;

class TripScreen :
    public PluginClassHandler <TripScreen, CompScreen>,
    public TripOptions,
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface
{
    public:
	TripScreen (CompScreen *screen);
	~TripScreen ();

	CompositeScreen *cScreen;
	GLScreen	*gScreen;

	std::vector <Ripple> ripples;

	GLuint texture;
	GLenum target;

	GLuint program;

	bool	quiet;

	unsigned int intensity;
	
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

	void
	optionChanged (CompOption	*opt,
		       Options		num);

	void
	positionUpdate (const CompPoint &pos);

	int
	adjustZoom (float chunk, Ripple &r);

	bool
	terminate (CompAction	  *action,
	      	      CompAction::State   state,
	      	      CompOption::Vector options);

	bool
	takeHit (CompAction	  *action,
		 CompAction::State   state,
		 CompOption::Vector options);

	bool
	untensify (CompAction	  *action,
		   CompAction::State   state,
		   CompOption::Vector options);

	bool
	intensify (CompAction	  *action,
		   CompAction::State   state,
		   CompOption::Vector options);

	bool
	soberUp (CompAction	  *action,
		 CompAction::State   state,
		 CompOption::Vector options);

	void populateRippleSet ();

};

class Ripple
{
    public:
	Ripple ();
	~Ripple ();

	TripScreen *dScreen;

	int radius;
	int rMod;

	CompPoint coord;

	int duration;
	int timer;

	int width;
	int height;

	GLfloat zVelocity;
	GLfloat zTarget;
	GLfloat zoom;

	bool adjust;

	CompRect damageRect;

	void paint ();
	void spawnRandom ();
};

class TripPluginVTable :
    public CompPlugin::VTableForScreen <TripScreen>
{
    public:
	bool init ();
};

static const char *rippleFpString =
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
