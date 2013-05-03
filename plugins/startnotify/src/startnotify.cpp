/*
 *
 * Compiz show mouse pointer plugin
 *
 * showmouse.cpp
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

#include <X11/extensions/Xfixes.h>

#include "startnotify.h"

COMPIZ_PLUGIN_20090315 (startnotify, StartnotifyPluginVTable);

AnimCursor::AnimCursor()
{
    initAnimCursor ();
}

AnimCursor::~AnimCursor()
{
    finiAnimCursor();
}

void
AnimCursor::initAnimCursor ()
{
    animTex = 0;
    animTexIndex = 0;
    x = 0;
    y = 0;
    //read Texture image file
    CompSize size;
    CompString file (CURSOR_NAME);
    CompString pname (PLUGIN_NAME);
    animTex = GLTexture::readImageToTexture (file, pname, size);
    if (animTex.size())
        printf ("image load success\n");

    //set to true only we have a binding texture
    active = true;
}

void
AnimCursor::drawAnimCursor (const GLMatrix &transform)
{
    printf ("AnimCursor::drawAnimCursor: x = %lf, y = %lf\n", x, y);
    glEnable (GL_BLEND);

    if (animTex.size ())
    {
    foreach (GLTexture* tex, animTex)
    {
        tex->enable (GLTexture::Good);
        //calculate cursor gemoetry and texture coordinates
        //1. cursor vertices
        //first triangle
        cursor_vertices[0] = x;
        cursor_vertices[1] = y;
        cursor_vertices[2] = 0;

        cursor_vertices[3] = x;
        cursor_vertices[4] = y + CURSOR_HEIGHT;
        cursor_vertices[5] = 0;

        cursor_vertices[6] = x + CURSOR_WIDTH;
        cursor_vertices[7] = y + CURSOR_HEIGHT;
        cursor_vertices[8] = 0;

        //second triangle
        cursor_vertices[9] = x + CURSOR_WIDTH;
        cursor_vertices[10] = y + CURSOR_HEIGHT;
        cursor_vertices[11] = 0;

        cursor_vertices[12] = x + CURSOR_WIDTH;
        cursor_vertices[13] = y;
        cursor_vertices[14] = 0;

        cursor_vertices[15] = x;
        cursor_vertices[16] = y;
        cursor_vertices[17] = 0;
        //2. corresponding texture coordinates
        //FIXME:
        //first surface
        //GLfloat index = (GLfloat) animTexIndex;
        //GLfloat num = (GLfloat) CURSOR_NUM;
        cursor_texcoords[0] = 0.0;
        cursor_texcoords[1] = 0.0;
        //cursor_texcoords[1] = index / num;

        cursor_texcoords[2] = 0.0;
        cursor_texcoords[3] = 1.0;
        //cursor_texcoords[3] = (index + 1) / num;

        cursor_texcoords[4] = 1.0;
        cursor_texcoords[5] = 1.0;
        //cursor_texcoords[5] = (index + 1) / num;

        //second surface
        cursor_texcoords[6] = 1.0;
        cursor_texcoords[7] = 1.0;
        //cursor_texcoords[7] = (index + 1) / num;

        cursor_texcoords[8] = 1.0;
        cursor_texcoords[9] = 0.0;
        //cursor_texcoords[9] = index / num;

        cursor_texcoords[10] = 0.0;
        cursor_texcoords[11] = 0.0;
        //cursor_texcoords[11] = index / num;

        GLVertexBuffer *stream = GLVertexBuffer::streamingBuffer ();

        // draw the cursor
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        stream->begin (GL_TRIANGLES);

        stream->addVertices (6, cursor_vertices);
        stream->addTexCoords (0, 6, cursor_texcoords);

        if (stream->end ())
            stream->render (transform);

        tex->disable ();
    }
    }

    glDisable (GL_BLEND);
    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void
AnimCursor::finiAnimCursor ()
{
}

static void
toggleFunctions (bool enabled)
{
    STARTNOTIFY_SCREEN (screen);
    ss->cScreen->preparePaintSetEnabled (ss, enabled);
    ss->gScreen->glPaintOutputSetEnabled (ss, enabled);
    ss->cScreen->donePaintSetEnabled (ss, enabled);
}

void
StartnotifyScreen::doDamageRegion ()
{
    //printf ("StartnotifyScreen::doDamageRegion\n");

    CompRegion r (animCursor.x, animCursor.y,
				  CURSOR_WIDTH, CURSOR_HEIGHT);

    cScreen->damageRegion (r);
}

void
StartnotifyScreen::positionUpdate (const CompPoint &p)
{
    //printf("StartnotifyScreen::positionUpdate\n");
    mousePos = p;
    //printf ("updateAnimCursor: %lfs\n", f_time/1000.0);
    animCursor.x = mousePos.x() - CURSOR_HOTSPOT_X;
    animCursor.y = mousePos.y() - CURSOR_HOTSPOT_Y;
}

void
StartnotifyScreen::preparePaint (int f_time)
{
    //printf ("StartnotifyScreen::preparePaint\n");
    if (active && !pollHandle.active ())
    {
        mousePos = MousePoller::getCurrentPosition ();
        pollHandle.start ();
    }

    if (active && !animCursor.active)
    {
        printf("load texture\n");
        animCursor.initAnimCursor ();
        animCursor.active = true;
    }

    if (animCursor.active)
    {

        GLuint n_frames = (GLuint)(CURSOR_FPS * f_time / 1000.0);
        animCursor.animTexIndex = (animCursor.animTexIndex + n_frames) % CURSOR_NUM;

        doDamageRegion ();
    }

    cScreen->preparePaint (f_time);
}

void
StartnotifyScreen::donePaint ()
{

    if (active || (animCursor.active))
        doDamageRegion ();

    if (!active && pollHandle.active ())
    {
        pollHandle.stop ();
    }

    if (!active && !animCursor.active)
    {
        animCursor.finiAnimCursor ();
        toggleFunctions (false);
    }

    //printf("StartnotifyScreen::donePaint\n");
    cScreen->donePaint ();
}

bool
StartnotifyScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
				const GLMatrix		  &transform,
				const CompRegion	  &region,
				CompOutput		  *output,
				unsigned int		  mask)
{
    GLMatrix       sTransform = transform;

    bool status = gScreen->glPaintOutput (attrib, transform, region, output, mask);

    if (!animCursor.active)
        return status;

    sTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);

    animCursor.drawAnimCursor (sTransform);

    //printf("StartnotifyScreen::glPaintOutput\n");
    return status;
}

bool
StartnotifyScreen::terminate (CompAction         *action,
			    CompAction::State  state,
			    CompOption::Vector options)
{
    active = false;
    animCursor.active = false;

    doDamageRegion ();

    gScreen->glPaintOutputSetEnabled (gScreen, false);

    printf("StartnotifyScreen::terminate\n");
    return true;
}

bool
StartnotifyScreen::initiate (CompAction         *action,
			   CompAction::State  state,
			   CompOption::Vector options)
{
    if (active)
        return terminate (action, state, options);

    active = true;
    animCursor.initAnimCursor();

    toggleFunctions (true);

    gScreen->glPaintOutputSetEnabled (gScreen, true);

    printf("StartnotifyScreen::initiate\n");
    return true;
}

StartnotifyScreen::StartnotifyScreen (CompScreen *screen) :
    PluginClassHandler <StartnotifyScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    active (false)
{
    printf("StartnotifyScreen::StartnotifyScreen\n");
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);

    pollHandle.setCallback (boost::bind (&StartnotifyScreen::positionUpdate, this,
					 _1));

    optionSetInitiateInitiate (boost::bind (&StartnotifyScreen::initiate, this,
					    _1, _2, _3));
    optionSetInitiateTerminate (boost::bind (&StartnotifyScreen::terminate, this,
					     _1, _2, _3));

    optionSetInitiateButtonInitiate (boost::bind (&StartnotifyScreen::initiate,
						  this,  _1, _2, _3));
    optionSetInitiateButtonTerminate (boost::bind (&StartnotifyScreen::terminate,
						   this,  _1, _2, _3));

    optionSetInitiateEdgeInitiate (boost::bind (&StartnotifyScreen::initiate,
						this,  _1, _2, _3));
    optionSetInitiateEdgeTerminate (boost::bind (&StartnotifyScreen::terminate,
						 this,  _1, _2, _3));

    animCursor.initAnimCursor ();
}

StartnotifyScreen::~StartnotifyScreen ()
{
    printf("StartnotifyScreen::~StartnotifyScreen\b");
    animCursor.finiAnimCursor ();

    if (pollHandle.active ())
        pollHandle.stop ();
}

bool
StartnotifyPluginVTable::init ()
{
    printf("StartnotifyPluginVTable::init\n");
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
	!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI) ||
	!CompPlugin::checkPluginABI ("mousepoll", COMPIZ_MOUSEPOLL_ABI))
	return false;

    return true;
}
