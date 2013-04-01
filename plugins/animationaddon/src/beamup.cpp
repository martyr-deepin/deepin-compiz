/*
 * Animation plugin for compiz/beryl
 *
 * animation.c
 *
 * Copyright : (C) 2006 Erkin Bahceci
 * E-mail    : erkinbah@gmail.com
 *
 * Based on Wobbly and Minimize plugins by
 *           : David Reveman
 * E-mail    : davidr@novell.com>
 *
 * Particle system added by : (C) 2006 Dennis Kasprzyk
 * E-mail                   : onestone@beryl-project.org
 *
 * Beam-Up added by : Florencio Guimaraes
 * E-mail           : florencio@nexcorp.com.br
 *
 * Hexagon tessellator added by : Mike Slegeir
 * E-mail                       : mikeslegeir@mail.utexas.edu>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "private.h"

// =====================  Effect: Beam Up  =========================

BeamUpAnim::BeamUpAnim (CompWindow *w,
                        WindowEvent curWindowEvent,
                        float duration,
                        const AnimEffect info,
                        const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    ParticleAnim::ParticleAnim (w, curWindowEvent, duration, info, icon)
{
    mLife         = optValF (AnimationaddonOptions::BeamLife);
    mColor        = optValC (AnimationaddonOptions::BeamColor);
    mSize         = optValF (AnimationaddonOptions::BeamSize);
    mSpacing      = optValI (AnimationaddonOptions::BeamSpacing);
    mSlowdown     = optValF (AnimationaddonOptions::BeamSlowdown);
}

void
BeamUpAnim::init ()
{
    int winWidth = mWindow->width () +
		   mWindow->output ().left + mWindow->output ().right;

    initLightDarkParticles (0, winWidth / mSpacing, 0, mSlowdown);
}

void
BeamUpAnim::genNewBeam (int x,
			int y,
			int width,
			int height,
			float size,
			float time)
{
    ParticleSystem &ps = mParticleSystems[0];

    unsigned numParticles = ps.particles ().size ();

    float beamLifeNeg = 1 - mLife;
    float fadeExtra = 0.2f * (1.01 - mLife);
    float maxNew = numParticles * (time / 50) * (1.05 - mLife);

    // set color ABAB ANIMADDON_SCREEN_OPTION_BEAMUP_COLOR
    unsigned short *c = mColor;
    float colr1 = (float)c[0] / 0xffff;
    float colg1 = (float)c[1] / 0xffff;
    float colb1 = (float)c[2] / 0xffff;
    float colr2 = 1 / 1.7 * (float)c[0] / 0xffff;
    float colg2 = 1 / 1.7 * (float)c[1] / 0xffff;
    float colb2 = 1 / 1.7 * (float)c[2] / 0xffff;
    float cola = (float)c[3] / 0xffff;
    float rVal;

    float partw = 2.5 * mSize;

    // Limit max number of new particles created simultaneously
    if (maxNew > numParticles)
	maxNew = numParticles;

    Particle *part = &ps.particles ()[0];
    for (unsigned i = 0; i < numParticles && maxNew > 0; i++, part++)
    {
	if (part->life <= 0.0f)
	{
	    // give gt new life
	    rVal = (float)(random () & 0xff) / 255.0;
	    part->life = 1.0f;
	    part->fade = rVal * beamLifeNeg + fadeExtra; // Random Fade Value

	    // set size
	    part->width = partw;
	    part->height = height;
	    part->w_mod = size * 0.2;
	    part->h_mod = size * 0.02;

	    // choose random x position
	    rVal = (float)(random () & 0xff) / 255.0;
	    part->x = x + ((width > 1) ? (rVal * width) : 0);
	    part->y = y;
	    part->z = 0.0;
	    part->xo = part->x;
	    part->yo = part->y;
	    part->zo = part->z;

	    // set speed and direction
	    part->xi = 0.0f;
	    part->yi = 0.0f;
	    part->zi = 0.0f;

	    part->r = colr1 - rVal * colr2;
	    part->g = colg1 - rVal * colg2;
	    part->b = colb1 - rVal * colb2;
	    part->a = cola;

	    // set gravity
	    part->xg = 0.0f;
	    part->yg = 0.0f;
	    part->zg = 0.0f;

	    if (!ps.active ())
		ps.activate ();
	    maxNew -= 1;
	}
	else
	{
	    part->xg = (part->x < part->xo) ? 1.0 : -1.0;
	}
    }
}

void
BeamUpAnim::step ()
{
    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());

    float timestep = mIntenseTimeStep;

    mRemainingTime -= timestep;
    if (mRemainingTime <= 0)
	mRemainingTime = 0;	// avoid sub-zero values

    float newProgress = 1 - mRemainingTime / (mTotalTime - timestep);

    bool creating = (mCurWindowEvent == WindowEventOpen ||
		     mCurWindowEvent == WindowEventUnminimize ||
		     mCurWindowEvent == WindowEventUnshade);

    if (creating)
	newProgress = 1 - newProgress;

    if (mRemainingTime > 0)
    {
	CompRect rect (((newProgress / 2) * outRect.width ()),
		       ((newProgress / 2) * outRect.height ()),
		       (1 - newProgress) * outRect.width (),
		       (1 - newProgress) * outRect.height ());
	rect.setX (rect.x () + outRect.x ());
	rect.setY (rect.y () + outRect.y ());

	mDrawRegion = CompRegion (rect);
    }
    else
    {
	mDrawRegion = emptyRegion;
    }

    mUseDrawRegion = (fabs (newProgress) > 1e-5);

    if (mRemainingTime > 0)
    {
	genNewBeam (outRect.x (), outRect.y () + (outRect.height () / 2),
	            outRect.width (),
	            creating ? (1 - newProgress / 2) * outRect.height ()
	                     : (1 - newProgress) * outRect.height (),
		    outRect.width () / 40.0,
		    mTimeSinceLastPaint);
    }
    if (mRemainingTime <= 0 && mParticleSystems[0].active ())
	// force animation to continue until particle systems are done
	mRemainingTime = 0.001f;

    if (mRemainingTime > 0)
    {
	vector<Particle> &particles = mParticleSystems[0].particles ();
	int nParticles = particles.size ();
	Particle *part = &particles[0];

	for (int i = 0; i < nParticles; i++, part++)
	    part->xg = (part->x < part->xo) ? 1.0 : -1.0;
    }
    mParticleSystems[0].setOrigin (outRect.x (), outRect.y ());
}

void
BeamUpAnim::updateAttrib (GLWindowPaintAttrib &attrib)
{
    float forwardProgress = 0;
    if (mTotalTime - mIntenseTimeStep != 0)
	forwardProgress =
	    1 - (mRemainingTime) / (mTotalTime - mIntenseTimeStep);
    forwardProgress = MIN(forwardProgress, 1);
    forwardProgress = MAX(forwardProgress, 0);
    //float forwardProgress = progressLinear ();

    if (mCurWindowEvent == WindowEventOpen ||
	mCurWindowEvent == WindowEventUnminimize)
    {
	//forwardProgress = 1 - forwardProgress;
	forwardProgress = forwardProgress * forwardProgress;
	forwardProgress = forwardProgress * forwardProgress;
	forwardProgress = 1 - forwardProgress;
    }

    attrib.opacity = (GLushort) (mStoredOpacity * (1 - forwardProgress));
}
