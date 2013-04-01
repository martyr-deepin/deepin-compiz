/**
 *
 * Compiz benchmark plugin
 *
 * bench.c
 *
 * Copyright : (C) 2006 by Dennis Kasprzyk
 * E-mail    : onestone@beryl-project.org
 *
 * New frame rate measurement algorithm:
 * Copyright (c) 2011 Daniel van Vugt <vanvugt@gmail.com>
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

#include <core/core.h>
#include <core/timer.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>

#include <sys/time.h>

#include "bench_tex.h"
#include "bench_options.h"

#define TIMEVALDIFFU(tv1, tv2)                                              \
    (((tv1)->tv_sec == (tv2)->tv_sec || (tv1)->tv_usec >= (tv2)->tv_usec) ? \
     ((((tv1)->tv_sec - (tv2)->tv_sec) * 1000000) +                      \
      ((tv1)->tv_usec - (tv2)->tv_usec)):                                   \
     ((((tv1)->tv_sec - 1 - (tv2)->tv_sec) * 1000000) +                  \
      (1000000 + (tv1)->tv_usec - (tv2)->tv_usec)))

#ifdef GL_DEBUG

static GLenum gl_error;

#define GLERR  gl_error=glGetError(); if (gl_error !=  GL_NO_ERROR) { fprintf (stderr,"GL error 0x%X has occured at %s:%d\n",gl_error,__FILE__,__LINE__); }
#else
#define GLERR
#endif

class BenchScreen :
    public CompositeScreenInterface,
    public GLScreenInterface,
    public PluginClassHandler<BenchScreen, CompScreen>,
    public BenchOptions
{
    public:
	BenchScreen (CompScreen *screen);
	~BenchScreen ();

	CompositeScreen *cScreen;
	GLScreen        *gScreen;

	GLuint mDList;
	float  mAlpha;

	enum {
	    MAX_FPS = 500,
	    FADE_FPS = 50,
	    SECONDS_PER_AVERAGE = 2,
	    MAX_SAMPLES = MAX_FPS * SECONDS_PER_AVERAGE,
	    MIN_MS_PER_UPDATE = 1000
	};

	bool      mFakedDamage;
	CompRect  mRect;
	CompTimer mTimer;

	int mSample[MAX_SAMPLES];
	int mFrames;
	int mLastPrintFrames;

	struct timeval mLastPrint;
	struct timeval mLastRedraw;

	GLuint mNumTex[10];
	GLuint mBackTex;

	bool mActive;

	CompositeFPSLimiterMode mOldLimiterMode;

	void damageSelf ();
	bool timedOut ();
	float averageFramerate () const;
	void postLoad ();

	template <class Archive>
	void serialize (Archive & ar, const unsigned int count)
	{
	    ar & mActive;
	}

	bool initiate (CompOption::Vector &options);

	void limiterModeChanged (CompOption *opt);

	void preparePaint (int msSinceLastPaint);

	bool glPaintOutput (const GLScreenPaintAttrib &,
			    const GLMatrix &, const CompRegion &,
			    CompOutput *, unsigned int);
};

class BenchPluginVTable :
    public CompPlugin::VTableForScreen<BenchScreen>
{
    public:

	bool init ();
};

