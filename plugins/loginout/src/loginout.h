/*
 * Compiz login/logout effect plugin
 *
 * loginout.cpp
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

#include <core/core.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>

#include "loginout_options.h"

class LoginoutScreen :
    public PluginClassHandler <LoginoutScreen, CompScreen>,
    public LoginoutOptions,
    public ScreenInterface,
    public CompositeScreenInterface
{
    public:

	LoginoutScreen (CompScreen *);
	~LoginoutScreen ();

	CompositeScreen *cScreen;

	Atom kdeLogoutInfoAtom;
	Atom wmSnSelectionWindow;

	int  numLoginWin;
	int  numLogoutWin;

	float brightness;
	float saturation;
	float opacity;

	float in;
	float out;

	void
	updateWindowMatch (CompWindow *);

	void
	optionChanged (CompOption 	        *opt,
		       LoginoutOptions::Options num);

	void
	preparePaint (int);

	void
	donePaint ();

	void
	matchExpHandlerChanged ();

	void
	matchPropertyChanged (CompWindow *);
};

class LoginoutWindow :
    public PluginClassHandler <LoginoutWindow, CompWindow>,
    public WindowInterface,
    public GLWindowInterface
{
    public:

	LoginoutWindow (CompWindow *);
	~LoginoutWindow ();

	CompWindow *window;
	GLWindow   *gWindow;

	bool login;
	bool logout;

	bool glPaint (const GLWindowPaintAttrib &,
		      const GLMatrix &,
		      const CompRegion &,
		      unsigned int);

	bool glDraw (const GLMatrix &,
		     GLFragment::Attrib &,
		     const CompRegion &,
		     unsigned int);
};

#define LOGINOUT_SCREEN(s)						       \
    LoginoutScreen *ls = LoginoutScreen::get (s);

#define LOGINOUT_WINDOW(w)						       \
    LoginoutWindow *lw = LoginoutWindow::get (w);

class LoginoutPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <LoginoutScreen, LoginoutWindow>
{
    public:

	bool init ();
};
