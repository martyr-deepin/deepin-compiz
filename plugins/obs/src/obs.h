/*
 * Copyright © 2008 Danny Baumann
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Danny Baumann not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Danny Baumann makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DANNY BAUMANN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DENNIS KASPRZYK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Danny Baumann <dannybaumann@web.de>
 */

#include <core/screen.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>

#include "obs_options.h"

extern const unsigned short MODIFIER_OPACITY;
extern const unsigned short MODIFIER_SATURATION;
extern const unsigned short MODIFIER_BRIGHTNESS;
#define MODIFIER_COUNT 3

class ObsScreen :
    public ScreenInterface,
    public PluginClassHandler <ObsScreen, CompScreen>,
    public ObsOptions
{
    public:
	ObsScreen (CompScreen *);

	bool setOption (const CompString &name, CompOption::Value &value);

	void matchPropertyChanged (CompWindow *);
	void matchExpHandlerChanged ();

	CompOption *stepOptions[MODIFIER_COUNT];
	CompOption *matchOptions[MODIFIER_COUNT];
	CompOption *valueOptions[MODIFIER_COUNT];
};

class ObsWindow :
    public GLWindowInterface,
    public PluginClassHandler<ObsWindow, CompWindow>
{
    public:
	ObsWindow (CompWindow *);
	~ObsWindow ();

	bool glPaint (const GLWindowPaintAttrib &, const GLMatrix &,
		      const CompRegion &, unsigned int);
	bool glDraw (const GLMatrix &, const GLWindowPaintAttrib &,
		     const CompRegion &, unsigned int);

	void changePaintModifier (unsigned int, int);
	void updatePaintModifier (unsigned int);
	void modifierChanged (unsigned int);
	bool updateTimeout ();

    private:
	CompWindow      *window;
	CompositeWindow *cWindow;
	GLWindow        *gWindow;
	ObsScreen       *oScreen;

	int customFactor[MODIFIER_COUNT];
	int matchFactor[MODIFIER_COUNT];
	
	CompTimer	updateHandle;
};

class ObsPluginVTable :
    public CompPlugin::VTableForScreenAndWindow<ObsScreen, ObsWindow>
{
    public:
	bool init ();
};
