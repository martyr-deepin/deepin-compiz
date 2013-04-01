/*
 * Compiz motion blur effect plugin
 *
 * mblur.c
 *
 * Copyright : (C) 2007 by Dennis Kasprzyk
 * E-mail    : onestone@beryl-project.org
 *
 * Ported to Compiz 0.9 by:
 * Copyright : (C) 2009 by Sam Spilsbury
 * E-mail    : smspillaz@gmail.com
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

#include "mblur.h"

COMPIZ_PLUGIN_20090315 (mblur, MblurPluginVTable);

static void
toggleFunctions (bool enabled)
{
    MBLUR_SCREEN (screen);

    ms->cScreen->preparePaintSetEnabled (ms, enabled);
    ms->gScreen->glPaintOutputSetEnabled (ms, enabled);
}

/* activate/deactivate motion blur */

bool
MblurScreen::toggle (CompAction         *action,
		     CompAction::State  state,
		     CompOption::Vector options)
{
    activated = !activated;

    if (activated)
	toggleFunctions (true);

    return true;
}

void
MblurScreen::preparePaint (int msec)
{
    active |= activated;

    /* fade motion blur out if no longer active */

    if (activated)
    {
	timer = 500;
	toggleFunctions (true);
    }
    else
    {
	timer -= msec;
    }

    // calculate motion blur strength dependent on framerate
    float val   = 101 - MIN (100, MAX (1, msec) );
    float a_val = optionGetStrength () / 20.0;

    a_val = a_val * a_val;
    a_val /= 100.0;

    alpha = 1.0 - pow (a_val, 1.0 / val);

    if (active && timer <= 0)
	cScreen->damageScreen ();

    if (timer <= 0)
    {
	active = FALSE;
    }

    if (timer <= 0 && !activated)
	toggleFunctions (false);

    if (update && active)
	cScreen->damageScreen ();

    cScreen->preparePaint (msec);
}

/* FIXME?: This function was originally paintScreen (!= paintOutput). However
 * no paintScreen exists in compiz 0.9. This might result in code being executed
 * twice!
 */

bool
MblurScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
			    const GLMatrix	    &transform,
			    const CompRegion	    &region,
			    CompOutput		    *output,
			    unsigned int		    mask)
{
    bool status;

    if (!active)
	update = true;

    status = gScreen->glPaintOutput (attrib, transform, region, output, mask);   

    bool enable_scissor = false;

    if (active && glIsEnabled (GL_SCISSOR_TEST) )
    {
	glDisable (GL_SCISSOR_TEST);
	enable_scissor = true;
    }

    if (active && optionGetMode () == ModeTextureCopy)
    {

	float tx, ty;
	GLuint target;

	if (GL::textureNonPowerOfTwo ||
	    (POWER_OF_TWO (screen->width ()) && POWER_OF_TWO (screen->height ()) ) )
	{
	    target = GL_TEXTURE_2D;
	    tx = 1.0f / screen->width ();
	    ty = 1.0f / screen->height ();
	}
	else
	{
	    target = GL_TEXTURE_RECTANGLE_NV;
	    tx = 1;
	    ty = 1;
	}


	if (!texture)
	{
	    glGenTextures (1, &texture);
	    glBindTexture (target, texture);

	    glTexParameteri (target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri (target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	    glTexParameteri (target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	    glTexParameteri (target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	    glBindTexture (target, 0);
	}

	// blend motion blur texture to screen
	glPushAttrib (GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT | GL_VIEWPORT_BIT);
	glPushMatrix ();
	glLoadIdentity ();

	glViewport (0, 0, screen->width (), screen->height ());
	glTranslatef (-0.5f, -0.5f, -DEFAULT_Z_CAMERA);
	glScalef (1.0f / screen->width (), -1.0f / screen->height (), 1.0f);
	glTranslatef (0.0f, -screen->height (), 0.0f);
	glBindTexture (target, texture);
	glEnable (target);

	if (!update)
	{
	    glEnable (GL_BLEND);

	    glBlendFunc (GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
	    alpha = (timer / 500.0) *
		     alpha + (1.0 - (timer / 500.0) ) * 0.5;

	    glColor4f (1, 1, 1, alpha);

	    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	    glBegin (GL_QUADS);
	    glTexCoord2f (0, screen->height () * ty);
	    glVertex2f (0, 0);
	    glTexCoord2f (0, 0);
	    glVertex2f (0, screen->height ());
	    glTexCoord2f (screen->width () * tx, 0);
	    glVertex2f (screen->width (), screen->height ());
	    glTexCoord2f (screen->width () * tx, screen->height () * ty);
	    glVertex2f (screen->width (), 0);
	    glEnd ();

	    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	    glDisable (GL_BLEND);

	    // copy new screen to motion blur texture
	    glCopyTexSubImage2D (target, 0, 0, 0, 0, 0, screen->width (), screen->height ());
	}
	else
	{
	    glCopyTexImage2D (target, 0, GL_RGB, 0, 0,
			      screen->width (), screen->height (), 0);
	}

	glBindTexture (target, 0);

	glDisable (target);

	glPopMatrix ();
	glPopAttrib ();

	update = false;
	cScreen->damageScreen ();
    }

    if (active && optionGetMode () == ModeAccumulationBuffer)
    {

	// create motion blur effect using accumulation buffer
	alpha = (timer / 500.0) *
		 alpha + (1.0 - (timer / 500.0) ) * 0.5;

	if (update)
	{
	    glAccum (GL_LOAD, 1.0);
	}
	else
	{
	    glAccum (GL_MULT, 1.0 - alpha);
	    glAccum (GL_ACCUM, alpha);
	    glAccum (GL_RETURN, 1.0);
	}

	update = FALSE;

	cScreen->damageScreen ();
    }

    if (enable_scissor)
	glEnable (GL_SCISSOR_TEST);

   return status;

}

void
MblurScreen::glPaintTransformedOutput (const GLScreenPaintAttrib &attrib,
		       		       const GLMatrix	         &transform,
		       		       const CompRegion	         &region,
		       		       CompOutput	         *output,
		       		       unsigned int		    mask)
{
    if (optionGetOnTransformedScreen () &&
	(mask & PAINT_SCREEN_TRANSFORMED_MASK) )
    {
	toggleFunctions (true);
	active = TRUE;
	timer = 500;
    }

    gScreen->glPaintTransformedOutput (attrib, transform, region, output, mask);
}

MblurScreen::MblurScreen (CompScreen *screen) :
    PluginClassHandler <MblurScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    active (false),
    update (true),
    timer (500),
    activated (false),
    texture (0)
{
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);

    gScreen->glPaintTransformedOutputSetEnabled (this, true);

    optionSetInitiateKeyInitiate (boost::bind (&MblurScreen::toggle, this,
						_1, _2, _3));

    cScreen->damageScreen ();
}

MblurScreen::~MblurScreen ()
{
    if (texture)
	glDeleteTextures (1, &texture);
}

bool
MblurPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
	!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

    return true;
}
