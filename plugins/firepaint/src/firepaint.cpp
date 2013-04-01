/*
 * Compiz fire effect plugin
 *
 * firepaint.c
 *
 * Copyright : (C) 2007 by Dennis Kasprzyk
 * E-mail    : onestone@beryl-project.org
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

#include "firepaint.h"

COMPIZ_PLUGIN_20090315 (firepaint, FirePluginVTable);

const unsigned int NUM_ADD_POINTS = 1000;

Particle::Particle () :
    life (0),
    fade (0),
    width (0),
    height (0),
    w_mod (0),
    h_mod (0),
    r (0),
    g (0),
    b (0),
    a (0),
    x (0),
    y (0),
    z (0),
    xi (0),
    yi (0),
    zi (0),
    xg (0),
    yg (0),
    zg (0),
    xo (0),
    yo (0),
    zo (0)
{
}

ParticleSystem::ParticleSystem (int n)
{
    initParticles (n);
}

ParticleSystem::ParticleSystem ()
{
    initParticles (0);
}

ParticleSystem::~ParticleSystem ()
{
    finiParticles ();
}

void
ParticleSystem::initParticles (int            f_numParticles)
{
    particles.clear ();

    tex = 0;
    slowdown = 1;
    active = false;
    darken = 0;

    // Initialize cache
    vertices_cache.cache = NULL;
    colors_cache.cache   = NULL;
    coords_cache.cache   = NULL;
    dcolors_cache.cache  = NULL;

    vertices_cache.count  = 0;
    colors_cache.count   = 0;
    coords_cache.count  = 0;
    dcolors_cache.count = 0;

    vertices_cache.size  = 0;
    colors_cache.size   = 0;
    coords_cache.size  = 0;
    dcolors_cache.size = 0;

    int i;

    for (i = 0; i < f_numParticles; i++)
    {
	Particle p;
	p.life = 0.0f;
	particles.push_back (p);
    }
}

void
ParticleSystem::drawParticles ()
{
    GLfloat *dcolors;
    GLfloat *vertices;
    GLfloat *coords;
    GLfloat *colors;

    glPushMatrix ();

    glEnable (GL_BLEND);

    if (tex)
    {
	glBindTexture (GL_TEXTURE_2D, tex);
	glEnable (GL_TEXTURE_2D);
    }

    glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    /* Check that the cache is big enough */

    if (particles.size () > vertices_cache.count)
    {
	vertices_cache.cache = (GLfloat *) realloc (vertices_cache.cache,
						    particles.size () * 4 * 3 *
						    sizeof (GLfloat));
	vertices_cache.size = particles.size () * 4 * 3;
	vertices_cache.count = particles.size ();
    }

    if (particles.size () > coords_cache.count)
    {
	coords_cache.cache = (GLfloat *) realloc (coords_cache.cache,
						  particles.size () * 4 * 2 *
						  sizeof (GLfloat));
	coords_cache.size = particles.size () * 4 * 2;
	coords_cache.count = particles.size ();
    }

    if (particles.size () > colors_cache.count)
    {
	colors_cache.cache = (GLfloat *) realloc (colors_cache.cache,
						  particles.size () * 4 * 4 *
						  sizeof (GLfloat));
	colors_cache.size = particles.size () * 4 * 4;
	colors_cache.count = particles.size ();
    }

    if (darken > 0)
    {
	if (dcolors_cache.count < particles.size ())
	{
	    dcolors_cache.cache = (GLfloat *) realloc (dcolors_cache.cache,
						       particles.size () * 4 * 4 *
						       sizeof (GLfloat));
	    dcolors_cache.size = particles.size () * 4 * 4;
	    dcolors_cache.count = particles.size ();
	}
    }

    dcolors  = dcolors_cache.cache;
    vertices = vertices_cache.cache;
    coords   = coords_cache.cache;
    colors   = colors_cache.cache;

    int numActive = 0;

    foreach (Particle &part, particles)
    {
	if (part.life > 0.0f)
	{
	    numActive += 4;

	    float w = part.width / 2;
	    float h = part.height / 2;

	    w += (w * part.w_mod) * part.life;
	    h += (h * part.h_mod) * part.life;

	    vertices[0]  = part.x - w;
	    vertices[1]  = part.y - h;
	    vertices[2]  = part.z;

	    vertices[3]  = part.x - w;
	    vertices[4]  = part.y + h;
	    vertices[5]  = part.z;

	    vertices[6]  = part.x + w;
	    vertices[7]  = part.y + h;
	    vertices[8]  = part.z;

	    vertices[9]  = part.x + w;
	    vertices[10] = part.y - h;
	    vertices[11] = part.z;

	    vertices += 12;

	    coords[0] = 0.0;
	    coords[1] = 0.0;

	    coords[2] = 0.0;
	    coords[3] = 1.0;

	    coords[4] = 1.0;
	    coords[5] = 1.0;

	    coords[6] = 1.0;
	    coords[7] = 0.0;

	    coords += 8;

	    colors[0]  = part.r;
	    colors[1]  = part.g;
	    colors[2]  = part.b;
	    colors[3]  = part.life * part.a;
	    colors[4]  = part.r;
	    colors[5]  = part.g;
	    colors[6]  = part.b;
	    colors[7]  = part.life * part.a;
	    colors[8]  = part.r;
	    colors[9]  = part.g;
	    colors[10] = part.b;
	    colors[11] = part.life * part.a;
	    colors[12] = part.r;
	    colors[13] = part.g;
	    colors[14] = part.b;
	    colors[15] = part.life * part.a;

	    colors += 16;

	    if (darken > 0)
	    {

		dcolors[0]  = part.r;
		dcolors[1]  = part.g;
		dcolors[2]  = part.b;
		dcolors[3]  = part.life * part.a * darken;
		dcolors[4]  = part.r;
		dcolors[5]  = part.g;
		dcolors[6]  = part.b;
		dcolors[7]  = part.life * part.a * darken;
		dcolors[8]  = part.r;
		dcolors[9]  = part.g;
		dcolors[10] = part.b;
		dcolors[11] = part.life * part.a * darken;
		dcolors[12] = part.r;
		dcolors[13] = part.g;
		dcolors[14] = part.b;
		dcolors[15] = part.life * part.a * darken;

		dcolors += 16;
	    }
	}
    }

    glEnableClientState (GL_COLOR_ARRAY);

    glTexCoordPointer (2, GL_FLOAT, 2 * sizeof (GLfloat), coords_cache.cache);
    glVertexPointer (3, GL_FLOAT, 3 * sizeof (GLfloat), vertices_cache.cache);

    // darken the background

    if (darken > 0)
    {
	glBlendFunc (GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
	glColorPointer (4, GL_FLOAT, 4 * sizeof (GLfloat), dcolors_cache.cache);
	glDrawArrays (GL_QUADS, 0, numActive);
    }

    // draw particles
    glBlendFunc (GL_SRC_ALPHA, blendMode);

    glColorPointer (4, GL_FLOAT, 4 * sizeof (GLfloat), colors_cache.cache);
    glDrawArrays (GL_QUADS, 0, numActive);
    glDisableClientState (GL_COLOR_ARRAY);

    glPopMatrix ();
    glColor4usv (defaultColor);

    GLScreen::get(screen)->setTexEnvMode (GL_REPLACE); // ???

    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable (GL_TEXTURE_2D);
    glDisable (GL_BLEND);
}

void
ParticleSystem::updateParticles (float          time)
{
    float speed = (time / 50.0);
    float f_slowdown = slowdown * (1 - MAX (0.99, time / 1000.0) ) * 1000;

    active = false;

    foreach (Particle &part, particles)
    {
	if (part.life > 0.0f)
	{
	    // move particle
	    part.x += part.xi / f_slowdown;
	    part.y += part.yi / f_slowdown;
	    part.z += part.zi / f_slowdown;

	    // modify speed
	    part.xi += part.xg * speed;
	    part.yi += part.yg * speed;
	    part.zi += part.zg * speed;

	    // modify life
	    part.life -= part.fade * speed;
	    active = true;
	}
    }
}

void
ParticleSystem::finiParticles ()
{
    particles.clear ();

    if (tex)
	glDeleteTextures (1, &tex);

    if (vertices_cache.cache)
    {
	free (vertices_cache.cache);
	vertices_cache.cache = NULL;
    }

    if (colors_cache.cache)
    {
	free (colors_cache.cache);
	colors_cache.cache = NULL;
    }

    if (coords_cache.cache)
    {
	free (coords_cache.cache);
	coords_cache.cache = NULL;
    }

    if (dcolors_cache.cache)
    {
	free (dcolors_cache.cache);
	dcolors_cache.cache = NULL;
    }
}

static void
toggleFunctions (bool enabled)
{
    FIRE_SCREEN (screen);
    screen->handleEventSetEnabled (fs, enabled);
    fs->cScreen->preparePaintSetEnabled (fs, enabled);
    fs->gScreen->glPaintOutputSetEnabled (fs, enabled);
    fs->cScreen->donePaintSetEnabled (fs, enabled);
}

void
FireScreen::fireAddPoint (int        x,
		          int        y,
		          bool       requireGrab)
{

    if (!requireGrab || grabIndex)
    {
	XPoint p;

	p.x = x;
	p.y = y;

	points.push_back (p);

	toggleFunctions (true);

    }

}


bool
FireScreen::addParticle (CompAction         *action,
			 CompAction::State  state,
			 CompOption::Vector options)
{
    float x, y;

    x = CompOption::getFloatOptionNamed (options, "x", 0);
    y = CompOption::getFloatOptionNamed (options, "y", 0);

    fireAddPoint (x, y, false);

    cScreen->damageScreen ();

    return true;
}


bool
FireScreen::initiate (CompAction         *action,
		      CompAction::State  state,
		      CompOption::Vector options)
{
    if (screen->otherGrabExist (NULL))
        return false;

    if (!grabIndex)
        grabIndex = screen->pushGrab (None, "firepaint");

    if (state & CompAction::StateInitButton)
        action->setState (action->state () | CompAction::StateTermButton);

    if (state & CompAction::StateInitKey)
        action->setState (action->state () | CompAction::StateTermKey);

    fireAddPoint (pointerX, pointerY, true);

    return true;
}

bool
FireScreen::terminate (CompAction         *action,
		       CompAction::State  state,
		       CompOption::Vector options)
{

    if (grabIndex)
    {
	screen->removeGrab (grabIndex, NULL);
	grabIndex = 0;
    }

    action->setState (action->state () & ~(CompAction::StateTermKey |
				           CompAction::StateTermButton));

    return false;
}


bool
FireScreen::clear (CompAction         *action,
		   CompAction::State  state,
		   CompOption::Vector options)
{
    points.clear ();
    return true;
}


void
FireScreen::preparePaint (int      time)
{
    float bg = (float) optionGetBgBrightness () / 100.0;

    if (init && !points.empty ())
    {
	ps.initParticles (optionGetNumParticles ());
	init = false;

	glGenTextures (1, &ps.tex);
	glBindTexture (GL_TEXTURE_2D, ps.tex);

	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0,
		      GL_RGBA, GL_UNSIGNED_BYTE, fireTex);
	glBindTexture (GL_TEXTURE_2D, 0);

	ps.slowdown  = optionGetFireSlowdown ();
	ps.darken    = 0.5;
	ps.blendMode = GL_ONE;

    }

    if (!init)
	ps.updateParticles (time);

    if (!points.empty ())
    {
	float max_new = MIN ((int) ps.particles.size (),  (int) points.size () * 2) *
			((float) time / 50.0) *
			(1.05 -	optionGetFireLife());
	float rVal, size = 4;
	int rVal2;

	for (unsigned int i = 0;
	     i < ps.particles.size () && max_new > 0; i++)
	{
	    Particle &part = ps.particles.at (i);
	    if (part.life <= 0.0f)
	    {
		/* give gt new life */
		rVal = (float) (random () & 0xff) / 255.0;
		part.life = 1.0f;
		/* Random Fade Value */
		part.fade = (rVal * (1 - optionGetFireLife ()) +
			      (0.2f * (1.01 - optionGetFireLife ())));

		/* set size */
		part.width  = optionGetFireSize ();
		part.height = optionGetFireSize () * 1.5;
		rVal = (float) (random () & 0xff) / 255.0;
		part.w_mod = size * rVal;
		part.h_mod = size * rVal;

		/* choose random position */
		rVal2 = random () % points.size ();
		part.x = points.at (rVal2).x;
		part.y = points.at (rVal2).y;
		part.z = 0.0;
		part.xo = part.x;
		part.yo = part.y;
		part.zo = part.z;

		/* set speed and direction */
		rVal = (float) (random () & 0xff) / 255.0;
		part.xi = ( (rVal * 20.0) - 10.0f);
		rVal = (float) (random () & 0xff) / 255.0;
		part.yi = ( (rVal * 20.0) - 15.0f);
		part.zi = 0.0f;
		rVal = (float) (random () & 0xff) / 255.0;

		if (optionGetFireMystical () )
		{
		    /* Random colors! (aka Mystical Fire) */
		    rVal = (float) (random () & 0xff) / 255.0;
		    part.r = rVal;
		    rVal = (float) (random () & 0xff) / 255.0;
		    part.g = rVal;
		    rVal = (float) (random () & 0xff) / 255.0;
		    part.b = rVal;
		}
		else
		{
		    part.r = (float) optionGetFireColorRed () / 0xffff -
			      (rVal / 1.7 *
			       (float) optionGetFireColorRed () / 0xffff);
		    part.g = (float) optionGetFireColorGreen () / 0xffff -
			      (rVal / 1.7 *
			      (float) optionGetFireColorGreen () / 0xffff);
		    part.b = (float) optionGetFireColorBlue () / 0xffff -
			      (rVal / 1.7 *
			      (float) optionGetFireColorBlue () / 0xffff);
		}

		/* set transparancy */
		part.a = (float) optionGetFireColorAlpha () / 0xffff;

		/* set gravity */
		part.xg = (part.x < part.xo) ? 1.0 : -1.0;
		part.yg = -3.0f;
		part.zg = 0.0f;

		ps.active = true;

		max_new -= 1;
	    }
	    else
	    {
		part.xg = (part.x < part.xo) ? 1.0 : -1.0;
	    }
	}
    }

    if (points.size () && brightness != bg)
    {
	float div = 1.0 - bg;
	div *= (float) time / 500.0;
	brightness = MAX (bg, brightness - div);

    }

    if (points.empty () && brightness != 1.0)
    {
	float div = 1.0 - bg;
	div *= (float) time / 500.0;
	brightness = MIN (1.0, brightness + div);

    }

    if (!init && points.empty () && !ps.active)
    {
	ps.finiParticles ();
	init = true;
    }

    cScreen->preparePaint (time);
}

bool
FireScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
			   const GLMatrix	     &transform,
			   const CompRegion	     &region,
			   CompOutput 		     *output,
			   unsigned int		     mask)
{
    bool status;

    status = gScreen->glPaintOutput (attrib, transform, region, output, mask);

    if ( (!init && ps.active) || brightness < 1.0)
    {
	GLMatrix sTransform = transform;

	sTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);

	glPushMatrix ();
	glLoadMatrixf (sTransform.getMatrix ());

	if (brightness < 1.0)
	{
	    glColor4f (0.0, 0.0, 0.0, 1.0 - brightness);
	    glEnable (GL_BLEND);
	    glBegin (GL_QUADS);
	    glVertex2d (output->region ()->extents.x1,
			output->region ()->extents.y1);
	    glVertex2d (output->region ()->extents.x1,
			output->region ()->extents.y2);
	    glVertex2d (output->region ()->extents.x2,
			output->region ()->extents.y2);
	    glVertex2d (output->region ()->extents.x2,
			output->region ()->extents.y1);
	    glEnd ();
	    glDisable (GL_BLEND);
	    glColor4usv (defaultColor);
	}

	if (!init && ps.active)
	    ps.drawParticles ();

	glPopMatrix ();
    }

    return status;
}



void
FireScreen::donePaint ()
{
    if ( (!init && ps.active) || !points.empty () || brightness < 1.0)
    {
	cScreen->damageScreen ();
    }
    else
	toggleFunctions (false);

    cScreen->donePaint ();
}

void
FireScreen::handleEvent (XEvent *event)
{
    switch (event->type)
    {

    case MotionNotify:
	fireAddPoint (pointerX, pointerY, true);
	break;

    case EnterNotify:
    case LeaveNotify:
	fireAddPoint (pointerX, pointerY, true);
    default:
	break;
    }

    screen->handleEvent (event);
}

FireScreen::FireScreen (CompScreen *screen) :
    PluginClassHandler <FireScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    init (true),
    brightness (1.0),
    grabIndex (0)
{
    ScreenInterface::setHandler (screen, false);
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);

    optionSetInitiateKeyInitiate (boost::bind (&FireScreen::initiate, this, _1,
						_2, _3));
    optionSetInitiateButtonInitiate (boost::bind (&FireScreen::initiate, this,
						   _1, _2, _3));
    optionSetInitiateKeyTerminate (boost::bind (&FireScreen::terminate, this,
						 _1, _2, _3));
    optionSetInitiateButtonTerminate (boost::bind (&FireScreen::terminate, this,
						   _1, _2, _3));

    optionSetClearKeyInitiate (boost::bind (&FireScreen::clear, this, _1, _2,
					    _3));
    optionSetClearButtonInitiate (boost::bind (&FireScreen::clear, this, _1, _2,
					       _3));

    optionSetAddParticleInitiate (boost::bind (&FireScreen::addParticle, this,
						_1, _2, _3));
}

FireScreen::~FireScreen ()
{
    if (!init)
	ps.finiParticles ();
}

bool
FirePluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
	!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

    return true;
}
