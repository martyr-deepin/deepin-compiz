/*
 * Compiz reflection effect plugin
 *
 * reflex.cpp
 *
 * Copyright : (C) 2007 by Dennis Kasprzyk
 * E-mail    : onestone@beryl-project.org
 *
 * Ported to Compiz 0.9 by:
 * Copyright : (C) 2009 by Sam Spilsbury
 * E-mail    : smspillaz@gmail.com
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

#include "reflex.h"

COMPIZ_PLUGIN_20090315 (reflex, ReflexPluginVTable);

GLFragment::FunctionId
ReflexScreen::getReflexFragmentFunction (GLTexture   *texture,
					 int         param,
					 int         unit)
{
    GLFragment::FunctionData *data;

    int target;
    ReflexFunction *function;
    CompString targetString;

    if (texture->target () == GL_TEXTURE_2D)
	target = COMP_FETCH_TARGET_2D;
    else
	target = COMP_FETCH_TARGET_RECT;

    foreach (GLTexture *tex, image)
    {
	if (tex->target () == GL_TEXTURE_2D)
	    targetString = CompString ("2D");
	else
	    targetString = CompString ("RECT");
    }

    /* Do we already have a function for this? */

    foreach (ReflexFunction *f, reflexFunctions)
    {
	if (f->param  == param  &&
	    f->target == target &&
	    f->unit   == unit)
	    return f->handle;
    }

    data = new GLFragment::FunctionData ();

    if (data)
    {
	GLFragment::FunctionId handle = 0;
	char str[1024];

	data->addTempHeaderOp ("image");
	data->addTempHeaderOp ("tmp");

	data->addFetchOp ("output", NULL, target);
	data->addColorOp ("output", "output");

	snprintf (str, 1024,
		  "MAD tmp, fragment.position, program.env[%d],"
		  " program.env[%d];", param, param + 1);
	data->addDataOp (str);

	snprintf (str, 1024,
		  "TEX image, tmp, texture[%d], %s;", unit,
		  targetString.c_str ());
	data->addDataOp (str);

	snprintf (str, 1024,
		  "MUL_SAT tmp, output.a, program.env[%d].b;"
		  "MAD image, -output.a, image, image;"
		  "MAD output, image, tmp.a, output;", param + 1);
	data->addDataOp (str);

	if (!data->status ())
	{
	    delete data;
	    return 0;
	}

	function = new ReflexFunction ();
	if (function)
	{
	    handle = data->createFragmentFunction ("reflex");

	    function->handle = handle;
	    function->target = target;
	    function->param  = param;
	    function->unit   = unit;

	    reflexFunctions.push_back (function);

	}

	delete data;

	return handle;
    }

    return 0;
}

void
ReflexWindow::updateMatch ()
{
    bool      f_active;

    REFLEX_SCREEN (screen);

    f_active = rs->optionGetMatch ().evaluate (window);
    if (f_active != active)
    {
	active = f_active;
	if (active)
	    gWindow->glDrawTextureSetEnabled (this, true);
	else
	    gWindow->glDrawTextureSetEnabled (this, false);
	cWindow->addDamage ();
    }
}

void
ReflexScreen::optionChanged (CompOption         *opt,
			     ReflexOptions::Options num)
{
    CompString pname ("reflex");

    switch (num)
    {

    case ReflexOptions::File:
    {
	CompSize size;
	CompString string (optionGetFile ());
	if (imageLoaded)
	{
	    image.clear ();
	}
	image = GLTexture::readImageToTexture (string,
					       pname,
					       size);
	imageLoaded = image.size ();

	width = size.width ();
	height = size.height ();
	cScreen->damageScreen ();
    }
    break;

    case ReflexOptions::Match:
	foreach (CompWindow *w, screen->windows ())
	{
	    REFLEX_WINDOW (w);
	    rw->updateMatch ();
	}
	cScreen->damageScreen ();
	break;

    case ReflexOptions::Window:
    case ReflexOptions::Decoration:
	{
	    bool shouldEnable = (optionGetWindow () ||
				 optionGetDecoration ());

	    foreach (CompWindow *w, screen->windows ())
	    {
		REFLEX_WINDOW (w);
		shouldEnable |= optionGetMatch ().evaluate (w);
		rw->gWindow->glDrawTextureSetEnabled (rw, shouldEnable);
	    }
	    cScreen->damageScreen ();
	}

    default:
	/* FIXME: this isn't right.... */
	cScreen->damageScreen ();
	break;
    }
}

void
ReflexWindow::glDrawTexture (GLTexture		      *texture,
			     GLFragment::Attrib       &attrib,
			     unsigned int	      mask)
{
    REFLEX_SCREEN (screen);

    bool enabled;
    bool windowTexture = false;

    foreach (GLTexture *tex, gWindow->textures ())
    {
	if (tex == texture)
	    windowTexture = true;
    }

    enabled = windowTexture ? rs->optionGetWindow () :
			      rs->optionGetDecoration ();

    if (enabled && active && rs->imageLoaded &&
	GL::fragmentProgram)
    {
	GLFragment::Attrib fa = attrib;
	int function;
	int unit = 0;
	int param;
	float tx = 0.0f, ty = 0.0f, dx = 0.0f, mx = 0.0f;

	if (rs->optionGetMoving ())
	{
	    mx = window->x () + (window->width () / 2);
	    mx /= screen->width () / 2.0;
	    mx -= 1.0;
	    mx *= -0.065;
	}
	else
	    mx = 0.0;

	foreach (GLTexture *tex, rs->image)
	{
	    if (tex->target () == GL_TEXTURE_2D)
	    {
		tx = 1.0 / screen->width ();
		ty = 1.0 / screen->height ();
		dx = mx;
	    }
	    else
	    {
		tx = 1.0f / screen->width () * rs->width;
		ty = 1.0f / screen->height () * rs->height;
		dx = mx * rs->width;
	    }
	}

	unit     = fa.allocTextureUnits (1);
	param    = fa.allocParameters (2);
	function = rs->getReflexFragmentFunction (texture, param, unit);

	if (function)
	{
	    fa.addFunction (function);
	    GL::activeTexture (GL_TEXTURE0_ARB + unit);
	    foreach (GLTexture *tex, rs->image)
	    {
		tex->enable (GLTexture::Good);
		GL::activeTexture (GL_TEXTURE0_ARB);
		GL::programEnvParameter4f (GL_FRAGMENT_PROGRAM_ARB, param,
					   tx, ty, 0.0f, 0.0f);
		GL::programEnvParameter4f (GL_FRAGMENT_PROGRAM_ARB, param + 1,
					   dx, 0.0f,
					   rs->optionGetThreshold (), 0.0f);

		tex->disable ();
	    }

	}

	gWindow->glDrawTexture (texture, fa, mask);

	if (unit)
	{
	    GL::activeTexture (GL_TEXTURE0_ARB + unit);
	    foreach (GLTexture *tex, rs->image)
		tex->disable ();
	    GL::activeTexture (GL_TEXTURE0_ARB);
	}
    }
    else
    {
	gWindow->glDrawTexture (texture, attrib, mask);
    }
}

void
ReflexScreen::matchExpHandlerChanged ()
{
    screen->matchExpHandlerChanged ();

    foreach (CompWindow *w, screen->windows ())
    {
	REFLEX_WINDOW (w);
	rw->updateMatch ();
    }
}

void
ReflexScreen::matchPropertyChanged (CompWindow *w)
{
    REFLEX_WINDOW (w);

    rw->updateMatch ();

    screen->matchPropertyChanged (w);
}

void
ReflexScreen::destroyFragmentFunctions ()
{
    while (!reflexFunctions.empty ())
    {
	ReflexFunction *func = reflexFunctions.front ();
	GLFragment::destroyFragmentFunction  (func->handle);
	reflexFunctions.remove (func);
    }
}

ReflexScreen::ReflexScreen (CompScreen *) :
    PluginClassHandler <ReflexScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    imageLoaded (false),
    width (0),
    height (0)
{
    CompSize size;
    CompString string (optionGetFile ());
    CompString pname ("reflex");
    image = GLTexture::readImageToTexture (string, pname, size);
    imageLoaded = image.size ();

    width = size.width ();
    height = size.height ();

    optionSetFileNotify (boost::bind (&ReflexScreen::optionChanged, this,
				      _1, _2));
    optionSetMatchNotify (boost::bind (&ReflexScreen::optionChanged, this,
					_1, _2));
}

ReflexScreen::~ReflexScreen ()
{
    if (reflexFunctions.size ())
	destroyFragmentFunctions ();
}

ReflexWindow::ReflexWindow (CompWindow *window) :
    PluginClassHandler <ReflexWindow, CompWindow> (window),
    window (window),
    cWindow (CompositeWindow::get (window)),
    gWindow (GLWindow::get (window)),
    active (false)
{
    REFLEX_SCREEN (screen);

    GLWindowInterface::setHandler (gWindow, false);

    if (rs->optionGetWindow () || rs->optionGetDecoration ())
	gWindow->glDrawTextureSetEnabled (this, true);

    updateMatch ();
}

bool
ReflexPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;
    if (!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI))
	return false;
    if (!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

    return true;
}
