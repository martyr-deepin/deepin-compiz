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
#include "glow.h"
#include "viewport-member-window.h"
#include "client-list-generator.h"

#define WIN_REAL_X(w) (w->x () - w->border ().left)
#define WIN_REAL_Y(w) (w->y () - w->border ().top)
#define WIN_REAL_WIDTH(w) (w->width () + 2 * w->geometry ().border () + \
			   w->border ().left + w->border ().right)
#define WIN_REAL_HEIGHT(w) (w->height () + 2 * w->geometry ().border () + \
			    w->border ().top + w->border ().bottom)

namespace compiz
{
    namespace expo
    {
	namespace impl
	{
	    namespace ce = compiz::expo;

	    class CompizClientListGenerator :
		public ce::ClientListGenerator
	    {
		public:

		    CompizClientListGenerator (CompScreen *screen);

		    void refreshClientList ();
		    ViewportMemberWindow * nextClient ();

		private:

		    CompScreen                 *mScreen;
		    const CompWindowVector     *mClientList;
		    CompWindowVector::const_iterator mClientListIterator;
	    };
	}
    }
}

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
	                               CompOutput*, unsigned int);

	const CompWindowList & getWindowPaintList ();

	bool dndInit (CompAction *, CompAction::State, CompOption::Vector&);
	bool dndFini (CompAction *, CompAction::State, CompOption::Vector&);
	bool doExpo (CompAction *, CompAction::State, CompOption::Vector&);
	bool exitExpo (CompAction *, CompAction::State, CompOption::Vector&);
	bool termExpo (CompAction *, CompAction::State, CompOption::Vector&);
	bool nextVp (CompAction *, CompAction::State, CompOption::Vector&);
	bool prevVp (CompAction *, CompAction::State, CompOption::Vector&);

	CompPoint currentViewport ();

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
	CompWindowList dndWindows;

	CompPoint prevCursor;
	CompPoint newCursor;
	CompPoint prevClickPoint;

	CompPoint origVp;
	CompPoint selectedVp;
	CompPoint lastSelectedVp;
	CompPoint paintingVp;

	std::vector<float> vpActivity;
	std::vector<bool>  vpActive;
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

	GLTexture::List polkadots_texture;
	CompSize        polkadots_texture_size;
	CompSize        viewport_size;

	GLTexture::List outline_texture;
	CompSize        outline_texture_size;

	bool paintingDndWindow;

	const GlowTextureProperties *mGlowTextureProperties;

    private:
	void moveFocusViewport (int, int);
	void finishWindowMovement ();
	void updateWraps (bool);

	void invertTransformedVertex (const GLScreenPaintAttrib&,
				      const GLMatrix&, CompOutput *, int[2]);
	void paintWall (const GLScreenPaintAttrib&, const GLMatrix&,
			const CompRegion&, CompOutput *, unsigned int, bool);

	void paintViewport (const GLScreenPaintAttrib& attrib,
			    const GLMatrix&            transform,
			    const CompRegion&          region,
			    CompOutput                 *output,
			    unsigned int               mask,
			    CompPoint                  vpPos,
			    GLVector                   &vpCamPos,
			    bool                       reflection);

	bool windowsOnVp (compiz::expo::ClientListGenerator &clientList,
			  CompPoint                         &p,
			  const CompPoint		    &unprojectedCursor,
			  const CompSize		    &screenSize,
			  CompScreen			    *screen);

	KeyCode leftKey;
	KeyCode rightKey;
	KeyCode upKey;
	KeyCode downKey;

	Cursor  mMoveCursor;
};

class ExpoWindow :
    public compiz::expo::ViewportMemberWindow,
    public CompositeWindowInterface,
    public GLWindowInterface,
    public WindowInterface,
    public PluginClassHandler<ExpoWindow, CompWindow>
{
    public:
	ExpoWindow (CompWindow *);
	~ExpoWindow ();

	bool damageRect (bool, const CompRect&);

	void resizeNotify (int dx, int dy, int dw, int dh);
	void moveNotify (int dx, int dy, bool immediate);

	bool glDraw (const GLMatrix&, const GLWindowPaintAttrib&,
		     const CompRegion&, unsigned int);
	bool glPaint (const GLWindowPaintAttrib&, const GLMatrix&,
		      const CompRegion&, unsigned int);
	void glAddGeometry (const GLTexture::MatrixList&,
			    const CompRegion&, const CompRegion&,
			    unsigned int, unsigned int);
	void glDrawTexture (GLTexture*, const GLMatrix&,
	                    const GLWindowPaintAttrib&, unsigned int);
	void
	paintGlow (const GLMatrix            &transform,
	           const GLWindowPaintAttrib &attrib,
		   const CompRegion	     &paintRegion,
		   unsigned int		     mask);

	void
	computeGlowQuads (GLTexture::Matrix *matrix);

	CompWindow      *window;
	CompositeWindow *cWindow;
	GLWindow        *gWindow;
	ExpoScreen      *eScreen;

	float           dndOpacity;

	GlowQuad *mGlowQuads;

    private:

	bool isDesktopOrDock () const;
	bool dragged () const;
	const compiz::window::Geometry & absoluteGeometry () const;

	mutable compiz::window::Geometry mAbsoluteGeometry;
};

class ExpoPluginVTable :
    public CompPlugin::VTableForScreenAndWindow<ExpoScreen, ExpoWindow>
{
    public:
	bool init ();
};

