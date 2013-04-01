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
#include "animation_tex.h"

ParticleAnim::ParticleAnim (CompWindow *w,
			    WindowEvent curWindowEvent,
			    float duration,
			    const AnimEffect info,
			    const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    BaseAddonAnim::BaseAddonAnim (w, curWindowEvent, duration, info, icon),
    PartialWindowAnim::PartialWindowAnim (w, curWindowEvent, duration, info,
					  icon)
{
}

ParticleSystem::ParticleSystem (int    numParticles,
                                float  slowDown,
                                float  darkenAmount,
                                GLuint blendMode) :
    mParticles (numParticles),
    mSlowDown (slowDown),
    mDarkenAmount (darkenAmount),
    mBlendMode (blendMode),
    mTex (0),
    mActive (false),
    mGScreen (GLScreen::get (::screen))
{
    glGenTextures (1, &mTex);
    /*
    glBindTexture (GL_TEXTURE_2D, mTex);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0,
		 GL_RGBA, GL_UNSIGNED_BYTE, fireTex);
    glBindTexture (GL_TEXTURE_2D, 0);
    */
}

ParticleSystem::~ParticleSystem ()
{
    if (mTex)
	glDeleteTextures (1, &mTex);
}

void
ParticleSystem::draw (int offsetX, int offsetY)
{
    // TODO
    // The part below should ideally be done in ParticleSystem constructor
    // instead, but for some reason the texture image gets lost when we do that.
    glBindTexture (GL_TEXTURE_2D, mTex);

    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0,
		 GL_RGBA, GL_UNSIGNED_BYTE, fireTex);
    glBindTexture (GL_TEXTURE_2D, 0);


    glPushMatrix ();
    glTranslated (offsetX - mX, offsetY - mY, 0);

    glEnable (GL_BLEND);
    if (mTex)
    {
	glBindTexture (GL_TEXTURE_2D, mTex);
	glEnable (GL_TEXTURE_2D);
    }
    mGScreen->setTexEnvMode (GL_MODULATE);

    mVerticesCache.resize (4 * 3 * mParticles.size ());
    mCoordsCache.resize (4 * 2 * mParticles.size ());
    mColorsCache.resize (4 * 4 * mParticles.size ());
    if (mDarkenAmount > 0)
	mDColorsCache.resize (4 * 4 * mParticles.size ());

    GLfloat *dcolors = &mDColorsCache[0];
    GLfloat *vertices = &mVerticesCache[0];
    GLfloat *coords = &mCoordsCache[0];
    GLfloat *colors = &mColorsCache[0];

    int cornersSize = sizeof (GLfloat) * 8;
    int colorSize = sizeof (GLfloat) * 4;

    GLfloat cornerCoords[8] = {0.0, 0.0,
			       0.0, 1.0,
			       1.0, 1.0,
			       1.0, 0.0};

    int numActive = 0;

    foreach (Particle &part, mParticles)
    {
	if (part.life <= 0.0f)	     // Ignore dead particles
	    continue;

	numActive += 4;

	float w = part.width / 2;
	float h = part.height / 2;

	w += (w * part.w_mod) * part.life;
	h += (h * part.h_mod) * part.life;

	vertices[0] = part.x - w;
	vertices[1] = part.y - h;
	vertices[2] = part.z;

	vertices[3] = part.x - w;
	vertices[4] = part.y + h;
	vertices[5] = part.z;

	vertices[6] = part.x + w;
	vertices[7] = part.y + h;
	vertices[8] = part.z;

	vertices[9] = part.x + w;
	vertices[10] = part.y - h;
	vertices[11] = part.z;

	vertices += 12;

	memcpy (coords, cornerCoords, cornersSize);

	coords += 8;

	colors[0] = part.r;
	colors[1] = part.g;
	colors[2] = part.b;
	colors[3] = part.life * part.a;
	memcpy (colors + 4, colors, colorSize);
	memcpy (colors + 8, colors, colorSize);
	memcpy (colors + 12, colors, colorSize);

	colors += 16;

	if (mDarkenAmount > 0)
	{
	    dcolors[0] = part.r;
	    dcolors[1] = part.g;
	    dcolors[2] = part.b;
	    dcolors[3] = part.life * part.a * mDarkenAmount;
	    memcpy (dcolors + 4, dcolors, colorSize);
	    memcpy (dcolors + 8, dcolors, colorSize);
	    memcpy (dcolors + 12, dcolors, colorSize);

	    dcolors += 16;
	}
    }

    glEnableClientState (GL_COLOR_ARRAY);

    glTexCoordPointer (2, GL_FLOAT, 2 * sizeof (GLfloat), &mCoordsCache[0]);
    glVertexPointer (3, GL_FLOAT, 3 * sizeof (GLfloat), &mVerticesCache[0]);

    // darken the background
    if (mDarkenAmount > 0)
    {
	glBlendFunc (GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
	glColorPointer (4, GL_FLOAT, 4 * sizeof (GLfloat), &mDColorsCache[0]);
	glDrawArrays (GL_QUADS, 0, numActive);
    }
    // draw particles
    glBlendFunc (GL_SRC_ALPHA, mBlendMode);

    glColorPointer (4, GL_FLOAT, 4 * sizeof (GLfloat), &mColorsCache[0]);
    glDrawArrays (GL_QUADS, 0, numActive);
    glDisableClientState (GL_COLOR_ARRAY);

    glPopMatrix ();
    glColor4usv (defaultColor);

    mGScreen->setTexEnvMode (GL_REPLACE);

    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable (GL_TEXTURE_2D);
    glDisable (GL_BLEND);
}

void
ParticleAnim::postPaintWindow ()
{
    foreach (ParticleSystem &ps, mParticleSystems)
	if (ps.active ())
	    // offset by window pos.
	    ps.draw (mWindow->x () - mWindow->output ().left,
	             mWindow->y () - mWindow->output ().top);
}

void
ParticleSystem::update (float time)
{
    float speed = (time / 50.0);
    float slowdown = mSlowDown * (1 - MAX (0.99, time / 1000.0)) * 1000;

    mActive = false;

    foreach (Particle &part, mParticles)
    {
	if (part.life <= 0.0f)	     // Ignore dead particles
	    continue;

	// move particle
	part.x += part.xi / slowdown;
	part.y += part.yi / slowdown;
	part.z += part.zi / slowdown;

	// modify speed
	part.xi += part.xg * speed;
	part.yi += part.yg * speed;
	part.zi += part.zg * speed;

	// modify life
	part.life -= part.fade * speed;
	if (!mActive)
	    mActive = true;
    }
}

void
ParticleAnim::updateBB (CompOutput &output)
{
    foreach (ParticleSystem &ps, mParticleSystems)
    {
	if (!ps.active ())
	    continue;

	foreach (Particle &part, ps.particles ())
	{
	    if (part.life <= 0.0f)	     // Ignore dead particles
		continue;

	    float w = part.width / 2;
	    float h = part.height / 2;

	    w += (w * part.w_mod) * part.life;
	    h += (h * part.h_mod) * part.life;

	    Box particleBox =
	    {
		static_cast <short int> (part.x - w), static_cast <short int> (part.x + w),
		static_cast <short int> (part.y - h), static_cast <short int> (part.y + h)
	    };

	    mAWindow->expandBBWithBox (particleBox);
	}
    }

    if (mUseDrawRegion && mDrawRegion != emptyRegion)
	// expand BB with bounding box of draw region
	mAWindow->expandBBWithBox (mDrawRegion.handle ()->extents);
    else // drawing full window
	mAWindow->expandBBWithWindow ();
}

bool
ParticleAnim::prePreparePaint (int msSinceLastPaint)
{
    bool particleAnimInProgress = false;

    foreach (ParticleSystem &ps, mParticleSystems)
    {
	if (!ps.active ())
	    continue;

	ps.update (msSinceLastPaint);
	particleAnimInProgress = true;
    }

    return particleAnimInProgress;
}

void
ParticleAnim::initLightDarkParticles (int numLightParticles,
                                      int numDarkParticles,
                                      float lightSlowDown,
                                      float darkSlowDown)
{
    if (numLightParticles > 0)
	mParticleSystems.push_back (new ParticleSystem (numLightParticles,
							lightSlowDown,
							0.0f,
							GL_ONE_MINUS_SRC_ALPHA));
    if (numDarkParticles > 0)
	mParticleSystems.push_back (new ParticleSystem (numDarkParticles,
							darkSlowDown,
							0.5f,
							GL_ONE));
}

