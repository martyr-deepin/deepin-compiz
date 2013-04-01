/*
 *
 * Compiz bicubic filter plugin
 *
 * bicubic.c
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

#include "bicubic.h"

COMPIZ_PLUGIN_20090315 (bicubic, BicubicPluginVTable);

int
BicubicScreen::getBicubicFragmentFunction (GLTexture   *texture,
					   int         param,
					   int         unit)
{
    GLFragment::FunctionData	*data;
    int				target;
    CompString			targetString;

    if (texture->target () == GL_TEXTURE_2D)
    {
	target	     = COMP_FETCH_TARGET_2D;
	targetString = "2D";
    }
    else
    {
	target	     = COMP_FETCH_TARGET_RECT;
	targetString = "RECT";
    }

    foreach (BicubicFunction *function, func)
	if (function->param  == param  &&
	    function->target == target &&
	    function->unit   == unit)
	    return function->handle;

    data = new GLFragment::FunctionData ();
    if (data)
    {
	int  handle = 0;
	BicubicFunction *function = NULL;

	CompString filterTemp[] = {
	    "hgX", "hgY", "cs00", "cs01", "cs10", "cs11"
	};

	for (unsigned int i = 0; i < sizeof (filterTemp) / sizeof (filterTemp[0]); i++)
	    data->addTempHeaderOp (filterTemp[i].c_str());


	data->addDataOp (
		"MAD cs00, fragment.texcoord[0], program.env[%d],"
		"{-0.5, -0.5, 0.0, 0.0};", param + 2);
	
	data->addDataOp ( 
		"TEX hgX, cs00.x, texture[%d], 1D;", unit);
	data->addDataOp ( 
		"TEX hgY, cs00.y, texture[%d], 1D;", unit);
	
	data->addDataOp (
		"MUL cs10, program.env[%d], hgX.y;", param);
	data->addDataOp (
		"MUL cs00, program.env[%d], -hgX.x;", param);
	data->addDataOp (
		"MAD cs11, program.env[%d], hgY.y, cs10;", param + 1);
	data->addDataOp (
		"MAD cs01, program.env[%d], hgY.y, cs00;", param + 1);
	data->addDataOp (
		"MAD cs10, program.env[%d], -hgY.x, cs10;", param + 1);
	data->addDataOp (
		"MAD cs00, program.env[%d], -hgY.x, cs00;", param + 1);

	data->addDataOp ( 
		"ADD cs00, cs00, fragment.texcoord[0];");
	data->addDataOp (
		"ADD cs01, cs01, fragment.texcoord[0];");
	data->addDataOp (
		"ADD cs10, cs10, fragment.texcoord[0];");
	data->addDataOp (
		"ADD cs11, cs11, fragment.texcoord[0];");

	data->addDataOp ( 
		"TEX cs00, cs00, texture[0], %s;", targetString.c_str());
	data->addDataOp (
		"TEX cs01, cs01, texture[0], %s;", targetString.c_str());
	data->addDataOp ( 
		"TEX cs10, cs10, texture[0], %s;", targetString.c_str());
	data->addDataOp ( 
		"TEX cs11, cs11, texture[0], %s;", targetString.c_str());

	data->addDataOp ( "LRP cs00, hgY.z, cs00, cs01;");
	data->addDataOp ( "LRP cs10, hgY.z, cs10, cs11;");
 
	data->addDataOp ( "LRP output, hgX.z, cs00, cs10;");
	
	data->addColorOp ( "output", "output");
	if (!data->status ())
	{
	    delete data;
	    return 0;
	}
	
	function = new BicubicFunction ();
	if (function)
	{
	    handle = data->createFragmentFunction ("bicubic");

	    function->handle = handle;
	    function->target = target;
	    function->param  = param;
	    function->unit   = unit;

	    func.push_back (function);
	}

	delete data;

	return handle;
    }

    return 0;
}

void
BicubicWindow::glDrawTexture (GLTexture 	 *texture,
			      GLFragment::Attrib &attrib,
			      unsigned int       mask)
{
    BICUBIC_SCREEN (screen);

    if ((mask & (PAINT_WINDOW_TRANSFORMED_MASK |
	         PAINT_WINDOW_ON_TRANSFORMED_SCREEN_MASK)) &&
        bs->gScreen->textureFilter () == GLTexture::Good)
    {
	GLFragment::Attrib fa = attrib;
	int            function, param;
	int            unit = 0;
	
	param = fa.allocParameters (3);
	unit  = fa.allocTextureUnits (1);

	function = bs->getBicubicFragmentFunction (texture, param, unit);
	
	if (function)
	{
	    fa.addFunction (function);

	    GL::activeTexture (GL_TEXTURE0_ARB + unit);
	    glBindTexture (GL_TEXTURE_1D, bs->lTexture);
	    GL::activeTexture (GL_TEXTURE0_ARB);


	    GL::programEnvParameter4f (GL_FRAGMENT_PROGRAM_ARB, param,
					 texture->matrix ().xx, 0.0f,
					 0.0f, 0.0f);
	    GL::programEnvParameter4f (GL_FRAGMENT_PROGRAM_ARB,
					 param + 1,
					 0.0f, -texture->matrix ().yy,
					 0.0f, 0.0f);
	    GL::programEnvParameter4f (GL_FRAGMENT_PROGRAM_ARB,
					 param + 2,
					 1.0 / texture->matrix ().xx, 
					 1.0 / -texture->matrix ().yy,
					 0.0f, 0.0f);
	}

	gWindow->glDrawTexture (texture, fa, mask);
	
	if (unit)
	{
	    GL::activeTexture (GL_TEXTURE0_ARB + unit);
	    glBindTexture (GL_TEXTURE_1D, 0);
	    GL::activeTexture (GL_TEXTURE0_ARB);
	}
    }
    else
    {
	gWindow->glDrawTexture (texture, attrib, mask);
    }
}

void
BicubicScreen::generateLookupTexture (GLenum format)
{
    GLfloat values[512];
    float   a, a2, a3, w0, w1, w2, w3;

    for (int i = 0; i < 512; i += 4)
    {
	a  = (float)i / 512.0;
	a2 = a * a;
	a3 = a2 * a;
	
	w0 = (1.0 / 6.0) * ((-a3) + (3.0 * a2) + (-3.0 * a) + 1.0);
	w1 = (1.0 / 6.0) * ((3.0 * a3) + (-6.0 * a2) + 4.0);
	w2 = (1.0 / 6.0) * ((-3.0 * a3) + (3.0 * a2) + (3.0 * a) + 1.0);
	w3 = (1.0 / 6.0) * a3;
	
	values[i]     = 1.0 - (w1 / (w0 + w1)) + a;
	values[i + 1] = 1.0 + (w3 / (w2 + w3)) - a;
	values[i + 2] = w0 + w1;
	values[i + 3] = w2 + w3;
    }

    glGenTextures (1, &lTexture);

    glBindTexture (GL_TEXTURE_1D, lTexture);

    glTexImage1D (GL_TEXTURE_1D, 0, format, 128, 0, GL_RGBA,
		  GL_FLOAT, values);

    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture (GL_TEXTURE_1D, 0);
}


BicubicScreen::BicubicScreen (CompScreen *screen) :
    PluginClassHandler <BicubicScreen, CompScreen> (screen),
    gScreen (GLScreen::get (screen)),
    cScreen (CompositeScreen::get (screen))
{
    bool           failed = false;
    const char     *glExtensions;
    GLenum         format = GL_RGBA16F_ARB;

    if (!GL::fragmentProgram)
    {
	compLogMessage ("bicube", CompLogLevelFatal,
			"GL_ARB_fragment_program not supported.");
	setFailed ();
	failed = true;
    }

    if (!failed)
    {

	glExtensions = (const char *) glGetString (GL_EXTENSIONS);
	if (!glExtensions)
	{
	    compLogMessage ("bicubic", CompLogLevelFatal,
			    "No valid GL extensions string found.");
	    setFailed ();
	    failed = true;
	}

    }

    if (!failed)
    {

	if (!strstr (glExtensions, "GL_ARB_texture_float"))
	{
	    compLogMessage ("bicubic", CompLogLevelFatal,
			    "GL_ARB_texture_float not supported. "
			    "This can lead to visual artifacts.");
	    format = GL_RGBA;
	}
    }

    generateLookupTexture (format);
}

BicubicScreen::~BicubicScreen ()
{
    BicubicFunction *f;
    while (func.size ())
    {
	f = func.front ();
	GLFragment::destroyFragmentFunction (f->handle);
	func.remove (f);
    }

    glDeleteTextures (1, &lTexture);
}

BicubicWindow::BicubicWindow (CompWindow *window) :
    PluginClassHandler <BicubicWindow, CompWindow> (window),
    cWindow (CompositeWindow::get (window)),
    gWindow (GLWindow::get (window))
{
    GLWindowInterface::setHandler (gWindow);
}

bool
BicubicPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;
    if (!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI))
	return false;
    if (!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

    return true;
}
