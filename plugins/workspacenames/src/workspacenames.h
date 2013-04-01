/*
 *
 * Compiz workspace name display plugin
 *
 * workspacenames.h
 *
 * Copyright : (C) 2008 by Danny Baumann
 * E-mail    : maniac@compiz-fusion.org
 * 
 * Ported to Compiz 0.9.x
 * Copyright : (c) 2010 Scott Moreau <oreaus@gmail.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <core/core.h>
#include <core/atoms.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <text/text.h>

#include "workspacenames_options.h"

class WSNamesScreen :
    public PluginClassHandler <WSNamesScreen, CompScreen>,
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public WorkspacenamesOptions
{
    public:
	WSNamesScreen (CompScreen *screen);
	~WSNamesScreen ();

	CompositeScreen *cScreen;
	GLScreen	*gScreen;

	CompText	textData;
	CompTimer	timeoutHandle;
	int		timer;

	CompString
	getCurrentWSName ();

	void
	renderNameText ();

	void
	drawText (const GLMatrix &);

	bool
	glPaintOutput (const GLScreenPaintAttrib &attrib,
		       const GLMatrix	         &transform,
		       const CompRegion	         &region,
		       CompOutput	         *output,
		       unsigned int              mask);

	void
	preparePaint (int ms);

	void
	donePaint ();

	bool
	hideTimeout ();

	void
	handleEvent (XEvent *);
};

class WorkspacenamesPluginVTable :
    public CompPlugin::VTableForScreen <WSNamesScreen>
{
    public:
	bool init ();
};

COMPIZ_PLUGIN_20090315 (workspacenames, WorkspacenamesPluginVTable);
