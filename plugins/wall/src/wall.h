/**
 *
 * Compiz wall plugin
 *
 * wall.h
 *
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
#include <mousepoll/mousepoll.h>

#include <cairo-xlib-xrender.h>
#include <cairo.h>

#include "offset-movement.h"

#include "wall_options.h"


/* enums */
typedef enum
{
    NoTransformation,
    MiniScreen,
    Sliding
} ScreenTransformation;

/* FIXME: put into own class? */
typedef struct _WallCairoContext
{
    Pixmap          pixmap;
    GLTexture::List texture;

    cairo_surface_t *surface;
    cairo_t         *cr;

    int width;
    int height;
} WallCairoContext;

/* classes */
class WallScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public PluginClassHandler <WallScreen, CompScreen>,
    public WallOptions
{
    public:
	enum Direction
	{
	    Up = 0,
	    Left,
	    Down,
	    Right,
	    Next,
	    Prev
	};

	WallScreen (CompScreen *s);
	~WallScreen ();

	void preparePaint (int);
	void paint (CompOutput::ptrList &, unsigned int);
	void donePaint ();
	void handleEvent (XEvent *event);

	bool glPaintOutput (const GLScreenPaintAttrib &,
			    const GLMatrix &, const CompRegion &,
			    CompOutput *, unsigned int);
	void glPaintTransformedOutput (const GLScreenPaintAttrib &,
				       const GLMatrix &,
				       const CompRegion &,
				       CompOutput *, unsigned int);

	bool setOptionForPlugin (const char *, const char *,
				 CompOption::Value&);
	void matchExpHandlerChanged ();
	void matchPropertyChanged (CompWindow *);

	void createCairoContexts (bool);
	void setupCairoContext (WallCairoContext &);
	void destroyCairoContext (WallCairoContext &);
	void clearCairoLayer (cairo_t *);
	void drawSwitcherBackground ();
	void drawThumb ();
	void drawHighlight ();
	void drawArrow ();
	void drawCairoTextureOnScreen (const GLMatrix &transform);

	void releaseMoveWindow ();
	void computeTranslation (float &, float &);
	void determineMovementAngle ();
	bool checkDestination (unsigned int, unsigned int);
	void checkAmount (int, int, int &, int &);

	bool initiate (CompAction *, CompAction::State, CompOption::Vector &,
		       Direction, bool);
	bool terminate (CompAction *, CompAction::State, CompOption::Vector &);
	bool initiateFlip (Direction, CompAction::State);

	bool moveViewport (int, int, Window);

	void optionChanged (CompOption *opt, WallOptions::Options num);
	void toggleEdges (bool);

	void positionUpdate (const CompPoint &pos);
	void updateScreenEdgeRegions ();

	CompositeScreen *cScreen;
	GLScreen        *glScreen;

	bool moving; /* Used to track miniview movement */
	bool showPreview;

	float        curPosX;
	float        curPosY;
	unsigned int gotoX;
	unsigned int gotoY;

	int direction; /* >= 0 : direction arrow angle, < 0 : no direction */

	int          boxTimeout;
	unsigned int boxOutputDevice;
	CompScreen::GrabHandle grabIndex;
	int          timer;

	Window moveWindow;

	bool focusDefault;

	ScreenTransformation transform;
	CompOutput          *currOutput;

	GLWindowPaintAttrib mSAttribs;
	float               mSzCamera;

	int firstViewportX;
	int firstViewportY;
	int viewportWidth;
	int viewportHeight;
	int viewportBorder;

	int moveWindowX;
	int moveWindowY;

	WallCairoContext switcherContext;
	WallCairoContext thumbContext;
	WallCairoContext highlightContext;
	WallCairoContext arrowContext;

	MousePoller	 poller;
	bool		 edgeDrag;
	CompRegion	 edgeRegion;
	CompRegion	 noEdgeRegion;
};

class WallWindow :
	public WindowInterface,
	public GLWindowInterface,
	public PluginClassHandler <WallWindow, CompWindow>
{
    public:
	WallWindow (CompWindow *);

	virtual void activate ();
	void grabNotify (int, int, unsigned int, unsigned int);
	void ungrabNotify ();
	bool glPaint (const GLWindowPaintAttrib &, const GLMatrix &,
		      const CompRegion &, unsigned int);

	CompWindow *window;
	GLWindow   *glWindow;

	bool isSliding;
};

#define WALL_SCREEN(s) \
    WallScreen *ws = WallScreen::get (s)

#define WALL_WINDOW(w) \
    WallWindow *ww = WallWindow::get (w)

class WallPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <WallScreen, WallWindow>
{
    public:

	bool init ();
};

