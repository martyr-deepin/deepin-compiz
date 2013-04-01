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

#include <math.h>
#include <string.h>

#include <core/core.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>

#include "showrepaint_options.h"

class ShowrepaintScreen :
    public GLScreenInterface,
    public PluginClassHandler <ShowrepaintScreen, CompScreen>,
    public ShowrepaintOptions
{
    public:

	ShowrepaintScreen (CompScreen *);
	~ShowrepaintScreen ();

    private:

	bool   active;
	CompRegion tmpRegion;

	// GLScreenInterface method
	bool glPaintOutput (const GLScreenPaintAttrib &,
			    const GLMatrix &,
			    const CompRegion &,
			    CompOutput *,
			    unsigned int);

	bool toggle (CompAction         *action,
		     CompAction::State  state,
		     CompOption::Vector options);

	CompositeScreen *cScreen;
	GLScreen        *gScreen;
};


class ShowrepaintPluginVTable :
    public CompPlugin::VTableForScreen <ShowrepaintScreen>
{
    public:

	bool init ();
};
