/*
 * Compiz Fusion Grid plugin
 *
 * Copyright (c) 2008 Stephen Kennedy <suasol@gmail.com>
 * Copyright (c) 2010 Scott Moreau <oreaus@gmail.com>
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
 * Description:
 *
 * Plugin to act like winsplit revolution (http://www.winsplit-revolution.com/)
 * use <Control><Alt>NUMPAD_KEY to move and tile your windows.
 *
 * Press the tiling keys several times to cycle through some tiling options.
 */

#include <core/core.h>
#include <core/atoms.h>
#include <core/pluginclasshandler.h>
#include <composite/composite.h>
#include <opengl/opengl.h>

#include "grid_options.h"

static const unsigned short SNAPOFF_THRESHOLD = 50;

namespace GridWindowType
{
    static const unsigned int GridUnknown = (1 << 0);
    static const unsigned int GridBottomLeft  = (1 << 1);
    static const unsigned int GridBottom  = (1 << 2);
    static const unsigned int GridBottomRight = (1 << 3);
    static const unsigned int GridLeft  = (1 << 4);
    static const unsigned int GridCenter  = (1 << 5);
    static const unsigned int GridRight  = (1 << 6);
    static const unsigned int GridTopLeft  = (1 << 7);
    static const unsigned int GridTop  = (1 << 8);
    static const unsigned int GridTopRight  = (1 << 9);
    static const unsigned int GridMaximize  = (1 << 10);
};

typedef unsigned int GridType;

class GridProps
{
public:

    GridProps ():
	gravityRight (0),
	gravityDown (0),
	numCellsX (0),
	numCellsY (0)
    {}

    GridProps (int r, int d, int x, int y):
	gravityRight (r),
	gravityDown (d),
	numCellsX (x),
	numCellsY (y)
    {
    }

    int gravityRight;
    int gravityDown;
    int numCellsX;
    int numCellsY;
};

enum Edges
{
    NoEdge = 0,
    BottomLeft,
    Bottom,
    BottomRight,
    Left,
    Right,
    TopLeft,
    Top,
    TopRight
};

class Animation
{
	public:

	Animation ();

	GLfloat progress;
	CompRect fromRect;
	CompRect targetRect;
	CompRect currentRect;
	GLfloat opacity;
	GLfloat timer;
	Window window;
	int duration;
	bool complete;
	bool fadingOut;
};

class GridScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public PluginClassHandler <GridScreen, CompScreen>,
    public GridOptions
{
    public:

	GridScreen (CompScreen *);
	CompositeScreen *cScreen;
	GLScreen        *glScreen;

	CompRect workarea, currentRect, desiredSlot, lastSlot,
		 desiredRect, lastWorkarea, currentWorkarea;
	GridProps props;
	Edges edge, lastEdge, lastResizeEdge;
	CompOption::Vector o;
	bool centerCheck;
	CompWindow *mGrabWindow;
	bool animating;
	bool mSwitchingVp;

	void getPaintRectangle (CompRect&);
	void setCurrentRect (Animation&);

	bool initiateCommon (CompAction*, CompAction::State,
			     CompOption::Vector&, unsigned int, bool, bool);

	void glPaintRectangle (const GLScreenPaintAttrib&,
			       const GLMatrix&, CompOutput *);

	bool glPaintOutput (const GLScreenPaintAttrib &,
			    const GLMatrix &, const CompRegion &,
			    CompOutput *, unsigned int);

	void preparePaint (int msSinceLastPaint);
	void donePaint ();

	std::vector <Animation> animations;

	int edgeToGridType ();
	unsigned int typeToMask (int);

	void handleEvent (XEvent *event);
	void handleCompizEvent (const char *plugin, const char *event, CompOption::Vector &options);

	bool restoreWindow (CompAction*,
			    CompAction::State,
			    CompOption::Vector&);

	void
	snapbackOptionChanged (CompOption *option,
				Options    num);

	CompRect
	slotToRect (CompWindow      *w,
		    const CompRect& slot);
	CompRect
	constrainSize (CompWindow *w,
		       const CompRect& slot);
};

class GridWindow :
    public WindowInterface,
    public GLWindowInterface,
    public PluginClassHandler <GridWindow, CompWindow>
{
    public:

	GridWindow (CompWindow *);
	~GridWindow ();
	CompWindow *window;
    	GLWindow *gWindow;
	GridScreen *gScreen;

	bool isGridResized;
	bool isGridHorzMaximized;
	bool isGridVertMaximized;
	unsigned int grabMask;
	int pointerBufDx;
	int pointerBufDy;
	int resizeCount;
	CompRect currentSize;
	CompRect originalSize;
	GridType lastTarget;
	unsigned int sizeHintsFlags;

	bool glPaint (const GLWindowPaintAttrib&, const GLMatrix&,
		      const CompRegion&, unsigned int);

	void grabNotify (int, int, unsigned int, unsigned int);

	void ungrabNotify ();

	void moveNotify (int, int, bool);

	void stateChangeNotify (unsigned int);
	void validateResizeRequest (unsigned int &valueMask,
				    XWindowChanges *xwc,
				    unsigned int source);
};

#define GRID_WINDOW(w) \
    GridWindow *gw = GridWindow::get (w)

class GridPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <GridScreen, GridWindow>
{
    public:

	bool init ();
};

COMPIZ_PLUGIN_20090315 (grid, GridPluginVTable);

