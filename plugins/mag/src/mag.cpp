/*
 *
 * Compiz magnifier plugin
 *
 * mag.c
 *
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

#include "mag.h"

COMPIZ_PLUGIN_20090315 (mag, MagPluginVTable);

void
MagScreen::cleanup ()
{
    if (overlay.size ())
    {
	overlay.clear ();
    }
    if (mask.size ())
    {
	mask.clear ();
    }

    if (program)
    {
#if 0
	GL::deletePrograms (1, &program);
#endif
	program = 0;
    }
}

bool
MagScreen::loadFragmentProgram ()
{
#if 0
    char  buffer[1024];
    GLsizei bufSize;
    GLint errorPos;

    if (!GL::fragmentProgram)
	return false;

    if (target == GL_TEXTURE_2D)
	sprintf (buffer, fisheyeFpString, "2D");
    else
	sprintf (buffer, fisheyeFpString, "RECT");

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
	compLogMessage ("mag", CompLogLevelError,
			"failed to load fisheye fragment program");

	GL::deletePrograms (1, &program);
	program = 0;

	return false;
    }
    GL::bindProgram (GL_FRAGMENT_PROGRAM_ARB, 0);

    return true;
#endif
    return false;
}

bool
MagScreen::loadImages ()
{
#if 0
    CompString overlay_s = optionGetOverlay ();
    CompString mask_s = optionGetMask ();
    CompString pname ("mag");
    if (!GL::multiTexCoord2f)
	return false;

    overlay = GLTexture::readImageToTexture (overlay_s, pname,
				    	       overlaySize);
    
    if (!overlay.size ())
    {
	compLogMessage ("mag", CompLogLevelWarn,
			"Could not load magnifier overlay image \"%s\"!",
			overlay_s.c_str ());
	return false;
    }

    mask = GLTexture::readImageToTexture (mask_s, pname,
				    	  maskSize);

    if (!mask.size ())
    {
	compLogMessage ("mag", CompLogLevelWarn,
			"Could not load magnifier mask image \"%s\"!",
			mask_s.c_str ());
	overlay.clear ();
	return false;
    }

    if (overlaySize.width () != maskSize.width () ||
	overlaySize.height () != maskSize.height ())
    {
	compLogMessage ("mag", CompLogLevelWarn,
			"Image dimensions do not match!");
	overlay.clear ();
	mask.clear ();
	return false;
    }

    return true;
#endif
    return false;
}

void
MagScreen::optionChanged (CompOption	      *opt,
		   	  MagOptions::Options num)
{
    cleanup ();

    switch (optionGetMode ())
    {
    case ModeImageOverlay:
	if (loadImages ())
	    mode = MagOptions::ModeImageOverlay;
	else
	    mode = MagOptions::ModeSimple;
	break;
    case MagOptions::ModeFisheye:
	if (loadFragmentProgram ())
	    mode = MagOptions::ModeFisheye;
	else
	    mode = MagOptions::ModeSimple;
	break;
    default:
	mode = MagOptions::ModeSimple;
    }
    
    if (zoom != 1.0)
	cScreen->damageScreen ();
}

void
MagScreen::doDamageRegion ()
{
    int  w, h, x, y;

    CompRegion region;

    switch (mode)
    {
    case MagOptions::ModeSimple:
        {
	    int b;
	    w = optionGetBoxWidth ();
	    h = optionGetBoxHeight ();
	    b = optionGetBorder ();
	    w += 2 * b;
	    h += 2 * b;
	    x = MAX (0, MIN (posX - (w / 2), screen->width () - w));
	    y = MAX (0, MIN (posY - (h / 2), screen->height () - h));

	    CompRegion tmpRegion (x, y, w, h);

	    region = tmpRegion;
	}
	break;
    case MagOptions::ModeImageOverlay:
	{
	    x = posX - optionGetXOffset ();
	    y = posY - optionGetYOffset ();
	    w = overlaySize.width ();
	    h = overlaySize.height ();

	    CompRegion tmpRegion (x, y, w, h);

	    region = tmpRegion;
	}
	break;
    case MagOptions::ModeFisheye:
        {
	    int radius = optionGetRadius ();
	    int x2, y2;

	    x = MAX (0.0, posX - radius);
	    y = MAX (0.0, posY - radius);
	    x2 = MIN (screen->width (), posX + radius);
	    y2 = MIN (screen->height (), posY + radius);
	    w = x2 - x;
	    h = y2 - y;

	    CompRegion tmpRegion (x, y, w, h);

	    region = tmpRegion;
	}
	break;
    default:
	break;
    }

    cScreen->damageRegion (region);
}

void
MagScreen::positionUpdate (const CompPoint &pos)
{
    doDamageRegion ();

    posX = pos.x ();
    posY = pos.y ();

    doDamageRegion ();
}

int
MagScreen::adjustZoom (float chunk)
{
    float dx, adjust, amount;
    float change;

    dx = zTarget - zoom;

    adjust = dx * 0.15f;
    amount = fabs(dx) * 1.5f;
    if (amount < 0.2f)
	amount = 0.2f;
    else if (amount > 2.0f)
	amount = 2.0f;

    zVelocity = (amount * zVelocity + adjust) / (amount + 1.0f);

    if (fabs (dx) < 0.002f && fabs (zVelocity) < 0.004f)
    {
	zVelocity = 0.0f;
	zoom = zTarget;
	return false;
    }

    change = zVelocity * chunk;
    if (!change)
    {
	if (zVelocity)
	    change = (dx > 0) ? 0.01 : -0.01;
    }

    zoom += change;

    return true;
}

void
MagScreen::preparePaint (int        time)
{
    if (adjust)
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
	    adjust = adjustZoom (chunk);
	    if (adjust)
		break;
	}
    }

    if (zoom != 1.0)
    {
	if (!poller.active ())
	{
	    CompPoint pos;
	    pos = poller.getCurrentPosition ();
	    posX = pos.x ();
	    posY = pos.y ();
	    poller.start ();
	}
	doDamageRegion ();
    }

    cScreen->preparePaint (time);
}

void
MagScreen::donePaint ()
{

    if (adjust)
	doDamageRegion ();

    if (!adjust && zoom == 1.0 && (width || height))
    {
	glBindTexture (target, texture);

	glTexImage2D (target, 0, GL_RGB, 0, 0, 0,
		      GL_RGB, GL_UNSIGNED_BYTE, NULL);

	width = 0;
	height = 0;
	
	glBindTexture (target, 0);
    }

    if (zoom == 1.0 && !adjust)
    {
	// Mag mode has ended
	cScreen->preparePaintSetEnabled (this, false);
	cScreen->donePaintSetEnabled (this, false);
	gScreen->glPaintOutputSetEnabled (this, false);

    	if (poller.active ())
    	    poller.stop ();
    }

    cScreen->donePaint ();
}

void
MagScreen::paintSimple ()
{
    float          pw, ph, bw, bh;
    int            x1, x2, y1, y2;
    float          vc[4];
    float          tc[4];
    int            w, h, cw, ch, cx, cy;
    bool           kScreen;
    unsigned short *color;
    float          tmp;
    GLMatrix       projection;
    GLMatrix       modelview;
    GLVertexBuffer *vb = GLVertexBuffer::streamingBuffer ();
    const GLWindowPaintAttrib attrib = { OPAQUE, BRIGHT, COLOR, 0, 0, 0, 0 };

    w = optionGetBoxWidth ();
    h = optionGetBoxHeight ();

    kScreen = optionGetKeepScreen ();

    x1 = posX - (w / 2);
    if (kScreen)
	x1 = MAX (0, MIN (x1, screen->width () - w));
    x2 = x1 + w;
    y1 = posY - (h / 2);
    if (kScreen)
	y1 = MAX (0, MIN (y1, screen->height () - h));
    y2 = y1 + h;
 
    cw = ceil ((float)w / (zoom * 2.0)) * 2.0;
    ch = ceil ((float)h / (zoom * 2.0)) * 2.0;
    cw = MIN (w, cw + 2);
    ch = MIN (h, ch + 2);
    cx = (w - cw) / 2;
    cy = (h - ch) / 2;

    cx = MAX (0, MIN (w - cw, cx));
    cy = MAX (0, MIN (h - ch, cy));

    if (x1 != (posX - (w / 2)))
    {
	cx = 0;
	cw = w;
    }
    if (y1 != (posY - (h / 2)))
    {
	cy = 0;
	ch = h;
    }

    glBindTexture (target, texture);

    if (width != w || height != h)
    {
	glCopyTexImage2D(target, 0, GL_RGB, x1, screen->height () - y2,
			 w, h, 0);
	width = w;
	height = h;
    }
    else
	glCopyTexSubImage2D (target, 0, cx, cy,
			     x1 + cx, screen->height () - y2 + cy, cw, ch);

    if (target == GL_TEXTURE_2D)
    {
	pw = 1.0 / width;
	ph = 1.0 / height;
    }
    else
    {
	pw = 1.0;
	ph = 1.0;
    }

    vc[0] = ((x1 * 2.0) / screen->width ()) - 1.0;
    vc[1] = ((x2 * 2.0) / screen->width ()) - 1.0;
    vc[2] = ((y1 * -2.0) / screen->height ()) + 1.0;
    vc[3] = ((y2 * -2.0) / screen->height ()) + 1.0;

    tc[0] = 0.0;
    tc[1] = w * pw;
    tc[2] = h * ph;
    tc[3] = 0.0;

    /* Draw zoom box contents */
    glScissor (x1, screen->height () - y2, w, h);

    glEnable (GL_SCISSOR_TEST);

    modelview.translate ((float)(posX - (screen->width () / 2)) * 2 / screen->width (),
		  (float)(posY - (screen->height () / 2)) * 2 / -screen->height (), 0.0);

    modelview.scale (zoom, zoom, 1.0);

    modelview.translate ((float)((screen->width () / 2) - posX) * 2 / screen->width (),
		  (float)((screen->height () / 2) - posY) * 2 / -screen->height (), 0.0);

    GLfloat vertices[] = {
	vc[0], vc[2], 0,
	vc[0], vc[3], 0,
	vc[1], vc[2], 0,
	vc[1], vc[3], 0,
    };

    GLfloat texcoords[] = {
	tc[0], tc[2],
	tc[0], tc[3],
	tc[1], tc[2],
	tc[1], tc[3],
    };

    vb->begin (GL_TRIANGLE_STRIP);
    vb->colorDefault ();
    vb->addVertices (4, vertices);
    vb->addTexCoords (0, 4, texcoords);
    vb->end ();

    vb->render (projection, modelview, attrib);

    glDisable (GL_SCISSOR_TEST);
    modelview.reset ();
    glBindTexture (target, 0);

    /* Save blending state */
#if USE_GLES
    GLboolean isBlendingEnabled = GL_TRUE;
    GLint blendSrcRGB = GL_ONE;
    GLint blendSrcAlpha = GL_ONE;
    GLint blendDstRGB = GL_ZERO;
    GLint blendDstAlpha = GL_ZERO;

    glGetBooleanv (GL_BLEND, &isBlendingEnabled);
    glGetIntegerv (GL_BLEND_SRC_RGB, &blendSrcRGB);
    glGetIntegerv (GL_BLEND_DST_RGB, &blendDstRGB);
    glGetIntegerv (GL_BLEND_SRC_ALPHA, &blendSrcAlpha);
    glGetIntegerv (GL_BLEND_DST_ALPHA, &blendDstAlpha);
#else
    glPushAttrib (GL_COLOR_BUFFER_BIT);
#endif

    /* Draw zoom box border */
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    tmp = MIN (1.0, (zoom - 1) * 3.0);

    bw = bh = optionGetBorder ();

    bw = bw * 2.0 / screen->width ();
    bh = bh * 2.0 / screen->height ();

    bw = bh = optionGetBorder ();

    bw *= 2.0 / (float)screen->width ();
    bh *= 2.0 / (float)screen->height ();

    color = optionGetBoxColor ();

    GLfloat verticesBorder[] = {
	vc[0] - bw, vc[2] + bh, 0,
	vc[0], vc[2], 0,
	vc[1] + bw, vc[2] + bh, 0,
	vc[1], vc[2], 0,
	vc[1] + bw, vc[3] - bh, 0,
	vc[1], vc[3], 0,
	vc[0] - bw, vc[3] - bh, 0,
	vc[0], vc[3], 0,
	vc[0] - bw, vc[2] + bh, 0,
	vc[0], vc[2], 0,
    };

    vb->begin (GL_TRIANGLE_STRIP);
    vb->color4f (color[0] / 65535.0, color[1] / 65535.0,
	         color[2] / 65535.0, color[3] * tmp / 65535.0);
    vb->addVertices (10, verticesBorder);
    vb->end ();

    vb->render (projection, modelview, attrib);

    vb->colorDefault ();

    /* Restore blending state */
#if USE_GLES
    if (!isBlendingEnabled)
    glDisable (GL_BLEND);
    glBlendFuncSeparate (blendSrcRGB, blendDstRGB, blendSrcAlpha, blendDstAlpha);
#else
    glPopAttrib ();
#endif
}

void
MagScreen::paintImage ()
{
#if 0
    float          pw, ph;
    int            x1, x2, y1, y2;
    float          vc[4];
    float          tc[4];
    int            w, h, cw, ch, cx, cy;
    float          tmp, xOff, yOff;

    w = overlaySize.width ();
    h = overlaySize.height ();

    xOff = MIN (w, optionGetXOffset ());
    yOff = MIN (h, optionGetYOffset ());

    x1 = posX - xOff;
    x2 = x1 + w;
    y1 = posY - yOff;
    y2 = y1 + h;
 
    cw = ceil ((float)w / (zoom * 2.0)) * 2.0;
    ch = ceil ((float)h / (zoom * 2.0)) * 2.0;
    cw = MIN (w, cw + 2);
    ch = MIN (h, ch + 2);
    cx = floor (xOff - (xOff / zoom));
    cy = h - ch - floor (yOff - (yOff / zoom));

    cx = MAX (0, MIN (w - cw, cx));
    cy = MAX (0, MIN (h - ch, cy));

    glPushAttrib (GL_TEXTURE_BIT);
    
    glEnable (target);

    glBindTexture (target, texture);

    if (width != w || height != h)
    {
	glCopyTexImage2D(target, 0, GL_RGB, x1, screen->height () - y2,
			 w, h, 0);
	width = w;
	height = h;
    }
    else
	glCopyTexSubImage2D (target, 0, cx, cy,
			     x1 + cx, screen->height () - y2 + cy, cw, ch);

    if (target == GL_TEXTURE_2D)
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

    vc[0] = ((x1 * 2.0) / screen->width ()) - 1.0;
    vc[1] = ((x2 * 2.0) / screen->width ()) - 1.0;
    vc[2] = ((y1 * -2.0) / screen->height ()) + 1.0;
    vc[3] = ((y2 * -2.0) / screen->height ()) + 1.0;

    tc[0] = xOff - (xOff / zoom);
    tc[1] = tc[0] + (w / zoom);

    tc[2] = h - (yOff - (yOff / zoom));
    tc[3] = tc[2] - (h / zoom);

    tc[0] *= pw;
    tc[1] *= pw;
    tc[2] *= ph;
    tc[3] *= ph;

    glEnable (GL_BLEND);

    glColor4usv (defaultColor);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    GL::activeTexture (GL_TEXTURE1_ARB);
    foreach (GLTexture *tex, mask)
    {
	tex->enable (GLTexture::Good);
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBegin (GL_QUADS);
	GL::multiTexCoord2f (GL_TEXTURE0_ARB, tc[0], tc[2]);
	GL::multiTexCoord2f (GL_TEXTURE1_ARB,
			   COMP_TEX_COORD_X (tex->matrix (), 0),
			   COMP_TEX_COORD_Y (tex->matrix (), 0));
	glVertex2f (vc[0], vc[2]);
	GL::multiTexCoord2f (GL_TEXTURE0_ARB, tc[0], tc[3]);
	GL::multiTexCoord2f (GL_TEXTURE1_ARB,
			   COMP_TEX_COORD_X (tex->matrix (), 0),
			   COMP_TEX_COORD_Y (tex->matrix (), h));
	glVertex2f (vc[0], vc[3]);
	GL::multiTexCoord2f (GL_TEXTURE0_ARB, tc[1], tc[3]);
	GL::multiTexCoord2f (GL_TEXTURE1_ARB,
			   COMP_TEX_COORD_X (tex->matrix (), w),
			   COMP_TEX_COORD_Y (tex->matrix (), h));
	glVertex2f (vc[1], vc[3]);
	GL::multiTexCoord2f (GL_TEXTURE0_ARB, tc[1], tc[2]);
	GL::multiTexCoord2f (GL_TEXTURE1_ARB,
			   COMP_TEX_COORD_X (tex->matrix (), w),
			   COMP_TEX_COORD_Y (tex->matrix (), 0));
	glVertex2f (vc[1], vc[2]);
	glEnd ();

	tex->disable ();
    }
    GL::activeTexture (GL_TEXTURE0_ARB);

    glBindTexture (target, 0);

    glDisable (target);

    tmp = MIN (1.0, (zoom - 1) * 3.0);

    glColor4f (tmp, tmp, tmp, tmp);

    foreach (GLTexture *tex, overlay)
    {
	tex->enable (GLTexture::Fast);
	glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glBegin (GL_QUADS);
	glTexCoord2f (COMP_TEX_COORD_X (tex->matrix (), 0),
		  COMP_TEX_COORD_Y (tex->matrix (), 0));
	glVertex2f (vc[0], vc[2]);
	glTexCoord2f (COMP_TEX_COORD_X (tex->matrix (), 0),
		  COMP_TEX_COORD_Y (tex->matrix (), h));
	glVertex2f (vc[0], vc[3]);
	glTexCoord2f (COMP_TEX_COORD_X (tex->matrix (), w),
		  COMP_TEX_COORD_Y (tex->matrix (), h));
	glVertex2f (vc[1], vc[3]);
	glTexCoord2f (COMP_TEX_COORD_X (tex->matrix (), w),
		  COMP_TEX_COORD_Y (tex->matrix (), 0));
	glVertex2f (vc[1], vc[2]);
	glEnd ();

	tex->disable ();
    }

    glColor4usv (defaultColor);
    glDisable (GL_BLEND);
    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glPopMatrix();
    glMatrixMode (GL_PROJECTION);
    glPopMatrix ();
    glMatrixMode (GL_MODELVIEW);

    glPopAttrib ();
#endif

}

void
MagScreen::paintFisheye ()
{
#if 0
    float      pw, ph;
    float      radius, fZoom, base; // fZoom is the local zoom variable
    int        x1, x2, y1, y2;
    float      vc[4];
    int        size;

    radius = optionGetRadius ();
    base   = 0.5 + (0.0015 * radius);
    fZoom   = (zoom * base) + 1.0 - base;

    size = radius + 1;

    x1 = MAX (0.0, posX - size);
    x2 = MIN (screen->width (), posX + size);
    y1 = MAX (0.0, posY - size);
    y2 = MIN (screen->height (), posY + size);
 
    glEnable (target);

    glBindTexture (target, texture);

    if (width != 2 * size || height != 2 * size)
    {
	glCopyTexImage2D(target, 0, GL_RGB, x1, screen->height () - y2,
			 size * 2, size * 2, 0);
	width = height = 2 * size;
    }
    else
	glCopyTexSubImage2D (target, 0, 0, 0,
			     x1, screen->height () - y2, x2 - x1, y2 - y1);

    if (target == GL_TEXTURE_2D)
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
    GL::bindProgram (GL_FRAGMENT_PROGRAM_ARB, program);

    GL::programEnvParameter4f (GL_FRAGMENT_PROGRAM_ARB, 0,
				 posX, screen->height () - posY,
				 1.0 / radius, 0.0f);
    GL::programEnvParameter4f (GL_FRAGMENT_PROGRAM_ARB, 1,
				 pw, ph, M_PI / radius,
				 (fZoom - 1.0) * fZoom);
    GL::programEnvParameter4f (GL_FRAGMENT_PROGRAM_ARB, 2,
				 -x1 * pw, -(screen->height () - y2) * ph,
				 -M_PI / 2.0, 0.0);

    x1 = MAX (0.0, posX - radius);
    x2 = MIN (screen->width (), posX + radius);
    y1 = MAX (0.0, posY - radius);
    y2 = MIN (screen->height (), posY + radius);

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

    glBindTexture (target, 0);

    glDisable (target);
#endif
}


bool
MagScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
			  const GLMatrix	    &transform,
			  const CompRegion	    &region,
			  CompOutput	            *output,
			  unsigned int	            mask)
{
    bool status = gScreen->glPaintOutput (attrib, transform, region, output, mask);

    if (zoom == 1.0)
	return status;

    /* Temporarily set the viewport to fullscreen */

    glViewport (0, 0, screen->width (), screen->height ());

    switch (mode)
    {
    case MagOptions::ModeImageOverlay:
	paintImage ();
	break;
    case MagOptions::ModeFisheye:
	paintFisheye ();
	break;
    default:
	paintSimple ();
    }

    gScreen->setDefaultViewport ();

    return status;
}

bool
MagScreen::terminate (CompAction	  *action,
	      	      CompAction::State   state,
	      	      CompOption::Vector options)
{
    zTarget = 1.0;
    adjust  = true;
    cScreen->damageScreen ();
    return true;
}

bool
MagScreen::initiate (CompAction	  *action,
		     CompAction::State   state,
		     CompOption::Vector options)
{
    float      factor;
    factor = CompOption::getFloatOptionNamed (options, "factor", 0);

    if (factor == 0.0 && zTarget != 1.0)
        return terminate (action, state, options);

    if (mode == MagOptions::ModeFisheye)
    {
        if (factor != 1.0)
	    factor = optionGetZoomFactor () * 3;

        zTarget = MAX (1.0, MIN (10.0, factor));

    }
    else
    {
        if (factor != 1.0)
	    factor = optionGetZoomFactor ();

	zTarget = MAX (1.0, MIN (64.0, factor));
    }
    adjust  = true;
    cScreen->damageScreen ();

    // Mag mode is starting
    cScreen->preparePaintSetEnabled (this, true);
    cScreen->donePaintSetEnabled (this, true);
    gScreen->glPaintOutputSetEnabled (this, true);

    return true;
}

bool
MagScreen::zoomIn (CompAction	  *action,
		   CompAction::State   state,
		   CompOption::Vector options)
{
    if (mode == MagOptions::ModeFisheye)
        zTarget = MIN (10.0, zTarget + 1.0);
    else
        zTarget = MIN (64.0, zTarget * 1.2);
    adjust  = true;
    cScreen->damageScreen ();

    // Mag mode is starting
    cScreen->preparePaintSetEnabled (this, true);
    cScreen->donePaintSetEnabled (this, true);
    gScreen->glPaintOutputSetEnabled (this, true);

    return true;
}

bool
MagScreen::zoomOut (CompAction	  *action,
		    CompAction::State   state,
		    CompOption::Vector options)
{
    if (mode == MagOptions::ModeFisheye)
        zTarget = MAX (1.0, zTarget - 1.0);
    else
        zTarget = MAX (1.0, zTarget / 1.2);
    adjust  = true;
    cScreen->damageScreen ();

    return true;
}

MagScreen::MagScreen (CompScreen *screen) :
    PluginClassHandler <MagScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    posX (0),
    posY (0),
    adjust (false),
    zVelocity (0.0f),
    zTarget (1.0f),
    zoom (1.0f),
    program (0)
{
    ScreenInterface::setHandler (screen, false);
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);

    poller.setCallback (boost::bind (&MagScreen::positionUpdate, this, _1));

    glGenTextures (1, &texture);

#ifdef USE_GLES
    target = GL_TEXTURE_2D;
#else
    if (GL::textureNonPowerOfTwo)
	target = GL_TEXTURE_2D;
    else
	target = GL_TEXTURE_RECTANGLE_ARB;
#endif

    /* Bind the texture */
    glBindTexture (target, texture);

    /* Load the parameters */
    glTexParameteri (target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D (target, 0, GL_RGB, 0, 0, 0,
		  GL_RGB, GL_UNSIGNED_BYTE, NULL);

    width = 0;
    height = 0;

    glBindTexture (target, 0);

#define optionNotify(name)						       \
    optionSet##name##Notify (boost::bind (&MagScreen::optionChanged,	       \
					    this, _1, _2))

    optionNotify (Overlay);
    optionNotify (Mask);
    optionNotify (Mode);

#undef optionNotify

    optionSetInitiateInitiate (boost::bind (&MagScreen::initiate, this, _1, _2,
						_3));
    optionSetInitiateTerminate (boost::bind (&MagScreen::initiate, this, _1, _2,
						_3));

    optionSetZoomInButtonInitiate (boost::bind (&MagScreen::zoomIn, this, _1, _2,
						_3));

    optionSetZoomOutButtonInitiate (boost::bind (&MagScreen::zoomOut, this, _1, _2,
						_3));

    switch (optionGetMode ())
    {
    case MagOptions::ModeImageOverlay:
	if (loadImages ())
	    mode = MagOptions::ModeImageOverlay;
	else
	    mode = MagOptions::ModeSimple;
	break;
    case MagOptions::ModeFisheye:
	if (loadFragmentProgram ())
	    mode = MagOptions::ModeFisheye;
	else
	    mode = MagOptions::ModeSimple;
	break;
    default:
	mode = MagOptions::ModeSimple;
    }

#if 0
    if (!GL::fragmentProgram)
	compLogMessage ("mag", CompLogLevelWarn,
			"GL_ARB_fragment_program not supported. "
			"Fisheye mode will not work.");
#endif
}

MagScreen::~MagScreen ()
{
    poller.stop ();

    if (zoom)
	cScreen->damageScreen ();

    glDeleteTextures (1, &target);

    cleanup ();
}

bool
MagPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;
    if (!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI))
	return false;
    if (!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;
    if (!CompPlugin::checkPluginABI ("mousepoll", COMPIZ_MOUSEPOLL_ABI))
	return false;

    return true;
}
