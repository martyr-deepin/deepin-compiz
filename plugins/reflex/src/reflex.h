/*
 * Compiz reflection effect plugin
 *
 * reflex.h
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

#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>

#include "reflex_options.h"

class ReflexFunction
{
    public:
	GLFragment::FunctionId handle;
	int target;
	int param;
	int unit;
};

class ReflexScreen :
    public PluginClassHandler <ReflexScreen, CompScreen>,
    public ScreenInterface,
    public ReflexOptions
{
    public:

	ReflexScreen (CompScreen *);
	~ReflexScreen ();

	CompositeScreen *cScreen;
	GLScreen	*gScreen;

	bool		 imageLoaded;
	GLTexture::List  image;

	unsigned int width;
	unsigned int height;

	std::list <ReflexFunction *> reflexFunctions;

	void matchExpHandlerChanged ();

	void matchPropertyChanged (CompWindow *window);

	GLFragment::FunctionId
	getReflexFragmentFunction (GLTexture   *texture,
				   int         param,
				   int         unit);
	void
	optionChanged (CompOption             *opt,
		       ReflexOptions::Options num);

	void
	destroyFragmentFunctions ();

};

#define REFLEX_SCREEN(s)						       \
    ReflexScreen *rs = ReflexScreen::get (s)

class ReflexWindow :
    public PluginClassHandler <ReflexWindow, CompWindow>,
    public GLWindowInterface
{
    public:

	ReflexWindow (CompWindow *);

	CompWindow      *window;
	CompositeWindow *cWindow;
	GLWindow	*gWindow;

	bool active;

	void glDrawTexture (GLTexture *texture, GLFragment::Attrib &,
			    unsigned int);

	void
	updateMatch ();
};

#define REFLEX_WINDOW(w)						       \
    ReflexWindow *rw = ReflexWindow::get (w)

class ReflexPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <ReflexScreen, ReflexWindow>
{
    public:

	bool init ();
};
