/*
 *
 * Compiz trip plugin
 *
 * trip.c
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

#include "trip.h"

COMPIZ_PLUGIN_20090315 (trip, TripPluginVTable);

void
TripScreen::cleanup ()
{
    if (program)
    {
	GL::deletePrograms (1, &program);
	program = 0;
    }
}

bool
TripScreen::loadFragmentProgram ()
{
    char  buffer[1024];
    GLsizei bufSize;
    GLint errorPos;

    if (!GL::fragmentProgram)
	return false;

    if (target == GL_TEXTURE_2D)
	sprintf (buffer, rippleFpString, "2D");
    else
	sprintf (buffer, rippleFpString, "RECT");

    /* clear errors */
    glGetError ();

    if (!program)
	GL::genPrograms (1, &program);

    bufSize = (GLsizei) strlen (buffer);

    GL::bindProgram (GL_FRAGMENT_PROGRAM_ARB, program);
    GL::programString (GL_FRAGMENT_PROGRAM_ARB,
			 GL_PROGRAM_FORMAT_ASCII_ARB,
			 bufSize, buffer);

    glGetIntegerv (GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
    if (glGetError () != GL_NO_ERROR || errorPos != -1)
    {
	compLogMessage ("trip", CompLogLevelError,
			"failed to load fragment program");

	GL::deletePrograms (1, &program);
	program = 0;

	return false;
    }
    GL::bindProgram (GL_FRAGMENT_PROGRAM_ARB, 0);

    return true;
}

void
TripScreen::optionChanged (CompOption	*opt,
			   Options	num)
{
    cleanup ();
    loadFragmentProgram ();

    quiet = true;

    cScreen->damageScreen ();
}

int
TripScreen::adjustZoom (float chunk, Ripple &r)
{
    float dx, adjust, amount;
    float change;

    dx = r.zTarget - r.zoom;

    adjust = dx * 0.15f;
    amount = fabs(dx) * 1.5f;
    if (amount < 0.2f)
	amount = 0.2f;
    else if (amount > 2.0f)
	amount = 2.0f;

    r.zVelocity = (amount * r.zVelocity + adjust) / (amount + 1.0f);

    if (fabs (dx) < 0.002f && fabs (r.zVelocity) < 0.004f)
    {
	r.zVelocity = 0.0f;
	r.zoom = r.zTarget;
	return false;
    }

    change = r.zVelocity * chunk;
    if (!change)
    {
	if (r.zVelocity)
	    change = (dx > 0) ? 0.01 : -0.01;
    }

    r.zoom += change;

    return true;
}

void
TripScreen::preparePaint (int        time)
{
    /* Be careful not to allow too much intensity.
     * Otherwise, we might have a bad trip ;-) */
    if (intensity > 70)
	intensity = 70;

    for (unsigned int i = 0; i < ripples.size (); i++)
    {
	int   steps;
	float amount, chunk;

	amount = time * 0.35f * optionGetSpeed ();
	steps  = amount / (0.5f * optionGetTimestep ());

	if (!steps)
	    steps = 1;

	chunk  = amount / (float) steps;

	while (steps--)
	{
	    ripples.at (i).adjust = adjustZoom (chunk, ripples.at (i));
	    if (ripples.at (i).adjust)
		break;
	}

	TRIP_SCREEN (screen);

	/* Compute a 0.0 - 1.0 representation of the animation timeline */
	float progress = (float) (ripples.at (i).duration -
					ripples.at (i).timer) /
					    (float) ripples.at (i).duration;
	if (progress <= 0.5f)
	{
	    ripples.at (i).timer -= (ts->quiet ? (time * 2) : (time / 4));
	    ripples.at (i).zTarget = (MIN (10.0, (progress * 2) * 10.0)) + 1.0;
	}
	else
	{	ripples.at (i).timer -= (ts->quiet ? (time * 3) : (time / 5));
	    ripples.at (i).zTarget = (MIN (10.0, (2.0 - (progress * 2)) * 10.0)) + 1.0;
	}

	if (ts->quiet)
	{
	    intensity *= 0.8;

	    if (ripples.at (i).timer > 8000)
		ripples.at (i).timer *= 0.8;
	}

	ripples.at (i).zTarget *= (intensity * 0.01);
	ripples.at (i).radius += ripples.at (i).rMod;
    }
    cScreen->preparePaint (time);
    cScreen->damageScreen ();
}

void
TripScreen::donePaint ()
{
    glEnable (target);

    glBindTexture (target, texture);

    glTexImage2D (target, 0, GL_RGB, 0, 0, 0,
		  GL_RGB, GL_UNSIGNED_BYTE, NULL);
    
    glBindTexture (target, 0);

    glDisable (target);

    for (unsigned int i = 0; i < ripples.size (); i++)
    {
	ripples.at (i).width = 0;
	ripples.at (i).height = 0;

	if (ripples.at (i).zoom <= 1.0)
	{
	    if (!quiet)
		ripples.at (i). spawnRandom ();
	    else
		ripples.erase (ripples.begin () + i);
	}
    }

    if (ripples.empty ())
    {
	ripples.clear ();
	cScreen->preparePaintSetEnabled (this, false);
	cScreen->donePaintSetEnabled (this, false);
	gScreen->glPaintOutputSetEnabled (this, false);
    }

    cScreen->damageScreen ();

    cScreen->donePaint ();
}

void
Ripple::paint ()
{

    TRIP_SCREEN (screen);

    float	pw, ph;
    float	fZoom, base;
    int		x1, x2, y1, y2;
    float	vc[4];
    int		size;

    width = height = 0;

    base   = 0.5 + (0.0015 * radius);
    fZoom   = (zoom * base) + 1.0 - base;

    size = radius + 1;

    x1 = MAX (0.0, coord.x () - size);
    x2 = MIN (screen->width (), coord.x () + size);
    y1 = MAX (0.0, coord.y () - size);
    y2 = MIN (screen->height (), coord.y () + size);
 
    glEnable (ts->target);

    glBindTexture (ts->target, ts->texture);

    if (width != 2 * size || height != 2 * size)
    {
	glCopyTexImage2D(ts->target, 0, GL_RGB, x1, screen->height () - y2,
			 size * 2, size * 2, 0);
	width = height = 2 * size;
    }
    else
	glCopyTexSubImage2D (ts->target, 0, 0, 0,
			     x1, screen->height () - y2, x2 - x1, y2 - y1);

    if (ts->target == GL_TEXTURE_2D)
    {
	pw = 1.0 / width;
	ph = 1.0 / height;
    }
    else
    {
	pw = 1.0;
	ph = 1.0;
    }
    
    glMatrixMode (GL_PROJECTION);
    glPushMatrix ();
    glLoadIdentity ();
    glMatrixMode (GL_MODELVIEW);
    glPushMatrix ();
    glLoadIdentity ();

    glColor4usv (defaultColor);

    glEnable (GL_FRAGMENT_PROGRAM_ARB);
    GL::bindProgram (GL_FRAGMENT_PROGRAM_ARB, ts->program);

    GL::programEnvParameter4f (GL_FRAGMENT_PROGRAM_ARB, 0,
				 coord.x (), screen->height () - coord.y (),
				 1.0 / radius, 0.0f);
    GL::programEnvParameter4f (GL_FRAGMENT_PROGRAM_ARB, 1,
				 pw, ph, M_PI / radius,
				 (fZoom - 1.0) * fZoom);
    GL::programEnvParameter4f (GL_FRAGMENT_PROGRAM_ARB, 2,
				 -x1 * pw, -(screen->height () - y2) * ph,
				 -M_PI / 2.0, 0.0);

    x1 = MAX (0.0, coord.x () - radius);
    x2 = MIN (screen->width (), coord.x () + radius);
    y1 = MAX (0.0, coord.y () - radius);
    y2 = MIN (screen->height (), coord.y () + radius);

    vc[0] = ((x1 * 2.0) / screen->width ()) - 1.0;
    vc[1] = ((x2 * 2.0) / screen->width ()) - 1.0;
    vc[2] = ((y1 * -2.0) / screen->height ()) + 1.0;
    vc[3] = ((y2 * -2.0) / screen->height ()) + 1.0;

    y1 = screen->height () - y1;
    y2 = screen->height () - y2;

    glBegin (GL_QUADS);
    glTexCoord2f (x1, y1);
    glVertex2f (vc[0], vc[2]);
    glTexCoord2f (x1, y2);
    glVertex2f (vc[0], vc[3]);
    glTexCoord2f (x2, y2);
    glVertex2f (vc[1], vc[3]);
    glTexCoord2f (x2, y1);
    glVertex2f (vc[1], vc[2]);
    glEnd ();

    glDisable (GL_FRAGMENT_PROGRAM_ARB);

    glColor4usv (defaultColor);
    
    glPopMatrix();
    glMatrixMode (GL_PROJECTION);
    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);

    glBindTexture (ts->target, 0);

    glDisable (ts->target);
}


bool
TripScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
			  const GLMatrix	    &transform,
			  const CompRegion	    &region,
			  CompOutput	            *output,
			  unsigned int	            mask)
{
    bool status = gScreen->glPaintOutput (attrib, transform, region, output, mask);

    if (ripples.empty ())
	return status;

    /* Temporarily set the viewport to fullscreen */
    glViewport (0, 0, screen->width (), screen->height ());

    for (unsigned int i = 0; i < ripples.size (); i++)
	ripples.at (i).paint ();

    gScreen->setDefaultViewport ();

    return status;
}

void
Ripple::spawnRandom ()
{
    TRIP_SCREEN (screen);

    ts->cleanup ();
    ts->loadFragmentProgram ();

    radius = MAX (100, (rand () % ts->optionGetMaxRadius ()));
    zoom = 1.0f;

    rMod = (rand () % 3);

    coord.setX (rand () % screen->width ());
    coord.setY (rand () % screen->height ());


    width = 0;
    height = 0;

    int x, y, w, h;

    x = MAX (0.0, coord.x () - radius);
    y = MAX (0.0, coord.y () - radius);
    w = MIN (screen->width (), coord.x () + radius) - x;
    h = MIN (screen->height (), coord.y () + radius) - y;

    damageRect.setGeometry (x, y, w, h);

    zTarget = MAX (1.0, MIN (10.0, (rand () % 10)));

    duration = MAX(3000, (rand () % (ts->optionGetMaxDuration () * 1000)));
    timer = duration;
    adjust  = true;

}

void
TripScreen::populateRippleSet ()
{

    ripples.clear ();
    intensity = 30;
    for (int i = 0; i < optionGetMaxRipples (); i++)
    {
	ripples.push_back (Ripple ());
	ripples.at (i). spawnRandom ();
    }
}

bool
TripScreen::takeHit (CompAction	  *action,
		     CompAction::State   state,
		     CompOption::Vector options)
{
    intensity += 5;

    if (quiet)
	populateRippleSet ();

    quiet = false;

    /* Trip mode starting */
    cScreen->preparePaintSetEnabled (this, true);
    cScreen->donePaintSetEnabled (this, true);
    gScreen->glPaintOutputSetEnabled (this, true);
    return true;
}

bool
TripScreen::untensify (CompAction	  *action,
		       CompAction::State   state,
		       CompOption::Vector options)
{
    intensity -= 5;

    if (intensity < 15)
	quiet = true;

    return true;
}

bool
TripScreen::intensify (CompAction	  *action,
		       CompAction::State   state,
		       CompOption::Vector options)
{
    intensity += 2;
    cScreen->damageScreen ();

    if (quiet)
	populateRippleSet ();

    quiet = false;
 
    /* Trip mode starting */
    cScreen->preparePaintSetEnabled (this, true);
    cScreen->donePaintSetEnabled (this, true);
    gScreen->glPaintOutputSetEnabled (this, true);
    return true;
}

bool
TripScreen::soberUp (CompAction	  *action,
		     CompAction::State   state,
		     CompOption::Vector options)
{
    /* Time to end ripples quickly */
    quiet = true;

    intensity -= 5;

    cScreen->damageScreen ();

    return true;
}

TripScreen::TripScreen (CompScreen *screen) :
    PluginClassHandler <TripScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    program (0),
    quiet (false),
    intensity (25)
{
    ScreenInterface::setHandler (screen, false);
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);


    glGenTextures (1, &texture);

    if (GL::textureNonPowerOfTwo)
	target = GL_TEXTURE_2D;
    else
	target = GL_TEXTURE_RECTANGLE_ARB;

    glEnable (target);

    /* Bind the texture */
    glBindTexture (target, texture);

    /* Load the parameters */
    glTexParameteri (target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (target, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (target, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D (target, 0, GL_RGB, 0, 0, 0,
		  GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glBindTexture (target, 0);

    glDisable (target);

    optionSetMaxRadiusNotify (boost::bind (&TripScreen::optionChanged, this, _1, _2));
    optionSetMaxRipplesNotify (boost::bind (&TripScreen::optionChanged, this, _1, _2));
    optionSetMaxDurationNotify (boost::bind (&TripScreen::optionChanged, this, _1, _2));
    optionSetZoomFactorNotify (boost::bind (&TripScreen::optionChanged, this, _1, _2));
    optionSetSpeedNotify (boost::bind (&TripScreen::optionChanged, this, _1, _2));
    optionSetTimestepNotify (boost::bind (&TripScreen::optionChanged, this, _1, _2));

    optionSetTakeHitInitiate (boost::bind (&TripScreen::takeHit, this, _1, _2,
						_3));

    optionSetDecreaseIntensityInitiate (boost::bind (&TripScreen::untensify, this, _1, _2,
						_3));

    optionSetIncreaseIntensityInitiate (boost::bind (&TripScreen::intensify, this, _1, _2,
						_3));

    optionSetSoberKeyInitiate (boost::bind (&TripScreen::soberUp, this, _1, _2,
						_3));

    optionSetSoberButtonInitiate (boost::bind (&TripScreen::soberUp, this, _1, _2,
						_3));

    populateRippleSet ();

    if (!GL::fragmentProgram || !loadFragmentProgram ())
	compLogMessage ("trip", CompLogLevelWarn,
			"GL_ARB_fragment_program not supported. "
			"This plugin will not work.");
}

TripScreen::~TripScreen ()
{
    cScreen->damageScreen ();

    glDeleteTextures (1, &target);
    
    ripples.clear ();
}

Ripple::Ripple () :    
    dScreen (TripScreen::get (screen))
{
}

Ripple::~Ripple ()
{
}

bool
TripPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
	!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

    return true;
}
