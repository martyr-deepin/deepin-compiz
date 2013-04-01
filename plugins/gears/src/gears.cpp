/*
 * Compiz cube gears plugin
 *
 * gears.cpp
 *
 * This is an example plugin to show how to render something inside
 * of the transparent cube
 *
 * Copyright : (C) 2007 by Dennis Kasprzyk
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
 * Based on glxgears.c:
 *    http://cvsweb.xfree86.org/cvsweb/xc/programs/glxgears/glxgears.c
 */

#include "gears.h"


COMPIZ_PLUGIN_20090315 (gears, GearsPluginVTable);

static void
gear (GLfloat inner_radius,
      GLfloat outer_radius,
      GLfloat width,
      GLint   teeth,
      GLfloat tooth_depth)
{
    GLint i;
    GLfloat r0, r1, r2, maxr2, minr2;
    GLfloat angle, da;
    GLfloat u, v, len;

    r0 = inner_radius;
    r1 = outer_radius - tooth_depth / 2.0;
    maxr2 = r2 = outer_radius + tooth_depth / 2.0;
    minr2 = r2;

    da = 2.0 * M_PI / teeth / 4.0;

    glShadeModel (GL_FLAT);

    glNormal3f (0.0, 0.0, 1.0);

    /* draw front face */
    glBegin (GL_QUAD_STRIP);

    for (i = 0; i <= teeth; i++)
    {
	angle = i * 2.0 * M_PI / teeth;
	glVertex3f (r0 * cos (angle), r0 * sin (angle), width * 0.5);
	glVertex3f (r1 * cos (angle), r1 * sin (angle), width * 0.5);

	if (i < teeth)
	{
	    glVertex3f (r0 * cos (angle), r0 * sin (angle), width * 0.5);
	    glVertex3f (r1 * cos (angle + 3 * da), r1 * sin (angle + 3 * da),
			width * 0.5);
	}
    }

    glEnd();

    /* draw front sides of teeth */
    glBegin (GL_QUADS);

    for (i = 0; i < teeth; i++)
    {
	angle = i * 2.0 * M_PI / teeth;

	glVertex3f (r1 * cos (angle), r1 * sin (angle), width * 0.5);
	glVertex3f (r2 * cos (angle + da), r2 * sin (angle + da), width * 0.5);
	glVertex3f (r2 * cos (angle + 2 * da), r2 * sin (angle + 2 * da),
		    width * 0.5);
	glVertex3f (r1 * cos (angle + 3 * da), r1 * sin (angle + 3 * da),
		    width * 0.5);
	r2 = minr2;
    }

    r2 = maxr2;

    glEnd();

    glNormal3f (0.0, 0.0, -1.0);

    /* draw back face */
    glBegin (GL_QUAD_STRIP);

    for (i = 0; i <= teeth; i++)
    {
	angle = i * 2.0 * M_PI / teeth;
	glVertex3f (r1 * cos (angle), r1 * sin (angle), -width * 0.5);
	glVertex3f (r0 * cos (angle), r0 * sin (angle), -width * 0.5);

	if (i < teeth)
	{
	    glVertex3f (r1 * cos (angle + 3 * da), r1 * sin (angle + 3 * da),
			-width * 0.5);
	    glVertex3f (r0 * cos (angle), r0 * sin (angle), -width * 0.5);
	}
    }

    glEnd();

    /* draw back sides of teeth */
    glBegin (GL_QUADS);
    da = 2.0 * M_PI / teeth / 4.0;

    for (i = 0; i < teeth; i++)
    {
	angle = i * 2.0 * M_PI / teeth;

	glVertex3f (r1 * cos (angle + 3 * da), r1 * sin (angle + 3 * da),
		    -width * 0.5);
	glVertex3f (r2 * cos (angle + 2 * da), r2 * sin (angle + 2 * da),
		    -width * 0.5);
	glVertex3f (r2 * cos (angle + da), r2 * sin (angle + da), -width * 0.5);
	glVertex3f (r1 * cos (angle), r1 * sin (angle), -width * 0.5);
	r2 = minr2;
    }

    r2 = maxr2;

    glEnd();

    /* draw outward faces of teeth */
    glBegin (GL_QUAD_STRIP);

    for (i = 0; i < teeth; i++)
    {
	angle = i * 2.0 * M_PI / teeth;

	glVertex3f (r1 * cos (angle), r1 * sin (angle), width * 0.5);
	glVertex3f (r1 * cos (angle), r1 * sin (angle), -width * 0.5);
	u = r2 * cos (angle + da) - r1 * cos (angle);
	v = r2 * sin (angle + da) - r1 * sin (angle);
	len = sqrt (u * u + v * v);
	u /= len;
	v /= len;
	glNormal3f (v, -u, 0.0);
	glVertex3f (r2 * cos (angle + da), r2 * sin (angle + da), width * 0.5);
	glVertex3f (r2 * cos (angle + da), r2 * sin (angle + da), -width * 0.5);
	glNormal3f (cos (angle + 1.5 * da), sin (angle + 1.5 * da), 0.0);
	glVertex3f (r2 * cos (angle + 2 * da), r2 * sin (angle + 2 * da),
		    width * 0.5);
	glVertex3f (r2 * cos (angle + 2 * da), r2 * sin (angle + 2 * da),
		    -width * 0.5);
	u = r1 * cos (angle + 3 * da) - r2 * cos (angle + 2 * da);
	v = r1 * sin (angle + 3 * da) - r2 * sin (angle + 2 * da);
	glNormal3f (v, -u, 0.0);
	glVertex3f (r1 * cos (angle + 3 * da), r1 * sin (angle + 3 * da),
		    width * 0.5);
	glVertex3f (r1 * cos (angle + 3 * da), r1 * sin (angle + 3 * da),
		    -width * 0.5);
	glNormal3f (cos (angle + 3.5 * da), sin (angle + 3.5 * da), 0.0);
	r2 = minr2;
    }

    r2 = maxr2;

    glVertex3f (r1 * cos (0), r1 * sin (0), width * 0.5);
    glVertex3f (r1 * cos (0), r1 * sin (0), -width * 0.5);

    glEnd();

    glShadeModel (GL_SMOOTH);

    /* draw inside radius cylinder */
    glBegin (GL_QUAD_STRIP);

    for (i = 0; i <= teeth; i++)
    {
	angle = i * 2.0 * M_PI / teeth;
	glNormal3f (-cos (angle), -sin (angle), 0.0);
	glVertex3f (r0 * cos (angle), r0 * sin (angle), -width * 0.5);
	glVertex3f (r0 * cos (angle), r0 * sin (angle), width * 0.5);
    }

    glEnd();
}

void
GearsScreen::cubeClearTargetOutput (float      xRotate,
				    float      vRotate)
{
    csScreen->cubeClearTargetOutput (xRotate, vRotate);

    glClear (GL_DEPTH_BUFFER_BIT);
}


void GearsScreen::cubePaintInside (const GLScreenPaintAttrib &sAttrib,
			           const GLMatrix            &transform,
				   CompOutput                *output,
				   int                       size)
{
//    CUBE_SCREEN (screen);

    static GLfloat white[4] = { 1.0, 1.0, 1.0, 1.0 };

    GLScreenPaintAttrib sA = sAttrib;

    sA.yRotate += csScreen->invert () * (360.0f / size) *
		  (csScreen->xRotations () - (screen->vp ().x () * csScreen->nOutput ()));

    //CompTransform mT = *transform;
    GLMatrix mT = transform;

    gScreen->glApplyTransform (sA, output, &mT);
//    (*s->applyScreenTransform) (s, &sA, output, &mT);

    glPushMatrix();
    glLoadMatrixf (mT.getMatrix ());
    glTranslatef (csScreen->outputXOffset (), -csScreen->outputYOffset (), 0.0f);
    glScalef (csScreen->outputXScale (), csScreen->outputYScale (), 1.0f);

    bool enabledCull = false;

    glPushAttrib (GL_COLOR_BUFFER_BIT | GL_TEXTURE_BIT);

    glDisable (GL_BLEND);

    if (!glIsEnabled (GL_CULL_FACE) )
    {
	enabledCull = true;
	glEnable (GL_CULL_FACE);
    }

    glPushMatrix();

    glRotatef (contentRotation, 0.0, 1.0, 0.0);

    glScalef (0.05, 0.05, 0.05);
    glColor4usv (defaultColor);

    glEnable (GL_NORMALIZE);
    glEnable (GL_LIGHTING);
    glEnable (GL_LIGHT1);
    glDisable (GL_COLOR_MATERIAL);

    glEnable (GL_DEPTH_TEST);
    glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glPushMatrix();
    glTranslatef (-3.0, -2.0, 0.0);
    glRotatef (angle, 0.0, 0.0, 1.0);
    glCallList (gear1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef (3.1, -2.0, 0.0);
    glRotatef (-2.0 * angle - 9.0, 0.0, 0.0, 1.0);
    glCallList (gear2);
    glPopMatrix();

    glPushMatrix();
    glTranslatef (-3.1, 4.2, 0.0);
    glRotatef (-2.0 * angle - 25.0, 0.0, 0.0, 1.0);
    glCallList (gear3);
    glPopMatrix();

    glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);

    glPopMatrix();

    glDisable (GL_LIGHT1);
    glDisable (GL_NORMALIZE);
    glEnable (GL_COLOR_MATERIAL);

    if (!gScreen->lighting ())
	glDisable (GL_LIGHTING);

    glDisable (GL_DEPTH_TEST);

    if (enabledCull)
	glDisable (GL_CULL_FACE);

    glPopMatrix();
    glPopAttrib();

    damage = true;

    csScreen->cubePaintInside (sAttrib, transform, output, size);
}
void
GearsScreen::preparePaint (int ms)
{

    contentRotation += ms * 360.0 / 20000.0;
    contentRotation = fmod (contentRotation, 360.0);
    angle += ms * 360.0 / 8000.0;
    angle = fmod (angle, 360.0);
    a1 += ms * 360.0 / 3000.0;
    a1 = fmod (a1, 360.0);
    a2 += ms * 360.0 / 2000.0;
    a2 = fmod (a2, 360.0);
    a3 += ms * 360.0 / 1000.0;
    a3 = fmod (a3, 360.0);

    cScreen->preparePaint (ms);
}

void
GearsScreen::donePaint ()
{
    if (damage)
    {
	cScreen->damageScreen ();
	damage = false;
    }

    cScreen->donePaint ();
}

GearsScreen::GearsScreen (CompScreen *screen) :
    PluginClassHandler <GearsScreen, CompScreen> (screen), 
    screen (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    csScreen (CubeScreen::get (screen)),
    damage(false),
    contentRotation(0.0),
    angle(0.0),
    a1(0.0),
    a2(0.0),
    a3(0.0)
{
    ScreenInterface::setHandler (screen); 
    CompositeScreenInterface::setHandler (cScreen);
    GLScreenInterface::setHandler (gScreen);
    CubeScreenInterface::setHandler (csScreen);

    static GLfloat pos[4]         = { 5.0, 5.0, 10.0, 0.0 };
    static GLfloat red[4]         = { 0.8, 0.1, 0.0, 1.0 };
    static GLfloat green[4]       = { 0.0, 0.8, 0.2, 1.0 };
    static GLfloat blue[4]        = { 0.2, 0.2, 1.0, 1.0 };
    static GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 0.3f };
    static GLfloat diffuseLight[] = { 0.5f, 0.5f, 0.5f, 0.5f };

    glLightfv (GL_LIGHT1, GL_AMBIENT, ambientLight);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, diffuseLight);
    glLightfv (GL_LIGHT1, GL_POSITION, pos);

    gear1 = glGenLists (1);
    glNewList (gear1, GL_COMPILE);
    glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
    gear (1.0, 4.0, 1.0, 20, 0.7);
    glEndList();

    gear2 = glGenLists (1);
    glNewList (gear2, GL_COMPILE);
    glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
    gear (0.5, 2.0, 2.0, 10, 0.7);
    glEndList();

    gear3 = glGenLists (1);
    glNewList (gear3, GL_COMPILE);
    glMaterialfv (GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
    gear (1.3, 2.0, 0.5, 10, 0.7);
    glEndList();

}

GearsScreen::~GearsScreen ()
{
    glDeleteLists (gear1, 1);
    glDeleteLists (gear2, 1);
    glDeleteLists (gear3, 1);

}

bool
GearsPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	 return false;
    if (!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI))
	 return false;
    if (!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	 return false;

    return true;
}
