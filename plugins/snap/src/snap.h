/*
 * Compiz snap plugin
 * Author : Guillaume "iXce" Seguin
 * Email  : ixce@beryl-project.org
 *
 * Ported to compiz by : Patrick "marex" Niklaus
 * Email               : marex@beryl-project.org
 *
 * Ported to C++ by : Travis Watkins
 * Email            : amaranth@ubuntu.com
 *
 * Copyright (C) 2009 Guillaume Seguin
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
 */

#include <vector>

#include <core/core.h>
#include <core/pluginclasshandler.h>

#include "snap_options.h"

/*
 * The window we should snap too if snapping to windows
 */
#define SNAP_WINDOW_TYPE (CompWindowTypeNormalMask  | \
			  CompWindowTypeToolbarMask | \
			  CompWindowTypeMenuMask    | \
			  CompWindowTypeUtilMask)

#define VerticalSnap	(1L << 0)
#define HorizontalSnap	(1L << 1)

#define MoveGrab		(1L << 0)
#define ResizeGrab		(1L << 1)

typedef enum
{
    LeftEdge = 0,
    RightEdge,
    TopEdge,
    BottomEdge
} EdgeType;

/* Custom Edge struct
 * Position, start, end meanings are specific to type :
 *  - LeftEdge/RightEdge : position : x, start/end : y1/y2
 *  - TopEdge/BottomEdge : position : y, start/end : x1/x2
 * id/passed are used during visibility detection when adding edges
 * snapped is straight forward
 */
typedef struct
{
    int position;
    int start;
    int end;
    EdgeType type;
    bool screenEdge;

    Window id;
    bool passed;

    bool snapped;
} Edge;

class SnapScreen :
    public ScreenInterface,
    public PluginClassHandler <SnapScreen, CompScreen>,
    public SnapOptions
{
    public:
	bool snapping;

	SnapScreen (CompScreen *s);

	void handleEvent (XEvent *event);
	bool enableSnapping (CompAction *action, CompAction::State state,
			     CompOption::Vector &options);
	bool disableSnapping (CompAction *action, CompAction::State state,
			      CompOption::Vector &options);
	void optionChanged (CompOption *opt, SnapOptions::Options num);

    private:
	// used to allow moving windows without snapping
	int avoidSnapMask;
};

class SnapWindow :
    public WindowInterface,
    public PluginClassHandler <SnapWindow, CompWindow>
{
    public:
	SnapWindow (CompWindow *window);
	~SnapWindow ();

	void resizeNotify (int dx, int dy, int dwidth, int dheight);
	void moveNotify (int dx, int dy, bool immediate);
	void grabNotify (int x, int y, unsigned int state, unsigned int mask);
	void stateChangeNotify (unsigned int lastState);
	void ungrabNotify ();

    private:
	CompWindow *window;

	// linked lists
	std::list<Edge> edges;

	// bitfield
	int snapDirection;

	// dx/dy/dw/dh when a window is resisting to user
	int m_dx;
	int m_dy;
	int m_dwidth;
	int m_dheight;

	// internals
	CompWindow::Geometry snapGeometry;
	int grabbed;

	// internal, avoids infinite notify loops
	bool skipNotify;

	void move (int dx, int dy, bool sync);
	void resize (int dx, int dy, int dwidth, int dheight);

	void addEdge (Window id, int position, int start, int end,
		      EdgeType type, bool screenEdge);
	void addRegionEdges (Edge *parent, CompRegion region);
	void updateWindowsEdges ();
	void updateScreenEdges ();
	void updateEdges ();
	void moveCheckNearestEdge (int position, int start, int end,
				   bool before, EdgeType type,
				   int snapDirection);
	void moveCheckEdges (int snapDirection);
	void resizeCheckNearestEdge (int position, int start, int end,
				     bool before, EdgeType type,
				     int snapDirection);
	void resizeCheckEdges (int dx, int dy, int dwidth, int dheight);
};

#define SNAP_SCREEN(s) \
    SnapScreen *ss = SnapScreen::get (s)

#define SNAP_WINDOW(w) \
    SnapWindow *sw = SnapWindow::get (w)

class SnapPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <SnapScreen, SnapWindow>
{
    public:
	bool init ();
};

