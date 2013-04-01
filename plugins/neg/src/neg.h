/*
 * Copyright (c) 2006 Darryll Truchan <moppsy@comcast.net>
 *
 * Pixel shader negating by Dennis Kasprzyk <onestone@beryl-project.org>
 * Usage of matches by Danny Baumann <maniac@beryl-project.org>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <core/core.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>


#include "neg_options.h"

class NegScreen :
    public PluginClassHandler <NegScreen, CompScreen>,
    public NegOptions
{
    public:

	NegScreen (CompScreen *);

	int negFunction;
	int negAlphaFunction;

	bool isNeg;
	
	bool
	checkStateTimeout ();

	void
	optionChanged (CompOption          *opt,
		       NegOptions::Options num);

	bool
	toggle (CompAction         *action,
		CompAction::State  state,
		CompOption::Vector opt,
		bool		   all);

	GLScreen *gScreen;
};

class NegWindow :
    public PluginClassHandler <NegWindow, CompWindow>,
    public GLWindowInterface
{
    public:
    
	NegWindow (CompWindow *);

	CompWindow      *window;
	CompositeWindow *cWindow;
	GLWindow        *gWindow;

	bool isNeg;

	void
	glDrawTexture (GLTexture                 *texture,
	               const GLMatrix            &transform,
	               const GLWindowPaintAttrib &attrib,
		      unsigned int       mask);

	void toggle ();
};

#define NEG_SCREEN(s)							      \
    NegScreen *ns = NegScreen::get (s);

#define NEG_WINDOW(w)							      \
    NegWindow *nw = NegWindow::get (w);

class NegPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <NegScreen, NegWindow>
{
    public:

	bool init ();
};
