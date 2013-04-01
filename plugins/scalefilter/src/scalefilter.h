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
#include <scale/scale.h>
#include <text/text.h>
#include "scalefilter_options.h"

/* forward declaration */
class ScalefilterScreen;

class FilterInfo
{
    public:
	FilterInfo (ScalefilterScreen *, const CompOutput&);

	void update ();
	bool hasText () const;
	void renderText ();
	void drawText (const CompOutput *, const GLMatrix&) const;
	void damageTextRect () const;

	bool handleInput (const wchar_t input);
	bool handleBackspace ();

	const CompMatch& getMatch () const;

    private:
	static const unsigned int maxFilterSize = 32;
	static const unsigned int maxFilterStringLength = maxFilterSize + 1;

	bool timeout ();

	const CompOutput& outputDevice;

	wchar_t      filterString[maxFilterStringLength];
	unsigned int stringLength;

	CompMatch filterMatch;

	bool      textValid;
	CompText  text;
	CompTimer timer;

	ScalefilterScreen *fScreen;
};

class ScalefilterScreen :
    public PluginClassHandler <ScalefilterScreen, CompScreen>,
    public ScreenInterface,
    public ScaleScreenInterface,
    public GLScreenInterface,
    public ScalefilterOptions
{
    public:

	ScalefilterScreen (CompScreen *);
	~ScalefilterScreen ();

	bool
	glPaintOutput (const GLScreenPaintAttrib &,
		       const GLMatrix &, const CompRegion &,
		       CompOutput *, unsigned int);

	void
	handleCompizEvent (const char *, const char *, CompOption::Vector &);

	void
	handleEvent (XEvent *);

	bool
	removeFilter ();

	bool
	hasFilter () const;

    private:
	bool
	handleSpecialKeyPress (XKeyEvent *, bool &);

	void
	handleTextKeyPress (XKeyEvent *);

	void
	handleWindowRemove (Window);

	bool
	filterTimeout ();

	void
	doRelayout ();

	void
	relayout ();

	void
	optionChanged (CompOption *, Options);

	XIM xim;
	XIC xic;

	FilterInfo *filterInfo;

	bool      matchApplied;
	CompMatch persistentMatch;

    public:
	GLScreen        *gScreen;
	CompositeScreen *cScreen;
	ScaleScreen     *sScreen;
};

class ScalefilterWindow :
    public PluginClassHandler <ScalefilterWindow, CompWindow>,
    public ScaleWindowInterface
{
    public:
	ScalefilterWindow (CompWindow *);

	CompWindow  *window;
	ScaleWindow *sWindow;

	bool
	setScaledPaintAttributes (GLWindowPaintAttrib &);
};

class ScalefilterPluginVTable :
    public CompPlugin::VTableForScreenAndWindow
    <ScalefilterScreen, ScalefilterWindow>
{
    public:

	bool init ();
};
