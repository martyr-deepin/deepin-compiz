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
 * Ported to GLVertexBuffer by Daniel van Vugt <daniel.van.vugt@canonical.com>
 * Spring model implemented by Kristian Hogsberg.
 */

#include "wobbly.h"

COMPIZ_PLUGIN_20090315 (wobbly, WobblyPluginVTable)

void
WobblyWindow::findNextWestEdge (Object *object)
{
    int v1, v2;
    int start;
    int end;
    int x;
    int output;
    int workAreaEdge;

    start = -65535.0f;
    end   =  65535.0f;

    v1 = -65535.0f;
    v2 =  65535.0f;

    x = object->position.x + window->output ().left - window->border ().left;

    output = ::screen->outputDeviceForPoint (x, object->position.y);
    const CompRect &workArea =
	::screen->outputDevs ()[(unsigned) output].workArea ();
    workAreaEdge = workArea.x1 ();

    if (x >= workAreaEdge)
    {
	int v, s, e;
	v1 = workAreaEdge;

	foreach (CompWindow *p, ::screen->windows ())
	{
	    if (window == p)
		continue;

	    if (p->mapNum () && p->struts ())
	    {
		s = p->struts ()->left.y - window->output ().top;
		e = p->struts ()->left.y + p->struts ()->left.height +
		    window->output ().bottom;
	    }
	    else if (!p->invisible () && (p->type () & SNAP_WINDOW_TYPE))
	    {
		s = p->geometry ().y () - p->border ().top -
		    window->output ().top;
		e = p->geometry ().y () + p->height () + p->border ().bottom +
		    window->output ().bottom;
	    }
	    else
	    {
		continue;
	    }

	    if (s > object->position.y)
	    {
		if (s < end)
		    end = s;
	    }
	    else if (e < object->position.y)
	    {
		if (e > start)
		    start = e;
	    }
	    else
	    {
		if (s > start)
		    start = s;

		if (e < end)
		    end = e;

		if (p->mapNum () && p->struts ())
		    v = p->struts ()->left.x + p->struts ()->left.width;
		else
		    v = p->geometry ().x () + p->width () +
			p->border ().right;

		if (v <= x)
		{
		    if (v > v1)
			v1 = v;
		}
		else
		{
		    if (v < v2)
			v2 = v;
		}
	    }
	}
    }
    else
    {
	v2 = workAreaEdge;
    }

    v1 = v1 - window->output ().left + window->border ().left;
    v2 = v2 - window->output ().left + window->border ().left;

    if (v1 != (int) object->vertEdge.next)
	object->vertEdge.snapped = false;

    object->vertEdge.start = start;
    object->vertEdge.end   = end;

    object->vertEdge.next = v1;
    object->vertEdge.prev = v2;

    object->vertEdge.attract  = v1 + EDGE_DISTANCE;
    object->vertEdge.velocity = EDGE_VELOCITY;
}

void
WobblyWindow::findNextEastEdge (Object *object)
{
    int v1, v2;
    int start;
    int end;
    int x;
    int output;
    int workAreaEdge;

    start = -65535.0f;
    end   =  65535.0f;

    v1 =  65535.0f;
    v2 = -65535.0f;

    x = object->position.x - window->output ().right + window->border ().right;

    output = ::screen->outputDeviceForPoint (x, object->position.y);
    const CompRect &workArea =
	::screen->outputDevs ()[(unsigned) output].workArea ();
    workAreaEdge = workArea.x2 ();

    if (x <= workAreaEdge)
    {
	int v, s, e;
	v1 = workAreaEdge;

	foreach (CompWindow *p, ::screen->windows ())
	{
	    if (window == p)
		continue;

	    if (p->mapNum () && p->struts ())
	    {
		s = p->struts ()->right.y - window->output ().top;
		e = p->struts ()->right.y + p->struts ()->right.height +
		    window->output ().bottom;
	    }
	    else if (!p->invisible () && (p->type () & SNAP_WINDOW_TYPE))
	    {
		s = p->geometry ().y () - p->border ().top -
		    window->output ().top;
		e = p->geometry ().y () + p->height () + p->border ().bottom +
		    window->output ().bottom;
	    }
	    else
	    {
		continue;
	    }

	    if (s > object->position.y)
	    {
		if (s < end)
		    end = s;
	    }
	    else if (e < object->position.y)
	    {
		if (e > start)
		    start = e;
	    }
	    else
	    {
		if (s > start)
		    start = s;

		if (e < end)
		    end = e;

		if (p->mapNum () && p->struts ())
		    v = p->struts ()->right.x;
		else
		    v = p->geometry ().x () - p->border ().left;

		if (v >= x)
		{
		    if (v < v1)
			v1 = v;
		}
		else
		{
		    if (v > v2)
			v2 = v;
		}
	    }
	}
    }
    else
    {
	v2 = workAreaEdge;
    }

    v1 = v1 + window->output ().right - window->border ().right;
    v2 = v2 + window->output ().right - window->border ().right;

    if (v1 != (int) object->vertEdge.next)
	object->vertEdge.snapped = false;

    object->vertEdge.start = start;
    object->vertEdge.end   = end;

    object->vertEdge.next = v1;
    object->vertEdge.prev = v2;

    object->vertEdge.attract  = v1 - EDGE_DISTANCE;
    object->vertEdge.velocity = EDGE_VELOCITY;
}

void
WobblyWindow::findNextNorthEdge (Object *object)
{
    int v1, v2;
    int start;
    int end;
    int y;
    int output;
    int workAreaEdge;

    start = -65535.0f;
    end   =  65535.0f;

    v1 = -65535.0f;
    v2 =  65535.0f;

    y = object->position.y + window->output ().top - window->border ().top;

    output = ::screen->outputDeviceForPoint (object->position.x, y);
    const CompRect &workArea =
	::screen->outputDevs ()[(unsigned) output].workArea ();
    workAreaEdge = workArea.y1 ();

    if (y >= workAreaEdge)
    {
	int v, s, e;
	v1 = workAreaEdge;

	foreach (CompWindow *p, ::screen->windows ())
	{
	    if (window == p)
		continue;

	    if (p->mapNum () && p->struts ())
	    {
		s = p->struts ()->top.x - window->output ().left;
		e = p->struts ()->top.x + p->struts ()->top.width +
		    window->output ().right;
	    }
	    else if (!p->invisible () && (p->type () & SNAP_WINDOW_TYPE))
	    {
		s = p->geometry ().x () - p->border ().left -
		    window->output ().left;
		e = p->geometry ().x () + p->width () + p->border ().right +
		    window->output ().right;
	    }
	    else
	    {
		continue;
	    }

	    if (s > object->position.x)
	    {
		if (s < end)
		    end = s;
	    }
	    else if (e < object->position.x)
	    {
		if (e > start)
		    start = e;
	    }
	    else
	    {
		if (s > start)
		    start = s;

		if (e < end)
		    end = e;

		if (p->mapNum () && p->struts ())
		    v = p->struts ()->top.y + p->struts ()->top.height;
		else
		    v = p->geometry ().y () + p->height () + p->border ().bottom;

		if (v <= y)
		{
		    if (v > v1)
			v1 = v;
		}
		else
		{
		    if (v < v2)
			v2 = v;
		}
	    }
	}
    }
    else
    {
	v2 = workAreaEdge;
    }

    v1 = v1 - window->output ().top + window->border ().top;
    v2 = v2 - window->output ().top + window->border ().top;

    if (v1 != (int) object->horzEdge.next)
	object->horzEdge.snapped = false;

    object->horzEdge.start = start;
    object->horzEdge.end   = end;

    object->horzEdge.next = v1;
    object->horzEdge.prev = v2;

    object->horzEdge.attract  = v1 + EDGE_DISTANCE;
    object->horzEdge.velocity = EDGE_VELOCITY;
}

void
WobblyWindow::findNextSouthEdge (Object *object)
{
    int v1, v2;
    int start;
    int end;
    int y;
    int output;
    int workAreaEdge;

    start = -65535.0f;
    end   =  65535.0f;

    v1 =  65535.0f;
    v2 = -65535.0f;

    y = object->position.y - window->output ().bottom + window->border ().bottom;

    output = ::screen->outputDeviceForPoint (object->position.x, y);
    const CompRect &workArea =
	::screen->outputDevs ()[(unsigned) output].workArea ();
    workAreaEdge = workArea.y2 ();

    if (y <= workAreaEdge)
    {
	int v, s, e;
	v1 = workAreaEdge;

	foreach (CompWindow *p, ::screen->windows ())
	{
	    if (window == p)
		continue;

	    if (p->mapNum () && p->struts ())
	    {
		s = p->struts ()->bottom.x - window->output ().left;
		e = p->struts ()->bottom.x + p->struts ()->bottom.width +
		    window->output ().right;
	    }
	    else if (!p->invisible () && (p->type () & SNAP_WINDOW_TYPE))
	    {
		s = p->geometry ().x () - p->border ().left -
		    window->output ().left;
		e = p->geometry ().x () + p->width () + p->border ().right +
		    window->output ().right;
	    }
	    else
	    {
		continue;
	    }

	    if (s > object->position.x)
	    {
		if (s < end)
		    end = s;
	    }
	    else if (e < object->position.x)
	    {
		if (e > start)
		    start = e;
	    }
	    else
	    {
		if (s > start)
		    start = s;

		if (e < end)
		    end = e;

		if (p->mapNum () && p->struts ())
		    v = p->struts ()->bottom.y;
		else
		    v = p->geometry ().y () - p->border ().top;

		if (v >= y)
		{
		    if (v < v1)
			v1 = v;
		}
		else
		{
		    if (v > v2)
			v2 = v;
		}
	    }
	}
    }
    else
    {
	v2 = workAreaEdge;
    }

    v1 = v1 + window->output ().bottom - window->border ().bottom;
    v2 = v2 + window->output ().bottom - window->border ().bottom;

    if (v1 != (int) object->horzEdge.next)
	object->horzEdge.snapped = false;

    object->horzEdge.start = start;
    object->horzEdge.end   = end;

    object->horzEdge.next = v1;
    object->horzEdge.prev = v2;

    object->horzEdge.attract  = v1 - EDGE_DISTANCE;
    object->horzEdge.velocity = EDGE_VELOCITY;
}

void
Object::init (float  positionX,
	      float  positionY,
	      float  velocityX,
	      float  velocityY)
{
    force.x = 0;
    force.y = 0;

    position.x = positionX;
    position.y = positionY;

    velocity.x = velocityX;
    velocity.y = velocityY;

    theta    = 0;
    immobile = false;

    edgeMask = 0;

    vertEdge.snapped = false;
    horzEdge.snapped = false;

    vertEdge.next = 0.0f;
    horzEdge.next = 0.0f;
}

void
Spring::init (Object *newA,
	      Object *newB,
	      float  newOffsetX,
	      float  newOffsetY)
{
    a        = newA;
    b        = newB;
    offset.x = newOffsetX;
    offset.y = newOffsetY;
}

void
Model::calcBounds ()
{
    topLeft.x     = MAXSHORT;
    topLeft.y     = MAXSHORT;
    bottomRight.x = MINSHORT;
    bottomRight.y = MINSHORT;

    Object *object = objects;
    for (int i = 0; i < numObjects; i++, object++)
    {
	if (topLeft.x > object->position.x)
	    topLeft.x = object->position.x;
	else if (bottomRight.x < object->position.x)
	    bottomRight.x = object->position.x;

	if (topLeft.y > object->position.y)
	    topLeft.y = object->position.y;
	else if (bottomRight.y < object->position.y)
	    bottomRight.y = object->position.y;
    }
}

void
Model::addSpring (Object *a,
		  Object *b,
		  float  offsetX,
		  float  offsetY)
{
    Spring *spring;

    spring = &springs[numSprings];
    numSprings++;

    spring->init (a, b, offsetX, offsetY);
}

void
Model::setMiddleAnchor (int   x,
			int   y,
			int   width,
			int   height)
{
    float gx, gy;

    gx = ((GRID_WIDTH  - 1) / 2 * width)  / (float) (GRID_WIDTH  - 1);
    gy = ((GRID_HEIGHT - 1) / 2 * height) / (float) (GRID_HEIGHT - 1);

    if (anchorObject)
	anchorObject->immobile = false;

    anchorObject = &objects[GRID_WIDTH * ((GRID_HEIGHT - 1) / 2) +
			    (GRID_WIDTH - 1) / 2];
    anchorObject->position.x = x + gx;
    anchorObject->position.y = y + gy;

    anchorObject->immobile = true;
}

void
Model::setTopAnchor (int x,
		     int y,
		     int width)
{
    float gx;

    gx = ((GRID_WIDTH - 1) / 2 * width)  / (float) (GRID_WIDTH - 1);

    if (anchorObject)
	anchorObject->immobile = false;

    anchorObject = &objects[(GRID_WIDTH - 1) / 2];
    anchorObject->position.x = x + gx;
    anchorObject->position.y = y;

    anchorObject->immobile = true;
}

void
Model::addEdgeAnchors (int x,
		       int y,
		       int width,
		       int height)
{
    Object *o;

    o = &objects[0];
    o->position.x = x;
    o->position.y = y;
    o->immobile = true;

    o = &objects[GRID_WIDTH - 1];
    o->position.x = x + width;
    o->position.y = y;
    o->immobile = true;

    o = &objects[GRID_WIDTH * (GRID_HEIGHT - 1)];
    o->position.x = x;
    o->position.y = y + height;
    o->immobile = true;

    o = &objects[numObjects - 1];
    o->position.x = x + width;
    o->position.y = y + height;
    o->immobile = true;

    if (!anchorObject)
	anchorObject = &objects[0];
}

void
Model::removeEdgeAnchors (int x,
			  int y,
			  int width,
			  int height)
{
    Object *o;

    o = &objects[0];
    o->position.x = x;
    o->position.y = y;
    if (o != anchorObject)
	o->immobile = false;

    o = &objects[GRID_WIDTH - 1];
    o->position.x = x + width;
    o->position.y = y;
    if (o != anchorObject)
	o->immobile = false;

    o = &objects[GRID_WIDTH * (GRID_HEIGHT - 1)];
    o->position.x = x;
    o->position.y = y + height;
    if (o != anchorObject)
	o->immobile = false;

    o = &objects[numObjects - 1];
    o->position.x = x + width;
    o->position.y = y + height;
    if (o != anchorObject)
	o->immobile = false;
}

void
Model::adjustObjectPosition (Object *object,
			     int    x,
			     int    y,
			     int    width,
			     int    height)
{
    Object *o;
    int	   gridX, gridY, i = 0;

    for (gridY = 0; gridY < GRID_HEIGHT; gridY++)
    {
	for (gridX = 0; gridX < GRID_WIDTH; gridX++, i++)
	{
	    o = &objects[i];
	    if (o == object)
	    {
		o->position.x = x + (gridX * width) / (GRID_WIDTH - 1);
		o->position.y = y + (gridY * height) / (GRID_HEIGHT - 1);

		return;
	    }
	}
    }
}

void
Model::initObjects (int	x,
		    int y,
		    int	width,
		    int	height)
{
    float gw, gh;

    gw = GRID_WIDTH  - 1;
    gh = GRID_HEIGHT - 1;

    Object *object = objects;
    for (int gridY = 0; gridY < GRID_HEIGHT; gridY++)
    {
	for (int gridX = 0; gridX < GRID_WIDTH; gridX++, object++)
	{
	    object->init (x + (gridX * width) / gw,
			  y + (gridY * height) / gh,
			  0, 0);
	}
    }

    setMiddleAnchor (x, y, width, height);
}

void
WobblyWindow::updateModelSnapping ()
{
    unsigned int edgeMask, gridMask, mask;

    edgeMask = model->edgeMask;

    if (model->snapCnt[North])
	edgeMask &= ~SouthEdgeMask;
    else if (model->snapCnt[South])
	edgeMask &= ~NorthEdgeMask;

    if (model->snapCnt[West])
	edgeMask &= ~EastEdgeMask;
    else if (model->snapCnt[East])
	edgeMask &= ~WestEdgeMask;

    Object *object = model->objects;
    for (int gridY = 0; gridY < GRID_HEIGHT; gridY++)
    {
	if (gridY == 0)
	    gridMask = edgeMask & NorthEdgeMask;
	else if (gridY == GRID_HEIGHT - 1)
	    gridMask = edgeMask & SouthEdgeMask;
	else
	    gridMask = 0;

	for (int gridX = 0; gridX < GRID_WIDTH; gridX++, object++)
	{
	    mask = gridMask;

	    if (gridX == 0)
		mask |= edgeMask & WestEdgeMask;
	    else if (gridX == GRID_WIDTH - 1)
		mask |= edgeMask & EastEdgeMask;

	    if (mask != object->edgeMask)
	    {
		object->edgeMask = mask;

		if (mask & WestEdgeMask)
		{
		    if (!object->vertEdge.snapped)
			findNextWestEdge (object);
		}
		else if (mask & EastEdgeMask)
		{
		    if (!object->vertEdge.snapped)
			findNextEastEdge (object);
		}
		else
		    object->vertEdge.snapped = false;

		if (mask & NorthEdgeMask)
		{
		    if (!object->horzEdge.snapped)
			findNextNorthEdge (object);
		}
		else if (mask & SouthEdgeMask)
		{
		    if (!object->horzEdge.snapped)
			findNextSouthEdge (object);
		}
		else
		    object->horzEdge.snapped = false;
	    }
	}
    }
}

void
Model::reduceEdgeEscapeVelocity ()
{
    Object *object = objects;
    for (int gridY = 0; gridY < GRID_HEIGHT; gridY++)
    {
	for (int gridX = 0; gridX < GRID_WIDTH; gridX++, object++)
	{
	    if (object->vertEdge.snapped)
		object->vertEdge.velocity *= drand48 () * 0.25f;

	    if (object->horzEdge.snapped)
		object->horzEdge.velocity *= drand48 () * 0.25f;
	}
    }
}

bool
Model::disableSnapping ()
{
    bool snapped = false;

    Object *object = objects;
    for (int gridY = 0; gridY < GRID_HEIGHT; gridY++)
    {
	for (int gridX = 0; gridX < GRID_WIDTH; gridX++, object++)
	{
	    if (object->vertEdge.snapped ||
		object->horzEdge.snapped)
		snapped = true;

	    object->vertEdge.snapped = false;
	    object->horzEdge.snapped = false;

	    object->edgeMask = 0;
	}
    }

    memset (snapCnt, 0, sizeof (snapCnt));

    return snapped;
}

void
Model::adjustObjectsForShiver (int   x,
			       int   y,
			       int   width,
			       int   height)
{
    float vX, vY;
    float w, h;
    float scale;

    w = width;
    h = height;

    Object *object = objects;
    for (int gridY = 0; gridY < GRID_HEIGHT; gridY++)
    {
	for (int gridX = 0; gridX < GRID_WIDTH; gridX++, object++)
	{
	    if (!object->immobile)
	    {
		vX = object->position.x - (x + w / 2);
		vY = object->position.y - (y + h / 2);

		vX /= w;
		vY /= h;

		scale = ((float) rand () * 7.5f) / RAND_MAX;

		object->velocity.x += vX * scale;
		object->velocity.y += vY * scale;
	    }
	}
    }
}

void
Model::initSprings (int   x,
		    int   y,
		    int   width,
		    int   height)
{
    int   i = 0;
    float hpad, vpad;

    numSprings = 0;

    hpad = ((float) width) / (GRID_WIDTH  - 1);
    vpad = ((float) height) / (GRID_HEIGHT - 1);

    for (int gridY = 0; gridY < GRID_HEIGHT; gridY++)
    {
	for (int gridX = 0; gridX < GRID_WIDTH; gridX++, i++)
	{
	    if (gridX > 0)
		addSpring (&objects[i - 1],
			   &objects[i],
			   hpad, 0);

	    if (gridY > 0)
		addSpring (&objects[i - GRID_WIDTH],
			   &objects[i],
			   0, vpad);
	}
    }
}

void
Model::move (float tx,
	     float ty)
{
    Object *object = objects;
    for (int i = 0; i < numObjects; i++, object++)
    {
	object->position.x += tx;
	object->position.y += ty;
    }
}

Model::Model (int	   x,
	      int	   y,
	      int	   width,
	      int	   height,
	      unsigned int edgeMask) :
    numObjects (GRID_WIDTH * GRID_HEIGHT),
    numSprings (0),
    anchorObject (0),
    steps (0),
    edgeMask (edgeMask)
{
    objects = new Object [numObjects];

    memset (snapCnt, 0, sizeof (snapCnt));

    initObjects (x, y, width, height);
    initSprings (x, y, width, height);

    calcBounds ();
}

void
Object::applyForce (float  fx,
		    float  fy)
{
    force.x += fx;
    force.y += fy;
}

void
Spring::exertForces (float k)
{
    Vector da, db;
    Vector &aPos = a->position;
    Vector &bPos = b->position;

    da.x = 0.5f * (bPos.x - aPos.x - offset.x);
    da.y = 0.5f * (bPos.y - aPos.y - offset.y);

    db.x = 0.5f * (aPos.x - bPos.x + offset.x);
    db.y = 0.5f * (aPos.y - bPos.y + offset.y);

    a->applyForce (k * da.x, k * da.y);
    b->applyForce (k * db.x, k * db.y);
}

bool
WobblyWindow::objectReleaseWestEastEdge (Object	*object,
					 Direction dir)
{
    if (fabs (object->velocity.x) > object->vertEdge.velocity)
    {
	object->position.x += object->velocity.x * 2.0f;

	model->snapCnt[dir]--;

	object->vertEdge.snapped = false;
	object->edgeMask = 0;

	updateModelSnapping ();

	return true;
    }

    object->velocity.x = 0.0f;

    return false;
}

bool
WobblyWindow::objectReleaseNorthSouthEdge (Object    *object,
					   Direction dir)
{
    if (fabs (object->velocity.y) > object->horzEdge.velocity)
    {
	object->position.y += object->velocity.y * 2.0f;

	model->snapCnt[dir]--;

	object->horzEdge.snapped = false;
	object->edgeMask = 0;

	updateModelSnapping ();

	return true;
    }

    object->velocity.y = 0.0f;

    return false;
}

float
WobblyWindow::modelStepObject (Object *object,
			       float  friction,
			       float  *force)
{
    object->theta += 0.05f;

    if (object->immobile)
    {
	object->velocity.x = 0.0f;
	object->velocity.y = 0.0f;

	object->force.x = 0.0f;
	object->force.y = 0.0f;

	*force = 0.0f;

	return 0.0f;
    }
    else
    {
	object->force.x -= friction * object->velocity.x;
	object->force.y -= friction * object->velocity.y;

	object->velocity.x += object->force.x / MASS;
	object->velocity.y += object->force.y / MASS;

	if (object->edgeMask)
	{
	    if (object->edgeMask & WestEdgeMask)
	    {
		if (object->position.y < object->vertEdge.start ||
		    object->position.y > object->vertEdge.end)
		    findNextWestEdge (object);

		if (!object->vertEdge.snapped ||
		    objectReleaseWestEastEdge (object, West))
		{
		    object->position.x += object->velocity.x;

		    if (object->velocity.x < 0.0f &&
			object->position.x < object->vertEdge.attract)
		    {
			if (object->position.x < object->vertEdge.next)
			{
			    object->vertEdge.snapped = true;
			    object->position.x = object->vertEdge.next;
			    object->velocity.x = 0.0f;

			    model->snapCnt[West]++;

			    updateModelSnapping ();
			}
			else
			{
			    object->velocity.x -=
				object->vertEdge.attract - object->position.x;
			}
		    }

		    if (object->position.x > object->vertEdge.prev)
			findNextWestEdge (object);
		}
	    }
	    else if (object->edgeMask & EastEdgeMask)
	    {
		if (object->position.y < object->vertEdge.start ||
		    object->position.y > object->vertEdge.end)
		    findNextEastEdge (object);

		if (!object->vertEdge.snapped ||
		    objectReleaseWestEastEdge (object, East))
		{
		    object->position.x += object->velocity.x;

		    if (object->velocity.x > 0.0f &&
			object->position.x > object->vertEdge.attract)
		    {
			if (object->position.x > object->vertEdge.next)
			{
			    object->vertEdge.snapped = true;
			    object->position.x = object->vertEdge.next;
			    object->velocity.x = 0.0f;

			    model->snapCnt[East]++;

			    updateModelSnapping ();
			}
			else
			{
			    object->velocity.x =
				object->position.x - object->vertEdge.attract;
			}
		    }

		    if (object->position.x < object->vertEdge.prev)
			findNextEastEdge (object);
		}
	    }
	    else
		object->position.x += object->velocity.x;

	    if (object->edgeMask & NorthEdgeMask)
	    {
		if (object->position.x < object->horzEdge.start ||
		    object->position.x > object->horzEdge.end)
		    findNextNorthEdge (object);

		if (!object->horzEdge.snapped ||
		    objectReleaseNorthSouthEdge (object, North))
		{
		    object->position.y += object->velocity.y;

		    if (object->velocity.y < 0.0f &&
			object->position.y < object->horzEdge.attract)
		    {
			if (object->position.y < object->horzEdge.next)
			{
			    object->horzEdge.snapped = true;
			    object->position.y = object->horzEdge.next;
			    object->velocity.y = 0.0f;

			    model->snapCnt[North]++;

			    updateModelSnapping ();
			}
			else
			{
			    object->velocity.y -=
				object->horzEdge.attract - object->position.y;
			}
		    }

		    if (object->position.y > object->horzEdge.prev)
			findNextNorthEdge (object);
		}
	    }
	    else if (object->edgeMask & SouthEdgeMask)
	    {
		if (object->position.x < object->horzEdge.start ||
		    object->position.x > object->horzEdge.end)
		    findNextSouthEdge (object);

		if (!object->horzEdge.snapped ||
		    objectReleaseNorthSouthEdge (object, South))
		{
		    object->position.y += object->velocity.y;

		    if (object->velocity.y > 0.0f &&
			object->position.y > object->horzEdge.attract)
		    {
			if (object->position.y > object->horzEdge.next)
			{
			    object->horzEdge.snapped = true;
			    object->position.y = object->horzEdge.next;
			    object->velocity.y = 0.0f;

			    model->snapCnt[South]++;

			    updateModelSnapping ();
			}
			else
			{
			    object->velocity.y =
				object->position.y - object->horzEdge.attract;
			}
		    }

		    if (object->position.y < object->horzEdge.prev)
			findNextSouthEdge (object);
		}
	    }
	    else
		object->position.y += object->velocity.y;
	}
	else
	{
	    object->position.x += object->velocity.x;
	    object->position.y += object->velocity.y;
	}

	*force = fabs (object->force.x) + fabs (object->force.y);

	object->force.x = 0.0f;
	object->force.y = 0.0f;

	return fabs (object->velocity.x) + fabs (object->velocity.y);
    }
}

unsigned int
WobblyWindow::modelStep (float friction,
			 float k,
			 float time)
{
    unsigned int wobbly = 0;
    int          steps;
    float        velocitySum = 0.0f;
    float        force, forceSum = 0.0f;

    model->steps += time / 15.0f;
    steps = floor (model->steps);
    model->steps -= steps;

    if (!steps)
	return WobblyInitialMask;

    for (int j = 0; j < steps; j++)
    {
	for (int i = 0; i < model->numSprings; i++)
	    model->springs[i].exertForces (k);

	for (int i = 0; i < model->numObjects; i++)
	{
	    velocitySum += modelStepObject (&model->objects[i],
					    friction,
					    &force);
	    forceSum += force;
	}
    }

    model->calcBounds ();

    if (velocitySum > 0.5f)
	wobbly |= WobblyVelocityMask;

    if (forceSum > 20.0f)
	wobbly |= WobblyForceMask;

    return wobbly;
}

void
Model::bezierPatchEvaluate (float u,
			    float v,
			    float *patchX,
			    float *patchY)
{
    float coeffsU[4], coeffsV[4];
    float x, y;
    int   i, j;

    coeffsU[0] = (1 - u) * (1 - u) * (1 - u);
    coeffsU[1] = 3 * u * (1 - u) * (1 - u);
    coeffsU[2] = 3 * u * u * (1 - u);
    coeffsU[3] = u * u * u;

    coeffsV[0] = (1 - v) * (1 - v) * (1 - v);
    coeffsV[1] = 3 * v * (1 - v) * (1 - v);
    coeffsV[2] = 3 * v * v * (1 - v);
    coeffsV[3] = v * v * v;

    x = y = 0.0f;

    for (i = 0; i < 4; i++)
    {
	for (j = 0; j < 4; j++)
	{
	    x += coeffsU[i] * coeffsV[j] *
		objects[j * GRID_WIDTH + i].position.x;
	    y += coeffsU[i] * coeffsV[j] *
		objects[j * GRID_WIDTH + i].position.y;
	}
    }

    *patchX = x;
    *patchY = y;
}

bool
WobblyWindow::ensureModel ()
{
    if (!model)
    {
	unsigned int edgeMask = 0;
	CompRect outRect (window->outputRect ());

	if (window->type () & CompWindowTypeNormalMask)
	    edgeMask = WestEdgeMask | EastEdgeMask | NorthEdgeMask |
		SouthEdgeMask;
	try
	{
	    model = new Model (outRect.x (), outRect.y (),
			       outRect.width (), outRect.height (),
			       edgeMask);
	}
	catch (std::bad_alloc &)
	{
	    return false;
	}
    }
    return true;
}

float
Object::distanceToPoint (float  x,
			 float  y)
{
    float dx = position.x - x;
    float dy = position.y - y;

    return sqrt (dx * dx + dy * dy);
}

Object *
Model::findNearestObject (float x,
			  float y)
{
    Object *object = &objects[0];
    float  distance, minDistance = 0.0;
    int    i;

    for (i = 0; i < numObjects; i++)
    {
	distance = objects[i].distanceToPoint (x, y);
	if (i == 0 || distance < minDistance)
	{
	    minDistance = distance;
	    object = &objects[i];
	}
    }

    return object;
}

bool
WobblyWindow::isWobblyWin ()
{
    if (model)
	return true;

    /* avoid tiny windows */
    if (window->width () == 1 && window->height () == 1)
	return false;

    CompWindow::Geometry &geom = window->geometry ();

    /* avoid fullscreen windows */
    if (geom.x () <= 0 &&
	geom.y () <= 0 &&
	geom.x () + window->width () >= ::screen->width () &&
	geom.y () + window->height () >= ::screen->height ())
	return false;

    return true;
}

void
WobblyScreen::preparePaint (int msSinceLastPaint)
{
    if (wobblingWindowsMask & (WobblyInitialMask | WobblyVelocityMask))
    {
	Point  topLeft, bottomRight;
	float  friction, springK;
	Model  *model;

	friction = optionGetFriction ();
	springK  = optionGetSpringK ();

	wobblingWindowsMask = false;
	foreach (CompWindow *w, ::screen->windows ())
	{
	    WobblyWindow *ww = WobblyWindow::get (w);

	    if (ww->wobblingMask)
	    {
		if (ww->wobblingMask & (WobblyInitialMask | WobblyVelocityMask))
		{
		    model = ww->model;

		    topLeft     = model->topLeft;
		    bottomRight = model->bottomRight;

		    ww->wobblingMask =
			ww->modelStep (friction, springK,
				       (ww->wobblingMask &
				        (unsigned) WobblyVelocityMask) ?
				       msSinceLastPaint :
				       cScreen->redrawTime ());

		    if ((ww->state & MAXIMIZE_STATE) && ww->grabbed)
			ww->wobblingMask |= WobblyForceMask;

		    if (ww->wobblingMask)
		    {
			/* snapped to more than one edge, we have to reduce
			   edge escape velocity until only one edge is snapped */
			if (ww->wobblingMask == WobblyForceMask && !ww->grabbed)
			{
			    ww->model->reduceEdgeEscapeVelocity ();
			    ww->wobblingMask |= WobblyInitialMask;
			}

			if (!ww->grabbed && constraintBox)
			{
			    float topmostYPos    = MAXSHORT;
			    float bottommostYPos = MINSHORT;
			    int   decorTop;
			    int   decorTitleBottom;

			    for (int i = 0; i < GRID_WIDTH; i++)
			    {
				int modelY = model->objects[i].position.y;

				/* find the bottommost top-row object */
				bottommostYPos = MAX (modelY, bottommostYPos);

				/* find the topmost top-row object */
				topmostYPos = MIN (modelY, topmostYPos);
			    }

			    decorTop = bottommostYPos +
				       w->output ().top - w->border ().top;
			    decorTitleBottom = topmostYPos + w->output ().top;

			    if (constraintBox->y () > decorTop)
			    {
				/* constrain to work area box top edge */
				model->move (0, constraintBox->y () - decorTop);
				model->calcBounds ();
			    }
			    else if (constraintBox->y2 () < decorTitleBottom)
			    {
				/* constrain to work area box bottom edge */
				model->move (0, constraintBox->y2 () -
						decorTitleBottom);
				model->calcBounds ();
			    }
			}
		    }
		    else
		    {
			ww->model = 0;

			if (w->geometry ().x () == w->serverX () &&
			    w->geometry ().y () == w->serverY ())
			{
			    w->move (model->topLeft.x +
				     w->output ().left -
				     w->geometry ().x (),
				     model->topLeft.y +
				     w->output ().top -
				     w->geometry ().y (),
				     true);
			    w->syncPosition ();
			}

			ww->model = model;
		    }

		    if (!(cScreen->damageMask () &
			  COMPOSITE_SCREEN_DAMAGE_ALL_MASK))
		    {
			CompositeWindow *cw = CompositeWindow::get (w);
			if (ww->wobblingMask)
			{
			    Point topLeft2     = ww->model->topLeft;
			    Point bottomRight2 = ww->model->bottomRight;

			    // Find the bounding box of the two rectangles
			    if (topLeft.x > topLeft2.x)
				topLeft.x = topLeft2.x;
			    if (topLeft.y > topLeft2.y)
				topLeft.y = topLeft2.y;
			    if (bottomRight.x < bottomRight2.x)
				bottomRight.x = bottomRight2.x;
			    if (bottomRight.y < bottomRight2.y)
				bottomRight.y = bottomRight2.y;
			}
			else
			    cw->addDamage ();

			int wx = w->geometry ().x ();
			int wy = w->geometry ().y ();
			int borderWidth = w->geometry ().border ();

			// Damage a box that's 1-pixel larger on each side
			// to prevent artifacts
			topLeft.x -= 1;
			topLeft.y -= 1;
			bottomRight.x += 1;
			bottomRight.y += 1;

			topLeft.x -= wx + borderWidth;
			topLeft.y -= wy + borderWidth;
			bottomRight.x += 0.5f - (wx + borderWidth);
			bottomRight.y += 0.5f - (wy + borderWidth);

			cw->addDamageRect (CompRect (topLeft.x,
						     topLeft.y,
						     bottomRight.x - topLeft.x,
						     bottomRight.y - topLeft.y));
		    }
		}
		if (!ww->wobblingMask)
		{
		    // Wobbling just finished for this window
		    ww->enableWobbling (false);
		}

		wobblingWindowsMask |= ww->wobblingMask;
	    }
	}
    }

    cScreen->preparePaint (msSinceLastPaint);
}

void
WobblyScreen::donePaint ()
{
    if (wobblingWindowsMask & (WobblyVelocityMask | WobblyInitialMask))
	cScreen->damagePending ();

    if (!wobblingWindowsMask)
    {
	// Wobbling has finished for all windows
	cScreen->preparePaintSetEnabled (this, false);
	cScreen->donePaintSetEnabled (this, false);
	gScreen->glPaintOutputSetEnabled (this, false);

	constraintBox = NULL;
    }

    cScreen->donePaint ();
}

void
WobblyWindow::glAddGeometry (const GLTexture::MatrixList &matrix,
			     const CompRegion            &region,
			     const CompRegion            &clip,
			     unsigned int                maxGridWidth,
			     unsigned int                maxGridHeight)
{
    int wx, wy, width, height, gridW, gridH;

    CompRect outRect (window->outputRect ());
    wx     = outRect.x ();
    wy     = outRect.y ();
    width  = outRect.width ();
    height = outRect.height ();

    gridW = width / wScreen->optionGetGridResolution ();
    if (gridW < wScreen->optionGetMinGridSize ())
	gridW = wScreen->optionGetMinGridSize ();

    gridH = height / wScreen->optionGetGridResolution ();
    if (gridH < wScreen->optionGetMinGridSize ())
	gridH = wScreen->optionGetMinGridSize ();

    if (gridW > (int) maxGridWidth)
	gridW = (int) maxGridWidth;

    if (gridH > (int) maxGridHeight)
	gridH = (int) maxGridHeight;

    GLVertexBuffer *vb = gWindow->vertexBuffer ();

    int oldCount = vb->countVertices ();
    gWindow->glAddGeometry (matrix, region, clip, gridW, gridH);
    int newCount = vb->countVertices ();

    int stride = vb->getVertexStride ();
    GLfloat *v = vb->getVertices () + oldCount * stride;
    GLfloat *vMax = vb->getVertices () + newCount * stride;

    for (; v < vMax; v += stride)
    {
	float deformedX, deformedY;
	GLfloat normalizedX = (v[0] - wx) / width;
	GLfloat normalizedY = (v[1] - wy) / height;
	model->bezierPatchEvaluate (normalizedX, normalizedY,
	                            &deformedX, &deformedY);
	v[0] = deformedX;
	v[1] = deformedY;
    }
}

bool
WobblyWindow::glPaint (const GLWindowPaintAttrib &attrib,
		       const GLMatrix            &transform,
		       const CompRegion          &region,
		       unsigned int              mask)
{
    if (wobblingMask)
	mask |= PAINT_WINDOW_TRANSFORMED_MASK;

    return gWindow->glPaint (attrib, transform, region, mask);
}

bool
WobblyScreen::enableSnapping ()
{
    foreach (CompWindow *w, ::screen->windows ())
    {
	WobblyWindow *ww = WobblyWindow::get (w);

	if (ww->grabbed && ww->model)
	    ww->updateModelSnapping ();
    }

    snapping = true;

    return false;
}

bool
WobblyScreen::disableSnapping ()
{
    if (!snapping)
	return false;

    foreach (CompWindow *w, ::screen->windows ())
    {
	WobblyWindow *ww = WobblyWindow::get (w);

	if (ww->grabbed && ww->model)
	{
	    if (ww->model->disableSnapping ())
	    {
		startWobbling (ww);
	    }
	}
    }

    snapping = false;

    return false;
}

bool
WobblyScreen::shiver (CompOption::Vector &options)
{
    Window xid = (Window) CompOption::getIntOptionNamed (options, "window");

    CompWindow *w = ::screen->findWindow (xid);

    if (w)
    {
	WobblyWindow *ww = WobblyWindow::get (w);

	if (ww->isWobblyWin () && ww->ensureModel ())
	{
	    CompRect outRect (w->serverOutputRect ());

	    ww->model->setMiddleAnchor (outRect.x (), outRect.y (),
					outRect.width (), outRect.height ());
	    ww->model->adjustObjectsForShiver (outRect.x (), outRect.y (),
					       outRect.width (), outRect.height ());

	    startWobbling (ww);
	}
    }
    return false;
}

void
WobblyWindow::windowNotify (CompWindowNotify n)
{
    switch (n)
    {
	case CompWindowNotifyMap:
	    if (model && isWobblyWin ())
		initiateMapEffect ();
	    break;
	default:
	    break;
    }

    window->windowNotify (n);
}

void
WobblyScreen::handleEvent (XEvent *event)
{
    Window     activeWindow = ::screen->activeWindow ();
    CompWindow *w;

    if (event->type == ::screen->xkbEvent ())
    {
	XkbAnyEvent *xkbEvent = (XkbAnyEvent *) event;

	if (xkbEvent->xkb_type == XkbStateNotify)
	{
	    XkbStateNotifyEvent *stateEvent = (XkbStateNotifyEvent *) event;
	    CompAction	        *action;
	    bool		inverted;
	    unsigned int	mods = 0xffffffff;

	    action   = &optionGetSnapKey ();
	    inverted = optionGetSnapInverted ();

	    if (action->type () & CompAction::BindingTypeKey)
		mods = action->key ().modifiers ();

	    if ((stateEvent->mods & mods) == mods)
	    {
		if (inverted)
		    disableSnapping ();
		else
		    enableSnapping ();
	    }
	    else
	    {
		if (inverted)
		    enableSnapping ();
		else
		    disableSnapping ();
	    }
	}
    }

    ::screen->handleEvent (event);

    switch (event->type)
    {
    case MotionNotify:
	if (event->xmotion.root == ::screen->root () &&
	    grabWindow &&
	    moveWindow &&
	    optionGetMaximizeEffect ())
	{
	    WobblyWindow *ww = WobblyWindow::get (grabWindow);

	    if (ww && (ww->state & MAXIMIZE_STATE))
	    {
		if (ww->model && ww->grabbed)
		{
		    int dx, dy;

		    if (ww->state & CompWindowStateMaximizedHorzMask)
			dx = pointerX - lastPointerX;
		    else
			dx = 0;

		    if (ww->state & CompWindowStateMaximizedVertMask)
			dy = pointerY - lastPointerY;
		    else
			dy = 0;

		    ww->model->anchorObject->position.x += dx;
		    ww->model->anchorObject->position.y += dy;

		    startWobbling (ww);
		}
	    }
	}
    default:
	break;
    }

    if (::screen->activeWindow () != activeWindow)
    {
	w = screen->findWindow (::screen->activeWindow ());
	if (!w)
	    return;

	WobblyWindow *ww = WobblyWindow::get (w);

	if (ww->isWobblyWin ())
	{
	    int focusEffect;

	    focusEffect = optionGetFocusEffect ();

	    if ((focusEffect != WobblyOptions::FocusEffectNone) &&
		optionGetFocusWindowMatch ().evaluate (w) &&
		ww->ensureModel ())
	    {
		switch (focusEffect) {
		case WobblyOptions::FocusEffectShiver:
		    {
			CompRect outRect (w->serverOutputRect ());

			ww->model->adjustObjectsForShiver (outRect.x (),
							   outRect.y (),
							   outRect.width (),
							   outRect.height ());
		    }
		default:
		    break;
		}

		startWobbling (ww);
	    }
	}
    }
}

void
WobblyWindow::enableWobbling (bool enabling)
{
    gWindow->glPaintSetEnabled (this, enabling);
    gWindow->glAddGeometrySetEnabled (this, enabling);
    cWindow->damageRectSetEnabled (this, enabling);
}

void
WobblyScreen::startWobbling (WobblyWindow *ww)
{
    if (!ww->wobblingMask)
	ww->enableWobbling (true);

    if (!wobblingWindowsMask)
    {
	cScreen->preparePaintSetEnabled (this, true);
	cScreen->donePaintSetEnabled (this, true);
	gScreen->glPaintOutputSetEnabled (this, true);
    }
    ww->wobblingMask |= WobblyInitialMask;
    wobblingWindowsMask |= ww->wobblingMask;

    cScreen->damagePending ();
}

bool
WobblyWindow::damageRect (bool initial,
			  const CompRect &rect)
{
    if (!initial)
    {
	if (wobblingMask == WobblyForceMask)
	{
	    int x1 = model->topLeft.x;
	    int y1 = model->topLeft.y;
	    int x2 = model->bottomRight.x + 0.5f;
	    int y2 = model->bottomRight.y + 0.5f;

	    wScreen->cScreen->damageRegion (CompRegion (x1, y1,
							x2 - x1, y2 - y1));

	    return true;
	}
    }

    return cWindow->damageRect (initial, rect);
}

void
WobblyWindow::initiateMapEffect ()
{
    int mapEffect = wScreen->optionGetMapEffect ();

    if ((mapEffect != WobblyOptions::MapEffectNone)    &&
	wScreen->optionGetMapWindowMatch ().evaluate (window) &&
	ensureModel ())
    {
	CompRect outRect (window->outputRect ());

	model->initObjects (outRect.x (), outRect.y (),
			    outRect.width (), outRect.height ());
	model->initSprings (outRect.x (), outRect.y (),
			    outRect.width (), outRect.height ());

	switch (mapEffect)
	{
	case WobblyOptions::MapEffectShiver:
	    model->adjustObjectsForShiver (outRect.x (),
					   outRect.y (),
					   outRect.width (),
					   outRect.height ());
	    break;
	default:
	    break;
	}

	wScreen->startWobbling (this);
    }
}

void
WobblyWindow::resizeNotify (int dx,
			    int dy,
			    int dwidth,
			    int dheight)
{
    CompRect outRect (window->outputRect ());

    if (wScreen->optionGetMaximizeEffect () &&
	isWobblyWin () &&
	/* prevent wobbling when shading maximized windows - assuming that
	   the height difference shaded - non-shaded will hardly be -1 and
	   a lack of wobbly animation in that corner case is tolerable */
	(dheight != -1) &&
	((window->state () | state) & MAXIMIZE_STATE))
    {
	state &= (unsigned)~MAXIMIZE_STATE;
	state |= window->state () & MAXIMIZE_STATE;

	if (ensureModel ())
	{
	    if (window->state () & MAXIMIZE_STATE)
	    {
		if (!grabbed && model->anchorObject)
		{
		    model->anchorObject->immobile = false;
		    model->anchorObject = NULL;
		}

		model->addEdgeAnchors (outRect.x (), outRect.y (),
				       outRect.width (), outRect.height ());
	    }
	    else
	    {
		model->removeEdgeAnchors (outRect.x (), outRect.y (),
					  outRect.width (), outRect.height ());
		model->setMiddleAnchor (outRect.x (), outRect.y (),
					outRect.width (), outRect.height ());
	    }

	    model->initSprings (outRect.x (), outRect.y (),
				outRect.width (), outRect.height ());

	    wScreen->startWobbling (this);
	}
    }
    else if (model)
    {
	if (wobblingMask)
	{
	    if (!(state & MAXIMIZE_STATE))
		model->setTopAnchor (outRect.x (), outRect.y (),
				     outRect.width ());
	}
	else
	{
	    model->initObjects (outRect.x (), outRect.y (),
				outRect.width (), outRect.height ());
	}

	model->initSprings (outRect.x (), outRect.y (),
			    outRect.width (), outRect.height ());
    }

    /* update grab */
    if (model && grabbed)
    {
	if (model->anchorObject)
	    model->anchorObject->immobile = false;

	model->anchorObject = model->findNearestObject (pointerX,
							pointerY);
	model->anchorObject->immobile = true;

	model->adjustObjectPosition (model->anchorObject,
				     outRect.x (), outRect.y (),
				     outRect.width (), outRect.height ());
    }

    window->resizeNotify (dx, dy, dwidth, dheight);
}

void
WobblyWindow::moveNotify (int  dx,
			  int  dy,
			  bool immediate)
{
    if (model)
    {
	if (grabbed && !immediate)
	{
	    if (state & MAXIMIZE_STATE)
	    {
		Object *object = model->objects;
		for (int i = 0; i < model->numObjects; i++, object++)
		{
		    if (object->immobile)
		    {
			object->position.x += dx;
			object->position.y += dy;
		    }
		}
	    }
	    else
	    {
		model->anchorObject->position.x += dx;
		model->anchorObject->position.y += dy;
	    }

	    wScreen->startWobbling (this);
	}
	else
	    model->move (dx, dy);
    }

    window->moveNotify (dx, dy, immediate);
}

void
WobblyWindow::grabNotify (int          x,
			  int          y,
			  unsigned int state,
			  unsigned int mask)
{
    if (!wScreen->grabWindow)
    {
	wScreen->grabMask   = mask;
	wScreen->grabWindow = window;
    }
    wScreen->moveWindow = false;

    if (mask & (CompWindowGrabButtonMask) &&
	mask & (CompWindowGrabMoveMask) &&
	wScreen->optionGetMoveWindowMatch ().evaluate (window) &&
	isWobblyWin ())
    {
	wScreen->moveWindow = true;

	if (ensureModel ())
	{
	    Spring *s;

	    if (wScreen->optionGetMaximizeEffect ())
	    {
		CompRect outRect (window->outputRect ());

		if (window->state () & MAXIMIZE_STATE)
		{
		    model->addEdgeAnchors (outRect.x (), outRect.y (),
					   outRect.width (), outRect.height ());
		}
		else
		{
		    model->removeEdgeAnchors (outRect.x (), outRect.y (),
					      outRect.width (), outRect.height ());

		    if (model->anchorObject)
			model->anchorObject->immobile = false;
		}
	    }
	    else
	    {
		if (model->anchorObject)
		    model->anchorObject->immobile = false;
	    }

	    model->anchorObject = model->findNearestObject (x, y);
	    model->anchorObject->immobile = true;

	    grabbed = true;

	    /* Update isConstrained and work area box at grab time */
	    wScreen->yConstrained = false;
	    if (mask & CompWindowGrabExternalAppMask)
	    {
		CompPlugin *pMove;

		pMove = CompPlugin::find ("move");
		if (pMove)
		{
		    CompOption::Vector &moveOptions =
			pMove->vTable->getOptions ();

		    wScreen->yConstrained =
			CompOption::getBoolOptionNamed (moveOptions,
							"constrain_y", true);
		}
	    }

	    if (wScreen->yConstrained)
	    {
		int output =
		    ::screen->outputDeviceForGeometry (window->serverGeometry ());
		wScreen->constraintBox =
		    &::screen->outputDevs ()[output].workArea ();
	    }

	    if (mask & CompWindowGrabMoveMask)
	    {
		model->disableSnapping ();
		if (wScreen->snapping)
		    updateModelSnapping ();
	    }

	    if (wScreen->optionGetGrabWindowMatch ().evaluate (window))
	    {
		int i;
		for (i = 0; i < model->numSprings; i++)
		{
		    s = &model->springs[i];

		    if (s->a == model->anchorObject)
		    {
			s->b->velocity.x -= s->offset.x * 0.05f;
			s->b->velocity.y -= s->offset.y * 0.05f;
		    }
		    else if (s->b == model->anchorObject)
		    {
			s->a->velocity.x += s->offset.x * 0.05f;
			s->a->velocity.y += s->offset.y * 0.05f;
		    }
		}

		wScreen->startWobbling (this);
	    }
	}
    }

    window->grabNotify (x, y, state, mask);
}

void
WobblyWindow::ungrabNotify ()
{
    if (window == wScreen->grabWindow)
    {
	wScreen->grabMask   = 0;
	wScreen->grabWindow = NULL;
	wScreen->constraintBox = NULL;
    }

    if (grabbed)
    {
	if (model)
	{
	    if (model->anchorObject)
		model->anchorObject->immobile = false;

	    model->anchorObject = NULL;

	    if (wScreen->optionGetMaximizeEffect () &&
		(state & MAXIMIZE_STATE))
	    {
		CompRect outRect (window->outputRect ());

		model->addEdgeAnchors (outRect.x (), outRect.y (),
				       outRect.width (), outRect.height ());
	    }

	    wScreen->startWobbling (this);
	}

	grabbed = false;
    }

    window->ungrabNotify ();
}

bool
WobblyScreen::glPaintOutput (const GLScreenPaintAttrib &sAttrib,
			     const GLMatrix            &transform,
			     const CompRegion          &region,
			     CompOutput                *output,
			     unsigned int              mask)
{
    if (wobblingWindowsMask)
	mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK;

    return gScreen->glPaintOutput (sAttrib, transform, region, output, mask);
}

void
WobblyScreen::snapKeyChanged (CompOption *opt)
{
    // ignore the key
    CompAction::KeyBinding newKeyBinding
	(0, opt->value ().action ().key ().modifiers ());
    opt->value ().action ().setKey (newKeyBinding);
}

void
WobblyScreen::snapInvertedChanged (CompOption *opt)
{
    // ignore the key
    if (opt->value ().b ())
	enableSnapping ();
    else
	disableSnapping ();
}

WobblyScreen::WobblyScreen (CompScreen *s) :
    PluginClassHandler<WobblyScreen, CompScreen> (s),
    cScreen (CompositeScreen::get (s)),
    gScreen (GLScreen::get (s)),
    wobblingWindowsMask (0),
    grabMask (0),
    grabWindow (NULL),
    moveWindow (false),
    snapping (false),
    yConstrained (false),
    constraintBox (NULL)
{
    optionSetSnapKeyInitiate (boost::bind
			      (&WobblyScreen::enableSnapping, this));
    optionSetSnapKeyTerminate (boost::bind
			       (&WobblyScreen::disableSnapping, this));
    optionSetShiverInitiate (boost::bind (&WobblyScreen::shiver, this, _3));

    optionSetSnapKeyNotify (boost::bind (&WobblyScreen::snapKeyChanged, _1));
    optionSetSnapInvertedNotify (boost::bind
				 (&WobblyScreen::snapInvertedChanged,
				  this, _1));

    ScreenInterface::setHandler (::screen);
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);
}

WobblyWindow::WobblyWindow (CompWindow *w) :
    PluginClassHandler<WobblyWindow, CompWindow> (w),
    wScreen (WobblyScreen::get (::screen)),
    window (w),
    cWindow (CompositeWindow::get (w)),
    gWindow (GLWindow::get (w)),
    model (0),
    wobblingMask (0),
    grabbed (false),
    state (w->state ())
{
    if ((w->mapNum () && wScreen->optionGetMaximizeEffect ()) ||
	wScreen->optionGetMapEffect () != WobblyOptions::MapEffectNone)
    {
	if (isWobblyWin ())
	    ensureModel ();
    }

    WindowInterface::setHandler (window);
    CompositeWindowInterface::setHandler (cWindow, false);
    GLWindowInterface::setHandler (gWindow, false);
}

WobblyWindow::~WobblyWindow ()
{
    if (wScreen->grabWindow == window)
    {
	wScreen->grabWindow = NULL;
	wScreen->grabMask   = 0;
    }

    if (model)
	delete model;
}

Model::~Model ()
{
    delete[] objects;
}

bool
WobblyPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) |
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) |
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	 return false;

    return true;
}

