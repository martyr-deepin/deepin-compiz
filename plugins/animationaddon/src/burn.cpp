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

// =====================  Effect: Burn  =========================

BurnAnim::BurnAnim (CompWindow *w,
                    WindowEvent curWindowEvent,
                    float duration,
                    const AnimEffect info,
                    const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    ParticleAnim::ParticleAnim (w, curWindowEvent, duration, info, icon)
{
    mDirection =
	getActualAnimDirection ((AnimDirection) optValI
	                        (AnimationaddonOptions::FireDirection),
	                        false);

    if (optValB (AnimationaddonOptions::FireConstantSpeed))
    {
	int winHeight = w->height () + w->output ().top + w->output ().bottom;

	mTotalTime *= winHeight / 500.0;
	mRemainingTime *= winHeight / 500.0;
    }

    mMysticalFire = optValB (AnimationaddonOptions::FireMystical);
    mLife         = optValF (AnimationaddonOptions::FireLife);
    mColor        = optValC (AnimationaddonOptions::FireColor);
    mSize         = optValF (AnimationaddonOptions::FireSize);
    mHasSmoke     = optValB (AnimationaddonOptions::FireSmoke);

    mFirePSId  = mHasSmoke ? 1 : 0;
    mSmokePSId = 0;

    int numFireParticles = optValI (AnimationaddonOptions::FireParticles);
    float slowDown = optValF (AnimationaddonOptions::FireSlowdown);

    // Light ParticleSystem is for smoke, which is optional.
    // Dark ParticleSystem is for fire.
    initLightDarkParticles (mHasSmoke ? numFireParticles / 10 : 0,
                            numFireParticles,
                            slowDown / 2.0f, slowDown);
}

void
BurnAnim::genNewFire (int x,
                      int y,
                      int width,
                      int height,
                      float size,
                      float time)
{
    ParticleSystem &ps = mParticleSystems[mFirePSId];

    unsigned numParticles = ps.particles ().size ();

    float fireLifeNeg = 1 - mLife;
    float fadeExtra = 0.2f * (1.01 - mLife);
    float max_new = numParticles * (time / 50) * (1.05 - mLife);

    float colr1 = (float)mColor[0] / 0xffff;
    float colg1 = (float)mColor[1] / 0xffff;
    float colb1 = (float)mColor[2] / 0xffff;
    float colr2 = 1 / 1.7 * (float)mColor[0] / 0xffff;
    float colg2 = 1 / 1.7 * (float)mColor[1] / 0xffff;
    float colb2 = 1 / 1.7 * (float)mColor[2] / 0xffff;
    float cola = (float)mColor[3] / 0xffff;
    float rVal;

    float partw = mSize;
    float parth = partw * 1.5;

    // Limit max number of new particles created simultaneously
    if (max_new > numParticles / 5)
	max_new = numParticles / 5;

    Particle *part = &ps.particles ()[0];
    for (unsigned i = 0; i < numParticles && max_new > 0; i++, part++)
    {
	if (part->life <= 0.0f)
	{
	    // give gt new life
	    rVal = (float)(random () & 0xff) / 255.0;
	    part->life = 1.0f;
	    part->fade = rVal * fireLifeNeg + fadeExtra; // Random Fade Value

	    // set size
	    part->width = partw;
	    part->height = parth;
	    rVal = (float)(random () & 0xff) / 255.0;
	    part->w_mod = part->h_mod = size * rVal;

	    // choose random position
	    rVal = (float)(random () & 0xff) / 255.0;
	    part->x = x + ((width > 1) ? (rVal * width) : 0);
	    rVal = (float)(random () & 0xff) / 255.0;
	    part->y = y + ((height > 1) ? (rVal * height) : 0);
	    part->z = 0.0;
	    part->xo = part->x;
	    part->yo = part->y;
	    part->zo = part->z;

	    // set speed and direction
	    rVal = (float)(random () & 0xff) / 255.0;
	    part->xi = ((rVal * 20.0) - 10.0f);
	    rVal = (float)(random () & 0xff) / 255.0;
	    part->yi = ((rVal * 20.0) - 15.0f);
	    part->zi = 0.0f;

	    if (mMysticalFire)
	    {
		// Random colors! (aka Mystical Fire)
		rVal = (float)(random () & 0xff) / 255.0;
		part->r = rVal;
		rVal = (float)(random () & 0xff) / 255.0;
		part->g = rVal;
		rVal = (float)(random () & 0xff) / 255.0;
		part->b = rVal;
	    }
	    else
	    {
		rVal = (float)(random () & 0xff) / 255.0;
		part->r = colr1 - rVal * colr2;
		part->g = colg1 - rVal * colg2;
		part->b = colb1 - rVal * colb2;
	    }
	    // set transparancy
	    part->a = cola;

	    // set gravity
	    part->xg = (part->x < part->xo) ? 1.0 : -1.0;
	    part->yg = -3.0f;
	    part->zg = 0.0f;

	    ps.activate ();
	    max_new -= 1;
	}
	else
	{
	    part->xg = (part->x < part->xo) ? 1.0 : -1.0;
	}
    }

}

void
BurnAnim::genNewSmoke (int x,
		       int y,
		       int width,
		       int height,
		       float size,
		       float time)
{
    ParticleSystem &ps = mParticleSystems[mSmokePSId];

    unsigned numParticles = ps.particles ().size ();

    float fireLifeNeg = 1 - mLife;
    float fadeExtra = 0.2f * (1.01 - mLife);

    float max_new = numParticles * (time / 50) * (1.05 - mLife);
    float rVal;

    float partSize = mSize * size * 5;
    float sizeNeg = -size;

    // Limit max number of new particles created simultaneously
    if (max_new > numParticles)
	max_new = numParticles;

    Particle *part = &ps.particles ()[0];
    for (unsigned i = 0; i < numParticles && max_new > 0; i++, part++)
    {
	if (part->life <= 0.0f)
	{
	    // give gt new life
	    rVal = (float)(random () & 0xff) / 255.0;
	    part->life = 1.0f;
	    part->fade = rVal * fireLifeNeg + fadeExtra; // Random Fade Value

	    // set size
	    part->width = partSize;
	    part->height = partSize;
	    part->w_mod = -0.8;
	    part->h_mod = -0.8;

	    // choose random position
	    rVal = (float)(random () & 0xff) / 255.0;
	    part->x = x + ((width > 1) ? (rVal * width) : 0);
	    rVal = (float)(random () & 0xff) / 255.0;
	    part->y = y + ((height > 1) ? (rVal * height) : 0);
	    part->z = 0.0;
	    part->xo = part->x;
	    part->yo = part->y;
	    part->zo = part->z;

	    // set speed and direction
	    rVal = (float)(random () & 0xff) / 255.0;
	    part->xi = ((rVal * 20.0) - 10.0f);
	    rVal = (float)(random () & 0xff) / 255.0;
	    part->yi = (rVal + 0.2) * -size;
	    part->zi = 0.0f;

	    // set color
	    rVal = (float)(random () & 0xff) / 255.0;
	    part->r = rVal / 4.0;
	    part->g = rVal / 4.0;
	    part->b = rVal / 4.0;
	    rVal = (float)(random () & 0xff) / 255.0;
	    part->a = 0.5 + (rVal / 2.0);

	    // set gravity
	    part->xg = (part->x < part->xo) ? size : sizeNeg;
	    part->yg = sizeNeg;
	    part->zg = 0.0f;

	    ps.activate ();
	    max_new -= 1;
	}
	else
	{
	    part->xg = (part->x < part->xo) ? size : sizeNeg;
	}
    }
}

void
BurnAnim::step ()
{
    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());

    float timestep = mIntenseTimeStep;
    float old = 1 - (mRemainingTime) / (mTotalTime - timestep);
    float stepSize;

    mRemainingTime -= timestep;
    if (mRemainingTime <= 0)
	mRemainingTime = 0;	// avoid sub-zero values
    float newProgress = 1 - (mRemainingTime) / (mTotalTime - timestep);

    stepSize = newProgress - old;

    if (mCurWindowEvent == WindowEventOpen ||
	mCurWindowEvent == WindowEventUnminimize ||
	mCurWindowEvent == WindowEventUnshade)
    {
	newProgress = 1 - newProgress;
    }

    if (mRemainingTime > 0)
    {
	CompRect rect;

	switch (mDirection)
	{
	case AnimDirectionUp:
	    rect = CompRect (0, 0,
	                     outRect.width (),
	                     outRect.height () -
	                     (newProgress * outRect.height ()));
	    break;
	case AnimDirectionRight:
	    rect = CompRect (newProgress * outRect.width (),
			     0,
			     outRect.width () -
			     (newProgress * outRect.width ()),
			     outRect.height ());
	    break;
	case AnimDirectionLeft:
	    rect = CompRect (0, 0,
			     outRect.width () -
			     (newProgress * outRect.width ()),
			     outRect.height ());
	    break;
	case AnimDirectionDown:
	default:
	    rect = CompRect (0,
			     newProgress * outRect.height (),
			     outRect.width (),
			     outRect.height () -
			     (newProgress * outRect.height ()));
	    break;
	}
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
	switch (mDirection)
	{
	case AnimDirectionUp:
	    if (mHasSmoke)
		genNewSmoke (outRect.x (),
			     outRect.y () + ((1 - newProgress) * outRect.height ()),
			     outRect.width (), 1, outRect.width () / 40.0,
			     mTimeSinceLastPaint);
	    genNewFire (outRect.x (),
			outRect.y () + ((1 - newProgress) * outRect.height ()),
			outRect.width (), (stepSize) * outRect.height (),
			outRect.width () / 40.0,
			mTimeSinceLastPaint);
	    break;
	case AnimDirectionLeft:
	    if (mHasSmoke)
		genNewSmoke (outRect.x () + ((1 - newProgress) * outRect.width ()),
			     outRect.y (),
			     (stepSize) * outRect.width (),
			     outRect.height (), outRect.height () / 40.0,
			     mTimeSinceLastPaint);
	    genNewFire (outRect.x () + ((1 - newProgress) * outRect.width ()),
			outRect.y (), (stepSize) * outRect.width (),
			outRect.height (), outRect.height () / 40.0,
			mTimeSinceLastPaint);
	    break;
	case AnimDirectionRight:
	    if (mHasSmoke)
		genNewSmoke (outRect.x () + (newProgress * outRect.width ()),
			     outRect.y (),
			     (stepSize) * outRect.width (),
			     outRect.height (), outRect.height () / 40.0,
			     mTimeSinceLastPaint);
	    genNewFire (outRect.x () + (newProgress * outRect.width ()),
			outRect.y (), (stepSize) * outRect.width (),
			outRect.height (), outRect.height () / 40.0,
			mTimeSinceLastPaint);
	    break;
	case AnimDirectionDown:
	default:
	    if (mHasSmoke)
		genNewSmoke (outRect.x (),
			     outRect.y () + (newProgress * outRect.height ()),
			     outRect.width (), 1, outRect.width () / 40.0,
			     mTimeSinceLastPaint);
	    genNewFire (outRect.x (),
			outRect.y () + (newProgress * outRect.height ()),
			outRect.width (), (stepSize) * outRect.height (),
			outRect.width () / 40.0,
			mTimeSinceLastPaint);
	    break;
	}

    }
    if (mRemainingTime <= 0 &&
	(mParticleSystems[0].active () ||
	 (mHasSmoke && mParticleSystems[1].active ())))
	// force animation to continue until particle systems are done
	mRemainingTime = timestep;

    Particle *part;

    if (mRemainingTime > 0)
    {
	int nParticles;
	if (mHasSmoke)
	{
	    float partxg = outRect.width () / 40.0;
	    float partxgNeg = -partxg;

	    vector<Particle> &particles = mParticleSystems[mSmokePSId].particles ();
	    nParticles = particles.size ();
	    part = &particles[0];

	    for (int i = 0; i < nParticles; i++, part++)
		part->xg = (part->x < part->xo) ? partxg : partxgNeg;

	    mParticleSystems[mSmokePSId].setOrigin (outRect.x (), outRect.y ());
	}

	vector<Particle> &particles = mParticleSystems[mFirePSId].particles ();
	nParticles = particles.size ();
	part = &particles[0];

	for (int i = 0; i < nParticles; i++, part++)
	    part->xg = (part->x < part->xo) ? 1.0 : -1.0;
    }
    mParticleSystems[mFirePSId].setOrigin (outRect.x (), outRect.y ());
}

