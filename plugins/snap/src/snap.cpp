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

/*
 * TODO
 *  - Apply Edge Resistance to resize
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "snap.h"


COMPIZ_PLUGIN_20090315 (snap, SnapPluginVTable);

// helper functions

/*
 * Wrapper functions to avoid infinite notify loops
 */
void
SnapWindow::move (int dx, int dy, bool sync)
{
    skipNotify = true;
    window->move (dx, dy, true);
    /* warp the pointer in the case of
     * snap release */
    if (sync)
	window->syncPosition ();
    skipNotify = false;
}

void
SnapWindow::resize (int dx, int dy, int dwidth, int dheight)
{
    const CompWindow::Geometry &geometry = window->serverGeometry ();
    skipNotify = true;
    window->resize (geometry.x () + dx, geometry.y () + dy,
		    geometry.width () + dwidth, geometry.height () + dheight,
		    geometry.border ());
    skipNotify = false;
}

void
SnapWindow::addEdge (Window   id,
		     int       position,
		     int      start,
		     int      end,
		     EdgeType type,
		     bool     screenEdge)
{
    Edge edge;

    edge.position = position;
    edge.start = start;
    edge.end = end;
    edge.type = type;
    edge.screenEdge = screenEdge;
    edge.snapped = false;
    edge.passed = false;
    edge.id = id;

    edges.push_back (edge);
}

/*
 * Add an edge for each rectangle of the region
 */
void
SnapWindow::addRegionEdges (Edge *parent, CompRegion region)
{
    int position, start, end;

    foreach (const CompRect &r, region.rects ())
    {
	switch (parent->type)
	{
	case LeftEdge:
	case RightEdge:
	    position = r.x1 ();
	    start = r.y1 ();
	    end = r.y2 ();
	    break;
	case TopEdge:
	case BottomEdge:
	default:
	    position = r.y1 ();
	    start = r.x1 ();
	    end = r.x2 ();
	}
	
	addEdge (parent->id, position, start, end,
		 parent->type, parent->screenEdge);
	edges.back ().passed = parent->passed;
    }
}

/* Checks if a window is considered a snap window. If it's
 * not visible, returns false. If it's a panel and we're
 * snapping to screen edges, it's considered a snap-window.
 */

#define UNLIKELY(x) __builtin_expect(!!(x),0)

static inline bool
isSnapWindow (CompWindow *w)
{
    SNAP_SCREEN (screen);

    if (UNLIKELY(!w))
	return false;
    if (!w->isViewable ())
	return false;
    if ((w->type () & SNAP_WINDOW_TYPE) && 
	(ss->optionGetEdgesCategoriesMask () & EdgesCategoriesWindowEdgesMask))
	return true;
    if (w->struts () && 
	(ss->optionGetEdgesCategoriesMask () & EdgesCategoriesScreenEdgesMask))
	return true;
    return false;
}

// Edges update functions ------------------------------------------------------
/*
 * Detect visible windows edges
 */

void
SnapWindow::updateWindowsEdges ()
{
    CompRegion edgeRegion, resultRegion;
    CompRect   input;
    bool       remove = false;

    // First add all the windows
    foreach (CompWindow *w, screen->windows ())
    {

	// Just check that we're not trying to snap to current window,
	// that the window is not invisible and of a valid type
	if (w == window || !isSnapWindow (w))
	{
	    continue;
	}

	input = w->serverBorderRect ();
	addEdge (w->id (), input.top (), input.left (),
		 input.right (), TopEdge, false);
	addEdge (w->id (), input.bottom (), input.left (),
		 input.right (), BottomEdge, false);
	addEdge (w->id (), input.left (), input.top (),
		 input.bottom (), LeftEdge, false);
	addEdge (w->id (), input.right (), input.top (),
		 input.bottom (), RightEdge, false);
    }

    // Now strip invisible edges
    // Loop through all the windows stack, and through all the edges
    // If an edge has been passed, check if it's in the region window,
    // if the edge is fully under the window, drop it, or if it's only
    // partly covered, cut it/split it in one/two smaller visible edges
    foreach (CompWindow *w, screen->windows ())
    {
	if (w == window || !isSnapWindow (w))
	    continue;

	// can't use foreach here because we need the iterator for erase()
	for (std::list<Edge>::iterator it = edges.begin (); it != edges.end (); )
	{
	    Edge     *e = &*it;
	    CompRect rect;

	    if (!e->passed)
	    {
		if (e->id == w->id ())
		    e->passed = true;
		++it;
		continue;
	    }

	    switch (e->type)
	    {
		case LeftEdge:
		case RightEdge:
		    rect.setGeometry (e->position,
				      e->start,
				      1,
				      e->end - e->start);
		    break;
		case TopEdge:
		case BottomEdge:
		default:
		    rect.setGeometry (e->start,
				      e->position,
				      e->end - e->start,
				      1);
	    }

	    // If the edge is in the window region, remove it,
	    // if it's partly in the region, split it
	    edgeRegion = CompRegion (rect);
	    resultRegion = edgeRegion - w->region ();
	    if (resultRegion.isEmpty ())
	    {
		remove = true;
	    }
	    else if (edgeRegion != resultRegion)
	    {
		addRegionEdges (e, resultRegion);
		remove = true;
	    }

	    if (remove)
	    {
		it = edges.erase (it);
		remove = false;
	    }
	    else
	    {
		++it;
	    }
	}
    }
}

/*
 * Loop on outputDevs and add the extents as edges
 * Note that left side is a right edge, right side a left edge,
 * top side a bottom edge and bottom side a top edge,
 * since they will be snapped as the right/left/bottom/top edge of a window
 */
void
SnapWindow::updateScreenEdges ()
{
    CompRegion edgeRegion, resultRegion;
    bool remove = false;

    foreach (CompOutput output, screen->outputDevs ())
    {
	const CompRect& area = output.workArea ();
	addEdge (0, area.top (), area.left (), area.right () - 1,
		 BottomEdge, true);
	addEdge (0, area.bottom (), area.left (), area.right () - 1,
		 TopEdge, true);
	addEdge (0, area.left (), area.top (), area.bottom () - 1,
		 RightEdge, true);
	addEdge (0, area.right (), area.top (), area.bottom () - 1,
		 LeftEdge, true);
    }

    // Drop screen edges parts that are under struts, basically apply the
    // same strategy than for windows edges visibility
    foreach (CompWindow *w, screen->windows ())
    {
	if (w == window || !w->struts ())
	    continue;

	for (std::list<Edge>::iterator it = edges.begin (); it != edges.end ();)
	{
	    Edge     *e = &*it;
	    CompRect rect;

	    if (!e->screenEdge)
	    {
		++it;
		continue;
	    }

	    switch (e->type)
	    {
		case LeftEdge:
		case RightEdge:
		    rect.setGeometry (e->position,
				      e->start,
				      1,
				      e->end - e->start);
		    break;
		case TopEdge:
		case BottomEdge:
		default:
		    rect.setGeometry (e->start,
				      e->position,
				      e->end - e->start,
				      1);
	    }

            edgeRegion = CompRegion (rect);
	    resultRegion = edgeRegion - w->region ();
	    if (resultRegion.isEmpty ())
	    {
		remove = true;
	    }
	    else if (edgeRegion != resultRegion)
	    {
		addRegionEdges (e, resultRegion);
		remove = true;
	    }

	    if (remove)
	    {
		it = edges.erase (it);
		remove = false;
	    }
	    else
	    {
		++it;
	    }
	}
    }
}

/*
 * Clean edges and fill it again with appropriate edges
 */
void
SnapWindow::updateEdges ()
{
    SNAP_SCREEN (screen);

    edges.clear ();
    updateWindowsEdges ();

    if (ss->optionGetEdgesCategoriesMask () & EdgesCategoriesScreenEdgesMask)
	updateScreenEdges ();
}

// Edges checking functions (move) ---------------------------------------------

/*
 * Find nearest edge in the direction set by "type",
 * w is the grabbed window, position/start/end are the window edges coordinates
 * before : if true the window has to be before the edge (top/left being origin)
 * snapDirection : just an helper, related to type
 */
void
SnapWindow::moveCheckNearestEdge (int position,
				  int start,
				  int end,
				  bool before,
				  EdgeType type,
				  int snapDirection)
{
    SNAP_SCREEN (screen);

    Edge *edge = &edges.front ();
    int dist, min = 65535;

    foreach (Edge &current, edges)
    {
	// Skip wrong type or outbound edges
	if (current.type != type || current.end < start || current.start > end)
	    continue;

	// Compute distance
	dist = before ? position - current.position : current.position - position;
	// Update minimum distance if needed
	if (dist < min && dist >= 0)
	{
	    min = dist;
	    edge = &current;
	}
	// 0-dist edge, just break
	if (dist == 0)
	    break;
	// Unsnap edges that aren't snapped anymore
	if (current.snapped && dist > ss->optionGetResistanceDistance ())
	    current.snapped = false;
    }
    // We found a 0-dist edge, or we have a snapping candidate
    if (min == 0 || (min <= ss->optionGetAttractionDistance ()
	&& ss->optionGetSnapTypeMask () & SnapTypeEdgeAttractionMask))
    {
	// Update snapping data
	if (ss->optionGetSnapTypeMask () & SnapTypeEdgeResistanceMask)
	{
	    snapGeometry = window->serverGeometry ();
	    this->snapDirection |= snapDirection;
	}
	// Attract the window if needed, moving it of the correct dist
	if (min != 0 && !edge->snapped)
	{
	    edge->snapped = true;
	    switch (type)
	    {
	    case LeftEdge:
		move (min, 0, false);
		break;
	    case RightEdge:
		move (-min, 0, false);
		break;
	    case TopEdge:
		move (0, min, false);
		break;
	    case BottomEdge:
		move (0, -min, false);
		break;
	    default:
		break;
	    }
	}
    }
}

/*
 * Call the previous function for each of the 4 sides of the window
 */
void
SnapWindow::moveCheckEdges (int snapDirection)
{
    CompRect input (window->serverBorderRect ());
    moveCheckNearestEdge (input.left (), input.top (), input.bottom (),
			  true, RightEdge, HorizontalSnap & snapDirection);
    moveCheckNearestEdge (input.right (), input.top (), input.bottom (),
			  false, LeftEdge, HorizontalSnap & snapDirection);
    moveCheckNearestEdge (input.top (), input.left (), input.right (),
			  true, BottomEdge, VerticalSnap & snapDirection);
    moveCheckNearestEdge (input.bottom (), input.left (), input.right (),
			  false, TopEdge, VerticalSnap & snapDirection);
}

// Edges checking functions (resize) -------------------------------------------

/*
 * Similar function for Snap on Resize
 */
void
SnapWindow::resizeCheckNearestEdge (int position,
				    int start,
				    int end,
				    bool before,
				    EdgeType type,
				    int snapDirection)
{
    SNAP_SCREEN (screen);

    Edge *edge = &edges.front ();
    int dist, min = 65535;

    foreach (Edge &current, edges)
    {
	// Skip wrong type or outbound edges
	if (current.type != type || current.end < start || current.start > end)
	    continue;

	// Compute distance
	dist = before ? position - current.position : current.position - position;
	// Update minimum distance if needed
	if (dist < min && dist >= 0)
	{
	    min = dist;
	    edge = &current;
	}
	// 0-dist edge, just break
	if (dist == 0)
	    break;
	// Unsnap edges that aren't snapped anymore
	if (current.snapped && dist > ss->optionGetResistanceDistance ())
	    current.snapped = false;
    }
    // We found a 0-dist edge, or we have a snapping candidate
    if (min == 0 || (min <= ss->optionGetAttractionDistance ()
	&& ss->optionGetSnapTypeMask () & SnapTypeEdgeAttractionMask))
    {
	// Update snapping data
	if (ss->optionGetSnapTypeMask () & SnapTypeEdgeResistanceMask)
	{
	    snapGeometry = window->serverGeometry ();
	    this->snapDirection |= snapDirection;
	}
	// FIXME : this needs resize-specific code.
	// Attract the window if needed, moving it of the correct dist
	if (min != 0 && !edge->snapped)
	{
	    edge->snapped = true;
	    switch (type)
	    {
	    case LeftEdge:
		resize (min, 0, 0, 0);
		break;
	    case RightEdge:
		resize (-min, 0, 0, 0);
		break;
	    case TopEdge:
		resize (0, min, 0, 0);
		break;
	    case BottomEdge:
		resize (0, -min, 0, 0);
		break;
	    default:
		break;
	    }
	}
    }
}

/*
 * Call the previous function for each of the 4 sides of the window
 */
void
SnapWindow::resizeCheckEdges (int dx, int dy, int dwidth, int dheight)
{
    CompRect input (window->serverBorderRect ());

    resizeCheckNearestEdge (input.left (), input.top (), input.bottom (),
			    true, RightEdge, HorizontalSnap);
    resizeCheckNearestEdge (input.right (), input.top (), input.bottom (),
			    false, LeftEdge, HorizontalSnap);
    resizeCheckNearestEdge (input.top (), input.left (), input.right (),
			    true, BottomEdge, VerticalSnap);
    resizeCheckNearestEdge (input.bottom (), input.left (), input.right (),
			    false, TopEdge, VerticalSnap);
}

// Check if avoidSnap is matched, and enable/disable snap consequently
void
SnapScreen::handleEvent (XEvent *event)
{
    if (event->type == screen->xkbEvent ())
    {
	XkbAnyEvent *xkbEvent = (XkbAnyEvent *) event;

	if (xkbEvent->xkb_type == XkbStateNotify)
	{
	    XkbStateNotifyEvent *stateEvent = (XkbStateNotifyEvent *) event;

	    unsigned int mods = 0xffffffff;
	    if (avoidSnapMask)
		mods = avoidSnapMask;

	    if ((stateEvent->mods & mods) == mods)
		snapping = false;
	    else
		snapping = true;
	}
    }

    screen->handleEvent (event);
}

// Events notifications --------------------------------------------------------

void
SnapWindow::resizeNotify (int dx, int dy, int dwidth, int dheight)
{
    SNAP_SCREEN (screen);

    window->resizeNotify (dx, dy, dwidth, dheight);

    // avoid-infinite-notify-loop mode/not grabbed
    if (skipNotify || !(grabbed & ResizeGrab))
	return;

    // we have to avoid snapping but there's still some buffered moving
    if (!ss->snapping && (m_dx || m_dy || m_dwidth || m_dheight))
    {
	resize (m_dx, m_dy, m_dwidth, m_dheight);
	m_dx = m_dy = m_dwidth = m_dheight = 0;
	return;
    }

    // avoiding snap, nothing buffered
    if (!ss->snapping)
	return;

    // apply edge resistance
    if (ss->optionGetSnapTypeMask () & SnapTypeEdgeResistanceMask)
    {
	// If there's horizontal snapping, add dx to current buffered
	// dx and resist (move by -dx) or release the window and move
	// by buffered dx - dx, same for dh
	if (!snapGeometry.isEmpty () && snapDirection & HorizontalSnap)
	{
	    m_dx += dx;
	    if (m_dx < ss->optionGetResistanceDistance ()
		&& m_dx > -ss->optionGetResistanceDistance ())
	    {
		resize (-dx, 0, 0, 0);
	    }
	    else
	    {
		resize (m_dx - dx, 0, 0, 0);
		m_dx = 0;
		if (!m_dwidth)
		    snapDirection &= VerticalSnap;
	    }
	    m_dwidth += dwidth;
	    if (m_dwidth < ss->optionGetResistanceDistance ()
		&& m_dwidth > -ss->optionGetResistanceDistance ())
	    {
		resize (0, 0, -dwidth, 0);
	    }
	    else
	    {
		resize (0, 0, m_dwidth - dwidth, 0);
		m_dwidth = 0;
		if (!m_dx)
		    snapDirection &= VerticalSnap;
	    }
	}

	// Same for vertical snapping and dy/dh
	if (snapGeometry.isEmpty () && snapDirection & VerticalSnap)
	{
	    m_dy += dy;
	    if (m_dy < ss->optionGetResistanceDistance ()
		&& m_dy > -ss->optionGetResistanceDistance ())
	    {
		resize (0, -dy, 0, 0);
	    }
	    else
	    {
		resize (0, m_dy - dy, 0, 0);
		m_dy = 0;
		if (!m_dheight)
		    snapDirection &= HorizontalSnap;
	    }
	    m_dheight += dheight;
	    if (m_dheight < ss->optionGetResistanceDistance ()
		&& m_dheight > -ss->optionGetResistanceDistance ())
	    {
		resize (0, 0, 0, -dheight);
	    }
	    else
	    {
		resize (0, 0, 0, m_dheight - dheight);
		m_dheight = 0;
		if (!m_dy)
		    snapDirection &= HorizontalSnap;
	    }
	}
	// If we are no longer snapping in any direction, reset snapped
	if (!snapGeometry.isEmpty () && !snapDirection)
	    snapGeometry = CompWindow::Geometry ();
    }

    // If we don't already snap vertically and horizontally,
    // check edges status
    if (snapDirection != (VerticalSnap | HorizontalSnap))
	resizeCheckEdges (dx, dy, dwidth, dheight);
}

void
SnapWindow::stateChangeNotify (unsigned int lastState)
{
    if (window->state () & CompWindowStateMaximizedHorzMask)
    {
	snapGeometry.setWidth (0);
	snapGeometry.setX (0);
	snapDirection &= VerticalSnap;
    }

    if (window->state () & CompWindowStateMaximizedVertMask)
    {
	snapGeometry.setHeight (0);
	snapGeometry.setY (0);
	snapDirection &= HorizontalSnap;
    }

    window->stateChangeNotify (lastState);
}

void
SnapWindow::moveNotify (int dx, int dy, bool immediate)
{
    unsigned int allowedSnapDirection = VerticalSnap | HorizontalSnap;
    SNAP_SCREEN (screen);

    window->moveNotify (dx, dy, immediate);

    // avoid-infinite-notify-loop mode/not grabbed
    if (skipNotify || !(grabbed & MoveGrab))
	return;

    // we have to avoid snapping but there's still some buffered moving
    if (!ss->snapping && (m_dx || m_dy))
    {
	move (m_dx, m_dy, false);
	m_dx = m_dy = 0;
	return;
    }

    dx = window->serverGeometry ().x () - snapGeometry.x ();
    dy = window->serverGeometry ().y () - snapGeometry.y ();

    // don't snap maximized windows
    if (window->state () & CompWindowStateMaximizedHorzMask)
    {
	allowedSnapDirection &= ~(VerticalSnap);
	dx = 0;
    }

    if (window->state () & CompWindowStateMaximizedVertMask)
    {
	allowedSnapDirection &= ~(HorizontalSnap);
	dy = 0;
    }

    // avoiding snap, nothing buffered
    if (!ss->snapping)
	return;

    // apply edge resistance
    if (ss->optionGetSnapTypeMask () & SnapTypeEdgeResistanceMask)
    {
	// If there's horizontal snapping, add dx to current buffered
	// dx and resist (move by -dx) or release the window and move
	// by buffered dx - dx
	if (!snapGeometry.isEmpty () && snapDirection & HorizontalSnap)
	{
	    m_dx += dx;
	    if (m_dx < ss->optionGetResistanceDistance ()
		&& m_dx > -ss->optionGetResistanceDistance ())
	    {
		move (-dx, 0, false);
	    }
	    else
	    {
		move (m_dx - dx, 0, true);
		m_dx = 0;
		snapDirection &= VerticalSnap;
	    }
	}
	// Same for vertical snapping and dy
	if (!snapGeometry.isEmpty () && snapDirection & VerticalSnap)
	{
	    m_dy += dy;
	    if (m_dy < ss->optionGetResistanceDistance ()
		&& m_dy > -ss->optionGetResistanceDistance ())
	    {
		move (0, -dy, false);
	    }
	    else
	    {
		move (0, m_dy - dy, true);
		m_dy = 0;
		snapDirection &= HorizontalSnap;
	    }
	}
	// If we are no longer snapping in any direction, reset snapped
	if (!snapGeometry.isEmpty () && !snapDirection)
	    snapGeometry = CompWindow::Geometry ();
    }
    // If we don't already snap vertically and horizontally,
    // check edges status
    if (snapDirection != (VerticalSnap | HorizontalSnap))
	moveCheckEdges (allowedSnapDirection);
}

/*
 * Initiate snap, get edges
 */
void
SnapWindow::grabNotify (int x, int y, unsigned int state, unsigned int mask)
{
    grabbed = (mask & CompWindowGrabResizeMask) ? ResizeGrab : MoveGrab;
    updateEdges ();

    window->grabNotify (x, y, state, mask);
}

/*
 * Clean edges data, reset dx/dy to avoid buggy moves
 * when snap will be triggered again.
 */
void
SnapWindow::ungrabNotify ()
{
    edges.clear ();

    snapGeometry = CompWindow::Geometry ();
    snapDirection = 0;
    grabbed = 0;
    m_dx = m_dy = m_dwidth = m_dheight = 0;

    window->ungrabNotify ();
}

// Internal stuff --------------------------------------------------------------

void
SnapScreen::optionChanged (CompOption *opt, SnapOptions::Options num)
{
    switch (num)
    {
    case SnapOptions::AvoidSnap:
    {
	unsigned int mask = optionGetAvoidSnapMask ();
	avoidSnapMask = 0;
	if (mask & AvoidSnapShiftMask)
	    avoidSnapMask |= ShiftMask;
	if (mask & AvoidSnapAltMask)
	    avoidSnapMask |= CompAltMask;
	if (mask & AvoidSnapControlMask)
	    avoidSnapMask |= ControlMask;
	if (mask & AvoidSnapMetaMask)
	    avoidSnapMask |= CompMetaMask;
    }

    default:
	break;
    }
}

SnapScreen::SnapScreen (CompScreen *screen) :
    PluginClassHandler <SnapScreen, CompScreen> (screen),
    SnapOptions (),
    snapping (true),
    avoidSnapMask (0)
{
    ScreenInterface::setHandler (screen);

#define setNotify(func) \
    optionSet##func##Notify (boost::bind (&SnapScreen::optionChanged, this, _1, _2))

    setNotify (AvoidSnap);
}

SnapWindow::SnapWindow (CompWindow *window) :
    PluginClassHandler <SnapWindow, CompWindow> (window),
    window (window),
    snapDirection (0),
    m_dx (0),
    m_dy (0),
    m_dwidth (0),
    m_dheight (0),
    snapGeometry (0, 0, 0, 0, 0),
    grabbed (0),
    skipNotify (false)
{
    WindowInterface::setHandler (window);
}

SnapWindow::~SnapWindow ()
{
}

bool
SnapPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    return true;
}

