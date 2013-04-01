/*
 * Compiz motion blur effect plugin
 *
 * mblur.c
 *
 * Copyright : (C) 2007 by Dennis Kasprzyk
 * E-mail    : onestone@beryl-project.org
 *
 * Ported to Compiz 0.9 by
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

#include <cmath>

#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>

#include "mblur_options.h"

class MblurScreen :
    public PluginClassHandler <MblurScreen, CompScreen>,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public MblurOptions
{
    public:

	MblurScreen (CompScreen *);
	~MblurScreen ();

	CompositeScreen *cScreen;
	GLScreen        *gScreen;

	bool     active;
	bool	 update; /* is an update of the motion blur texture needed? */

	float    alpha; /* motion blur blending value */
	float    timer; /* motion blur fadeout time */
	bool     activated;

	GLuint   texture;

	/* functions that we will intercept */
	void
	preparePaint (int);

	bool
	glPaintOutput (const GLScreenPaintAttrib &,
		       const GLMatrix		 &,
		       const CompRegion	         &,
		       CompOutput		 *,
		       unsigned int		   );

	void
	glPaintTransformedOutput (const GLScreenPaintAttrib &,
		       		  const GLMatrix	    &,
		       		  const CompRegion	    &,
		       		  CompOutput		    *,
		       		  unsigned int		     );

	bool
	toggle (CompAction         *action,
		CompAction::State  state,
		CompOption::Vector options);
};

#define MBLUR_SCREEN(s)							       \
    MblurScreen *ms = MblurScreen::get (s)

class MblurPluginVTable :
    public CompPlugin::VTableForScreen <MblurScreen>
{
    public:

	bool init ();
};
