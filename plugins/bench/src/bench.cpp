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

#include "bench.h"

using namespace compiz::core;

COMPIZ_PLUGIN_20090315 (bench, BenchPluginVTable)

static const unsigned int TEX_WIDTH = 512;
static const unsigned short TEX_HEIGHT = 256;

void
BenchScreen::preparePaint (int msSinceLastPaint)
{
    struct timeval now;
    gettimeofday (&now, 0);

    int timediff = TIMEVALDIFFU (&now, &mLastRedraw);
    mSample[mFrames % MAX_SAMPLES] = timediff;
    timediff /= 1000;
    mFrames++;
    mLastRedraw = now;

    if (optionGetOutputConsole () && mActive)
    {
	int dTime = timer::timeval_diff (&now, &mLastPrint);
	if (dTime > optionGetConsoleUpdateTime () * 1000)
	{
	    int dFrames = mFrames - mLastPrintFrames;
	    mLastPrintFrames = mFrames;
	    g_print ("[BENCH] : %d frames in %d.%01d seconds = %d.%03d FPS\n",
		    dFrames, dTime / 1000, (dTime % 1000) / 100,
		    dFrames * 1000 / dTime, ((dFrames * 1000) % dTime) / 10);
	    mLastPrint = now;
	}
    }

    if (mActive)
    {
	mAlpha += timediff / 1000.0;
	if (mAlpha >= 1.0f)
	{
	    mAlpha = 1.0f;
	    /*
	     * If we're only creating "fake" damage to update the benchmark
	     * and no other damage is pending, then do it progressively
	     * less often so the framerate can steadily decrease toward zero.
	     */
	    if (mFakedDamage)
		mTimer.setTimes (mTimer.minTime () * 2);
	    else
	    {
	        /*
		 * Piggyback on damage events other than our own, so the
		 * benchmark updates at least as often as the rest
		 * of the screen.
		 */
		damageSelf ();
		if (mTimer.minTime () != MIN_MS_PER_UPDATE)
		    mTimer.setTimes (MIN_MS_PER_UPDATE);
	    }
	}
    }
    else
    {
	if (mAlpha <= 0.0)
	{
	    cScreen->preparePaintSetEnabled (this, false);
	    gScreen->glPaintOutputSetEnabled (this, false);
	    mTimer.stop ();
	}
	mAlpha -= timediff / 1000.0;
	if (mAlpha < 0.0f)
	    mAlpha = 0.0f;
    }

    mFakedDamage = false;

    cScreen->preparePaint (msSinceLastPaint);
}

float
BenchScreen::averageFramerate () const
/*
 * Returns the average frame rate of the last SECONDS_PER_AVERAGE seconds.
 * This calculation is accurate no matter how often/seldom the screen
 * gets painted. No timers required. Calculus rocks :)
 */
{
    const int usPerAverage = SECONDS_PER_AVERAGE * 1000000;
    int i = (mFrames + MAX_SAMPLES - 1) % MAX_SAMPLES;
    int lastSample = 0;
    int timeSum = 0;
    int count = 0;
    int maxCount = MIN (MAX_SAMPLES, mFrames);

    while (timeSum < usPerAverage && count < maxCount)
    {
	lastSample = mSample[i];
	timeSum += lastSample;
	i = (i + MAX_SAMPLES - 1) % MAX_SAMPLES;
	count++;
    }

    float fps = 0.0f;
    if (timeSum < usPerAverage)
    {
	if (timeSum > 0)
	    fps = (float)(count * 1000000) / timeSum;
    }
    else
    {
	fps = (float)(count - 1);
	if (lastSample > 0)
	    fps += (float)(usPerAverage - (timeSum - lastSample)) / lastSample;
	fps /= SECONDS_PER_AVERAGE;
    }

    return fps;
}

bool
BenchScreen::glPaintOutput (const GLScreenPaintAttrib &sAttrib,
			    const GLMatrix            &transform,
			    const CompRegion          &region,
			    CompOutput                *output,
			    unsigned int              mask)
{
    bool status;
    bool isSet;
    unsigned int  fps;
    GLMatrix sTransform (transform);

    status = gScreen->glPaintOutput (sAttrib, transform, region, output, mask);

    if (mAlpha <= 0.0 || !optionGetOutputScreen ())
	return status;

    glGetError();
    glPushAttrib (GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT);
    GLERR;

    sTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);

    glPushMatrix ();
    glLoadMatrixf (sTransform.getMatrix ());

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f (1.0, 1.0, 1.0, mAlpha);
    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    mRect.setX (optionGetPositionX ());
    mRect.setY (optionGetPositionY ());
    glTranslatef (mRect.x (), mRect.y (), 0);

    glEnable (GL_TEXTURE_2D);
    glBindTexture (GL_TEXTURE_2D, mBackTex);

    glBegin (GL_QUADS);
    glTexCoord2f (0, 0);
    glVertex2f (0, 0);
    glTexCoord2f (0, 1);
    glVertex2f (0, TEX_HEIGHT);
    glTexCoord2f (1, 1);
    glVertex2f (TEX_WIDTH, TEX_HEIGHT);
    glTexCoord2f (1, 0);
    glVertex2f (TEX_WIDTH, 0);
    glEnd();

    glBindTexture (GL_TEXTURE_2D, 0);
    glDisable (GL_TEXTURE_2D);

    glTranslatef (53, 83, 0);

    float avgFps = averageFramerate ();
    float rrVal = avgFps * cScreen->optimalRedrawTime () / 1000.0;
    /*
     * rrVal is slightly inaccurate and can be off by a couple of FPS.
     * This means the graph for a 60 FPS config goes up to 62.5 FPS.
     * This is because cScreen->optimalRedrawTime only has millisec precision
     * and can't be avoided without improving the precision of the composite
     * plugin.
     */
    rrVal = MIN (1.0, MAX (0.0, rrVal) );

    if (rrVal < 0.5)
    {
	glBegin (GL_QUADS);
	glColor4f (0.0, 1.0, 0.0, mAlpha);
	glVertex2f (0.0, 0.0);
	glVertex2f (0.0, 25.0);
	glColor4f (rrVal * 2.0, 1.0, 0.0, mAlpha);
	glVertex2f (330.0 * rrVal, 25.0);
	glVertex2f (330.0 * rrVal, 0.0);
	glEnd();
    }
    else
    {
	glBegin (GL_QUADS);
	glColor4f (0.0, 1.0, 0.0, mAlpha);
	glVertex2f (0.0, 0.0);
	glVertex2f (0.0, 25.0);
	glColor4f (1.0, 1.0, 0.0, mAlpha);
	glVertex2f (165.0, 25.0);
	glVertex2f (165.0, 0.0);
	glEnd();

	glBegin (GL_QUADS);
	glColor4f (1.0, 1.0, 0.0, mAlpha);
	glVertex2f (165.0, 0.0);
	glVertex2f (165.0, 25.0);
	glColor4f (1.0, 1.0 - ( (rrVal - 0.5) * 2.0), 0.0, mAlpha);
	glVertex2f (165.0 + 330.0 * (rrVal - 0.5), 25.0);
	glVertex2f (165.0 + 330.0 * (rrVal - 0.5), 0.0);
	glEnd();
    }

    glColor4f (0.0, 0.0, 0.0, mAlpha);
    glCallList (mDList);
    glTranslatef (72, 45, 0);
    glEnable (GL_TEXTURE_2D);

    isSet = false;

    fps = (avgFps * 100.0);
    fps = MIN (999999, fps);

    for (unsigned int pos = 100000; pos >= 1; pos /= 10)
    {
	if (fps >= pos || isSet || pos <= 100)
	{
	    unsigned int digit = fps / pos;
	    glBindTexture (GL_TEXTURE_2D, mNumTex[digit]);
	    glCallList (mDList + 1);
	    isSet = true;
	    fps %= pos;
	}
	glTranslatef ((pos == 100) ? 19 : 12, 0, 0);
    }

    glBindTexture (GL_TEXTURE_2D, 0);
    glDisable (GL_TEXTURE_2D);

    glPopMatrix();

    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glColor4f (1.0, 1.0, 1.0, 1.0);

    glPopAttrib();
    glGetError();

    return status;
}

void
BenchScreen::limiterModeChanged (CompOption *opt)
{
    if (mActive)
	cScreen->setFPSLimiterMode ((CompositeFPSLimiterMode)
				    opt->value ().i ());
}

BenchScreen::BenchScreen (CompScreen *screen) :
    PluginClassHandler<BenchScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    mAlpha (0),
    mFakedDamage (false),
    mFrames (0),
    mLastPrintFrames (0),
    mActive (false),
    mOldLimiterMode ((CompositeFPSLimiterMode)
    		     BenchOptions::FpsLimiterModeDefaultLimiter)
{
    optionSetInitiateKeyInitiate (boost::bind (&BenchScreen::initiate, this,
					       _3));

    optionSetFpsLimiterModeNotify
	(boost::bind (&BenchScreen::limiterModeChanged, this, _1));

    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);

    mRect.setGeometry (optionGetPositionX (), optionGetPositionY (),
                       TEX_WIDTH, TEX_HEIGHT);
    mTimer.setCallback (boost::bind (&BenchScreen::timedOut, this));

    glGenTextures (10, mNumTex);
    glGenTextures (1, &mBackTex);

    glGetError();

    glEnable (GL_TEXTURE_2D);

    for (int i = 0; i < 10; i++)
    {
	//Bind the texture
	glBindTexture (GL_TEXTURE_2D, mNumTex[i]);

	//Load the parameters
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D (GL_TEXTURE_2D, 0, GL_ALPHA, 16, 32, 0,
		      GL_RGBA, GL_UNSIGNED_BYTE, number_data[i]);
	GLERR;
    }

    glBindTexture (GL_TEXTURE_2D, mBackTex);

    //Load the parameters
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D (GL_TEXTURE_2D, 0, 4, TEX_WIDTH, TEX_HEIGHT, 0, GL_RGBA,
		  GL_UNSIGNED_BYTE, image_data);
    GLERR;

    glBindTexture (GL_TEXTURE_2D, 0);
    glDisable (GL_TEXTURE_2D);

    mDList = glGenLists (2);
    glNewList (mDList, GL_COMPILE);

    glLineWidth (2.0);

    glBegin (GL_LINE_LOOP);
    glVertex2f (0, 0);
    glVertex2f (0, 25);
    glVertex2f (330, 25);
    glVertex2f (330, 0);
    glEnd();

    glLineWidth (1.0);

    glBegin (GL_LINES);

    for (int i = 33; i < 330; i += 33)
    {
	glVertex2f (i, 15);
	glVertex2f (i, 25);
    }

    for (int i = 16; i < 330; i += 33)
    {
	glVertex2f (i, 20);
	glVertex2f (i, 25);
    }

    glEnd();

    glEndList();

    glNewList (mDList + 1, GL_COMPILE);
    glBegin (GL_QUADS);
    glTexCoord2f (0, 0);
    glVertex2f (0, 0);
    glTexCoord2f (0, 1);
    glVertex2f (0, 32);
    glTexCoord2f (1, 1);
    glVertex2f (16, 32);
    glTexCoord2f (1, 0);
    glVertex2f (16, 0);
    glEnd();
    glEndList();
}

BenchScreen::~BenchScreen ()
{
    if (mActive)
    {
    	// Restore FPS limiter mode
    	cScreen->setFPSLimiterMode (mOldLimiterMode);
    }

    glDeleteLists (mDList, 2);

    glDeleteTextures (10, mNumTex);
    glDeleteTextures (1, &mBackTex);
}

void
BenchScreen::damageSelf ()
{
    CompRegion self (mRect);
    cScreen->damageRegion (self);
}

bool
BenchScreen::timedOut ()
{
    mFakedDamage = (cScreen->damageMask () == 0);
    damageSelf ();
    return true;
}

bool
BenchScreen::initiate (CompOption::Vector &options)
{
    mActive = !mActive;
    mActive &= optionGetOutputScreen () || optionGetOutputConsole ();

    Window     xid;

    xid = (Window) CompOption::getIntOptionNamed (options, "root");

    if (xid != ::screen->root ())
	return false;

    if (mActive)
    {
    	// Store current FPS limiter mode
    	mOldLimiterMode = cScreen->FPSLimiterMode ();

    	cScreen->setFPSLimiterMode ((CompositeFPSLimiterMode)
				    optionGetFpsLimiterMode ());

	cScreen->preparePaintSetEnabled (this, true);
	gScreen->glPaintOutputSetEnabled (this, true);

	for (int t = 0; t < MAX_SAMPLES; t++)
	    mSample[t] = 0;
    }
    else
    {
    	// Restore FPS limiter mode
    	cScreen->setFPSLimiterMode (mOldLimiterMode);
	mTimer.stop ();
    }
    mTimer.start (1000 / FADE_FPS);

    mFrames = 0;
    mLastPrintFrames = 0;

    gettimeofday (&mLastRedraw, 0);
    mLastPrint = mLastRedraw;

    return true;
}

bool
BenchPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) |
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) |
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	 return false;

    return true;
}

