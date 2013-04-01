/*
 * Copyright © 2005 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include <core/window.h>
#include <core/pluginclasshandler.h>
#include <composite/composite.h>

#include "fade_options.h"

#include <opengl/opengl.h>

class FadeScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public PluginClassHandler<FadeScreen, CompScreen>,
    public FadeOptions
{
    public:
	FadeScreen (CompScreen *s);

	bool setOption (const CompString &, CompOption::Value &);

	bool bell (CompAction *, CompAction::State state, CompOption::Vector &);
	void handleEvent (XEvent *);
	void preparePaint (int);

	int displayModals;
	int fadeTime;

	CompositeScreen *cScreen;
};

class FadeWindow :
    public WindowInterface,
    public GLWindowInterface,
    public PluginClassHandler<FadeWindow, CompWindow>
{
    public:
	FadeWindow (CompWindow *w);
	~FadeWindow ();

	void windowNotify (CompWindowNotify);
	void paintStep (unsigned int, int, int);

	bool glPaint (const GLWindowPaintAttrib&, const GLMatrix&,
		      const CompRegion&, unsigned int);

	void addDisplayModal ();
	void removeDisplayModal ();

	void dim (bool);

    private:
	FadeScreen      *fScreen;
	CompWindow      *window;
	CompositeWindow *cWindow;
	GLWindow        *gWindow;

	GLushort opacity;
	GLushort brightness;
	GLushort saturation;

	GLushort targetOpacity;
	GLushort targetBrightness;
	GLushort targetSaturation;

	bool dModal;

	int steps;
	int fadeTime;

	int opacityDiff;
	int brightnessDiff;
	int saturationDiff;
};

class FadePluginVTable :
    public CompPlugin::VTableForScreenAndWindow<FadeScreen, FadeWindow>
{
    public:
	bool init ();
};
