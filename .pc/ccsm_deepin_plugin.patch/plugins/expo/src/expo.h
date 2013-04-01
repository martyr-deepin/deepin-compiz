/**
 *
 * Compiz expo plugin
 *
 * expo.c
 *
 * Copyright (c) 2008 Dennis Kasprzyk <racarr@opencompositing.org>
 * Copyright (c) 2006 Robert Carr <racarr@beryl-project.org>
 *
 * Authors:
 * Robert Carr <racarr@beryl-project.org>
 * Dennis Kasprzyk <onestone@opencompositing.org>
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
 **/

#include <core/core.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>

#include "expo_options.h"

class ExpoScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public PluginClassHandler<ExpoScreen, CompScreen>,
    public ExpoOptions
{
    public:
	ExpoScreen (CompScreen *);
	~ExpoScreen ();

	void handleEvent (XEvent *);

	void preparePaint (int);
	void paint (CompOutput::ptrList&, unsigned int);
	void donePaint ();

	bool glPaintOutput (const GLScreenPaintAttrib&, const GLMatrix&,
			    const CompRegion&, CompOutput *, unsigned int);
	void glPaintTransformedOutput (const GLScreenPaintAttrib&,
				       const GLMatrix&, const CompRegion&,
				       CompOutput *, unsigned int);

	bool dndInit (CompAction *, CompAction::State, CompOption::Vector&);
	bool dndFini (CompAction *, CompAction::State, CompOption::Vector&);
	bool doExpo (CompAction *, CompAction::State, CompOption::Vector&);
	bool exitExpo (CompAction *, CompAction::State, CompOption::Vector&);
	bool termExpo (CompAction *, CompAction::State, CompOption::Vector&);
	bool nextVp (CompAction *, CompAction::State, CompOption::Vector&);
	bool prevVp (CompAction *, CompAction::State, CompOption::Vector&);

	typedef enum {
	    DnDNone,
	    DnDDuring,
	    DnDStart
	} DnDState;

	typedef enum {
	    VPUpdateNone,
	    VPUpdateMouseOver,
	    VPUpdatePrevious
	} VPUpdateMode;

	CompositeScreen *cScreen;
	GLScreen        *gScreen;

	float expoCam;
	bool  expoActive;
	bool  expoMode;

	DnDState   dndState;
	CompWindow *dndWindow;

	CompPoint prevCursor;
	CompPoint newCursor;
	CompPoint prevClickPoint;

	CompPoint origVp;
	CompPoint selectedVp;
	CompPoint lastSelectedVp;
	CompPoint paintingVp;

	std::vector<float> vpActivity;
	float              vpBrightness;
	float              vpSaturation;

	VPUpdateMode vpUpdateMode;

	bool         anyClick;
	unsigned int clickTime;
	bool         doubleClick;

	CompRegion tmpRegion;

	float curveAngle;
	float curveDistance;
	float curveRadius;

	std::vector<GLfloat> vpNormals;

	CompScreen::GrabHandle grabIndex;

    private:
	void moveFocusViewport (int, int);
	void finishWindowMovement ();
	void updateWraps (bool);

	void invertTransformedVertex (const GLScreenPaintAttrib&,
				      const GLMatrix&, CompOutput *, int[2]);
	void paintWall (const GLScreenPaintAttrib&, const GLMatrix&,
			const CompRegion&, CompOutput *, unsigned int, bool);

	KeyCode leftKey;
	KeyCode rightKey;
	KeyCode upKey;
	KeyCode downKey;

	Cursor dragCursor;
};

class ExpoWindow :
    public CompositeWindowInterface,
    public GLWindowInterface,
    public PluginClassHandler<ExpoWindow, CompWindow>
{
    public:
	ExpoWindow (CompWindow *);

	bool damageRect (bool, const CompRect&);

	bool glDraw (const GLMatrix&, const GLWindowPaintAttrib&,
		     const CompRegion&, unsigned int);
	bool glPaint (const GLWindowPaintAttrib&, const GLMatrix&,
		      const CompRegion&, unsigned int);
	void glAddGeometry (const GLTexture::MatrixList&,
			    const CompRegion&, const CompRegion&,
			    unsigned int, unsigned int);
	void glDrawTexture (GLTexture*, const GLMatrix&,
	                    const GLWindowPaintAttrib&, unsigned int);

	CompWindow      *window;
	CompositeWindow *cWindow;
	GLWindow        *gWindow;
	ExpoScreen      *eScreen;
};

class ExpoPluginVTable :
    public CompPlugin::VTableForScreenAndWindow<ExpoScreen, ExpoWindow>
{
    public:
	bool init ();
};

