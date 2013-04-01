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

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xproto.h>
#include <X11/extensions/shape.h>

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <math.h>

#include <boost/bind.hpp>

#include <core/icon.h>
#include <core/atoms.h>
#include "core/windowconstrainment.h"
#include "privatewindow.h"
#include "privatescreen.h"
#include "privatestackdebugger.h"

#include <boost/scoped_array.hpp>

PluginClassStorage::Indices windowPluginClassIndices (0);

unsigned int
CompWindow::allocPluginClassIndex ()
{
    unsigned int i = PluginClassStorage::allocatePluginClassIndex (windowPluginClassIndices);

    foreach (CompWindow *w, screen->windows ())
	if (windowPluginClassIndices.size () != w->pluginClasses.size ())
	    w->pluginClasses.resize (windowPluginClassIndices.size ());

    return i;
}

void
CompWindow::freePluginClassIndex (unsigned int index)
{
    PluginClassStorage::freePluginClassIndex (windowPluginClassIndices, index);

    foreach (CompWindow *w, ::screen->windows ())
	if (windowPluginClassIndices.size () != w->pluginClasses.size ())
	    w->pluginClasses.resize (windowPluginClassIndices.size ());
}

inline bool
PrivateWindow::isInvisible() const
{
    return attrib.map_state != IsViewable ||
     attrib.x + geometry.width ()  + output.right  <= 0 ||
     attrib.y + geometry.height () + output.bottom <= 0 ||
     attrib.x - output.left >= (int) screen->width () ||
     attrib.y - output.top >= (int) screen->height ();
}

bool
PrivateWindow::isAncestorTo (CompWindow *transient,
			     CompWindow *ancestor)
{
    if (transient->priv->transientFor)
    {
	if (transient->priv->transientFor == ancestor->priv->id)
	    return true;

	transient = screen->findWindow (transient->priv->transientFor);
	if (transient)
	    return isAncestorTo (transient, ancestor);
    }

    return false;
}

void
PrivateWindow::recalcNormalHints ()
{
    int maxSize;

/* FIXME to max Texture size */
    maxSize  = MAXSHORT;
    maxSize -= serverGeometry.border () * 2;

    sizeHints.x      = serverGeometry.x ();
    sizeHints.y      = serverGeometry.y ();
    sizeHints.width  = serverGeometry.width ();
    sizeHints.height = serverGeometry.height ();

    if (!(sizeHints.flags & PBaseSize))
    {
	if (sizeHints.flags & PMinSize)
	{
	    sizeHints.base_width  = sizeHints.min_width;
	    sizeHints.base_height = sizeHints.min_height;
	}
	else
	{
	    sizeHints.base_width  = 0;
	    sizeHints.base_height = 0;
	}

	sizeHints.flags |= PBaseSize;
    }

    if (!(sizeHints.flags & PMinSize))
    {
	sizeHints.min_width  = sizeHints.base_width;
	sizeHints.min_height = sizeHints.base_height;
	sizeHints.flags |= PMinSize;
    }

    if (!(sizeHints.flags & PMaxSize))
    {
	sizeHints.max_width  = 65535;
	sizeHints.max_height = 65535;
	sizeHints.flags |= PMaxSize;
    }

    if (sizeHints.max_width < sizeHints.min_width)
	sizeHints.max_width = sizeHints.min_width;

    if (sizeHints.max_height < sizeHints.min_height)
	sizeHints.max_height = sizeHints.min_height;

    if (sizeHints.min_width < 1)
	sizeHints.min_width = 1;

    if (sizeHints.max_width < 1)
	sizeHints.max_width = 1;

    if (sizeHints.min_height < 1)
	sizeHints.min_height = 1;

    if (sizeHints.max_height < 1)
	sizeHints.max_height = 1;

    if (sizeHints.max_width > maxSize)
	sizeHints.max_width = maxSize;

    if (sizeHints.max_height > maxSize)
	sizeHints.max_height = maxSize;

    if (sizeHints.min_width > maxSize)
	sizeHints.min_width = maxSize;

    if (sizeHints.min_height > maxSize)
	sizeHints.min_height = maxSize;

    if (sizeHints.base_width > maxSize)
	sizeHints.base_width = maxSize;

    if (sizeHints.base_height > maxSize)
	sizeHints.base_height = maxSize;

    if (sizeHints.flags & PResizeInc)
    {
	if (sizeHints.width_inc == 0)
	    sizeHints.width_inc = 1;

	if (sizeHints.height_inc == 0)
	    sizeHints.height_inc = 1;
    }
    else
    {
	sizeHints.width_inc  = 1;
	sizeHints.height_inc = 1;
	sizeHints.flags |= PResizeInc;
    }

    if (sizeHints.flags & PAspect)
    {
	/* don't divide by 0 */
	if (sizeHints.min_aspect.y < 1)
	    sizeHints.min_aspect.y = 1;

	if (sizeHints.max_aspect.y < 1)
	    sizeHints.max_aspect.y = 1;
    }
    else
    {
	sizeHints.min_aspect.x = 1;
	sizeHints.min_aspect.y = 65535;
	sizeHints.max_aspect.x = 65535;
	sizeHints.max_aspect.y = 1;
	sizeHints.flags |= PAspect;
    }

    if (!(sizeHints.flags & PWinGravity))
    {
	sizeHints.win_gravity = NorthWestGravity;
	sizeHints.flags |= PWinGravity;
    }
}

void
PrivateWindow::updateNormalHints ()
{
    Status status;
    long   supplied;

    status = XGetWMNormalHints (screen->dpy (), priv->id,
				&priv->sizeHints, &supplied);

    if (!status)
	priv->sizeHints.flags = 0;

    priv->recalcNormalHints ();
}

void
PrivateWindow::updateWmHints ()
{
    XWMHints *newHints;
    long     dFlags = 0;
    bool     iconChanged = false;

    if (hints)
	dFlags = hints->flags;

    inputHint = true;

    newHints = XGetWMHints (screen->dpy (), id);
    if (newHints)
    {
	dFlags ^= newHints->flags;

	if (newHints->flags & InputHint)
	    inputHint = newHints->input;

	if (hints)
	{
	    if ((newHints->flags & IconPixmapHint) &&
		(hints->icon_pixmap != newHints->icon_pixmap))
	    {
		iconChanged = true;
	    }
	    else if ((newHints->flags & IconMaskHint) &&
		     (hints->icon_mask != newHints->icon_mask))
	    {
		iconChanged = true;
	    }
	}
    }

    iconChanged |= (dFlags & (IconPixmapHint | IconMaskHint));

    if (iconChanged)
	freeIcons ();

    if (hints)
	XFree (hints);

    hints = newHints;
}

void
PrivateWindow::updateClassHints ()
{
    XClassHint classHint;
    int	       status;

    if (priv->resName)
    {
	free (priv->resName);
	priv->resName = NULL;
    }

    if (priv->resClass)
    {
	free (priv->resClass);
	priv->resClass = NULL;
    }

    status = XGetClassHint (screen->dpy (),
			    priv->id, &classHint);
    if (status)
    {
	if (classHint.res_name)
	{
	    priv->resName = strdup (classHint.res_name);
	    XFree (classHint.res_name);
	}

	if (classHint.res_class)
	{
	    priv->resClass = strdup (classHint.res_class);
	    XFree (classHint.res_class);
	}
    }
}

void
PrivateWindow::updateTransientHint ()
{
    Window transientFor;
    Status status;

    priv->transientFor = None;

    status = XGetTransientForHint (screen->dpy (),
				   priv->id, &transientFor);

    if (status)
    {
	CompWindow *ancestor;

	ancestor = screen->findWindow (transientFor);
	if (!ancestor)
	    return;

	/* protect against circular transient dependencies */
	if (transientFor == priv->id ||
	    PrivateWindow::isAncestorTo (ancestor, window))
	    return;

	priv->transientFor = transientFor;
    }
}

void
PrivateWindow::updateIconGeometry ()
{
    Atom	  actual;
    int		  result, format;
    unsigned long n, left;
    unsigned char *data;

    priv->iconGeometry.setGeometry (0, 0, 0, 0);

    result = XGetWindowProperty (screen->dpy (), priv->id,
				 Atoms::wmIconGeometry,
				 0L, 1024L, False, XA_CARDINAL,
				 &actual, &format, &n, &left, &data);

    if (result == Success && data)
    {
	if (n == 4)
	{
	    unsigned long *geometry = (unsigned long *) data;

	    priv->iconGeometry.setX (geometry[0]);
	    priv->iconGeometry.setY (geometry[1]);
	    priv->iconGeometry.setWidth (geometry[2]);
	    priv->iconGeometry.setHeight (geometry[3]);
	}

	XFree (data);
    }
}

Window
PrivateWindow::getClientLeaderOfAncestor ()
{
    if (transientFor)
    {
	CompWindow *w = screen->findWindow (transientFor);
	if (w)
	{
	    if (w->priv->clientLeader)
		return w->priv->clientLeader;

	    return w->priv->getClientLeaderOfAncestor ();
	}
    }

    return None;
}

Window
PrivateWindow::getClientLeader ()
{
    Atom	  actual;
    int		  result, format;
    unsigned long n, left;
    unsigned char *data;

    result = XGetWindowProperty (screen->dpy (), priv->id,
				 Atoms::wmClientLeader,
				 0L, 1L, False, XA_WINDOW, &actual, &format,
				 &n, &left, &data);

    if (result == Success && data)
    {
	Window win = None;

	if (n)
	    memcpy (&win, data, sizeof (Window));

	XFree ((void *) data);

	if (win)
	    return win;
    }

    return priv->getClientLeaderOfAncestor ();
}

char *
PrivateWindow::getStartupId ()
{
    Atom	  actual;
    int		  result, format;
    unsigned long n, left;
    unsigned char *data;

    result = XGetWindowProperty (screen->dpy (), priv->id,
				 Atoms::startupId,
				 0L, 1024L, False,
				 Atoms::utf8String,
				 &actual, &format,
				 &n, &left, &data);

    if (result == Success && data)
    {
	char *id = NULL;

	if (n)
	    id = strdup ((char *) data);
	XFree ((void *) data);

	return id;
    }

    return NULL;
}

void
PrivateWindow::setFullscreenMonitors (CompFullscreenMonitorSet *monitors)
{
    bool         hadFsMonitors = fullscreenMonitorsSet;
    unsigned int outputs = screen->outputDevs ().size ();

    fullscreenMonitorsSet = false;

    if (monitors                   &&
	(unsigned int) monitors->left   < outputs &&
	(unsigned int) monitors->right  < outputs &&
	(unsigned int) monitors->top    < outputs &&
	(unsigned int) monitors->bottom < outputs)
    {
	CompRect fsRect (screen->outputDevs ()[monitors->left].x1 (),
			 screen->outputDevs ()[monitors->top].y1 (),
			 screen->outputDevs ()[monitors->right].x2 (),
			 screen->outputDevs ()[monitors->bottom].y2 ());

	if (fsRect.x1 () < fsRect.x2 () && fsRect.y1 () < fsRect.y2 ())
	{
	    fullscreenMonitorsSet = true;
	    fullscreenMonitorRect = fsRect;
	}
    }

    if (fullscreenMonitorsSet)
    {
	long data[4];

	data[0] = monitors->top;
	data[1] = monitors->bottom;
	data[2] = monitors->left;
	data[3] = monitors->right;

	XChangeProperty (screen->dpy (), id, Atoms::wmFullscreenMonitors,
			 XA_CARDINAL, 32, PropModeReplace,
			 (unsigned char *) data, 4);
    }
    else if (hadFsMonitors)
    {
	XDeleteProperty (screen->dpy (), id, Atoms::wmFullscreenMonitors);
    }

    if (state & CompWindowStateFullscreenMask)
	if (fullscreenMonitorsSet || hadFsMonitors)
	    window->updateAttributes (CompStackingUpdateModeNone);
}

void
CompWindow::changeState (unsigned int newState)
{
    unsigned int oldState;

    if (priv->state == newState)
	return;

    oldState = priv->state;
    priv->state = newState;

    recalcType ();
    recalcActions ();

    if (priv->managed)
	screen->setWindowState (priv->state, priv->id);

    stateChangeNotify (oldState);
    screen->matchPropertyChanged (this);
}

static void
setWindowActions (CompScreen   *s,
		  unsigned int actions,
		  Window       id)
{
    Atom data[32];
    int	 i = 0;

    if (actions & CompWindowActionMoveMask)
	data[i++] = Atoms::winActionMove;
    if (actions & CompWindowActionResizeMask)
	data[i++] = Atoms::winActionResize;
    if (actions & CompWindowActionStickMask)
	data[i++] = Atoms::winActionStick;
    if (actions & CompWindowActionMinimizeMask)
	data[i++] = Atoms::winActionMinimize;
    if (actions & CompWindowActionMaximizeHorzMask)
	data[i++] = Atoms::winActionMaximizeHorz;
    if (actions & CompWindowActionMaximizeVertMask)
	data[i++] = Atoms::winActionMaximizeVert;
    if (actions & CompWindowActionFullscreenMask)
	data[i++] = Atoms::winActionFullscreen;
    if (actions & CompWindowActionCloseMask)
	data[i++] = Atoms::winActionClose;
    if (actions & CompWindowActionShadeMask)
	data[i++] = Atoms::winActionShade;
    if (actions & CompWindowActionChangeDesktopMask)
	data[i++] = Atoms::winActionChangeDesktop;
    if (actions & CompWindowActionAboveMask)
	data[i++] = Atoms::winActionAbove;
    if (actions & CompWindowActionBelowMask)
	data[i++] = Atoms::winActionBelow;

    XChangeProperty (s->dpy (), id, Atoms::wmAllowedActions,
		     XA_ATOM, 32, PropModeReplace,
		     (unsigned char *) data, i);
}

void
CompWindow::recalcActions ()
{
    unsigned int actions = 0;
    unsigned int setActions, clearActions;

    switch (priv->type) {
    case CompWindowTypeFullscreenMask:
    case CompWindowTypeNormalMask:
	actions =
	    CompWindowActionMaximizeHorzMask |
	    CompWindowActionMaximizeVertMask |
	    CompWindowActionFullscreenMask   |
	    CompWindowActionMoveMask         |
	    CompWindowActionResizeMask       |
	    CompWindowActionStickMask        |
	    CompWindowActionMinimizeMask     |
	    CompWindowActionCloseMask	     |
	    CompWindowActionChangeDesktopMask;
	break;
    case CompWindowTypeUtilMask:
    case CompWindowTypeMenuMask:
    case CompWindowTypeToolbarMask:
	actions =
	    CompWindowActionMoveMask   |
	    CompWindowActionResizeMask |
	    CompWindowActionStickMask  |
	    CompWindowActionCloseMask  |
	    CompWindowActionChangeDesktopMask;
	break;
    case CompWindowTypeDialogMask:
    case CompWindowTypeModalDialogMask:
	actions =
	    CompWindowActionMaximizeHorzMask |
	    CompWindowActionMaximizeVertMask |
	    CompWindowActionMoveMask         |
	    CompWindowActionResizeMask       |
	    CompWindowActionStickMask        |
	    CompWindowActionCloseMask        |
	    CompWindowActionChangeDesktopMask;

	/* allow minimization for dialog windows if they
	   a) are not a transient (transients can be minimized
	      with their parent)
	   b) don't have the skip taskbar hint set (as those
	      have no target to be minimized to)
	*/
	if (!priv->transientFor &&
	    !(priv->state & CompWindowStateSkipTaskbarMask))
	{
	    actions |= CompWindowActionMinimizeMask;
	}
    default:
	break;
    }

    if (priv->serverInput.top)
	actions |= CompWindowActionShadeMask;

    actions |= (CompWindowActionAboveMask | CompWindowActionBelowMask);

    switch (priv->wmType) {
    case CompWindowTypeNormalMask:
	actions |= CompWindowActionFullscreenMask |
	           CompWindowActionMinimizeMask;
    default:
	break;
    }

    if (priv->sizeHints.min_width  == priv->sizeHints.max_width &&
	priv->sizeHints.min_height == priv->sizeHints.max_height)
	actions &= ~(CompWindowActionResizeMask	      |
		     CompWindowActionMaximizeHorzMask |
		     CompWindowActionMaximizeVertMask |
		     CompWindowActionFullscreenMask);

    /* Don't allow maximization or fullscreen
     * of windows which are too big to fit
     * the screen */
    bool foundVert = false;
    bool foundHorz = false;
    bool foundFull = false;

    foreach (CompOutput &o, screen->outputDevs ())
    {
	if (o.width () >= (priv->sizeHints.min_width + priv->border.left + priv->border.right))
	    foundHorz = true;
	if (o.height () >= (priv->sizeHints.min_height + priv->border.top + priv->border.bottom))
	    foundVert = true;

	/* Fullscreen windows don't need to fit borders... */
	if (o.width () >= priv->sizeHints.min_width &&
	    o.height () >= priv->sizeHints.min_height)
	    foundFull = true;
    }

    if (!foundHorz)
	actions &= ~CompWindowActionMaximizeHorzMask;

    if (!foundVert)
	actions &= ~CompWindowActionMaximizeVertMask;

    if (!foundFull)
	actions &= ~CompWindowActionFullscreenMask;

    if (!(priv->mwmFunc & MwmFuncAll))
    {
	if (!(priv->mwmFunc & MwmFuncResize))
	    actions &= ~(CompWindowActionResizeMask	  |
			 CompWindowActionMaximizeHorzMask |
			 CompWindowActionMaximizeVertMask |
			 CompWindowActionFullscreenMask);

	if (!(priv->mwmFunc & MwmFuncMove))
	    actions &= ~(CompWindowActionMoveMask	  |
			 CompWindowActionMaximizeHorzMask |
			 CompWindowActionMaximizeVertMask |
			 CompWindowActionFullscreenMask);

	if (!(priv->mwmFunc & MwmFuncIconify))
	    actions &= ~CompWindowActionMinimizeMask;

	if (!(priv->mwmFunc & MwmFuncClose))
	    actions &= ~CompWindowActionCloseMask;
    }

    getAllowedActions (setActions, clearActions);
    actions &= ~clearActions;
    actions |= setActions;

    if (actions != priv->actions)
    {
	priv->actions = actions;
	setWindowActions (screen, actions, priv->id);
    }
}

void
CompWindow::getAllowedActions (unsigned int &setActions,
			       unsigned int &clearActions)
{
    WRAPABLE_HND_FUNCTN (getAllowedActions, setActions, clearActions)

    setActions   = 0;
    clearActions = 0;
}

unsigned int
CompWindow::constrainWindowState (unsigned int state,
				  unsigned int actions)
{
    if (!(actions & CompWindowActionMaximizeHorzMask))
	state &= ~CompWindowStateMaximizedHorzMask;

    if (!(actions & CompWindowActionMaximizeVertMask))
	state &= ~CompWindowStateMaximizedVertMask;

    if (!(actions & CompWindowActionShadeMask))
	state &= ~CompWindowStateShadedMask;

    if (!(actions & CompWindowActionFullscreenMask))
	state &= ~CompWindowStateFullscreenMask;

    return state;
}

unsigned int
PrivateWindow::windowTypeFromString (const char *str)
{
    if (strcasecmp (str, "desktop") == 0)
	return CompWindowTypeDesktopMask;
    else if (strcasecmp (str, "dock") == 0)
	return CompWindowTypeDockMask;
    else if (strcasecmp (str, "toolbar") == 0)
	return CompWindowTypeToolbarMask;
    else if (strcasecmp (str, "menu") == 0)
	return CompWindowTypeMenuMask;
    else if (strcasecmp (str, "utility") == 0)
	return CompWindowTypeUtilMask;
    else if (strcasecmp (str, "splash") == 0)
	return CompWindowTypeSplashMask;
    else if (strcasecmp (str, "dialog") == 0)
	return CompWindowTypeDialogMask;
    else if (strcasecmp (str, "normal") == 0)
	return CompWindowTypeNormalMask;
    else if (strcasecmp (str, "dropdownmenu") == 0)
	return CompWindowTypeDropdownMenuMask;
    else if (strcasecmp (str, "popupmenu") == 0)
	return CompWindowTypePopupMenuMask;
    else if (strcasecmp (str, "tooltip") == 0)
	return CompWindowTypeTooltipMask;
    else if (strcasecmp (str, "notification") == 0)
	return CompWindowTypeNotificationMask;
    else if (strcasecmp (str, "combo") == 0)
	return CompWindowTypeComboMask;
    else if (strcasecmp (str, "dnd") == 0)
	return CompWindowTypeDndMask;
    else if (strcasecmp (str, "modaldialog") == 0)
	return CompWindowTypeModalDialogMask;
    else if (strcasecmp (str, "fullscreen") == 0)
	return CompWindowTypeFullscreenMask;
    else if (strcasecmp (str, "unknown") == 0)
	return CompWindowTypeUnknownMask;
    else if (strcasecmp (str, "any") == 0)
	return ~0;

    return 0;
}

void
CompWindow::recalcType ()
{
    unsigned int type;

    type = priv->wmType;

    if (!overrideRedirect () && priv->wmType == CompWindowTypeUnknownMask)
	type = CompWindowTypeNormalMask;

    if (priv->state & CompWindowStateFullscreenMask)
	type = CompWindowTypeFullscreenMask;

    if (type == CompWindowTypeNormalMask)
    {
	if (priv->transientFor)
	    type = CompWindowTypeDialogMask;
    }

    if (type == CompWindowTypeDockMask &&
	(priv->state & CompWindowStateBelowMask))
    {
	type = CompWindowTypeNormalMask;
    }

    if ((type & (CompWindowTypeNormalMask | CompWindowTypeDialogMask)) &&
	(priv->state & CompWindowStateModalMask))
    {
	type = CompWindowTypeModalDialogMask;
    }

    priv->type = type;
}


void
PrivateWindow::updateFrameWindow ()
{
    XWindowChanges xwc = XWINDOWCHANGES_INIT;
    unsigned int   valueMask = CWX | CWY | CWWidth | CWHeight;

    if (!serverFrame)
	return;

    xwc.x = serverGeometry.x ();
    xwc.y = serverGeometry.y ();
    xwc.width = serverGeometry.width ();
    xwc.height = serverGeometry.height ();
    xwc.border_width = serverGeometry.border ();

    window->configureXWindow (valueMask, &xwc);
    window->windowNotify (CompWindowNotifyFrameUpdate);
    window->recalcActions ();
}



void
CompWindow::updateWindowOutputExtents ()
{
    CompWindowExtents output (priv->output);

    getOutputExtents (output);

    if (output.left   != priv->output.left  ||
	output.right  != priv->output.right ||
	output.top    != priv->output.top   ||
	output.bottom != priv->output.bottom)
    {
	priv->output = output;

	resizeNotify (0, 0, 0, 0);
    }
}

void
CompWindow::getOutputExtents (CompWindowExtents& output)
{
    WRAPABLE_HND_FUNCTN (getOutputExtents, output)

    output.left   = 0;
    output.right  = 0;
    output.top    = 0;
    output.bottom = 0;
}

CompRegion
PrivateWindow::rectsToRegion (unsigned int n, XRectangle *rects)
{
    CompRegion ret;
    int        x1, x2, y1, y2;
    const CompWindow::Geometry &geom = attrib.override_redirect ? priv->geometry : priv->serverGeometry;

    for (unsigned int i = 0; i < n; i++)
    {
	x1 = rects[i].x + geom.border ();
	y1 = rects[i].y + geom.border ();
	x2 = x1 + rects[i].width;
	y2 = y1 + rects[i].height;

	if (x1 < 0)
	    x1 = 0;
	if (y1 < 0)
	    y1 = 0;
	if (x2 > geom.width ())
	    x2 = geom.width ();
	if (y2 > geom.height ())
	    y2 = geom.height ();

	if (y1 < y2 && x1 < x2)
	{
	    x1 += geom.x ();
	    y1 += geom.y ();
	    x2 += geom.x ();
	    y2 += geom.y ();

	    ret += CompRect (x1, y1, x2 - x1, y2 - y1);
	}
    }

    return ret;
}

/* TODO: This function should be able to check the XShape event
 * kind and only get/set shape rectangles for either ShapeInput
 * or ShapeBounding, but not both at the same time
 */

void
PrivateWindow::updateRegion ()
{
    XRectangle r, *boundingShapeRects = NULL;
    XRectangle *inputShapeRects = NULL;
    int	       nBounding = 0, nInput = 0;
    const CompWindow::Geometry &geom = attrib.override_redirect ? priv->geometry : priv->serverGeometry;

    priv->region = priv->inputRegion = emptyRegion;

    if (screen->XShape ())
    {
	int order;

	/* We should update the server here */
	XSync (screen->dpy (), false);

	boundingShapeRects = XShapeGetRectangles (screen->dpy (), priv->id,
					 	  ShapeBounding, &nBounding, &order);
	inputShapeRects = XShapeGetRectangles (screen->dpy (), priv->id,
					       ShapeInput, &nInput, &order);
    }

    r.x      = -geom.border ();
    r.y      = -geom.border ();
    r.width  = geom.widthIncBorders ();
    r.height = geom.heightIncBorders ();

    if (nBounding < 1)
    {
	boundingShapeRects = &r;
	nBounding = 1;
    }

    if (nInput < 1)
    {
	inputShapeRects = &r;
	nBounding = 1;
    }

    priv->region += rectsToRegion (nBounding, boundingShapeRects);
    priv->inputRegion += rectsToRegion (nInput, inputShapeRects);

    if (boundingShapeRects && boundingShapeRects != &r)
	XFree (boundingShapeRects);
    if (inputShapeRects && inputShapeRects != &r)
	XFree (inputShapeRects);

    window->updateFrameRegion ();
}

bool
CompWindow::updateStruts ()
{
    Atom	  actual;
    int		  result, format;
    unsigned long n, left;
    unsigned char *data;
    bool	  hasOld, hasNew;
    CompStruts    oldStrut, newStrut;

    if (priv->struts)
    {
	hasOld = true;

	oldStrut.left   = priv->struts->left;
	oldStrut.right  = priv->struts->right;
	oldStrut.top    = priv->struts->top;
	oldStrut.bottom = priv->struts->bottom;
    }
    else
    {
	hasOld = false;
    }

    hasNew = false;

    newStrut.left.x	    = 0;
    newStrut.left.y	    = 0;
    newStrut.left.width  = 0;
    newStrut.left.height = screen->height ();

    newStrut.right.x      = screen->width ();
    newStrut.right.y      = 0;
    newStrut.right.width  = 0;
    newStrut.right.height = screen->height ();

    newStrut.top.x	   = 0;
    newStrut.top.y	   = 0;
    newStrut.top.width  = screen->width ();
    newStrut.top.height = 0;

    newStrut.bottom.x      = 0;
    newStrut.bottom.y      = screen->height ();
    newStrut.bottom.width  = screen->width ();
    newStrut.bottom.height = 0;

    result = XGetWindowProperty (screen->dpy (), priv->id,
				 Atoms::wmStrutPartial,
				 0L, 12L, false, XA_CARDINAL, &actual, &format,
				 &n, &left, &data);

    if (result == Success && data)
    {
	unsigned long *struts = (unsigned long *) data;

	if (n == 12)
	{
	    hasNew = true;

	    newStrut.left.y        = struts[4];
	    newStrut.left.width    = struts[0];
	    newStrut.left.height   = struts[5] - newStrut.left.y + 1;

	    newStrut.right.width   = struts[1];
	    newStrut.right.x       = screen->width () - newStrut.right.width;
	    newStrut.right.y       = struts[6];
	    newStrut.right.height  = struts[7] - newStrut.right.y + 1;

	    newStrut.top.x         = struts[8];
	    newStrut.top.width     = struts[9] - newStrut.top.x + 1;
	    newStrut.top.height    = struts[2];

	    newStrut.bottom.x      = struts[10];
	    newStrut.bottom.width  = struts[11] - newStrut.bottom.x + 1;
	    newStrut.bottom.height = struts[3];
	    newStrut.bottom.y      = screen->height () - newStrut.bottom.height;
	}

	XFree (data);
    }

    if (!hasNew)
    {
	result = XGetWindowProperty (screen->dpy (), priv->id,
				     Atoms::wmStrut,
				     0L, 4L, false, XA_CARDINAL,
				     &actual, &format, &n, &left, &data);

	if (result == Success && data)
	{
	    unsigned long *struts = (unsigned long *) data;

	    if (n == 4)
	    {
		hasNew = true;

		newStrut.left.x        = 0;
		newStrut.left.width    = struts[0];

		newStrut.right.width   = struts[1];
		newStrut.right.x       = screen->width () - newStrut.right.width;

		newStrut.top.y         = 0;
		newStrut.top.height    = struts[2];

		newStrut.bottom.height = struts[3];
		newStrut.bottom.y      = screen->height () - newStrut.bottom.height;
	    }

	    XFree (data);
	}
    }

    if (hasNew)
    {
	int strutX1, strutY1, strutX2, strutY2;
	int x1, y1, x2, y2;

	/* applications expect us to clip struts to xinerama edges */
	for (unsigned int i = 0;
	     i < screen->screenInfo ().size (); i++)
	{
	    x1 = screen->screenInfo ()[i].x_org;
	    y1 = screen->screenInfo ()[i].y_org;
	    x2 = x1 + screen->screenInfo ()[i].width;
	    y2 = y1 + screen->screenInfo ()[i].height;

	    strutX1 = newStrut.left.x;
	    strutX2 = strutX1 + newStrut.left.width;
	    strutY1 = newStrut.left.y;
	    strutY2 = strutY1 + newStrut.left.height;

	    if (strutX2 > x1 && strutX2 <= x2 &&
		strutY1 < y2 && strutY2 > y1)
	    {
		newStrut.left.x     = x1;
		newStrut.left.width = strutX2 - x1;
	    }

	    strutX1 = newStrut.right.x;
	    strutX2 = strutX1 + newStrut.right.width;
	    strutY1 = newStrut.right.y;
	    strutY2 = strutY1 + newStrut.right.height;

	    if (strutX1 > x1 && strutX1 <= x2 &&
		strutY1 < y2 && strutY2 > y1)
	    {
		newStrut.right.x     = strutX1;
		newStrut.right.width = x2 - strutX1;
	    }

	    strutX1 = newStrut.top.x;
	    strutX2 = strutX1 + newStrut.top.width;
	    strutY1 = newStrut.top.y;
	    strutY2 = strutY1 + newStrut.top.height;

	    if (strutX1 < x2 && strutX2 > x1 &&
		strutY2 > y1 && strutY2 <= y2)
	    {
		newStrut.top.y      = y1;
		newStrut.top.height = strutY2 - y1;
	    }

	    strutX1 = newStrut.bottom.x;
	    strutX2 = strutX1 + newStrut.bottom.width;
	    strutY1 = newStrut.bottom.y;
	    strutY2 = strutY1 + newStrut.bottom.height;

	    if (strutX1 < x2 && strutX2 > x1 &&
		strutY1 > y1 && strutY1 <= y2)
	    {
		newStrut.bottom.y      = strutY1;
		newStrut.bottom.height = y2 - strutY1;
	    }
	}
    }

    if (hasOld != hasNew ||
	(hasNew && hasOld &&
	 memcmp (&newStrut, &oldStrut, sizeof (CompStruts))))
    {
	if (hasNew)
	{
	    if (!priv->struts)
	    {
		priv->struts = (CompStruts *) malloc (sizeof (CompStruts));
		if (!priv->struts)
		    return false;
	    }

	    *priv->struts = newStrut;
	}
	else
	{
	    free (priv->struts);
	    priv->struts = NULL;
	}

	return true;
    }

    return false;
}

void
CompWindow::incrementDestroyReference ()
{
    priv->destroyRefCnt++;
}

void
CompWindow::destroy ()
{
    if (priv->id)
    {
	CompWindow *oldServerNext, *oldServerPrev, *oldNext, *oldPrev;
	StackDebugger *dbg = StackDebugger::Default ();

	windowNotify (CompWindowNotifyBeforeDestroy);

	/* Don't allow frame windows to block input */
	if (priv->serverFrame)
	    XUnmapWindow (screen->dpy (), priv->serverFrame);

	if (priv->wrapper)
	    XUnmapWindow (screen->dpy (), priv->wrapper);

	oldServerNext = serverNext;
	oldServerPrev = serverPrev;
	oldNext = next;
	oldPrev = prev;

	/* This is where things get tricky ... it is possible
	 * to receive a ConfigureNotify relative to a frame window
	 * for a destroyed window in case we process a ConfigureRequest
	 * for the destroyed window and then a DestroyNotify for it directly
	 * afterwards. In that case, we will receive the ConfigureNotify
	 * for the XConfigureWindow request we made relative to that frame
	 * window. Because of this, we must keep the frame window in the stack
	 * as a new toplevel window so that the ConfigureNotify will be processed
	 * properly until it too receives a DestroyNotify */

	if (priv->serverFrame)
	{
	    XWindowAttributes attrib;

	    /* It's possible that the frame window was already destroyed because
	     * the client was unreparented before it was destroyed (eg
	     * UnmapNotify before DestroyNotify). In that case the frame window
	     * is going to be an invalid window but since we haven't received
	     * a DestroyNotify for it yet, it is possible that restacking
	     * operations could occurr relative to it so we need to hold it
	     * in the stack for now. Ensure that it is marked override redirect */
	    XGetWindowAttributes (screen->dpy (), priv->serverFrame, &attrib);

	    /* Put the frame window "above" the client window
	     * in the stack */
	    PrivateWindow::createCompWindow (priv->id, priv->id, attrib, priv->serverFrame);
	}

	/* Immediately unhook the window once destroyed
	 * as the stacking order will be invalid if we don't
	 * and will continue to be invalid for the period
	 * that we keep it around in the stack. Instead, push
	 * it to another stack and keep the next and prev members
	 * in tact, letting plugins sort out where those windows
	 * might be in case they need to use them relative to
	 * other windows */

	screen->unhookWindow (this);
	screen->unhookServerWindow (this);

	/* We must immediately insert the window into the debugging
	 * stack */
	if (dbg)
	    dbg->removeServerWindow (id ());

	/* Unhooking the window will also NULL the next/prev
	 * linked list links but we don't want that so don't
	 * do that */

	next = oldNext;
	prev = oldPrev;
	serverNext = oldServerNext;
	serverPrev = oldServerPrev;

	screen->addToDestroyedWindows (this);

	/* We must set the xid of this window
	 * to zero as it no longer references
	 * a valid window */
	priv->mapNum = 0;
	priv->id = 0;
	priv->frame = 0;
	priv->serverFrame = 0;
	priv->managed    = false;
    }

    priv->destroyRefCnt--;
    if (priv->destroyRefCnt)
	return;

    if (!priv->destroyed)
    {
	if (!priv->serverFrame)
	{
	    StackDebugger *dbg = StackDebugger::Default ();

	    if (dbg)
		dbg->addDestroyedFrame (priv->serverId);
	}

	priv->destroyed = true;
	screen->incrementPendingDestroys();
    }

}

void
CompWindow::sendConfigureNotify ()
{
    XConfigureEvent xev;
    XWindowAttributes attrib;
    unsigned int      nchildren;
    Window            rootRet, parentRet = 0;
    Window            *children;

    xev.type   = ConfigureNotify;
    xev.event  = priv->id;
    xev.window = priv->id;

    /* in order to avoid race conditions we must use the current
     * server configuration */

    XGrabServer (screen->dpy ());
    XSync (screen->dpy (), false);

    if (XGetWindowAttributes (screen->dpy (), priv->id, &attrib))
    {
	xev.x	     = attrib.x;
	xev.y	     = attrib.y;
	xev.width	     = attrib.width;
	xev.height	     = attrib.height;
	xev.border_width = attrib.border_width;
	xev.above = None;

	/* Translate co-ordinates to root space */
	XTranslateCoordinates (screen->dpy (), priv->id, screen->root (), 0, 0,
			       &xev.x, &xev.y, &parentRet);

	/* We need to ensure that the stacking order is
	 * based on the current server stacking order so
	 * find the sibling to this window's frame in the
	 * server side stack and stack above that */
	XQueryTree (screen->dpy (), screen->root (), &rootRet, &parentRet, &children, &nchildren);

	if (nchildren)
	{
	    for (unsigned int i = 0; i < nchildren; i++)
	    {
		if (i + 1 == nchildren ||
		    children[i + 1] == ROOTPARENT (this))
		{
		    xev.above = children[i];
		    break;
		}
	    }
	}

	if (children)
	    XFree (children);

	if (!xev.above)
	    xev.above		  = (serverPrev) ? ROOTPARENT (serverPrev) : None;
	xev.override_redirect = priv->attrib.override_redirect;

	XSendEvent (screen->dpy (), priv->id, false,
		    StructureNotifyMask, (XEvent *) &xev);
    }

    XUngrabServer (screen->dpy ());
    XSync (screen->dpy (), false);
}

void
CompWindow::map ()
{
    windowNotify (CompWindowNotifyBeforeMap);

    /* Previously not viewable */
    if (!isViewable ())
    {
	if (priv->pendingMaps > 0)
	    priv->pendingMaps = 0;

	priv->mapNum = screen->nextMapNum();

	if (priv->struts)
	    screen->updateWorkarea ();

	if (windowClass () == InputOnly)
	    return;

	priv->unmapRefCnt = 1;

	priv->attrib.map_state = IsViewable;

	if (!overrideRedirect ())
	    screen->setWmState (NormalState, priv->id);

	priv->invisible  = priv->isInvisible ();
	priv->alive      = true;

	priv->lastPong = screen->lastPing ();

	priv->updateRegion ();
	priv->updateSize ();

	screen->updateClientList ();

	if (priv->type & CompWindowTypeDesktopMask)
	    screen->incrementDesktopWindowCount();

	if (priv->protocols & CompWindowProtocolSyncRequestMask)
	{
	    sendSyncRequest ();
	    sendConfigureNotify ();
	}

	if (!overrideRedirect ())
	{
	    /* been shaded */
	    if (priv->shaded)
	    {
		priv->shaded = false;
		priv->updateFrameWindow ();
	    }
	}
    }

    windowNotify (CompWindowNotifyMap);
    /* Send a resizeNotify to plugins to indicate
     * that the map is complete */
    resizeNotify (0, 0, 0, 0);
}

void
CompWindow::incrementUnmapReference ()
{
    priv->unmapRefCnt++;
}

void
CompWindow::unmap ()
{
    if (priv->mapNum)
	priv->mapNum = 0;

    windowNotify (CompWindowNotifyBeforeUnmap);

    /* Even though we're still keeping the backing
     * pixmap of the window around, it's safe to
     * unmap the frame window since there's no use
     * for it at this point anyways and it just blocks
     * input, but keep it around if shaded */

    XUnmapWindow (screen->dpy (), priv->wrapper);

    if (!priv->shaded)
	XUnmapWindow (screen->dpy (), priv->serverFrame);

    priv->unmapRefCnt--;
    if (priv->unmapRefCnt > 0)
	return;

    if (priv->unmanaging)
    {
	XWindowChanges xwc = XWINDOWCHANGES_INIT;
	unsigned int   xwcm;
	int		   gravity = priv->sizeHints.win_gravity;

	/* revert gravity adjustment made at MapNotify time */
	xwc.x	= priv->serverGeometry.x ();
	xwc.y	= priv->serverGeometry.y ();
	xwc.width   = 0;
	xwc.height  = 0;

	xwcm = priv->adjustConfigureRequestForGravity (&xwc,
						       CWX | CWY,
						       gravity,
						       -1);
	if (xwcm)
	    configureXWindow (xwcm, &xwc);

	priv->unmanaging = false;
    }

    if (priv->serverFrame && !priv->shaded)
	priv->unreparent ();

    if (priv->struts)
	screen->updateWorkarea ();

    if (priv->attrib.map_state != IsViewable)
	return;

    if (priv->type == CompWindowTypeDesktopMask)
	screen->decrementDesktopWindowCount();

    priv->attrib.map_state = IsUnmapped;
    priv->invisible = true;

    if (priv->shaded)
	priv->updateFrameWindow ();

    screen->updateClientList ();

    windowNotify (CompWindowNotifyUnmap);
}

void
PrivateWindow::withdraw ()
{
    if (!attrib.override_redirect)
	screen->setWmState (WithdrawnState, id);

    placed     = false;
    unmanaging = managed;
    managed    = false;
}

bool
PrivateWindow::restack (Window aboveId)
{
    if (aboveId && (aboveId == id || aboveId == serverFrame))
	// Don't try to raise a window above itself
	return false;
    else if (window->prev)
    {
	if (aboveId && (aboveId == window->prev->id () ||
			aboveId == window->prev->priv->frame))
	    return false;
    }
    else if (aboveId == None && !window->next)
	return false;

    if (aboveId && !screen->findTopLevelWindow (aboveId, true))
    {
        return false;
    }

    screen->unhookWindow (window);
    screen->insertWindow (window, aboveId);

    /* Update the server side window list for
     * override redirect windows immediately
     * since there is no opportunity to update
     * the server side list when we configure them
     * since we never get a ConfigureRequest for those */
    if (attrib.override_redirect != 0)
    {
	StackDebugger *dbg = StackDebugger::Default ();

	screen->unhookServerWindow (window);
	screen->insertServerWindow (window, aboveId);

	if (dbg)
	    dbg->overrideRedirectRestack (window->id (), aboveId);
    }

    screen->updateClientList ();

    window->windowNotify (CompWindowNotifyRestack);

    return true;
}

bool
CompWindow::resize (XWindowAttributes attr)
{
    return resize (Geometry (attr.x, attr.y, attr.width, attr.height,
			     attr.border_width));
}

bool
CompWindow::resize (int          x,
		    int          y,
		    int          width,
		    int          height,
		    int          border)
{
    return resize (Geometry (x, y, width, height, border));
}

bool
PrivateWindow::resize (const CompWindow::Geometry &gm)
{
    /* Input extents are now the last thing sent
     * from the server. This might not work in some
     * cases though because setWindowFrameExtents may
     * be called more than once in an event processing
     * cycle so every set of input extents up until the
     * last one will be invalid. The real solution
     * here is to handle ConfigureNotify events on
     * frame windows and client windows separately */

    priv->input = priv->serverInput;

    if (priv->geometry.width ()   != gm.width ()  ||
	priv->geometry.height ()  != gm.height () ||
	priv->geometry.border ()  != gm.border ())
    {
	int dx, dy, dwidth, dheight;

	dx      = gm.x () - priv->geometry.x ();
	dy      = gm.y () - priv->geometry.y ();
	dwidth  = gm.width () - priv->geometry.width ();
	dheight = gm.height () - priv->geometry.height ();

	priv->geometry.set (gm.x (), gm.y (),
			    gm.width (), gm.height (),
			    gm.border ());

	priv->invisible = priv->isInvisible ();

	if (priv->attrib.override_redirect)
	{
	    priv->serverGeometry = priv->geometry;
	    priv->serverFrameGeometry = priv->frameGeometry;

	    if (priv->mapNum)
		priv->updateRegion ();

	    window->resizeNotify (dx, dy, dwidth, dheight);
	}
    }
    else if (priv->geometry.x () != gm.x () || priv->geometry.y () != gm.y ())
    {
	move (gm.x () - priv->geometry.x (),
	      gm.y () - priv->geometry.y (), true);
    }

    return true;
}

bool
PrivateWindow::resize (const XWindowAttributes &attr)
{
    return resize (CompWindow::Geometry (attr.x, attr.y, attr.width, attr.height,
					 attr.border_width));
}

bool
PrivateWindow::resize (int          x,
		       int          y,
		       int          width,
		       int          height,
		       int          border)
{
    return resize (CompWindow::Geometry (x, y, width, height, border));
}

bool
CompWindow::resize (CompWindow::Geometry gm)
{
    XWindowChanges xwc = XWINDOWCHANGES_INIT;
    unsigned int   valueMask = CWX | CWY | CWWidth | CWHeight | CWBorderWidth;

    xwc.x = gm.x ();
    xwc.y = gm.y ();
    xwc.width = gm.width ();
    xwc.height = gm.height ();
    xwc.border_width = gm.border ();

    configureXWindow (valueMask, &xwc);

    return true;
}

static void
syncValueIncrement (XSyncValue *value)
{
    XSyncValue one;
    int	       overflow;

    XSyncIntToValue (&one, 1);
    XSyncValueAdd (value, *value, one, &overflow);
}

bool
PrivateWindow::initializeSyncCounter ()
{
    XSyncAlarmAttributes values;
    Atom		 actual;
    int			 result, format;
    unsigned long	 n, left;
    unsigned char	 *data;

    if (syncCounter)
	return syncAlarm != None;

    if (!(protocols & CompWindowProtocolSyncRequestMask))
	return false;

    result = XGetWindowProperty (screen->dpy (), id,
				 Atoms::wmSyncRequestCounter,
				 0L, 1L, false, XA_CARDINAL, &actual, &format,
				 &n, &left, &data);

    if (result == Success && n && data)
    {
	unsigned long *counter = (unsigned long *) data;

	syncCounter = *counter;

	XFree (data);

	XSyncIntsToValue (&syncValue, (unsigned int) rand (), 0);
	XSyncSetCounter (screen->dpy (),
			 syncCounter,
			 syncValue);

	syncValueIncrement (&syncValue);

	values.events = true;

	values.trigger.counter    = syncCounter;
	values.trigger.wait_value = syncValue;

	values.trigger.value_type = XSyncAbsolute;
	values.trigger.test_type  = XSyncPositiveComparison;

	XSyncIntToValue (&values.delta, 1);

	values.events = true;

	CompScreenImpl::checkForError (screen->dpy ());

	/* Note that by default, the alarm increments the trigger value
	 * when it fires until the condition (counter.value < trigger.value)
	 * is false again.
	 */
	syncAlarm = XSyncCreateAlarm (screen->dpy (),
				      XSyncCACounter   |
				      XSyncCAValue     |
				      XSyncCAValueType |
				      XSyncCATestType  |
				      XSyncCADelta     |
				      XSyncCAEvents,
				      &values);

	if (CompScreenImpl::checkForError (screen->dpy ()))
	    return true;

	XSyncDestroyAlarm (screen->dpy (), syncAlarm);
	syncAlarm = None;
    }
    else if (result == Success && data)
    {
	XFree (data);
    }

    return false;
}

void
CompWindow::sendSyncRequest ()
{
    XClientMessageEvent xev;

    if (priv->syncWait)
	return;

    if (!priv->initializeSyncCounter ())
	return;

    xev.type	     = ClientMessage;
    xev.window	     = priv->id;
    xev.message_type = Atoms::wmProtocols;
    xev.format	     = 32;
    xev.data.l[0]    = Atoms::wmSyncRequest;
    xev.data.l[1]    = CurrentTime;
    xev.data.l[2]    = XSyncValueLow32 (priv->syncValue);
    xev.data.l[3]    = XSyncValueHigh32 (priv->syncValue);
    xev.data.l[4]    = 0;

    syncValueIncrement (&priv->syncValue);

    XSendEvent (screen->dpy (), priv->id, false, 0, (XEvent *) &xev);

    priv->syncWait     = true;
    priv->syncGeometry = priv->serverGeometry;

    if (!priv->syncWaitTimer.active ())
	priv->syncWaitTimer.start ();
}

void
PrivateWindow::configure (XConfigureEvent *ce)
{
    unsigned int valueMask = 0;

    if (priv->frame)
	return;

    /* remove configure event from pending configures */
    if (priv->geometry.x () != ce->x)
	valueMask |= CWX;

    if (priv->geometry.y () != ce->y)
	valueMask |= CWY;

    if (priv->geometry.width () != ce->width)
	valueMask |= CWWidth;

    if (priv->geometry.height () != ce->height)
	valueMask |= CWHeight;

    if (priv->geometry.border () != ce->border_width)
	valueMask |= CWBorderWidth;

    if (window->prev)
    {
	if (ROOTPARENT (window->prev) != ce->above)
	    valueMask |= CWSibling | CWStackMode;
    }
    else
    {
	if (ce->above != 0)
	    valueMask |= CWSibling | CWStackMode;
    }

    priv->attrib.override_redirect = ce->override_redirect;

    priv->frameGeometry.set (ce->x, ce->y, ce->width,
			     ce->height, ce->border_width);

    if (priv->syncWait)
	priv->syncGeometry.set (ce->x, ce->y, ce->width, ce->height,
				ce->border_width);
    else
    {
	resize (ce->x, ce->y, ce->width, ce->height, ce->border_width);
    }

    if (ce->event == screen->root ())
	priv->restack (ce->above);
}

void
PrivateWindow::configureFrame (XConfigureEvent *ce)
{
    int x, y, width, height;
    CompWindow	     *above;
    unsigned int     valueMask = 0;

    if (!priv->frame)
	return;

    /* remove configure event from pending configures */
    if (priv->frameGeometry.x () != ce->x)
	valueMask |= CWX;

    if (priv->frameGeometry.y () != ce->y)
	valueMask |= CWY;

    if (priv->frameGeometry.width () != ce->width)
	valueMask |= CWWidth;

    if (priv->frameGeometry.height () != ce->height)
	valueMask |= CWHeight;

    if (priv->frameGeometry.border () != ce->border_width)
	valueMask |= CWBorderWidth;

    if (window->prev)
    {
	if (ROOTPARENT (window->prev) != ce->above)
	    valueMask |= CWSibling | CWStackMode;
    }
    else
    {
	if (ce->above != 0)
	    valueMask |= CWSibling | CWStackMode;
    }

    if (!pendingConfigures.match ((XEvent *) ce))
    {
	compLogMessage ("core", CompLogLevelWarn, "unhandled ConfigureNotify on 0x%x!", serverFrame);
	compLogMessage ("core", CompLogLevelWarn, "this should never happen. you should "\
						  "probably file a bug about this.");
#ifdef DEBUG
	abort ();
#else
	pendingConfigures.clear ();
#endif
    }

    /* subtract the input extents last sent to the
     * server to calculate the client size and then
     * re-sync the input extents and extents last
     * sent to server on resize () */

    x      = ce->x + priv->serverInput.left;
    y      = ce->y + priv->serverInput.top;
    width  = ce->width - priv->serverGeometry.border () * 2 - priv->serverInput.left - priv->serverInput.right;

    /* Don't use the server side frame geometry
     * to determine the geometry of shaded
     * windows since we didn't resize them
     * on configureXWindow */
    if (priv->shaded)
	height = priv->serverGeometry.heightIncBorders () - priv->serverInput.top - priv->serverInput.bottom;
    else
	height = ce->height + priv->serverGeometry.border () * 2 - priv->serverInput.top - priv->serverInput.bottom;

    /* set the frame geometry */
    priv->frameGeometry.set (ce->x, ce->y, ce->width, ce->height, ce->border_width);


    if (priv->syncWait)
	priv->syncGeometry.set (x, y, width, height, ce->border_width);
    else
	resize (x, y, width, height, ce->border_width);

    if (priv->restack (ce->above))
	priv->updatePassiveButtonGrabs ();

    above = screen->findWindow (ce->above);

    if (above)
	above->priv->updatePassiveButtonGrabs ();
}

void
PrivateWindow::circulate (XCirculateEvent *ce)
{
    Window newAboveId;

    if (ce->place == PlaceOnTop)
    {
	CompWindow *newAbove = screen->getTopWindow ();
	newAboveId = newAbove ? newAbove->id () : None;
    }
    else
	newAboveId = 0;

    priv->restack (newAboveId);
}

void
CompWindow::move (int  dx,
		  int  dy,
		  bool immediate)
{
    if (dx || dy)
    {
	XWindowChanges xwc = XWINDOWCHANGES_INIT;
	unsigned int   valueMask = CWX | CWY;

	xwc.x = priv->serverGeometry.x () + dx;
	xwc.y = priv->serverGeometry.y () + dy;

	priv->nextMoveImmediate = immediate;

	configureXWindow (valueMask, &xwc);
    }
}

void
PrivateWindow::move (int dx,
		     int dy,
		     bool immediate)
{
    if (dx || dy)
    {
	priv->geometry.setX (priv->geometry.x () + dx);
	priv->geometry.setY (priv->geometry.y () + dy);
	priv->frameGeometry.setX (priv->frameGeometry.x () + dx);
	priv->frameGeometry.setY (priv->frameGeometry.y () + dy);

	if (priv->attrib.override_redirect)
	{
	    priv->serverGeometry = priv->geometry;
	    priv->serverFrameGeometry = priv->frameGeometry;
	    priv->region.translate (dx, dy);
	    priv->inputRegion.translate (dx, dy);
	    if (!priv->frameRegion.isEmpty ())
		priv->frameRegion.translate (dx, dy);

	    priv->invisible = priv->isInvisible ();

	    window->moveNotify (dx, dy, true);
	}
    }
}

bool
compiz::X11::PendingEventQueue::pending ()
{
    return !mEvents.empty ();
}

void
compiz::X11::PendingEventQueue::add (PendingEvent::Ptr p)
{
    compLogMessage ("core", CompLogLevelDebug, "pending request:");
    p->dump ();

    mEvents.push_back (p);
}

bool
compiz::X11::PendingEventQueue::removeIfMatching (const PendingEvent::Ptr &p, XEvent *event)
{
    if (p->match (event))
    {
	compLogMessage ("core", CompLogLevelDebug, "received event:");
	p->dump ();
	return true;
    }

    return false;
}

void
compiz::X11::PendingEvent::dump ()
{
    compLogMessage ("core", CompLogLevelDebug, "- event serial: %i", mSerial);
    compLogMessage ("core", CompLogLevelDebug,  "- event window 0x%x", mWindow);
}

void
compiz::X11::PendingConfigureEvent::dump ()
{
    compiz::X11::PendingEvent::dump ();

    compLogMessage ("core", CompLogLevelDebug,  "- x: %i y: %i width: %i height: %i "\
						 "border: %i, sibling: 0x%x",
						 mXwc.x, mXwc.y, mXwc.width, mXwc.height, mXwc.border_width, mXwc.sibling);
}

bool
compiz::X11::PendingEventQueue::match (XEvent *event)
{
    unsigned int lastSize = mEvents.size ();

    mEvents.erase (std::remove_if (mEvents.begin (), mEvents.end (),
				   boost::bind (&compiz::X11::PendingEventQueue::removeIfMatching, this, _1, event)), mEvents.end ());

    return lastSize != mEvents.size ();
}

bool
compiz::X11::PendingEventQueue::forEachIf (boost::function<bool (compiz::X11::PendingEvent::Ptr)> f)
{
    foreach (compiz::X11::PendingEvent::Ptr p, mEvents)
    {
	if (f (p))
	    return true;
    }

    return false;
}

void
compiz::X11::PendingEventQueue::dump ()
{
    foreach (compiz::X11::PendingEvent::Ptr p, mEvents)
	p->dump ();
}

compiz::X11::PendingEventQueue::PendingEventQueue (Display *d)
{
    /* mClearCheckTimeout.setTimes (0, 0)
     *
     * XXX: For whatever reason, calling setTimes (0, 0) here causes
     * the destructor of the timer object to be called twice later on
     * in execution and the stack gets smashed. This could be a
     * compiler bug, but requires further investigation */
}

compiz::X11::PendingEventQueue::~PendingEventQueue ()
{
}

Window
compiz::X11::PendingEvent::getEventWindow (XEvent *event)
{
    return event->xany.window;
}

bool
compiz::X11::PendingEvent::match (XEvent *event)
{
    if (event->xany.serial != mSerial)
	return false;
    if (getEventWindow (event)!= mWindow)
	return false;

    return true;
}

compiz::X11::PendingEvent::PendingEvent (Display *d, Window w) :
    mSerial (XNextRequest (d)),
    mWindow (w)
{
}

compiz::X11::PendingEvent::~PendingEvent ()
{
}

Window
compiz::X11::PendingConfigureEvent::getEventWindow (XEvent *event)
{
    return event->xconfigure.window;
}

bool
compiz::X11::PendingConfigureEvent::matchVM (unsigned int valueMask)
{
    unsigned int result = mValueMask != 0 ? valueMask & mValueMask : 1;

    return result != 0;
}

bool
compiz::X11::PendingConfigureEvent::matchRequest (XWindowChanges &xwc, unsigned int valueMask)
{
    if (matchVM (valueMask))
    {
	if (valueMask & CWX)
	    if (xwc.x != mXwc.x)
		return false;

	if (valueMask & CWY)
	    if (xwc.y != mXwc.y)
		return false;

	if (valueMask & CWWidth)
	    if (xwc.width != mXwc.width)
		return false;

	if (valueMask & CWHeight)
	    if (xwc.height != mXwc.height)
		return false;

	if (valueMask & CWBorderWidth)
	    if (xwc.border_width != mXwc.border_width)
		return false;

	if (valueMask & (CWStackMode | CWSibling))
	    if (xwc.sibling != mXwc.sibling)
		return false;

	return true;
    }

    return false;
}

bool
compiz::X11::PendingConfigureEvent::match (XEvent *event)
{
    XConfigureEvent *ce = (XConfigureEvent *) event;
    bool matched = true;

    if (!compiz::X11::PendingEvent::match (event))
	return false;

    XWindowChanges xwc = XWINDOWCHANGES_INIT;

    xwc.x = ce->x;
    xwc.y = ce->y;
    xwc.width = ce->width;
    xwc.height = ce->height;
    xwc.border_width = ce->border_width;
    xwc.sibling = ce->above;

    matched = matchRequest (xwc, mValueMask);

    /* Remove events from the queue
     * even if they didn't match what
     * we expected them to be, but still
     * complain about it */
    if (!matched)
    {
	compLogMessage ("core", CompLogLevelWarn, "no exact match for ConfigureNotify on 0x%x!", mWindow);
	compLogMessage ("core", CompLogLevelWarn, "expected the following changes:");
	if (mValueMask & CWX)
	    compLogMessage ("core", CompLogLevelWarn, "x: %i", mXwc.x);
	if (mValueMask & CWY)
	    compLogMessage ("core", CompLogLevelWarn, "y: %i", mXwc.y);
	if (mValueMask & CWWidth)
	    compLogMessage ("core", CompLogLevelWarn, "width: %i", mXwc.width);
	if (mValueMask & CWHeight)
	    compLogMessage ("core", CompLogLevelWarn, "height: %i", mXwc.height);
	if (mValueMask & CWBorderWidth)
	    compLogMessage ("core", CompLogLevelWarn, "border: %i", mXwc.border_width);
	if (mValueMask & (CWStackMode | CWSibling))
	    compLogMessage ("core", CompLogLevelWarn, "sibling: 0x%x", mXwc.sibling);

	compLogMessage ("core", CompLogLevelWarn, "instead got:");
	compLogMessage ("core", CompLogLevelWarn, "x: %i", ce->x);
	compLogMessage ("core", CompLogLevelWarn, "y: %i", ce->y);
	compLogMessage ("core", CompLogLevelWarn, "width: %i", ce->width);
	compLogMessage ("core", CompLogLevelWarn, "height: %i", ce->height);
	compLogMessage ("core", CompLogLevelWarn, "above: %i", ce->above);
	compLogMessage ("core", CompLogLevelWarn, "this should never happen. you should "\
						  "probably file a bug about this.");
    }

    return true;
}

compiz::X11::PendingConfigureEvent::PendingConfigureEvent (Display *d,
							   Window   w,
							   unsigned int valueMask,
							   XWindowChanges *xwc) :
    compiz::X11::PendingEvent::PendingEvent (d, w),
    mValueMask (valueMask),
    mXwc (*xwc)
{
}

compiz::X11::PendingConfigureEvent::~PendingConfigureEvent ()
{
}

void
CompWindow::syncPosition ()
{
}

bool
CompWindow::focus ()
{
    WRAPABLE_HND_FUNCTN_RETURN (bool, focus)

    if (overrideRedirect ())
	return false;

    if (!priv->managed || priv->unmanaging)
	return false;

    if (!onCurrentDesktop ())
	return false;

    if (priv->destroyed)
	return false;

    if (!priv->shaded && (priv->state & CompWindowStateHiddenMask))
	return false;

    if (priv->serverGeometry.x () + priv->serverGeometry.width ()  <= 0	||
	priv->serverGeometry.y () + priv->serverGeometry.height () <= 0	||
	priv->serverGeometry.x () >= (int) screen->width ()||
	priv->serverGeometry.y () >= (int) screen->height ())
	return false;

    return true;
}

bool
CompWindow::place (CompPoint &pos)
{
    WRAPABLE_HND_FUNCTN_RETURN (bool, place, pos)
    return false;
}

void
CompWindow::validateResizeRequest (unsigned int   &mask,
				   XWindowChanges *xwc,
				   unsigned int   source)
{
    WRAPABLE_HND_FUNCTN (validateResizeRequest, mask, xwc, source)

    if (!(priv->type & (CompWindowTypeDockMask    |
		     CompWindowTypeFullscreenMask |
		     CompWindowTypeUnknownMask)))
    {
	if (mask & CWY)
	{
	    int min, max;

	    min = screen->workArea ().y () + priv->input.top;
	    max = screen->workArea ().bottom ();

	    if (priv->state & CompWindowStateStickyMask &&
	    	 (xwc->y < min || xwc->y > max))
	    {
		xwc->y = priv->serverGeometry.y ();
	    }
	    else
	    {
		min -= screen->vp ().y () * screen->height ();
		max += (screen->vpSize ().height () - screen->vp ().y () - 1) *
			screen->height ();

		if (xwc->y < min)
		    xwc->y = min;
		else if (xwc->y > max)
		    xwc->y = max;
	    }
	}

	if (mask & CWX)
	{
	    int min, max;

	    min = screen->workArea ().x () + priv->input.left;
	    max = screen->workArea ().right ();

	    if (priv->state & CompWindowStateStickyMask &&
		(xwc->x < min || xwc->x > max))
	    {
		xwc->x = priv->serverGeometry.x ();
	    }
	    else
	    {
		min -= screen->vp ().x () * screen->width ();
		max += (screen->vpSize ().width () - screen->vp ().x () - 1) *
			screen->width ();

		if (xwc->x < min)
		    xwc->x = min;
		else if (xwc->x > max)
		    xwc->x = max;
	    }
	}
    }
}

void
CompWindow::resizeNotify (int dx,
			  int dy,
			  int dwidth,
			  int dheight)
    WRAPABLE_HND_FUNCTN (resizeNotify, dx, dy, dwidth, dheight)

void
CompWindow::moveNotify (int  dx,
			int  dy,
			bool immediate)
    WRAPABLE_HND_FUNCTN (moveNotify, dx, dy, immediate)

void
CompWindow::windowNotify (CompWindowNotify n)
    WRAPABLE_HND_FUNCTN (windowNotify, n)

void
CompWindow::grabNotify (int	     x,
			int	     y,
			unsigned int state,
			unsigned int mask)
{
    WRAPABLE_HND_FUNCTN (grabNotify, x, y, state, mask)
    priv->grabbed = true;
}

void
CompWindow::ungrabNotify ()
{
    WRAPABLE_HND_FUNCTN (ungrabNotify)
    priv->grabbed = false;
}

void
CompWindow::stateChangeNotify (unsigned int lastState)
{
    WRAPABLE_HND_FUNCTN (stateChangeNotify, lastState);

    /* if being made sticky */
    if (!(lastState & CompWindowStateStickyMask) &&
	(priv->state & CompWindowStateStickyMask))
    {
	CompPoint vp;   /* index of the window's vp */

	/* Find which viewport the window falls in,
	   and check if it's the current viewport */
	vp = defaultViewport ();
	if (screen->vp () != vp)
	{
	    unsigned int valueMask = CWX | CWY;
	    XWindowChanges xwc = XWINDOWCHANGES_INIT;

	    xwc.x = serverGeometry ().x () +  (screen->vp ().x () - vp.x ()) * screen->width ();
	    xwc.y = serverGeometry ().y () +  (screen->vp ().y () - vp.y ()) * screen->height ();

	    configureXWindow (valueMask, &xwc);
	}
    }
}


bool
PrivateWindow::isGroupTransient (Window clientLeader)
{
    if (!clientLeader)
	return false;

    if (transientFor == None || transientFor == screen->root ())
    {
	if (type & (CompWindowTypeUtilMask    |
		    CompWindowTypeToolbarMask |
		    CompWindowTypeMenuMask    |
		    CompWindowTypeDialogMask  |
		    CompWindowTypeModalDialogMask))
	{
	    if (this->clientLeader == clientLeader)
		return true;
	}
    }

    return false;
}

CompWindow *
PrivateWindow::getModalTransient ()
{
    CompWindow *w, *modalTransient;

    modalTransient = window;

    for (w = screen->windows ().back (); w; w = w->prev)
    {
	if (w == modalTransient || w->priv->mapNum == 0)
	    continue;

	if (w->priv->transientFor == modalTransient->priv->id)
	{
	    if (w->priv->state & CompWindowStateModalMask)
	    {
		modalTransient = w;
		w = screen->windows ().back ();
	    }
	}
    }

    if (modalTransient == window)
    {
	/* don't look for group transients with modal state if current window
	   has modal state */
	if (state & CompWindowStateModalMask)
	    return NULL;

	for (w = screen->windows ().back (); w; w = w->prev)
	{
	    if (w == modalTransient || w->priv->mapNum == 0)
		continue;

	    if (isAncestorTo (modalTransient, w))
		continue;

	    if (w->priv->isGroupTransient (modalTransient->priv->clientLeader))
	    {
		if (w->priv->state & CompWindowStateModalMask)
		{
		    modalTransient = w;
		    w = w->priv->getModalTransient ();
		    if (w)
			modalTransient = w;

		    break;
		}
	    }
	}
    }

    if (modalTransient == window)
	modalTransient = NULL;

    return modalTransient;
}

void
CompWindow::moveInputFocusTo ()
{
    CompScreen  *s = screen;
    CompWindow  *modalTransient;

    modalTransient = priv->getModalTransient ();
    if (modalTransient)
	return modalTransient->moveInputFocusTo ();

    /* If the window is still hidden but not shaded
     * it probably meant that a plugin overloaded
     * CompWindow::focus to allow the focus to go
     * to this window, so only move the input focus
     * to the frame if the window is shaded */
    if (shaded ())
    {
	XSetInputFocus (s->dpy (), priv->serverFrame,
			RevertToPointerRoot, CurrentTime);
	XChangeProperty (s->dpy (), s->root (), Atoms::winActive,
			 XA_WINDOW, 32, PropModeReplace,
			 (unsigned char *) &priv->id, 1);

	screen->setNextActiveWindow(priv->serverFrame);
    }
    else
    {
	bool setFocus = false;

	if (priv->inputHint)
	{
	    XSetInputFocus (s->dpy (), priv->id, RevertToPointerRoot,
			    CurrentTime);
	    setFocus = true;
	}

	if (priv->protocols & CompWindowProtocolTakeFocusMask)
	{
	    XEvent ev;

	    ev.type		    = ClientMessage;
	    ev.xclient.window	    = priv->id;
	    ev.xclient.message_type = Atoms::wmProtocols;
	    ev.xclient.format	    = 32;
	    ev.xclient.data.l[0]    = Atoms::wmTakeFocus;
	    ev.xclient.data.l[1]    = s->getCurrentTime ();
	    ev.xclient.data.l[2]    = 0;
	    ev.xclient.data.l[3]    = 0;
	    ev.xclient.data.l[4]    = 0;

	    XSendEvent (s->dpy (), priv->id, false, NoEventMask, &ev);

	    setFocus = true;
	}

	if (setFocus)
	    screen->setNextActiveWindow(priv->id);

	if (!setFocus && !modalTransient)
	{
	    CompWindow *ancestor;

	    /* move input to closest ancestor */
	    for (ancestor = s->windows ().front (); ancestor;
		 ancestor = ancestor->next)
	    {
		if (PrivateWindow::isAncestorTo (this, ancestor))
		{
		    ancestor->moveInputFocusTo ();
		    break;
		}
	    }
	}
    }
}

void
CompWindow::moveInputFocusToOtherWindow ()
{
    if (priv->id == screen->activeWindow () ||
	priv->id == screen->getNextActiveWindow())
    {
	CompWindow *ancestor;
	CompWindow *nextActive = screen->findWindow (screen->getNextActiveWindow());

        /* Window pending focus */
	if (priv->id != screen->getNextActiveWindow() &&
	    nextActive &&
	    nextActive->focus ())
	{
	    nextActive->moveInputFocusTo ();
	}
	else if (priv->transientFor && priv->transientFor != screen->root ())
	{
	    ancestor = screen->findWindow (priv->transientFor);
	    if (ancestor &&
		ancestor->focus () &&
		!(ancestor->priv->type & (CompWindowTypeDesktopMask |
					  CompWindowTypeDockMask)))
	    {
		ancestor->moveInputFocusTo ();
	    }
	    else
		screen->focusDefaultWindow ();
	}
	else if (priv->type & (CompWindowTypeDialogMask |
			       CompWindowTypeModalDialogMask))
	{
	    CompWindow *a, *focus = NULL;

	    for (a = screen->windows ().back (); a; a = a->prev)
	    {
		if (a->priv->clientLeader == priv->clientLeader)
		{
		    if (a->focus ())
		    {
			if (focus)
			{
			    if (a->priv->type & (CompWindowTypeNormalMask |
						 CompWindowTypeDialogMask |
						 CompWindowTypeModalDialogMask))
			    {
				if (priv->compareWindowActiveness (focus, a) < 0)
				    focus = a;
			    }
			}
			else
			    focus = a;
		    }
		}
	    }

	    if (focus && !(focus->priv->type & (CompWindowTypeDesktopMask |
						CompWindowTypeDockMask)))
	    {
		focus->moveInputFocusTo ();
	    }
	    else
		screen->focusDefaultWindow ();
	}
	else
	    screen->focusDefaultWindow ();
    }
}


bool
PrivateWindow::stackLayerCheck (CompWindow *w,
				Window	    clientLeader,
				CompWindow *below)
{
    if (isAncestorTo (w, below))
	return true;

    if (isAncestorTo (below, w))
	return false;

    if (clientLeader && below->priv->clientLeader == clientLeader)
	if (below->priv->isGroupTransient (clientLeader))
	    return false;

    if (w->priv->state & CompWindowStateAboveMask)
    {
	return true;
    }
    else if (w->priv->state & CompWindowStateBelowMask)
    {
	if (below->priv->state & CompWindowStateBelowMask)
	    return true;
    }
    else if (!(below->priv->state & CompWindowStateAboveMask))
    {
	return true;
    }

    return false;
}

bool
PrivateWindow::avoidStackingRelativeTo (CompWindow *w)
{
    if (w->overrideRedirect ())
	return true;

    if (w->destroyed ())
	return true;

    if (!w->priv->shaded && !w->priv->pendingMaps)
    {
	if (!w->isViewable () || !w->isMapped ())
	    return true;
    }

    return false;
}

/* goes through the stack, top-down until we find a window we should
   stack above, normal windows can be stacked above fullscreen windows
   (and fullscreen windows over others in their layer) if aboveFs is true. */
CompWindow *
PrivateWindow::findSiblingBelow (CompWindow *w,
				 bool       aboveFs)
{
    CompWindow   *below;
    CompWindow   *t = screen->findWindow (w->transientFor ());
    Window	 clientLeader = w->priv->clientLeader;
    unsigned int type = w->priv->type;
    unsigned int belowMask;

    if (aboveFs)
	belowMask = CompWindowTypeDockMask;
    else
	belowMask = CompWindowTypeDockMask | CompWindowTypeFullscreenMask;

    /* normal stacking of fullscreen windows with below state */
    if ((type & CompWindowTypeFullscreenMask) &&
	(w->priv->state & CompWindowStateBelowMask))
	type = CompWindowTypeNormalMask;

    while (t && type != CompWindowTypeDockMask)
    {
	/* dock stacking of transients for docks */
	if (t->type () & CompWindowTypeDockMask)
	    type = CompWindowTypeDockMask;

	t = screen->findWindow (t->transientFor ());
    }

    if (w->priv->transientFor || w->priv->isGroupTransient (clientLeader))
	clientLeader = None;

    for (below = screen->serverWindows ().back (); below;
	 below = below->serverPrev)
    {
	if (below == w || avoidStackingRelativeTo (below))
	    continue;

	/* always above desktop windows */
	if (below->priv->type & CompWindowTypeDesktopMask)
	    return below;

	switch (type) {
	case CompWindowTypeDesktopMask:
	    /* desktop window layer */
	    break;
	case CompWindowTypeFullscreenMask:
	    if (aboveFs)
		return below;
	    /* otherwise fall-through */
	case CompWindowTypeDockMask:
	    /* fullscreen and dock layer */
	    if (below->priv->type & (CompWindowTypeFullscreenMask |
			       CompWindowTypeDockMask))
	    {
		if (stackLayerCheck (w, clientLeader, below))
		    return below;
	    }
	    else
	    {
		return below;
	    }
	    break;
	default:
	{
	    bool allowedRelativeToLayer = !(below->priv->type & belowMask);

	    if (aboveFs && below->priv->type & CompWindowTypeFullscreenMask)
		if (!below->focus ())
		    break;

	    t = screen->findWindow (below->transientFor ());

	    while (t && allowedRelativeToLayer)
	    {
		/* dock stacking of transients for docks */
		allowedRelativeToLayer = !(t->priv->type & belowMask);

		t = screen->findWindow (t->transientFor ());
	    }

	    /* fullscreen and normal layer */
	    if (allowedRelativeToLayer)
	    {
		if (stackLayerCheck (w, clientLeader, below))
		    return below;
	    }
	    break;
	}
	}
    }

    return NULL;
}

/* goes through the stack, top-down and returns the lowest window we
   can stack above. */
CompWindow *
PrivateWindow::findLowestSiblingBelow (CompWindow *w)
{
    CompWindow   *below, *lowest = screen->serverWindows ().back ();
    CompWindow   *t = screen->findWindow (w->transientFor ());
    Window	 clientLeader = w->priv->clientLeader;
    unsigned int type = w->priv->type;

    /* normal stacking fullscreen windows with below state */
    if ((type & CompWindowTypeFullscreenMask) &&
	(w->priv->state & CompWindowStateBelowMask))
	type = CompWindowTypeNormalMask;

    while (t && type != CompWindowTypeDockMask)
    {
	/* dock stacking of transients for docks */
	if (t->type () & CompWindowTypeDockMask)
	    type = CompWindowTypeDockMask;

	t = screen->findWindow (t->transientFor ());
    }


    if (w->priv->transientFor || w->priv->isGroupTransient (clientLeader))
	clientLeader = None;

    for (below = screen->serverWindows ().back (); below;
	 below = below->serverPrev)
    {
	if (below == w || avoidStackingRelativeTo (below))
	    continue;

	/* always above desktop windows */
	if (below->priv->type & CompWindowTypeDesktopMask)
	    return below;

	switch (type) {
	case CompWindowTypeDesktopMask:
	    /* desktop window layer - desktop windows always should be
	       stacked at the bottom; no other window should be below them */
	    return NULL;
	    break;
	case CompWindowTypeFullscreenMask:
	case CompWindowTypeDockMask:
	    /* fullscreen and dock layer */
	    if (below->priv->type & (CompWindowTypeFullscreenMask |
			       CompWindowTypeDockMask))
	    {
		if (!stackLayerCheck (below, clientLeader, w))
		    return lowest;
	    }
	    else
	    {
		return lowest;
	    }
	    break;
	default:
	{
	    bool allowedRelativeToLayer = !(below->priv->type & CompWindowTypeDockMask);

	    t = screen->findWindow (below->transientFor ());

	    while (t && allowedRelativeToLayer)
	    {
		/* dock stacking of transients for docks */
		allowedRelativeToLayer = !(t->priv->type & CompWindowTypeDockMask);

		t = screen->findWindow (t->transientFor ());
	    }

	    /* fullscreen and normal layer */
	    if (allowedRelativeToLayer)
	    {
		if (!stackLayerCheck (below, clientLeader, w))
		    return lowest;
	    }
	    break;
	}
	}

	lowest = below;
    }

    return lowest;
}

bool
PrivateWindow::validSiblingBelow (CompWindow *w,
				  CompWindow *sibling)
{
    CompWindow   *t = screen->findWindow (w->transientFor ());
    Window	 clientLeader = w->priv->clientLeader;
    unsigned int type = w->priv->type;

    /* normal stacking fullscreen windows with below state */
    if ((type & CompWindowTypeFullscreenMask) &&
	(w->priv->state & CompWindowStateBelowMask))
	type = CompWindowTypeNormalMask;

    while (t && type != CompWindowTypeDockMask)
    {
	/* dock stacking of transients for docks */
	if (t->type () & CompWindowTypeDockMask)
	    type = CompWindowTypeDockMask;

	t = screen->findWindow (t->transientFor ());
    }


    if (w->priv->transientFor || w->priv->isGroupTransient (clientLeader))
	clientLeader = None;

    if (sibling == w || avoidStackingRelativeTo (sibling))
	return false;

    /* always above desktop windows */
    if (sibling->priv->type & CompWindowTypeDesktopMask)
	return true;

    switch (type) {
    case CompWindowTypeDesktopMask:
	/* desktop window layer */
	break;
    case CompWindowTypeFullscreenMask:
    case CompWindowTypeDockMask:
	/* fullscreen and dock layer */
	if (sibling->priv->type & (CompWindowTypeFullscreenMask |
			     CompWindowTypeDockMask))
	{
	    if (stackLayerCheck (w, clientLeader, sibling))
		return true;
	}
	else
	{
	    return true;
	}
	break;
    default:
    {
	bool allowedRelativeToLayer = !(sibling->priv->type & CompWindowTypeDockMask);

	t = screen->findWindow (sibling->transientFor ());

	while (t && allowedRelativeToLayer)
	{
	    /* dock stacking of transients for docks */
	    allowedRelativeToLayer = !(t->priv->type & CompWindowTypeDockMask);

	    t = screen->findWindow (t->transientFor ());
	}

	/* fullscreen and normal layer */
	if (allowedRelativeToLayer)
	{
	    if (stackLayerCheck (w, clientLeader, sibling))
		return true;
	}
	break;
    }
    }

    return false;
}

void
PrivateWindow::saveGeometry (int mask)
{
    int m = mask & ~saveMask;

    /* only save geometry if window has been placed */
    if (!placed)
	return;

    if (m & CWX)
	saveWc.x = serverGeometry.x ();

    if (m & CWY)
	saveWc.y = serverGeometry.y ();

    if (m & CWWidth)
	saveWc.width = serverGeometry.width ();

    if (m & CWHeight)
	saveWc.height = serverGeometry.height ();

    if (m & CWBorderWidth)
	saveWc.border_width = serverGeometry.border ();

    saveMask |= m;
}

int
PrivateWindow::restoreGeometry (XWindowChanges *xwc,
				int            mask)
{
    int m = mask & saveMask;

    if (m & CWX)
	xwc->x = saveWc.x;

    if (m & CWY)
	xwc->y = saveWc.y;

    if (m & CWWidth)
    {
	xwc->width = saveWc.width;

	/* This is not perfect but it works OK for now. If the saved width is
	   the same as the current width then make it a little be smaller so
	   the user can see that it changed and it also makes sure that
	   windowResizeNotify is called and plugins are notified. */
	if (xwc->width == (int) serverGeometry.width ())
	{
	    xwc->width -= 10;
	    if (m & CWX)
		xwc->x += 5;
	}
    }

    if (m & CWHeight)
    {
	xwc->height = saveWc.height;

	/* As above, if the saved height is the same as the current height
	   then make it a little be smaller. */
	if (xwc->height == (int) serverGeometry.height ())
	{
	    xwc->height -= 10;
	    if (m & CWY)
		xwc->y += 5;
	}
    }

    if (m & CWBorderWidth)
	xwc->border_width = saveWc.border_width;

    saveMask &= ~mask;

    return m;
}

static bool isPendingRestack (compiz::X11::PendingEvent::Ptr p)
{
    compiz::X11::PendingConfigureEvent::Ptr pc = boost::shared_static_cast <compiz::X11::PendingConfigureEvent> (p);

    return pc->matchVM (CWStackMode | CWSibling);
}

static bool isExistingRequest (compiz::X11::PendingEvent::Ptr p, XWindowChanges &xwc, unsigned int valueMask)
{
    compiz::X11::PendingConfigureEvent::Ptr pc = boost::shared_static_cast <compiz::X11::PendingConfigureEvent> (p);

    return pc->matchRequest (xwc, valueMask);
}

void
PrivateWindow::reconfigureXWindow (unsigned int   valueMask,
				   XWindowChanges *xwc)
{
    unsigned int frameValueMask = 0;

    if (id == screen->root ())
    {
	compLogMessage ("core", CompLogLevelWarn, "attempted to reconfigure root window");
	return;
    }

    /* Remove redundant bits */

    xwc->x = valueMask & CWX ? xwc->x : serverGeometry.x ();
    xwc->y = valueMask & CWY ? xwc->y : serverGeometry.y ();
    xwc->width = valueMask & CWWidth ? xwc->width : serverGeometry.width ();
    xwc->height = valueMask & CWHeight ? xwc->height : serverGeometry.height ();
    xwc->border_width = valueMask & CWBorderWidth ? xwc->border_width : serverGeometry.border ();

    /* Don't allow anything that might generate a BadValue */
    if (valueMask & CWWidth && !xwc->width)
    {
	compLogMessage ("core", CompLogLevelWarn, "Attempted to set < 1 width on a window");
	xwc->width = 1;
    }

    if (valueMask & CWHeight && !xwc->height)
    {
	compLogMessage ("core", CompLogLevelWarn, "Attempted to set < 1 height on a window");
	xwc->height = 1;
    }

    int dx = valueMask & CWX ? xwc->x - serverGeometry.x () : 0;
    int dy = valueMask & CWY ? xwc->y - serverGeometry.y () : 0;
    int dwidth = valueMask & CWWidth ? xwc->width - serverGeometry.width () : 0;
    int dheight = valueMask & CWHeight ? xwc->height - serverGeometry.height () : 0;

    /* FIXME: This is a total fallacy for the reparenting case
     * at least since the client doesn't actually move here, it only
     * moves within the frame */
    if (valueMask & CWX && serverGeometry.x () == xwc->x)
	valueMask &= ~(CWX);

    if (valueMask & CWY && serverGeometry.y () == xwc->y)
	valueMask &= ~(CWY);

    if (valueMask & CWWidth && serverGeometry.width () == xwc->width)
	valueMask &= ~(CWWidth);

    if (valueMask & CWHeight && serverGeometry.height () == xwc->height)
	valueMask &= ~(CWHeight);

    if (valueMask & CWBorderWidth && serverGeometry.border () == xwc->border_width)
	valueMask &= ~(CWBorderWidth);

    if (valueMask & CWSibling && window->serverPrev)
    {
	/* check if the sibling is also pending a restack,
	 * if not, then setting this bit is useless */
	if (ROOTPARENT (window->serverPrev) == xwc->sibling)
	{
	    bool matchingRequest = priv->pendingConfigures.forEachIf (boost::bind (isExistingRequest, _1, *xwc, valueMask));
	    bool restackPending = window->serverPrev->priv->pendingConfigures.forEachIf (boost::bind (isPendingRestack, _1));
	    bool remove = matchingRequest;

	    if (!remove)
		remove = !restackPending;

	    if (remove)
		valueMask &= ~(CWSibling | CWStackMode);
	}
    }

    if (valueMask & CWBorderWidth)
	serverGeometry.setBorder (xwc->border_width);

    if (valueMask & CWX)
	serverGeometry.setX (xwc->x);

    if (valueMask & CWY)
	serverGeometry.setY (xwc->y);

    if (valueMask & CWWidth)
	serverGeometry.setWidth (xwc->width);

    if (valueMask & CWHeight)
	serverGeometry.setHeight (xwc->height);

    /* Update the server side window list on raise, lower and restack functions.
     * This function should only recieve stack_mode == Above
     * but warn incase something else does get through, to make the cause
     * of any potential misbehaviour obvious. */
    if (valueMask & (CWSibling | CWStackMode))
    {
	if (xwc->stack_mode == Above)
	{
	    if (xwc->sibling)
	    {
		screen->unhookServerWindow (window);
		screen->insertServerWindow (window, xwc->sibling);
	    }
	}
	else
	    compLogMessage ("core", CompLogLevelWarn, "restack_mode not Above");
    }

    frameValueMask = CWX | CWY | CWWidth | CWHeight | (valueMask & (CWStackMode | CWSibling));

    if (serverFrameGeometry.x () == xwc->x - serverGeometry.border () - serverInput.left)
	frameValueMask &= ~(CWX);

    if (serverFrameGeometry.y () == xwc->y - serverGeometry.border () - serverInput.top)
	frameValueMask &= ~(CWY);

   if (serverFrameGeometry.width () == xwc->width + serverGeometry.border () * 2
				      + serverInput.left + serverInput.right)
	frameValueMask &= ~(CWWidth);

    /* shaded windows are not allowed to have their frame window
     * height changed (but are allowed to have their client height
     * changed */

    if (shaded)
    {
	if (serverFrameGeometry.height () == serverGeometry.border () * 2
	    + serverInput.top + serverInput.bottom)
	    frameValueMask &= ~(CWHeight);
    }
    else
    {
	if (serverFrameGeometry.height () == xwc->height + serverGeometry.border () * 2
	    + serverInput.top + serverInput.bottom)
	    frameValueMask &= ~(CWHeight);
    }


    if (valueMask & CWStackMode &&
        ((xwc->stack_mode != TopIf) && (xwc->stack_mode != BottomIf) && (xwc->stack_mode != Opposite) &&
	 (xwc->stack_mode != Above) && (xwc->stack_mode != Below)))
    {
	compLogMessage ("core", CompLogLevelWarn, "Invalid stack mode %i", xwc->stack_mode);
	valueMask &= ~(CWStackMode | CWSibling);
    }

    /* Don't allow anything that might cause a BadMatch error */

    if (valueMask & CWSibling && !(valueMask & CWStackMode))
    {
	compLogMessage ("core", CompLogLevelWarn, "Didn't specify a CWStackMode for CWSibling");
	valueMask &= ~CWSibling;
    }

    if (valueMask & CWSibling && xwc->sibling == (serverFrame ? serverFrame : id))
    {
	compLogMessage ("core", CompLogLevelWarn, "Can't restack a window relative to itself");
	valueMask &= ~CWSibling;
    }

    if (valueMask & CWBorderWidth && attrib.c_class == InputOnly)
    {
	compLogMessage ("core", CompLogLevelWarn, "Cannot set border_width of an input_only window");
	valueMask &= ~CWBorderWidth;
    }

    if (valueMask & CWSibling)
    {
	CompWindow *sibling = screen->findTopLevelWindow (xwc->sibling);

	if (!sibling)
	{
	    compLogMessage ("core", CompLogLevelWarn, "Attempted to restack relative to 0x%x which is "\
			    "not a child of the root window or a window compiz owns", static_cast <unsigned int> (xwc->sibling));
	    valueMask &= ~(CWSibling | CWStackMode);
	}
	else if (sibling->frame () && xwc->sibling != sibling->frame ())
	{
	    compLogMessage ("core", CompLogLevelWarn, "Attempted to restack relative to 0x%x which is "\
			    "not a child of the root window", static_cast <unsigned int> (xwc->sibling));
	    valueMask &= ~(CWSibling | CWStackMode);
	}
    }

    /* Can't set the border width of frame windows */
    frameValueMask &= ~(CWBorderWidth);

    if (frameValueMask & CWX)
	serverFrameGeometry.setX (xwc->x - serverGeometry.border () - serverInput.left);

    if (frameValueMask & CWY)
	serverFrameGeometry.setY (xwc->y -serverGeometry.border () - serverInput.top);

    if (frameValueMask & CWWidth)
	serverFrameGeometry.setWidth (xwc->width + serverGeometry.border () * 2
				      + serverInput.left + serverInput.right);

    if (shaded)
    {
	if (frameValueMask & CWHeight)
	    serverFrameGeometry.setHeight (serverGeometry.border () * 2
					   + serverInput.top + serverInput.bottom);
    }
    else
    {
	if (frameValueMask & CWHeight)
	    serverFrameGeometry.setHeight (xwc->height + serverGeometry.border () * 2
					   + serverInput.top + serverInput.bottom);
    }

    if (serverFrame)
    {
	if (frameValueMask)
	{
	    XWindowChanges wc = *xwc;

	    wc.x      = serverFrameGeometry.x ();
	    wc.y      = serverFrameGeometry.y ();
	    wc.width  = serverFrameGeometry.width ();
	    wc.height = serverFrameGeometry.height ();

	    compiz::X11::PendingEvent::Ptr pc =
		    boost::shared_static_cast<compiz::X11::PendingEvent> (compiz::X11::PendingConfigureEvent::Ptr (
									      new compiz::X11::PendingConfigureEvent (
										  screen->dpy (), priv->serverFrame, frameValueMask, &wc)));

	    pendingConfigures.add (pc);

	    XConfigureWindow (screen->dpy (), serverFrame, frameValueMask, &wc);
	}

	valueMask &= ~(CWSibling | CWStackMode);

	/* If the frame has changed position (eg, serverInput.top
	 * or serverInput.left have changed) then we also need to
	 * update the client and wrapper position */
	if (!(valueMask & CWX))
	    valueMask |= frameValueMask & CWX;
	if (!(valueMask & CWY))
	    valueMask |= frameValueMask & CWY;

	if (valueMask)
	{
	    xwc->x = serverInput.left;
	    xwc->y = serverInput.top;
	    XConfigureWindow (screen->dpy (), wrapper, valueMask, xwc);

	    xwc->x = 0;
	    xwc->y = 0;
	}

	window->sendConfigureNotify ();
    }

    if (valueMask)
	XConfigureWindow (screen->dpy (), id, valueMask, xwc);

    /* When updating plugins we care about
     * the absolute position */
    if (dx)
	valueMask |= CWX;
    if (dy)
	valueMask |= CWY;
    if (dwidth)
	valueMask |= CWWidth;
    if (dheight)
	valueMask |= CWHeight;

    if (!attrib.override_redirect)
    {
	if (valueMask & (CWWidth | CWHeight))
	{
	    updateRegion ();
	    window->resizeNotify (dx, dy, dwidth, dheight);
	}
	else if (valueMask & (CWX | CWY))
	{
	    region.translate (dx, dy);
	    inputRegion.translate (dx, dy);
	    if (!frameRegion.isEmpty ())
		frameRegion.translate (dx, dy);

	    if (dx || dy)
	    {
		window->moveNotify (dx, dy, priv->nextMoveImmediate);
		priv->nextMoveImmediate = true;
	    }
	}
    }
}

bool
PrivateWindow::stackDocks (CompWindow     *w,
                           CompWindowList &updateList,
                           XWindowChanges *xwc,
                           unsigned int   *mask)
{
    CompWindow *firstFullscreenWindow = NULL;
    CompWindow *belowDocks = NULL;

    foreach (CompWindow *dw, screen->serverWindows ())
    {
        /* fullscreen window found */
        if (firstFullscreenWindow)
        {
	    /* If there is another toplevel window above the fullscreen one
	     * then we need to stack above that */
	    if ((dw->priv->managed && !dw->priv->unmanaging) &&
		!(dw->priv->state & CompWindowStateHiddenMask) &&
                !PrivateWindow::isAncestorTo (w, dw) &&
                !(dw->type () & (CompWindowTypeFullscreenMask |
                                 CompWindowTypeDockMask)) &&
		!dw->overrideRedirect () &&
		dw->isViewable ())
            {
                belowDocks = dw;
            }
        }
        else if (dw->type () & CompWindowTypeFullscreenMask)
        {
	    /* First fullscreen window found when checking up the stack
	     * now go back down to find a suitable candidate client
	     * window to put the docks above */
            firstFullscreenWindow = dw;
	    for (CompWindow *dww = dw->serverPrev; dww; dww = dww->serverPrev)
            {
		if ((dw->priv->managed && !dw->priv->unmanaging) &&
		    !(dw->priv->state & CompWindowStateHiddenMask) &&
		    !(dww->type () & (CompWindowTypeFullscreenMask |
                                      CompWindowTypeDockMask)) &&
		    !dww->overrideRedirect () &&
		    dww->isViewable ())
                {
                    belowDocks = dww;
                    break;
                }
            }
        }
    }

    if (belowDocks)
    {
        *mask = CWSibling | CWStackMode;
        xwc->sibling = ROOTPARENT (belowDocks);

        /* Collect all dock windows first */
	foreach (CompWindow *dw, screen->serverWindows ())
            if (dw->priv->type & CompWindowTypeDockMask)
                updateList.push_front (dw);

        return true;
    }

    return false;
}

bool
PrivateWindow::stackTransients (CompWindow	*w,
				CompWindow	*avoid,
				XWindowChanges *xwc,
				CompWindowList &updateList)
{
    CompWindow *t;
    Window     clientLeader = w->priv->clientLeader;

    if (w->priv->transientFor || w->priv->isGroupTransient (clientLeader))
	clientLeader = None;

    for (t = screen->serverWindows ().back (); t; t = t->serverPrev)
    {
	if (t == w || t == avoid)
	    continue;

	if (t->priv->transientFor == w->priv->id ||
	    t->priv->isGroupTransient (clientLeader))
	{
	    if (!stackTransients (t, avoid, xwc, updateList))
		return false;

	    if (xwc->sibling == t->priv->id ||
		(t->priv->serverFrame && xwc->sibling == t->priv->serverFrame))
		return false;

	    if (t->priv->mapNum || t->priv->pendingMaps)
		updateList.push_back (t);
	}
    }

    return true;
}

void
PrivateWindow::stackAncestors (CompWindow     *w,
			       XWindowChanges *xwc,
			       CompWindowList &updateList)
{
    CompWindow *transient = NULL;

    if (w->priv->transientFor)
	transient = screen->findWindow (w->priv->transientFor);

    if (transient                           &&
	xwc->sibling != transient->priv->id &&
	(!transient->priv->serverFrame || xwc->sibling != transient->priv->serverFrame))
    {
	CompWindow *ancestor;

	ancestor = screen->findWindow (w->priv->transientFor);
	if (ancestor)
	{
	    if (!stackTransients (ancestor, w, xwc, updateList))
		return;

	    if (ancestor->priv->type & CompWindowTypeDesktopMask)
		return;

	    if (ancestor->priv->type & CompWindowTypeDockMask)
		if (!(w->priv->type & CompWindowTypeDockMask))
		    return;

	    if (ancestor->priv->mapNum || ancestor->priv->pendingMaps)
		updateList.push_back (ancestor);

	    stackAncestors (ancestor, xwc, updateList);
	}
    }
    else if (w->priv->isGroupTransient (w->priv->clientLeader))
    {
	CompWindow *a;

	for (a = screen->serverWindows ().back (); a; a = a->serverPrev)
	{
	    if (a->priv->clientLeader == w->priv->clientLeader &&
		a->priv->transientFor == None		       &&
		!a->priv->isGroupTransient (w->priv->clientLeader))
	    {
		if (xwc->sibling == a->priv->id ||
		    (a->priv->serverFrame && xwc->sibling == a->priv->serverFrame))
		    break;

		if (!stackTransients (a, w, xwc, updateList))
		    break;

		if (a->priv->type & CompWindowTypeDesktopMask)
		    continue;

		if (a->priv->type & CompWindowTypeDockMask)
		    if (!(w->priv->type & CompWindowTypeDockMask))
			break;

		if (a->priv->mapNum || a->priv->pendingMaps)
		    updateList.push_back (a);
	    }
	}
    }
}

void
CompWindow::configureXWindow (unsigned int valueMask,
			      XWindowChanges *xwc)
{
    if (priv->managed && (valueMask & (CWSibling | CWStackMode)))
    {
	CompWindowList transients;
	CompWindowList ancestors;
	CompWindowList docks;

	/* Since the window list is being reordered in reconfigureXWindow
	   the list of windows which need to be restacked must be stored
	   first. The windows are stacked in the opposite order than they
	   were previously stacked, in order that they are above xwc->sibling
	   so that when compiz gets the ConfigureNotify event it doesn't
	   have to restack all the windows again. */

	/* transient children above */
	if (PrivateWindow::stackTransients (this, NULL, xwc, transients))
	{
	    /* ancestors, siblings and sibling transients below */
	    PrivateWindow::stackAncestors (this, xwc, ancestors);

	    for (CompWindowList::reverse_iterator w = ancestors.rbegin ();
		 w != ancestors.rend (); w++)
	    {
		(*w)->priv->reconfigureXWindow (CWSibling | CWStackMode, xwc);
		xwc->sibling = ROOTPARENT (*w);
	    }

	    this->priv->reconfigureXWindow (valueMask, xwc);
	    xwc->sibling = ROOTPARENT (this);

	    for (CompWindowList::reverse_iterator w = transients.rbegin ();
		 w != transients.rend (); w++)
	    {
		(*w)->priv->reconfigureXWindow (CWSibling | CWStackMode, xwc);
		xwc->sibling = ROOTPARENT (*w);
	    }

	    if (PrivateWindow::stackDocks (this, docks, xwc, &valueMask))
	    {
		Window sibling = xwc->sibling;
		xwc->stack_mode = Above;

                /* Then update the dock windows */
                foreach (CompWindow *dw, docks)
                {
                    xwc->sibling = sibling;
                    dw->priv->reconfigureXWindow (valueMask, xwc);
                }
            }
	}
    }
    else if (priv->id)
    {
	priv->reconfigureXWindow (valueMask, xwc);
    }
}

int
PrivateWindow::addWindowSizeChanges (XWindowChanges       *xwc,
				     CompWindow::Geometry old)
{
    CompRect  workArea;
    int	      mask = 0;
    int	      x, y;
    CompOutput *output;
    CompPoint viewport;

    screen->viewportForGeometry (old, viewport);

    x = (viewport.x () - screen->vp ().x ()) * screen->width ();
    y = (viewport.y () - screen->vp ().y ()) * screen->height ();

    /* Try to select and output device that the window is on first
     * and make sure if we are fullscreening or maximizing that the
     * window is actually able to fit on this output ... otherwise
     * we're going to have to use another output device which sucks
     * but at least the user will be able to see all of the window */
    output   = &screen->outputDevs ().at (screen->outputDeviceForGeometry (old));

    if (state & CompWindowStateFullscreenMask ||
	state & CompWindowStateMaximizedHorzMask)
    {
	int width = (mask & CWWidth) ? xwc->width : old.width ();
	int height = (mask & CWHeight) ? xwc->height : old.height ();

	window->constrainNewWindowSize (width, height, &width, &height);

	if (width > output->width ())
	{
	    int        distance = std::numeric_limits <int>::max ();
	    CompOutput *selected = output;
	    /* That's no good ... try and find the closest output device to this one
	     * which has a large enough size */
	    foreach (CompOutput &o, screen->outputDevs ())
	    {
		if (o.workArea ().width () > width)
		{
		    int tDistance = sqrt (pow (abs (o.x () - output->x ()), 2) +
					  pow (abs (o.y () - output->y ()), 2));

		    if (tDistance < distance)
		    {
			selected = &o;
			tDistance = distance;
		    }
		}
	    }

	    output = selected;
	}
    }

    if (state & CompWindowStateFullscreenMask ||
	state & CompWindowStateMaximizedVertMask)
    {
	int width = (mask & CWWidth) ? xwc->width : old.width ();
	int height = (mask & CWHeight) ? xwc->height : old.height ();

	window->constrainNewWindowSize (width, height, &width, &height);

	if (height > output->height ())
	{
	    int        distance = std::numeric_limits <int>::max ();
	    CompOutput *selected = output;
	    /* That's no good ... try and find the closest output device to this one
	     * which has a large enough size */
	    foreach (CompOutput &o, screen->outputDevs ())
	    {
		if (o.workArea ().height () > height)
		{
		    int tDistance = sqrt (pow (abs (o.x () - output->x ()), 2) +
					  pow (abs (o.y () - output->y ()), 2));

		    if (tDistance < distance)
		    {
			selected = &o;
			tDistance = distance;
		    }
		}
	    }

	    output = selected;
	}
    }

    workArea = output->workArea ();

    if (type & CompWindowTypeFullscreenMask)
    {
	saveGeometry (CWX | CWY | CWWidth | CWHeight | CWBorderWidth);

	if (fullscreenMonitorsSet)
	{
	    xwc->x      = x + fullscreenMonitorRect.x ();
	    xwc->y      = y + fullscreenMonitorRect.y ();
	    xwc->width  = fullscreenMonitorRect.width ();
	    xwc->height = fullscreenMonitorRect.height ();
	}
	else
	{
	    xwc->x      = x + output->x ();
	    xwc->y      = y + output->y ();
	    xwc->width  = output->width ();
	    xwc->height = output->height ();
	}

	xwc->border_width = 0;

	mask |= CWX | CWY | CWWidth | CWHeight | CWBorderWidth;
    }
    else
    {
	mask |= restoreGeometry (xwc, CWBorderWidth);
	if (state & CompWindowStateMaximizedVertMask)
	{
	    saveGeometry (CWY | CWHeight);

	    xwc->height = workArea.height () - border.top -
			  border.bottom - old.border () * 2;

	    mask |= CWHeight;
	}
	else
	{
	    mask |= restoreGeometry (xwc, CWY | CWHeight);
	}

	if (state & CompWindowStateMaximizedHorzMask)
	{
	    saveGeometry (CWX | CWWidth);

	    xwc->width = workArea.width () - border.left -
			 border.right - old.border () * 2;

	    mask |= CWWidth;
	}
	else
	{
	    mask |= restoreGeometry (xwc, CWX | CWWidth);
	}

	/* constrain window width if smaller than minimum width */
	if (!(mask & CWWidth) && (int) old.width () < sizeHints.min_width)
	{
	    xwc->width = sizeHints.min_width;
	    mask |= CWWidth;
	}

	/* constrain window width if greater than maximum width */
	if (!(mask & CWWidth) && (int) old.width () > sizeHints.max_width)
	{
	    xwc->width = sizeHints.max_width;
	    mask |= CWWidth;
	}

	/* constrain window height if smaller than minimum height */
	if (!(mask & CWHeight) && (int) old.height () < sizeHints.min_height)
	{
	    xwc->height = sizeHints.min_height;
	    mask |= CWHeight;
	}

	/* constrain window height if greater than maximum height */
	if (!(mask & CWHeight) && (int) old.height () > sizeHints.max_height)
	{
	    xwc->height = sizeHints.max_height;
	    mask |= CWHeight;
	}

	if (mask & (CWWidth | CWHeight))
	{
	    int width, height, max;

	    width  = (mask & CWWidth)  ? xwc->width  : old.width ();
	    height = (mask & CWHeight) ? xwc->height : old.height ();

	    xwc->width  = old.width ();
	    xwc->height = old.height ();

	    window->constrainNewWindowSize (width, height, &width, &height);

	    if (width != (int) old.width ())
	    {
		mask |= CWWidth;
		xwc->width = width;
	    }
	    else
		mask &= ~CWWidth;

	    if (height != (int) old.height ())
	    {
		mask |= CWHeight;
		xwc->height = height;
	    }
	    else
		mask &= ~CWHeight;

	    if (state & CompWindowStateMaximizedVertMask)
	    {
		/* If the window is still offscreen, then we need to constrain it
		 * by the gravity value (so that the corner that the gravity specifies
		 * is 'anchored' to that edge of the workarea) */

		xwc->y = y + workArea.y () + border.top;
		mask |= CWY;

		switch (priv->sizeHints.win_gravity)
		{
		    case SouthWestGravity:
		    case SouthEastGravity:
		    case SouthGravity:
			/* Shift the window so that the bottom meets the top of the bottom */
			height = xwc->height + old.border () * 2;

			max = y + workArea.bottom ();
			if (xwc->y + xwc->height + border.bottom > max)
			{
			    xwc->y = max - height - border.bottom;
			    mask |= CWY;
			}
			break;
		    /* For EastGravity, WestGravity and CenterGravity we default to the top
		     * of the window since the user should at least be able to close it
		     * (but not for SouthGravity, SouthWestGravity and SouthEastGravity since
		     * that indicates that the application has requested positioning in that area
		     */
		    case EastGravity:
		    case WestGravity:
		    case CenterGravity:
		    case NorthWestGravity:
		    case NorthEastGravity:
		    case NorthGravity:
		    default:
			/* Shift the window so that the top meets the top of the screen */
			break;
		}
	    }

	    if (state & CompWindowStateMaximizedHorzMask)
	    {
		xwc->x = x + workArea.x () + border.left;
		mask |= CWX;

		switch (priv->sizeHints.win_gravity)
		{
		    case NorthEastGravity:
		    case SouthEastGravity:
		    case EastGravity:
			width = xwc->width + old.border () * 2;

			max = x + workArea.right ();

			if (old.x () + (int) old.width () + border.right > max)
			{
			    xwc->x = max - width - border.right;
			    mask |= CWX;
			}
			else if (old.x () + width + border.right > max)
			{
			    xwc->x = x + workArea.x () +
				     (workArea.width () - border.left - width -
				      border.right) / 2 + border.left;
			    mask |= CWX;
			}
		    /* For NorthGravity, SouthGravity and CenterGravity we default to the top
		     * of the window since the user should at least be able to close it
		     * (but not for SouthGravity, SouthWestGravity and SouthEastGravity since
		     * that indicates that the application has requested positioning in that area
		     */
		    case NorthGravity:
		    case SouthGravity:
		    case CenterGravity:
		    case NorthWestGravity:
		    case SouthWestGravity:
		    case WestGravity:
		    default:
			break;
		}
	    }
	}
    }

    if ((mask & CWX) && (xwc->x == old.x ()))
	mask &= ~CWX;

    if ((mask & CWY) && (xwc->y == old.y ()))
	mask &= ~CWY;

    if ((mask & CWWidth) && (xwc->width == (int) old.width ()))
	mask &= ~CWWidth;

    if ((mask & CWHeight) && (xwc->height == (int) old.height ()))
	mask &= ~CWHeight;

    return mask;
}

unsigned int
PrivateWindow::adjustConfigureRequestForGravity (XWindowChanges *xwc,
						 unsigned int   xwcm,
						 int            gravity,
						 int		direction)
{
    int          newX, newY;
    unsigned int mask = 0;

    newX = xwc->x;
    newY = xwc->y;

    if (xwcm & (CWX | CWWidth))
    {
	switch (gravity) {
	case NorthWestGravity:
	case WestGravity:
	case SouthWestGravity:
	    if (xwcm & CWX)
		newX += priv->border.left * direction;
	    break;

	case NorthGravity:
	case CenterGravity:
	case SouthGravity:
	    if (xwcm & CWX)
		newX -= (xwc->width / 2 - priv->border.left +
			(priv->border.left + priv->border.right) / 2) * direction;
	    else
	        newX -= (xwc->width - priv->serverGeometry.width ()) * direction;
	    break;

	case NorthEastGravity:
	case EastGravity:
	case SouthEastGravity:
	    if (xwcm & CWX)
		newX -= xwc->width + priv->border.right * direction;
	    else
		newX -= (xwc->width - priv->serverGeometry.width ()) * direction;
	    break;

	case StaticGravity:
	default:
	    break;
	}
    }

    if (xwcm & (CWY | CWHeight))
    {
	switch (gravity) {
	case NorthWestGravity:
	case NorthGravity:
	case NorthEastGravity:
	    if (xwcm & CWY)
		newY = xwc->y + priv->border.top * direction;
	    break;

	case WestGravity:
	case CenterGravity:
	case EastGravity:
	    if (xwcm & CWY)
		newY -= (xwc->height / 2 - priv->border.top +
			(priv->border.top + priv->border.bottom) / 2) * direction;
	    else
		newY -= ((xwc->height - priv->serverGeometry.height ()) / 2) * direction;
	    break;

	case SouthWestGravity:
	case SouthGravity:
	case SouthEastGravity:
	    if (xwcm & CWY)
		newY -= xwc->height + priv->border.bottom * direction;
	    else
		newY -= (xwc->height - priv->serverGeometry.height ()) * direction;
	    break;

	case StaticGravity:
	default:
	    break;
	}
    }

    if (newX != xwc->x)
    {
	xwc->x += (newX - xwc->x);
	mask |= CWX;
    }

    if (newY != xwc->y)
    {
	xwc->y += (newY - xwc->y);
	mask |= CWY;
    }

    return mask;
}

void
CompWindow::moveResize (XWindowChanges *xwc,
			unsigned int   xwcm,
			int            gravity,
			unsigned int   source)
{
    bool placed = false;

    xwcm &= (CWX | CWY | CWWidth | CWHeight | CWBorderWidth);

    if (xwcm & (CWX | CWY))
	if (priv->sizeHints.flags & (USPosition | PPosition))
	    placed = true;

    if (gravity == 0)
	gravity = priv->sizeHints.win_gravity;

    if (!(xwcm & CWX))
	xwc->x = priv->serverGeometry.x ();
    if (!(xwcm & CWY))
	xwc->y = priv->serverGeometry.y ();
    if (!(xwcm & CWWidth))
	xwc->width = priv->serverGeometry.width ();
    if (!(xwcm & CWHeight))
	xwc->height = priv->serverGeometry.height ();

    if (xwcm & (CWWidth | CWHeight))
    {
	int width, height;

	if (constrainNewWindowSize (xwc->width, xwc->height, &width, &height))
	{
	    if (width != xwc->width)
		xwcm |= CWWidth;

	    if (height != xwc->height)
		xwcm |= CWHeight;

	    xwc->width = width;
	    xwc->height = height;
	}
    }

    xwcm |= priv->adjustConfigureRequestForGravity (xwc, xwcm, gravity, 1);

    validateResizeRequest (xwcm, xwc, source);

    /* when horizontally maximized only allow width changes added by
       addWindowSizeChanges */
    if (priv->state & CompWindowStateMaximizedHorzMask)
	xwcm &= ~CWWidth;

    /* when vertically maximized only allow height changes added by
       addWindowSizeChanges */
    if (priv->state & CompWindowStateMaximizedVertMask)
	xwcm &= ~CWHeight;

    xwcm |= priv->addWindowSizeChanges (xwc, Geometry (xwc->x, xwc->y,
					xwc->width, xwc->height,
					xwc->border_width));

    /* check if the new coordinates are useful and valid (different
       to current size); if not, we have to clear them to make sure
       we send a synthetic ConfigureNotify event if all coordinates
       match the server coordinates */
    if (xwc->x == priv->serverGeometry.x ())
	xwcm &= ~CWX;

    if (xwc->y == priv->serverGeometry.y ())
	xwcm &= ~CWY;

    if (xwc->width == (int) priv->serverGeometry.width ())
	xwcm &= ~CWWidth;

    if (xwc->height == (int) priv->serverGeometry.height ())
	xwcm &= ~CWHeight;

    if (xwc->border_width == (int) priv->serverGeometry.border ())
	xwcm &= ~CWBorderWidth;

    /* update saved window coordinates - if CWX or CWY is set for fullscreen
       or maximized windows after addWindowSizeChanges, it should be pretty
       safe to assume that the saved coordinates should be updated too, e.g.
       because the window was moved to another viewport by some client */
    if ((xwcm & CWX) && (priv->saveMask & CWX))
	priv->saveWc.x += (xwc->x - priv->serverGeometry.x ());

    if ((xwcm & CWY) && (priv->saveMask & CWY))
	priv->saveWc.y += (xwc->y - priv->serverGeometry.y ());

    if (priv->mapNum && (xwcm & (CWWidth | CWHeight)))
	sendSyncRequest ();

    if (xwcm)
	configureXWindow (xwcm, xwc);
    else
    {
	/* we have to send a configure notify on ConfigureRequest events if
	   we decide not to do anything according to ICCCM 4.1.5 */
	sendConfigureNotify ();
    }

    if (placed)
	priv->placed = true;
}

void
PrivateWindow::updateSize ()
{
    XWindowChanges xwc = XWINDOWCHANGES_INIT;
    int		   mask;

    if (window->overrideRedirect () || !managed)
	return;

    mask = priv->addWindowSizeChanges (&xwc, priv->serverGeometry);
    if (mask)
    {
	if (priv->mapNum && (mask & (CWWidth | CWHeight)))
	    window->sendSyncRequest ();

	window->configureXWindow (mask, &xwc);
    }
}

int
PrivateWindow::addWindowStackChanges (XWindowChanges *xwc,
				      CompWindow     *sibling)
{
    int	mask = 0;

    if (!sibling || sibling->priv->id != id)
    {
	/* Alow requests to go on top of serverPrev
	 * if serverPrev was recently restacked */
	if (window->serverPrev)
	{
	    if (!sibling && id)
	    {
		XWindowChanges lxwc = XWINDOWCHANGES_INIT;
		unsigned int   valueMask = CWStackMode;

		lxwc.stack_mode = Below;

		if (serverFrame)
		{
		    compiz::X11::PendingEvent::Ptr pc =
			    boost::shared_static_cast<compiz::X11::PendingEvent> (compiz::X11::PendingConfigureEvent::Ptr (
										      new compiz::X11::PendingConfigureEvent (
											  screen->dpy (), serverFrame, valueMask, &lxwc)));

		    pendingConfigures.add (pc);
		}

		/* Below with no sibling puts the window at the bottom
		 * of the stack */
		XConfigureWindow (screen->dpy (), ROOTPARENT (window), valueMask, &lxwc);

		/* Update the list of windows last sent to the server */
		screen->unhookServerWindow (window);
		screen->insertServerWindow (window, 0);
	    }
	    else if (sibling)
	    {
		bool matchingRequest = priv->pendingConfigures.forEachIf (boost::bind (isExistingRequest, _1, *xwc, (CWStackMode | CWSibling)));
		bool restackPending = window->serverPrev->priv->pendingConfigures.forEachIf (boost::bind (isPendingRestack, _1));
		bool processAnyways = restackPending;

		if (matchingRequest)
		    processAnyways = false;

		if (sibling->priv->id != window->serverPrev->priv->id ||
		    processAnyways)
		{
		    mask |= CWSibling | CWStackMode;

		    xwc->stack_mode = Above;
		    xwc->sibling    = ROOTPARENT (sibling);
		}
	    }
	}
	else if (sibling)
	{
	    mask |= CWSibling | CWStackMode;

	    xwc->stack_mode = Above;
	    xwc->sibling    = ROOTPARENT (sibling);
	}
    }

    return mask;
}

void
CompWindow::raise ()
{
    XWindowChanges xwc = XWINDOWCHANGES_INIT;
    int		   mask;
    bool	   aboveFs = false;

    /* an active fullscreen window should be raised over all other
       windows in its layer */
    if (priv->type & CompWindowTypeFullscreenMask)
	if (priv->id == screen->activeWindow ())
	    aboveFs = true;

    for (CompWindow *pw = serverPrev; pw; pw = pw->serverPrev)
    {
	if (pw->priv->type & CompWindowTypeFullscreenMask)
	{
	    if (priv->id == screen->activeWindow ())
		aboveFs = true;

	    break;
	}
    }

    mask = priv->addWindowStackChanges (&xwc,
	PrivateWindow::findSiblingBelow (this, aboveFs));

    if (mask)
	configureXWindow (mask, &xwc);
}

CompWindow *
CompScreenImpl::focusTopMostWindow ()
{
    using ::compiz::private_screen::WindowManager;

    CompWindow  *focus = NULL;
    WindowManager::reverse_iterator it = windowManager.rbegin ();

    for (; it != windowManager.rend (); it++)
    {
	CompWindow *w = *it;

	if (w->type () & CompWindowTypeDockMask)
	    continue;

	if (w->focus ())
	{
	    focus = w;
	    break;
	}
    }

    if (focus)
    {
	if (focus->id () != privateScreen.orphanData.activeWindow)
	    focus->moveInputFocusTo ();
    }
    else
	XSetInputFocus (privateScreen.dpy, privateScreen.rootWindow(), RevertToPointerRoot,
			CurrentTime);
    return focus;
}


void
CompWindow::lower ()
{
    XWindowChanges xwc = XWINDOWCHANGES_INIT;
    int		   mask;

    mask = priv->addWindowStackChanges (&xwc,
	PrivateWindow::findLowestSiblingBelow (this));
    if (mask)
	configureXWindow (mask, &xwc);

    /* when lowering a window, focus the topmost window if
       the click-to-focus option is on */
    if ((screen->getCoreOptions().optionGetClickToFocus ()))
    {
	CompWindow *focusedWindow = screen->focusTopMostWindow ();

	/* if the newly focused window is a desktop window,
	   give the focus back to w */
	if (focusedWindow &&
	    focusedWindow->type () & CompWindowTypeDesktopMask)
	{
	    moveInputFocusTo ();
	}
    }
}

void
CompWindow::restackAbove (CompWindow *sibling)
{
    for (; sibling; sibling = sibling->serverNext)
	if (PrivateWindow::validSiblingBelow (this, sibling))
	    break;

    if (sibling)
    {
	XWindowChanges xwc = XWINDOWCHANGES_INIT;
	int	       mask;

	mask = priv->addWindowStackChanges (&xwc, sibling);
	if (mask)
	    configureXWindow (mask, &xwc);
    }
}

/* finds the highest window under sibling we can stack above */
CompWindow *
PrivateWindow::findValidStackSiblingBelow (CompWindow *w,
					   CompWindow *sibling)
{
    CompWindow *lowest, *last, *p;

    /* check whether we're allowed to stack under a sibling by finding
     * the above 'sibling' and checking whether or not we're allowed
     * to stack under that - if not, then there is no valid sibling
     * underneath it */

    for (p = sibling; p; p = p->serverNext)
    {
	if (!avoidStackingRelativeTo (p))
	{
	    if (!validSiblingBelow (p, w))
		return NULL;
	    break;
	}
    }

    /* get lowest sibling we're allowed to stack above */
    lowest = last = findLowestSiblingBelow (w);

    /* walk from bottom up */
    for (p = screen->serverWindows ().front (); p; p = p->serverNext)
    {
	/* stop walking when we reach the sibling we should try to stack
	   below */
	if (p == sibling)
	    return lowest;

	/* skip windows that we should avoid */
	if (w == p || avoidStackingRelativeTo (p))
	    continue;

	if (validSiblingBelow (w, p))
	{
	    /* update lowest as we find windows below sibling that we're
	       allowed to stack above. last window must be equal to the
	       lowest as we shouldn't update lowest if we passed an
	       invalid window */
	    if (last == lowest)
		lowest = p;
	}

	/* update last pointer */
	last = p;
    }

    return lowest;
}

void
CompWindow::restackBelow (CompWindow *sibling)
{
    XWindowChanges xwc = XWINDOWCHANGES_INIT;
    unsigned int   mask;

    mask = priv->addWindowStackChanges (&xwc,
	PrivateWindow::findValidStackSiblingBelow (this, sibling));

    if (mask)
	configureXWindow (mask, &xwc);
}

void
CompWindow::updateAttributes (CompStackingUpdateMode stackingMode)
{
    XWindowChanges xwc = XWINDOWCHANGES_INIT;
    int		   mask = 0;

    if (overrideRedirect () || !priv->managed)
	return;

    if (priv->state & CompWindowStateShadedMask && !priv->shaded)
    {
	windowNotify (CompWindowNotifyShade);

	priv->hide ();
    }
    else if (priv->shaded)
    {
	windowNotify (CompWindowNotifyUnshade);

	priv->show ();
    }

    if (stackingMode != CompStackingUpdateModeNone)
    {
	bool       aboveFs;
	CompWindow *sibling;

	aboveFs = (stackingMode == CompStackingUpdateModeAboveFullscreen);
	if (priv->type & CompWindowTypeFullscreenMask)
	{
	    /* put active or soon-to-be-active fullscreen windows over
	       all others in their layer */
	    if (priv->id == screen->activeWindow () ||
		priv->id == screen->getNextActiveWindow())
	    {
		aboveFs = true;
	    }
	}

	/* put windows that are just mapped, over fullscreen windows */
	if (stackingMode == CompStackingUpdateModeInitialMap)
	    aboveFs = true;

	sibling = PrivateWindow::findSiblingBelow (this, aboveFs);

	if (sibling &&
	    (stackingMode == CompStackingUpdateModeInitialMapDeniedFocus))
	{
	    CompWindow *p;

	    for (p = sibling; p; p = p->serverPrev)
		if (p->priv->id == screen->activeWindow ())
		    break;

	    /* window is above active window so we should lower it,
	     * assuing that is allowed (if, for example, our window has
	     * the "above" state, then lowering beneath the active
	     * window may not be allowed). */
	    if (p && PrivateWindow::validSiblingBelow (p, this))
	    {
		p = PrivateWindow::findValidStackSiblingBelow (this, p);

		/* if we found a valid sibling under the active window, it's
		   our new sibling we want to stack above */
		if (p)
		    sibling = p;
	    }
	}

	/* If sibling is NULL, then this window will go on the bottom
	 * of the stack */
	mask |= priv->addWindowStackChanges (&xwc, sibling);
    }

    mask |= priv->addWindowSizeChanges (&xwc, priv->serverGeometry);

    if (priv->mapNum && (mask & (CWWidth | CWHeight)))
	sendSyncRequest ();

    if (mask)
	configureXWindow (mask, &xwc);
}

void
PrivateWindow::ensureWindowVisibility ()
{
    int x1, y1, x2, y2;
    int	width = serverGeometry.widthIncBorders ();
    int	height = serverGeometry.heightIncBorders ();
    int dx = 0;
    int dy = 0;

    if (struts || attrib.override_redirect)
	return;

    if (type & (CompWindowTypeDockMask	     |
		CompWindowTypeFullscreenMask |
		CompWindowTypeUnknownMask))
	return;

    x1 = screen->workArea ().x () - screen->width () * screen->vp ().x ();
    y1 = screen->workArea ().y () - screen->height () * screen->vp ().y ();
    x2 = x1 + screen->workArea ().width () + screen->vpSize ().width () *
	 screen->width ();
    y2 = y1 + screen->workArea ().height () + screen->vpSize ().height () *
	 screen->height ();

    if (serverGeometry.x () - serverInput.left >= x2)
	dx = (x2 - 25) - serverGeometry.x ();
    else if (serverGeometry.x () + width + serverInput.right <= x1)
	dx = (x1 + 25) - (serverGeometry.x () + width);

    if (serverGeometry.y () - serverInput.top >= y2)
	dy = (y2 - 25) - serverGeometry.y ();
    else if (serverGeometry.y () + height + serverInput.bottom <= y1)
	dy = (y1 + 25) - (serverGeometry.y () + height);

    if (dx || dy)
    {
	XWindowChanges xwc = XWINDOWCHANGES_INIT;

	xwc.x = serverGeometry.x () + dx;
	xwc.y = serverGeometry.y () + dy;

	window->configureXWindow (CWX | CWY, &xwc);
    }
}

void
PrivateWindow::reveal ()
{
    if (window->minimized ())
	window->unminimize ();
}

void
PrivateWindow::revealAncestors (CompWindow *w,
				CompWindow *transient)
{
    if (isAncestorTo (transient, w))
    {
	screen->forEachWindow (boost::bind (revealAncestors, _1, w));
	w->priv->reveal ();
    }
}

void
CompWindow::activate ()
{
    WRAPABLE_HND_FUNCTN (activate)

    screen->setCurrentDesktop (priv->desktop);

    screen->forEachWindow (
	boost::bind (PrivateWindow::revealAncestors, _1, this));
    priv->reveal ();

    screen->leaveShowDesktopMode (this);

    if (priv->state & CompWindowStateHiddenMask)
    {
	priv->state &= ~CompWindowStateShadedMask;
	if (priv->shaded)
	    priv->show ();
    }

    if (priv->state & CompWindowStateHiddenMask)
	return;

    if (!onCurrentDesktop ())
	return;

    priv->ensureWindowVisibility ();
    updateAttributes (CompStackingUpdateModeAboveFullscreen);
    moveInputFocusTo ();
}


#define PVertResizeInc (1 << 0)
#define PHorzResizeInc (1 << 1)

bool
CompWindow::constrainNewWindowSize (int        width,
				    int        height,
				    int        *newWidth,
				    int        *newHeight)
{
    CompSize         size (width, height);
    long	     ignoredHints = 0;
    long	     ignoredResizeHints = 0;

    if (screen->getCoreOptions().optionGetIgnoreHintsWhenMaximized ())
    {
	ignoredHints |= PAspect;

	if (priv->state & CompWindowStateMaximizedHorzMask)
	    ignoredResizeHints |= PHorzResizeInc;

	if (priv->state & CompWindowStateMaximizedVertMask)
	    ignoredResizeHints |= PVertResizeInc;
    }

    CompSize ret = compiz::window::constrainment::constrainToHints (priv->sizeHints,
								    size,
								    ignoredHints, ignoredResizeHints);

    *newWidth = ret.width ();
    *newHeight = ret.height ();

    return ret != size;
}

void
CompWindow::hide ()
{
    priv->hidden = true;
    priv->hide ();
}

void
CompWindow::show ()
{
    priv->hidden = false;
    priv->show ();
}

void
PrivateWindow::hide ()
{
    bool onDesktop = window->onCurrentDesktop ();

    if (!managed)
	return;

    if (!window->minimized () && !inShowDesktopMode &&
	!hidden && onDesktop)
    {
	if (state & CompWindowStateShadedMask)
	{
	    shaded = true;
	}
	else
	{
	    return;
	}
    }
    else
    {
	shaded = false;

	if ((state & CompWindowStateShadedMask) && serverFrame)
	    XUnmapWindow (screen->dpy (), serverFrame);
    }

    if (!pendingMaps && !window->isViewable ())
	return;

    window->windowNotify (CompWindowNotifyHide);

    pendingUnmaps++;

    if (serverFrame && !shaded)
	XUnmapWindow (screen->dpy (), serverFrame);

    XUnmapWindow (screen->dpy (), id);

    if (window->minimized () || inShowDesktopMode || hidden || shaded)
	window->changeState (state | CompWindowStateHiddenMask);

    if (shaded && id == screen->activeWindow ())
	window->moveInputFocusTo ();
}

void
PrivateWindow::show ()
{
    bool onDesktop = window->onCurrentDesktop ();

    if (!managed)
	return;

    if (minimized || inShowDesktopMode ||
	hidden    || !onDesktop)
    {
	/* no longer hidden but not on current desktop */
	if (!minimized && !inShowDesktopMode && !hidden)
	    window->changeState (state & ~CompWindowStateHiddenMask);

	return;
    }

    /* transition from minimized to shaded */
    if (state & CompWindowStateShadedMask)
    {
	shaded = true;

	if (serverFrame)
	    XMapWindow (screen->dpy (), serverFrame);

	updateFrameWindow ();

	return;
    }

    window->windowNotify (CompWindowNotifyShow);

    pendingMaps++;

    if (serverFrame)
    {
	XMapWindow (screen->dpy (), serverFrame);
	XMapWindow (screen->dpy (), wrapper);
    }

    XMapWindow (screen->dpy (), id);

    window->changeState (state & ~CompWindowStateHiddenMask);
    screen->setWindowState (state, id);
}

void
PrivateWindow::minimizeTransients (CompWindow *w,
				   CompWindow *ancestor)
{
    if (w->priv->transientFor == ancestor->priv->id ||
	w->priv->isGroupTransient (ancestor->priv->clientLeader))
    {
	w->minimize ();
    }
}

void
CompWindow::minimize ()
{
    WRAPABLE_HND_FUNCTN (minimize);

    if (!priv->managed)
	return;

    if (!priv->minimized)
    {
	windowNotify (CompWindowNotifyMinimize);

	priv->minimized = true;

	screen->forEachWindow (
	    boost::bind (PrivateWindow::minimizeTransients, _1, this));

	priv->hide ();
    }
}

void
PrivateWindow::unminimizeTransients (CompWindow *w,
				     CompWindow *ancestor)
{
    if (w->priv->transientFor == ancestor->priv->id ||
	w->priv->isGroupTransient (ancestor->priv->clientLeader))
	w->unminimize ();
}

void
CompWindow::unminimize ()
{
    WRAPABLE_HND_FUNCTN (unminimize);
    if (priv->minimized)
    {
	windowNotify (CompWindowNotifyUnminimize);

	priv->minimized = false;

	priv->show ();

	screen->forEachWindow (
	    boost::bind (PrivateWindow::unminimizeTransients, _1, this));
    }
}

void
CompWindow::maximize (unsigned int state)
{
    if (overrideRedirect ())
	return;

    state = constrainWindowState (state, priv->actions);

    state &= MAXIMIZE_STATE;

    if (state == (priv->state & MAXIMIZE_STATE))
	return;

    state |= (priv->state & ~MAXIMIZE_STATE);

    changeState (state);
    updateAttributes (CompStackingUpdateModeNone);
}

bool
PrivateWindow::getUserTime (Time& time)
{
    Atom	  actual;
    int		  result, format;
    unsigned long n, left;
    unsigned char *data;
    bool          retval = false;

    result = XGetWindowProperty (screen->dpy (), priv->id,
				 Atoms::wmUserTime,
				 0L, 1L, False, XA_CARDINAL, &actual, &format,
				 &n, &left, &data);

    if (result == Success && data)
    {
	if (n)
	{
	    CARD32 value;

	    memcpy (&value, data, sizeof (CARD32));
	    retval = true;
	    time   = (Time) value;
	}

	XFree ((void *) data);
    }

    return retval;
}

void
PrivateWindow::setUserTime (Time time)
{
    CARD32 value = (CARD32) time;

    XChangeProperty (screen->dpy (), priv->id,
		     Atoms::wmUserTime,
		     XA_CARDINAL, 32, PropModeReplace,
		     (unsigned char *) &value, 1);
}

/*
 * Macros from metacity
 *
 * Xserver time can wraparound, thus comparing two timestamps needs to
 * take this into account.  Here's a little macro to help out.  If no
 * wraparound has occurred, this is equivalent to
 *   time1 < time2
 * Of course, the rest of the ugliness of this macro comes from
 * accounting for the fact that wraparound can occur and the fact that
 * a timestamp of 0 must be special-cased since it means older than
 * anything else.
 *
 * Note that this is NOT an equivalent for time1 <= time2; if that's
 * what you need then you'll need to swap the order of the arguments
 * and negate the result.
 */
#define XSERVER_TIME_IS_BEFORE_ASSUMING_REAL_TIMESTAMPS(time1, time2) \
    ( (( (time1) < (time2) ) &&					      \
       ( (time2) - (time1) < ((unsigned long) -1) / 2 )) ||	      \
      (( (time1) > (time2) ) &&					      \
       ( (time1) - (time2) > ((unsigned long) -1) / 2 ))	      \
	)
#define XSERVER_TIME_IS_BEFORE(time1, time2)				 \
    ( (time1) == 0 ||							 \
      (XSERVER_TIME_IS_BEFORE_ASSUMING_REAL_TIMESTAMPS (time1, time2) && \
       (time2) != 0)							 \
	)

bool
PrivateWindow::getUsageTimestamp (Time& timestamp)
{
    if (getUserTime (timestamp))
	return true;

    if (initialTimestampSet)
    {
	timestamp = initialTimestamp;
	return true;
    }

    return false;
}

bool
PrivateWindow::isWindowFocusAllowed (Time timestamp)
{
    CompScreen   *s = screen;
    CompWindow   *active;
    Time	 wUserTime, aUserTime;
    bool         gotTimestamp = false;
    int          level;
    CompPoint    dvp;

    level = s->getCoreOptions().optionGetFocusPreventionLevel ();

    if (level == CoreOptions::FocusPreventionLevelOff)
	return true;

    if (timestamp)
    {
	/* the caller passed a timestamp, so use that
	   instead of the window's user time */
	wUserTime = timestamp;
	gotTimestamp = true;
    }
    else
    {
	gotTimestamp = getUsageTimestamp (wUserTime);
    }

    /* if we got no timestamp for the window, try to get at least a timestamp
       for its transient parent, if any */
    if (!gotTimestamp && transientFor)
    {
	CompWindow *parent;

	parent = screen->findWindow (transientFor);
	if (parent)
	    gotTimestamp = parent->priv->getUsageTimestamp (wUserTime);
    }

    if (gotTimestamp && !wUserTime)
    {
	/* window explicitly requested no focus */
	return false;
    }

    /* allow focus for excluded windows */
    CompMatch &match = s->getCoreOptions().optionGetFocusPreventionMatch ();
    if (!match.evaluate (window))
	return true;

    if (level == CoreOptions::FocusPreventionLevelVeryHigh)
	return false;

    active = s->findWindow (s->activeWindow ());

    /* no active window */
    if (!active || (active->type () & CompWindowTypeDesktopMask))
	return true;

    /* active window belongs to same application */
    if (window->clientLeader () == active->clientLeader ())
       return true;

    if (level == CoreOptions::FocusPreventionLevelHigh)
       return false;

    /* not in current viewport or desktop */
    if (!window->onCurrentDesktop ())
	return false;

    dvp = window->defaultViewport ();
    if (dvp.x () != s->vp ().x () || dvp.y () != s->vp ().y ())
	return false;

    if (!gotTimestamp)
    {
	/* unsure as we have nothing to compare - allow focus in low level,
	   don't allow in normal level */
	if (level == CoreOptions::FocusPreventionLevelNormal)
	    return false;

	return true;
    }

    /* can't get user time for active window */
    if (!active->priv->getUserTime (aUserTime))
	return true;

    if (XSERVER_TIME_IS_BEFORE (wUserTime, aUserTime))
	return false;

    return true;
}

bool
PrivateWindow::allowWindowFocus (unsigned int noFocusMask,
				 Time         timestamp)
{
    bool retval;

    if (priv->id == screen->activeWindow ())
	return true;

    /* do not focus windows of these types */
    if (priv->type & noFocusMask)
	return false;

    /* window doesn't take focus */
    if (!priv->inputHint &&
	!(priv->protocols & CompWindowProtocolTakeFocusMask))
    {
	return false;
    }

    retval = priv->isWindowFocusAllowed (timestamp);
    if (!retval)
    {
	/* add demands attention state if focus was prevented */
	window->changeState (priv->state | CompWindowStateDemandsAttentionMask);
    }

    return retval;
}

CompPoint
CompWindow::defaultViewport ()
{
    CompPoint viewport;

    if (priv->serverGeometry.x () < (int) screen->width ()            &&
	priv->serverGeometry.x () + priv->serverGeometry.width () > 0 &&
	priv->serverGeometry.y () < (int) screen->height ()           &&
	priv->serverGeometry.y ()+ priv->serverGeometry.height () > 0)
    {
	return screen->vp ();
    }

    screen->viewportForGeometry (priv->serverGeometry, viewport);

    return viewport;
}

CompPoint &
CompWindow::initialViewport () const
{
    return priv->initialViewport;
}

void
PrivateWindow::readIconHint ()
{
    XImage       *image, *maskImage = NULL;
    Display      *dpy = screen->dpy ();
    unsigned int width, height, dummy;
    unsigned int i, j, k;
    int		 iDummy;
    Window       wDummy;
    CompIcon     *icon;
    CARD32       *p;

    if (!XGetGeometry (dpy, hints->icon_pixmap, &wDummy, &iDummy,
		       &iDummy, &width, &height, &dummy, &dummy))
	return;

    image = XGetImage (dpy, hints->icon_pixmap, 0, 0, width, height,
		       AllPlanes, ZPixmap);
    if (!image)
	return;

    boost::scoped_array<XColor> colors(new XColor[width * height]);
    if (!colors)
    {
	XDestroyImage (image);
	return;
    }

    k = 0;
    for (j = 0; j < height; j++)
	for (i = 0; i < width; i++)
	    colors[k++].pixel = XGetPixel (image, i, j);

    for (i = 0; i < k; i += 256)
	XQueryColors (dpy, screen->colormap(),
		      &colors[i], MIN (k - i, 256));

    XDestroyImage (image);

    icon = new CompIcon (width, height);
    if (!icon)
    {
	return;
    }

    if (hints->flags & IconMaskHint)
	maskImage = XGetImage (dpy, hints->icon_mask, 0, 0,
			       width, height, AllPlanes, ZPixmap);

    k = 0;
    p = (CARD32 *) icon->data ();

    for (j = 0; j < height; j++)
    {
	for (i = 0; i < width; i++)
	{
	    if (maskImage && !XGetPixel (maskImage, i, j))
		*p++ = 0;
	    else if (image->depth == 1)  /* white   : black */
		*p++ = colors[k].pixel ? 0xffffffff : 0xff000000;
	    else
		*p++ = 0xff000000                             | /* alpha */
		       (((colors[k].red >> 8) & 0xff) << 16)  | /* red */
		       (((colors[k].green >> 8) & 0xff) << 8) | /* green */
		       ((colors[k].blue >> 8) & 0xff);          /* blue */

	    k++;
	}
    }

    if (maskImage)
	XDestroyImage (maskImage);

    icons.push_back (icon);
}

/* returns icon with dimensions as close as possible to width and height
   but never greater. */
CompIcon *
CompWindow::getIcon (int width,
		     int height)
{
    CompIcon     *icon;
    int          wh, diff, oldDiff;
    unsigned int i;

    /* need to fetch icon property */
    if (priv->icons.size () == 0 && !priv->noIcons)
    {
	Atom	      actual;
	int	      result, format;
	unsigned long n, left;
	unsigned char *data;

	result = XGetWindowProperty (screen->dpy (), priv->id, Atoms::wmIcon,
				     0L, 65536L, false, XA_CARDINAL,
				     &actual, &format, &n, &left, &data);

	if (result == Success && data)
	{
	    CARD32        *p;
	    CARD32        alpha, red, green, blue;
	    unsigned long iw, ih;

	    for (i = 0; i + 2 < n; i += iw * ih + 2)
	    {
		unsigned long *idata = (unsigned long *) data;

		iw = idata[i];
		ih = idata[i + 1];

		/* iw * ih may be larger than the value range of unsigned
		* long, so better do some checking for extremely weird
		* icon sizes first */
		if (iw > 2048 || ih > 2048 || iw * ih + 2 > n - i)
		    break;

		if (iw && ih)
		{
		    unsigned long j;
		    icon = new CompIcon (iw, ih);
		    if (!icon)
			continue;

		    priv->icons.push_back (icon);

		    p = (CARD32 *) (icon->data ());

		    /* EWMH doesn't say if icon data is premultiplied or
		       not but most applications seem to assume data should
		       be unpremultiplied. */
		    for (j = 0; j < iw * ih; j++)
		    {
			alpha = (idata[i + j + 2] >> 24) & 0xff;
			red   = (idata[i + j + 2] >> 16) & 0xff;
			green = (idata[i + j + 2] >>  8) & 0xff;
			blue  = (idata[i + j + 2] >>  0) & 0xff;

			red   = (red   * alpha) >> 8;
			green = (green * alpha) >> 8;
			blue  = (blue  * alpha) >> 8;

			p[j] =
			    (alpha << 24) |
			    (red   << 16) |
			    (green <<  8) |
			    (blue  <<  0);
		    }
		}
	    }

	    XFree (data);
	}
	else if (priv->hints && (priv->hints->flags & IconPixmapHint))
	{
	    priv->readIconHint ();
	}

	/* don't fetch property again */
	if (priv->icons.size () == 0)
	    priv->noIcons = true;
    }

    /* no icons available for this window */
    if (priv->noIcons)
	return NULL;

    icon = NULL;
    wh   = width + height;

    for (i = 0; i < priv->icons.size (); i++)
    {
	const CompSize iconSize = *priv->icons[i];

	if ((int) iconSize.width () > width ||
	    (int) iconSize.height () > height)
	    continue;

	if (icon)
	{
	    diff    = wh - (iconSize.width () + iconSize.height ());
	    oldDiff = wh - (icon->width () + icon->height ());

	    if (diff < oldDiff)
		icon = priv->icons[i];
	}
	else
	    icon = priv->icons[i];
    }

    return icon;
}

const CompRect&
CompWindow::iconGeometry () const
{
    return priv->iconGeometry;
}

void
PrivateWindow::freeIcons ()
{
    for (unsigned int i = 0; i < priv->icons.size (); i++)
	delete priv->icons[i];

    priv->icons.resize (0);
    priv->noIcons = false;
}

int
CompWindow::outputDevice ()
{
    return screen->outputDeviceForGeometry (priv->serverGeometry);
}

bool
CompWindow::onCurrentDesktop ()
{
    if (priv->desktop == 0xffffffff ||
	priv->desktop == screen->currentDesktop ())
    {
	return true;
    }

    return false;
}

void
CompWindow::setDesktop (unsigned int desktop)
{
    if (desktop != 0xffffffff)
    {
	if (priv->type & (CompWindowTypeDesktopMask | CompWindowTypeDockMask))
	    return;

	if (desktop >= screen->nDesktop ())
	    return;
    }

    if (desktop == priv->desktop)
	return;

    priv->desktop = desktop;

    if (desktop == 0xffffffff || desktop == screen->currentDesktop ())
	priv->show ();
    else
	priv->hide ();

    screen->setWindowProp (priv->id, Atoms::winDesktop, priv->desktop);
}

/* The compareWindowActiveness function compares the two windows 'w1'
   and 'w2'. It returns an integer less than, equal to, or greater
   than zero if 'w1' is found, respectively, to activated longer time
   ago than, to be activated at the same time, or be activated more
   recently than 'w2'. */
int
PrivateWindow::compareWindowActiveness (CompWindow *w1,
					CompWindow *w2)
{
    CompActiveWindowHistory *history = screen->currentHistory ();
    int			    i;

    /* check current window history first */
    for (i = 0; i < ACTIVE_WINDOW_HISTORY_SIZE; i++)
    {
	if (history->id[i] == w1->priv->id)
	    return 1;

	if (history->id[i] == w2->priv->id)
	    return -1;

	if (!history->id[i])
	    break;
    }

    return w1->priv->activeNum - w2->priv->activeNum;
}

bool
CompWindow::onAllViewports ()
{
    if (overrideRedirect ())
	return true;

    if (!priv->managed && !isViewable ())
	return true;

    if (priv->type & (CompWindowTypeDesktopMask | CompWindowTypeDockMask))
	return true;

    if (priv->state & CompWindowStateStickyMask)
	return true;

    return false;
}

CompPoint
CompWindow::getMovementForOffset (CompPoint offset)
{
    CompScreen *s = screen;
    int         m, vWidth, vHeight;
    int         offX = offset.x (), offY = offset.y ();
    CompPoint   rv;

    vWidth = s->width () * s->vpSize ().width ();
    vHeight = s->height () * s->vpSize ().height ();

    offX %= vWidth;
    offY %= vHeight;

    /* x */
    if (s->vpSize ().width () == 1)
    {
	rv.setX (offX);
    }
    else
    {
	m = priv->serverGeometry.x () + offX;
	if (m - priv->serverInput.left < (int) s->width () - vWidth)
	    rv.setX (offX + vWidth);
	else if (m + priv->serverGeometry.width () + priv->serverInput.right > vWidth)
	    rv.setX (offX - vWidth);
	else
	    rv.setX (offX);
    }

    if (s->vpSize ().height () == 1)
    {
	rv.setY (offY);
    }
    else
    {
	m = priv->serverGeometry.y () + offY;
	if (m - priv->serverInput.top < (int) s->height () - vHeight)
	    rv.setY (offY + vHeight);
	else if (m + priv->serverGeometry.height () + priv->serverInput.bottom > vHeight)
	    rv.setY (offY - vHeight);
	else
	    rv.setY (offY);
    }

    return rv;
}

void
WindowInterface::getOutputExtents (CompWindowExtents& output)
    WRAPABLE_DEF (getOutputExtents, output)

void
WindowInterface::getAllowedActions (unsigned int &setActions,
				    unsigned int &clearActions)
    WRAPABLE_DEF (getAllowedActions, setActions, clearActions)

bool
WindowInterface::focus ()
    WRAPABLE_DEF (focus)

void
WindowInterface::activate ()
    WRAPABLE_DEF (activate)

bool
WindowInterface::place (CompPoint &pos)
    WRAPABLE_DEF (place, pos)

void
WindowInterface::validateResizeRequest (unsigned int   &mask,
					XWindowChanges *xwc,
					unsigned int   source)
    WRAPABLE_DEF (validateResizeRequest, mask, xwc, source)

void
WindowInterface::resizeNotify (int dx,
			       int dy,
			       int dwidth,
			       int dheight)
    WRAPABLE_DEF (resizeNotify, dx, dy, dwidth, dheight)

void
WindowInterface::moveNotify (int  dx,
			     int  dy,
			     bool immediate)
    WRAPABLE_DEF (moveNotify, dx, dy, immediate)

void
WindowInterface::windowNotify (CompWindowNotify n)
    WRAPABLE_DEF (windowNotify, n)

void
WindowInterface::grabNotify (int          x,
			     int          y,
			     unsigned int state,
			     unsigned int mask)
    WRAPABLE_DEF (grabNotify, x, y, state, mask)

void
WindowInterface::ungrabNotify ()
    WRAPABLE_DEF (ungrabNotify)

void
WindowInterface::stateChangeNotify (unsigned int lastState)
    WRAPABLE_DEF (stateChangeNotify, lastState)

void
WindowInterface::updateFrameRegion (CompRegion &region)
    WRAPABLE_DEF (updateFrameRegion, region)

void
WindowInterface::minimize ()
    WRAPABLE_DEF (minimize);

void
WindowInterface::unminimize ()
    WRAPABLE_DEF (unminimize);

bool
WindowInterface::minimized ()
    WRAPABLE_DEF (minimized);

bool
WindowInterface::alpha ()
    WRAPABLE_DEF (alpha);

bool
WindowInterface::isFocussable ()
    WRAPABLE_DEF (isFocussable);

bool
WindowInterface::managed ()
    WRAPABLE_DEF (managed);

bool
WindowInterface::focused ()
    WRAPABLE_DEF (focused);

Window
CompWindow::id ()
{
    return priv->id;
}

unsigned int
CompWindow::type ()
{
    return priv->type;
}

unsigned int &
CompWindow::state ()
{
    return priv->state;
}

unsigned int
CompWindow::actions ()
{
    return priv->actions;
}

unsigned int &
CompWindow::protocols ()
{
    return priv->protocols;
}

void
CompWindow::close (Time serverTime)
{
    if (serverTime == 0)
	serverTime = screen->getCurrentTime ();

    if (priv->alive)
    {
	if (priv->protocols & CompWindowProtocolDeleteMask)
	{
	    XEvent ev;

	    ev.type		    = ClientMessage;
	    ev.xclient.window	    = priv->id;
	    ev.xclient.message_type = Atoms::wmProtocols;
	    ev.xclient.format	    = 32;
	    ev.xclient.data.l[0]    = Atoms::wmDeleteWindow;
	    ev.xclient.data.l[1]    = serverTime;
	    ev.xclient.data.l[2]    = 0;
	    ev.xclient.data.l[3]    = 0;
	    ev.xclient.data.l[4]    = 0;

	    XSendEvent (screen->dpy (), priv->id, false, NoEventMask, &ev);
	}
	else
	{
	    XKillClient (screen->dpy (), priv->id);
	}

	priv->closeRequests++;
    }
    else
    {
	screen->toolkitAction (Atoms::toolkitActionForceQuitDialog,
				     serverTime, priv->id, true, 0, 0);
    }

    priv->lastCloseRequestTime = serverTime;
}

bool
PrivateWindow::handlePingTimeout (unsigned int lastPing)
{
    if (!window->isViewable ())
	return false;

    if (!(priv->type & CompWindowTypeNormalMask))
	return false;

    if (priv->protocols & CompWindowProtocolPingMask)
    {
	if (priv->transientFor)
	    return false;

	if (priv->lastPong < lastPing)
	{
	    if (priv->alive)
	    {
		priv->alive = false;

		window->windowNotify (CompWindowNotifyAliveChanged);

		if (priv->closeRequests)
		{
		    screen->toolkitAction (Atoms::toolkitActionForceQuitDialog,
					   priv->lastCloseRequestTime,
					   priv->id, true, 0, 0);

		    priv->closeRequests = 0;
		}
	    }
	}

	return true;
    }
    return false;
}

void
PrivateWindow::handlePing (int lastPing)
{
    if (!priv->alive)
    {
	priv->alive = true;

	window->windowNotify (CompWindowNotifyAliveChanged);

	if (priv->lastCloseRequestTime)
	{
	    screen->toolkitAction (Atoms::toolkitActionForceQuitDialog,
				   priv->lastCloseRequestTime,
				   priv->id, false, 0, 0);

	    priv->lastCloseRequestTime = 0;
	}
    }
    priv->lastPong = lastPing;
}

void
PrivateWindow::processMap ()
{
    bool                   allowFocus;
    bool                   initiallyMinimized;
    CompStackingUpdateMode stackingMode;

    priv->initialViewport = screen->vp ();

    priv->initialTimestampSet = false;

    screen->applyStartupProperties (window);

    initiallyMinimized = (priv->hints &&
			  priv->hints->initial_state == IconicState &&
			  !window->minimized ());

    if (!serverFrame && !initiallyMinimized)
	reparent ();

    priv->managed = true;

    if (!priv->placed)
    {
	int            gravity = priv->sizeHints.win_gravity;
	XWindowChanges xwc = XWINDOWCHANGES_INIT;
	unsigned int   xwcm;

	/* adjust for gravity, but only for frame size */
	xwc.x      = priv->serverGeometry.x ();
	xwc.y      = priv->serverGeometry.y ();
	xwc.width  = 0;
	xwc.height = 0;

	xwcm = adjustConfigureRequestForGravity (&xwc, CWX | CWY, gravity, 1);

	window->validateResizeRequest (xwcm, &xwc, ClientTypeApplication);

	CompPoint pos (xwc.x, xwc.y);
	if (window->place (pos))
	{
	    xwc.x = pos.x ();
	    xwc.y = pos.y ();
	    xwcm |= CWX | CWY;
	}

	if (xwcm)
	    window->configureXWindow (xwcm, &xwc);

	priv->placed = true;
    }

    allowFocus = allowWindowFocus (NO_FOCUS_MASK, 0);

    if (!allowFocus && (priv->type & ~NO_FOCUS_MASK))
	stackingMode = CompStackingUpdateModeInitialMapDeniedFocus;
    else
	stackingMode = CompStackingUpdateModeInitialMap;

    window->updateAttributes (stackingMode);

    if (window->minimized () && !initiallyMinimized)
	window->unminimize ();

    screen->leaveShowDesktopMode (window);

    if (!initiallyMinimized)
    {
	if (allowFocus && !window->onCurrentDesktop ())
	    screen->setCurrentDesktop (priv->desktop);

	if (!(priv->state & CompWindowStateHiddenMask))
	    show ();

	if (allowFocus)
	{
	    window->moveInputFocusTo ();
	    if (!window->onCurrentDesktop ())
		screen->setCurrentDesktop (priv->desktop);
	}
    }
    else
    {
	window->minimize ();
	window->changeState (window->state () | CompWindowStateHiddenMask);
    }

    screen->updateClientList ();
}

/*
 * PrivateWindow::updatePassiveButtonGrabs
 *
 * Updates the passive button grabs for a window. When
 * one of the specified button + modifier combinations
 * for this window is activated, compiz will be given
 * an active grab for the window (which we can turn off
 * by calling XAllowEvents later in ::handleEvent)
 *
 * NOTE: ICCCM says that we are only allowed to grab
 * windows that we actually own as a client, so only
 * grab the frame window. Additionally, although there
 * isn't anything in the ICCCM that says we cannot
 * grab every button, some clients do not interpret
 * EnterNotify and LeaveNotify events caused by the
 * activation of the grab correctly, so ungrab button
 * and modifier combinations that we do not need on
 * active windows (but in reality we shouldn't be grabbing
 * for buttons that we don't actually need at that point
 * anyways)
 */

void
PrivateWindow::updatePassiveButtonGrabs ()
{
    bool onlyActions = (priv->id == screen->activeWindow() ||
			!screen->getCoreOptions().optionGetClickToFocus ());

    if (!priv->frame)
	return;

    /* Ungrab everything */
    XUngrabButton (screen->dpy(), AnyButton, AnyModifier, frame);

    /* We don't need the full grab in the following cases:
     * - This window has the focus and either
     *   - it is raised or
     *   - we don't want click raise
     */

    if (onlyActions)
    {
	if (screen->getCoreOptions().optionGetRaiseOnClick ())
	{
	    CompWindow *highestSibling =
		    PrivateWindow::findSiblingBelow (window, true);

	    /* Check if this window is permitted to be raised */
	    for (CompWindow *above = window->serverNext;
		above != NULL; above = above->serverNext)
	    {
		if (highestSibling == above)
		{
		    onlyActions = false;
		    break;
		}
	    }
	}
    }

    if (onlyActions)
    {
        screen->updatePassiveButtonGrabs(serverFrame);
    }
    else
    {
	/* Grab everything */
	XGrabButton (screen->dpy(),
		     AnyButton,
		     AnyModifier,
		     serverFrame, false,
		     ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
		     GrabModeSync,
		     GrabModeAsync,
		     None,
		     None);
    }
}


const CompRegion &
CompWindow::region () const
{
    return priv->region;
}

const CompRegion &
CompWindow::frameRegion () const
{
    return priv->frameRegion;
}

bool
CompWindow::inShowDesktopMode ()
{
    return priv->inShowDesktopMode;
}

void
CompWindow::setShowDesktopMode (bool value)
{
    priv->inShowDesktopMode = value;
}

bool
CompWindow::managed ()
{
    WRAPABLE_HND_FUNCTN_RETURN (bool, managed);
    return priv->managed;
}

bool
CompWindow::focused ()
{
    WRAPABLE_HND_FUNCTN_RETURN (bool, focused);
    return screen->activeWindow () == id ();
}

bool
CompWindow::grabbed ()
{
    return priv->grabbed;
}

int
CompWindow::pendingMaps ()
{
    return priv->pendingMaps;
}

unsigned int &
CompWindow::wmType ()
{
    return priv->wmType;
}

unsigned int
CompWindow::activeNum ()
{
    return priv->activeNum;
}

Window
CompWindow::frame ()
{
    return priv->serverFrame;
}

CompString
CompWindow::resName ()
{
    if (priv->resName)
	return priv->resName;

    return CompString ();
}

int
CompWindow::mapNum () const
{
    return priv->mapNum;
}

CompStruts *
CompWindow::struts ()
{
    return priv->struts;
}

int &
CompWindow::saveMask ()
{
    return priv->saveMask;
}

XWindowChanges &
CompWindow::saveWc ()
{
    return priv->saveWc;
}

void
CompWindow::moveToViewportPosition (int  x,
				    int  y,
				    bool sync)
{
    int tx, ty;
    int vWidth  = screen->width () * screen->vpSize ().width ();
    int vHeight = screen->height () * screen->vpSize ().height ();

    if (screen->vpSize ().width () != 1)
    {
	x += screen->vp ().x () * screen->width ();
	x = compiz::core::screen::wraparound_mod (x, vWidth);
	x -= screen->vp ().x () * screen->width ();
    }

    if (screen->vpSize ().height () != 1)
    {
	y += screen->vp ().y () * screen->height ();
	y = compiz::core::screen::wraparound_mod (y, vHeight);
	y -= screen->vp ().y () * screen->height ();
    }

    tx = x - priv->serverGeometry.x ();
    ty = y - priv->serverGeometry.y ();

    if (tx || ty)
    {
	unsigned int   valueMask = CWX | CWY;
	XWindowChanges xwc = XWINDOWCHANGES_INIT;
	int m, wx, wy;

	if (!priv->managed)
	    return;

	if (priv->type & (CompWindowTypeDesktopMask | CompWindowTypeDockMask))
	    return;

	if (priv->state & CompWindowStateStickyMask)
	    return;

	wx = tx;
	wy = ty;

	if (screen->vpSize ().width ()!= 1)
	{
	    m = priv->serverGeometry.x () + tx;

	    if (m - priv->output.left < (int) screen->width () - vWidth)
		wx = tx + vWidth;
	    else if (m + priv->serverGeometry.width () + priv->output.right > vWidth)
		wx = tx - vWidth;
	}

	if (screen->vpSize ().height () != 1)
	{
	    m = priv->serverGeometry.y () + ty;

	    if (m - priv->output.top < (int) screen->height () - vHeight)
		wy = ty + vHeight;
	    else if (m + priv->serverGeometry.height () + priv->output.bottom > vHeight)
		wy = ty - vHeight;
	}

	if (priv->saveMask & CWX)
	    priv->saveWc.x += wx;

	if (priv->saveMask & CWY)
	    priv->saveWc.y += wy;

	xwc.x = serverGeometry ().x () + wx;
	xwc.y = serverGeometry ().y () + wy;

	configureXWindow (valueMask, &xwc);
    }
}

char *
CompWindow::startupId ()
{
     return priv->startupId;
}

void
PrivateWindow::applyStartupProperties (CompStartupSequence *s)
{
    int workspace;

    priv->initialViewport.setX (s->viewportX);
    priv->initialViewport.setY (s->viewportY);

    workspace = sn_startup_sequence_get_workspace (s->sequence);
    if (workspace >= 0)
	window->setDesktop (workspace);

    priv->initialTimestamp    =
	sn_startup_sequence_get_timestamp (s->sequence);
    priv->initialTimestampSet = true;
}

unsigned int
CompWindow::desktop ()
{
    return priv->desktop;
}

Window
CompWindow::clientLeader (bool checkAncestor)
{
    if (priv->clientLeader)
	return priv->clientLeader;

    if (checkAncestor)
	return priv->getClientLeaderOfAncestor ();

    return None;
}

Window
CompWindow::transientFor ()
{
    return priv->transientFor;
}

int
CompWindow::pendingUnmaps ()
{
    return priv->pendingUnmaps;
}

bool
CompWindow::minimized ()
{
    WRAPABLE_HND_FUNCTN_RETURN (bool, minimized);
    return priv->minimized;
}

bool
CompWindow::placed ()
{
    return priv->placed;
}

bool
CompWindow::shaded ()
{
    return priv->shaded;
}

CompWindowExtents &
CompWindow::border () const
{
    return priv->border;
}

CompWindowExtents &
CompWindow::input () const
{
    return priv->serverInput;
}

CompWindowExtents &
CompWindow::output () const
{
    return priv->output;
}

XSizeHints &
CompWindow::sizeHints () const
{
    return priv->sizeHints;
}

void
PrivateWindow::updateMwmHints ()
{
    screen->getMwmHints (priv->id, &priv->mwmFunc, &priv->mwmDecor);
    window->recalcActions ();
}

void
PrivateWindow::updateStartupId ()
{
    char *oldId = startupId;
    bool newId = true;

    startupId = getStartupId ();

    if (oldId && startupId)
    {
	if (strcmp (startupId, oldId) == 0)
	    newId = false;
    }

    if (managed && startupId && newId)
    {
	Time       timestamp = 0;
	CompPoint  vp, svp;
	CompSize   size;
	int        x, y;

	initialTimestampSet = false;
	screen->applyStartupProperties (window);

	if (initialTimestampSet)
	    timestamp = initialTimestamp;

	/* as the viewport can't be transmitted via startup
	   notification, assume the client changing the ID
	   wanted to activate the window on the current viewport */

	vp   = window->defaultViewport ();
	svp  = screen->vp ();
	size = *screen;

	x = window->serverGeometry ().x () + (svp.x () - vp.x ()) * size.width ();
	y = window->serverGeometry ().y () + (svp.y () - vp.y ()) * size.height ();
	window->moveToViewportPosition (x, y, true);

	if (allowWindowFocus (0, timestamp))
	    window->activate ();
    }
    
    if (oldId)
	free (oldId);
}

bool
CompWindow::destroyed ()
{
    return priv->destroyed;
}

bool
CompWindow::invisible ()
{
    return priv->invisible;
}

XSyncAlarm
CompWindow::syncAlarm ()
{
    return priv->syncAlarm;
}

CompWindow *
PrivateWindow::createCompWindow (Window aboveId, Window aboveServerId, XWindowAttributes &wa, Window id)
{
    PrivateWindow* priv(new PrivateWindow ());
    priv->id = id;
    priv->serverId = id;

    CompWindow *fw = new CompWindow (aboveId, aboveServerId, wa, priv);

    return fw;
}


CompWindow::CompWindow (Window aboveId,
			Window aboveServerId,
			XWindowAttributes &wa,
			PrivateWindow *priv) :
    PluginClassStorage (windowPluginClassIndices),
    priv (priv)
{
    StackDebugger *dbg = StackDebugger::Default ();

    // TODO: Reparent first!

    priv->window = this;

    screen->insertWindow (this, aboveId);
    screen->insertServerWindow (this, aboveServerId);

    /* We must immediately insert the window into the debugging
     * stack */
    if (dbg)
	dbg->overrideRedirectRestack (priv->id, aboveId);

    priv->attrib = wa;
    priv->serverGeometry.set (priv->attrib.x, priv->attrib.y,
			      priv->attrib.width, priv->attrib.height,
			      priv->attrib.border_width);
    priv->serverFrameGeometry = priv->frameGeometry = priv->syncGeometry
	    = priv->geometry = priv->serverGeometry;

    priv->sizeHints.flags = 0;

    priv->recalcNormalHints ();

    priv->transientFor = None;
    priv->clientLeader = None;

    XSelectInput (screen->dpy (), priv->id,
		  wa.your_event_mask |
		  PropertyChangeMask |
		  EnterWindowMask    |
		  FocusChangeMask);

    priv->alpha     = (priv->attrib.depth == 32);
    priv->lastPong  = screen->lastPing ();

    if (screen->XShape ())
	XShapeSelectInput (screen->dpy (), priv->id, ShapeNotifyMask);

    if (priv->attrib.c_class != InputOnly)
    {
	priv->region = CompRegion (priv->serverGeometry);
	priv->inputRegion = priv->region;

	/* need to check for DisplayModal state on all windows */
	priv->state = screen->getWindowState (priv->id);

	priv->updateClassHints ();
    }
    else
    {
	priv->attrib.map_state = IsUnmapped;
    }

    priv->wmType    = screen->getWindowType (priv->id);
    priv->protocols = screen->getProtocols (priv->id);

    if (!overrideRedirect ())
    {
	priv->updateNormalHints ();
	updateStruts ();
	priv->updateWmHints ();
	priv->updateTransientHint ();

	priv->clientLeader = priv->getClientLeader ();
	priv->startupId = priv->getStartupId ();

	recalcType ();

	screen->getMwmHints (priv->id, &priv->mwmFunc, &priv->mwmDecor);

	if (!(priv->type & (CompWindowTypeDesktopMask | CompWindowTypeDockMask)))
	{
	    priv->desktop = screen->getWindowProp (priv->id, Atoms::winDesktop,
					           priv->desktop);
	    if (priv->desktop != 0xffffffff)
	    {
		if (priv->desktop >= screen->nDesktop ())
		    priv->desktop = screen->currentDesktop ();
	    }
	}
    }
    else
    {
	recalcType ();
    }

    if (priv->attrib.map_state == IsViewable)
    {
	priv->placed = true;

	if (!overrideRedirect ())
	{
	    // needs to happen right after maprequest
	    if (!priv->serverFrame)
		priv->reparent ();
	    priv->managed = true;

	    if (screen->getWmState (priv->id) == IconicState)
	    {
		if (priv->state & CompWindowStateShadedMask)
		    priv->shaded = true;
		else
		    priv->minimized = true;
	    }
	    else
	    {
		if (priv->wmType & (CompWindowTypeDockMask |
				 CompWindowTypeDesktopMask))
		{
		    setDesktop (0xffffffff);
		}
		else
		{
		    if (priv->desktop != 0xffffffff)
			priv->desktop = screen->currentDesktop ();

		    screen->setWindowProp (priv->id, Atoms::winDesktop,
				           priv->desktop);
		}
	    }
	}

	priv->attrib.map_state = IsUnmapped;
	priv->pendingMaps++;

	map ();

	updateAttributes (CompStackingUpdateModeNormal);

	if (priv->minimized || priv->inShowDesktopMode ||
	    priv->hidden || priv->shaded)
	{
	    priv->state |= CompWindowStateHiddenMask;

	    priv->pendingUnmaps++;

	    if (priv->serverFrame && !priv->shaded)
		XUnmapWindow (screen->dpy (), priv->serverFrame);

	    XUnmapWindow (screen->dpy (), priv->id);

	    screen->setWindowState (priv->state, priv->id);
	}
    }
    else if (!overrideRedirect ())
    {
	if (screen->getWmState (priv->id) == IconicState)
	{
	    // before everything else in maprequest
	    if (!priv->serverFrame)
		priv->reparent ();
	    priv->managed = true;
	    priv->placed  = true;

	    if (priv->state & CompWindowStateHiddenMask)
	    {
		if (priv->state & CompWindowStateShadedMask)
		    priv->shaded = true;
		else
		    priv->minimized = true;
	    }
	}
    }

    /* TODO: bailout properly when objectInitPlugins fails */
    assert (CompPlugin::windowInitPlugins (this));

    recalcActions ();
    priv->updateIconGeometry ();

    if (priv->shaded)
	priv->updateFrameWindow ();

    if (priv->attrib.map_state == IsViewable)
    {
	priv->invisible = priv->isInvisible ();
    }
}

CompWindow::~CompWindow ()
{
    if (priv->serverFrame)
	priv->unreparent ();

    /* Update the references of other windows
     * pending destroy if this was a sibling
     * of one of those */

    screen->destroyedWindows().remove (this);

    foreach (CompWindow *dw, screen->destroyedWindows())
    {
	if (dw->next == this)
	    dw->next = this->next;
	if (dw->prev == this)
	    dw->prev = this->prev;

	if (dw->serverNext == this)
	    dw->serverNext = this->serverNext;
	if (dw->serverPrev == this)
	    dw->serverPrev = this->serverPrev;
    }

    if (!priv->destroyed)
    {
	StackDebugger *dbg = StackDebugger::Default ();

	screen->unhookWindow (this);
	screen->unhookServerWindow (this);

	/* We must immediately insert the window into the debugging
	 * stack */
	if (dbg)
	    dbg->removeServerWindow (id ());

	/* restore saved geometry and map if hidden */
	if (!priv->attrib.override_redirect)
	{
	    if (priv->saveMask)
		XConfigureWindow (screen->dpy (), priv->id,
				  priv->saveMask, &priv->saveWc);

	    if (!priv->hidden)
	    {
		if (priv->state & CompWindowStateHiddenMask)
		    XMapWindow (screen->dpy (), priv->id);
	    }
	}

	if (screen->XShape ())
	    XShapeSelectInput (screen->dpy (), priv->id, NoEventMask);

	if (screen->grabWindowIsNot(priv->id))
	    XSelectInput (screen->dpy (), priv->id, NoEventMask);

	XUngrabButton (screen->dpy (), AnyButton, AnyModifier, priv->id);
    }


    if (priv->attrib.map_state == IsViewable)
    {
	if (priv->type == CompWindowTypeDesktopMask)
	    screen->decrementDesktopWindowCount();

	if (priv->destroyed && priv->struts)
	    screen->updateWorkarea ();
    }

    if (priv->destroyed)
	screen->updateClientList ();

    CompPlugin::windowFiniPlugins (this);

    delete priv;
}

PrivateWindow::PrivateWindow () :
    priv (this),
    refcnt (1),
    id (None),
    serverFrame (None),
    frame (None),
    wrapper (None),
    mapNum (0),
    activeNum (0),
    transientFor (None),
    clientLeader (None),
    hints (NULL),
    inputHint (true),
    alpha (false),
    region (),
    wmType (0),
    type (CompWindowTypeUnknownMask),
    state (0),
    actions (0),
    protocols (0),
    mwmDecor (MwmDecorAll),
    mwmFunc (MwmFuncAll),
    invisible (true),
    destroyed (false),
    managed (false),
    unmanaging (false),
    destroyRefCnt (1),
    unmapRefCnt (1),

    initialViewport (0, 0),

    initialTimestamp (0),
    initialTimestampSet (false),

    fullscreenMonitorsSet (false),

    placed (false),
    minimized (false),
    inShowDesktopMode (false),
    shaded (false),
    hidden (false),
    grabbed (false),

    desktop (0),

    pendingUnmaps (0),
    pendingMaps (0),
    pendingConfigures (screen->dpy ()),

    startupId (0),
    resName (0),
    resClass (0),

    group (0),

    lastPong (0),
    alive (true),

    struts (0),

    icons (0),
    noIcons (false),

    saveMask (0),
    syncCounter (0),
    syncAlarm (None),
    syncWaitTimer (),

    syncWait (false),
    closeRequests (false),
    lastCloseRequestTime (0)
{
    input.left   = 0;
    input.right  = 0;
    input.top    = 0;
    input.bottom = 0;

    serverInput.left   = 0;
    serverInput.right  = 0;
    serverInput.top    = 0;
    serverInput.bottom = 0;

    border.top    = 0;
    border.bottom = 0;
    border.left   = 0;
    border.right  = 0;

    output.left   = 0;
    output.right  = 0;
    output.top    = 0;
    output.bottom = 0;

    syncWaitTimer.setTimes (1000, 1200);
    syncWaitTimer.setCallback (boost::bind (&PrivateWindow::handleSyncAlarm,
					    this));
}

PrivateWindow::~PrivateWindow ()
{
     if (syncAlarm)
	XSyncDestroyAlarm (screen->dpy (), syncAlarm);

    syncWaitTimer.stop ();

    if (serverFrame)
	XDestroyWindow (screen->dpy (), serverFrame);
    else if (frame)
	XDestroyWindow (screen->dpy (), frame);

    if (struts)
	free (struts);

    if (hints)
	XFree (hints);

    if (icons.size ())
	freeIcons ();

    if (startupId)
	free (startupId);

    if (resName)
	free (resName);

    if (resClass)
	free (resClass);
}

bool
CompWindow::syncWait ()
{
    return priv->syncWait;
}

bool
CompWindow::alpha ()
{
    WRAPABLE_HND_FUNCTN_RETURN (bool, alpha);

    return priv->alpha;
}

bool
CompWindow::overrideRedirect ()
{
    return priv->attrib.override_redirect;
}

void
PrivateWindow::setOverrideRedirect (bool overrideRedirect)
{
    if (overrideRedirect == window->overrideRedirect ())
	return;

    priv->attrib.override_redirect = overrideRedirect ? 1 : 0;
    window->recalcType ();
    window->recalcActions ();

    screen->matchPropertyChanged (window);
}

bool
CompWindow::isMapped () const
{
    return priv->mapNum > 0;
}

bool
CompWindow::isViewable () const
{
    return (priv->attrib.map_state == IsViewable);
}

bool
CompWindow::isFocussable ()
{
    WRAPABLE_HND_FUNCTN_RETURN (bool, isFocussable);

    if (priv->inputHint)
	return true;

    if (priv->protocols & CompWindowProtocolTakeFocusMask)
	return true;

    return false;
}

int
CompWindow::windowClass ()
{
    return priv->attrib.c_class;
}

unsigned int
CompWindow::depth ()
{
    return priv->attrib.depth;
}

bool
CompWindow::alive ()
{
    return priv->alive;
}

unsigned int
CompWindow::mwmDecor ()
{
    return priv->mwmDecor;
}

unsigned int
CompWindow::mwmFunc ()
{
    return priv->mwmFunc;
}

/* TODO: This function should be able to check the XShape event
 * kind and only get/set shape rectangles for either ShapeInput
 * or ShapeBounding, but not both at the same time
 */

void
CompWindow::updateFrameRegion ()
{
    if (priv->serverFrame)
    {
	CompRect   r;
	int        x, y;

	priv->frameRegion = emptyRegion;

	updateFrameRegion (priv->frameRegion);

	if (!shaded ())
	{
	    r = priv->region.boundingRect ();
	    priv->frameRegion -= r;

	    r.setGeometry (r.x1 () - priv->serverInput.left,
			r.y1 () - priv->serverInput.top,
			r.width  () + priv->serverInput.right + priv->serverInput.left,
			r.height () + priv->serverInput.bottom + priv->serverInput.top);

	    priv->frameRegion &= CompRegion (r);
	}

	x = priv->serverGeometry.x () - priv->serverInput.left;
	y = priv->serverGeometry.y () - priv->serverInput.top;

	XShapeCombineRegion (screen->dpy (), priv->serverFrame,
			     ShapeBounding, -x, -y,
			     priv->frameRegion.united (priv->region).handle (),
			     ShapeSet);

	XShapeCombineRegion (screen->dpy (), priv->serverFrame,
			     ShapeInput, -x, -y,
			     priv->frameRegion.united (priv->inputRegion).handle (),
			     ShapeSet);
    }
}

void
CompWindow::setWindowFrameExtents (CompWindowExtents *b,
				   CompWindowExtents *i)
{
    /* override redirect windows can't have frame extents */
    if (priv->attrib.override_redirect)
	return;

    /* Input extents are used for frame size,
     * Border extents used for placement.
     */

    if (!i)
	i = b;

    if (priv->serverInput.left   != i->left ||
	priv->serverInput.right  != i->right ||
	priv->serverInput.top    != i->top ||
	priv->serverInput.bottom != i->bottom ||
	priv->border.left   != b->left ||
	priv->border.right  != b->right ||
	priv->border.top    != b->top ||
	priv->border.bottom != b->bottom)
    {
	priv->serverInput = *i;
	priv->border = *b;

	recalcActions ();

	priv->updateSize ();
	priv->updateFrameWindow ();

	/* Always send a moveNotify
	 * whenever the frame extents update
	 * so that plugins can re-position appropriately */
	moveNotify (0, 0, true);
    }

    /* Use b for _NET_WM_FRAME_EXTENTS here because
     * that is the representation of the actual decoration
     * around the window that the user sees and should
     * be used for placement and such */

    /* Also update frame extents regardless of whether or not
     * the frame extents actually changed, eg, a plugin could
     * suggest that a window has no frame extents and that it
     * might later get frame extents - this is mandatory if we
     * say that we support it, so set them
     * additionaly some applications might request frame extents
     * and we must respond by setting the property - ideally
     * this should only ever be done when some plugin actually
     * need to change the frame extents or the applications
     * requested it */

    unsigned long data[4];

    data[0] = b->left;
    data[1] = b->right;
    data[2] = b->top;
    data[3] = b->bottom;

    XChangeProperty (screen->dpy (), priv->id,
		     Atoms::frameExtents,
		     XA_CARDINAL, 32, PropModeReplace,
		     (unsigned char *) data, 4);
}

bool
CompWindow::hasUnmapReference ()
{
    return (priv && priv->unmapRefCnt > 1);
}

void
CompWindow::updateFrameRegion (CompRegion& region)
    WRAPABLE_HND_FUNCTN (updateFrameRegion, region)

bool
PrivateWindow::reparent ()
{
    XSetWindowAttributes attr;
    XWindowAttributes    wa;
    XWindowChanges       xwc = XWINDOWCHANGES_INIT;
    int                  mask;
    unsigned int         nchildren;
    Window		 *children, root_return, parent_return;
    Display              *dpy = screen->dpy ();
    Visual		 *visual = DefaultVisual (screen->dpy (),
						  screen->screenNum ());
    Colormap		 cmap = DefaultColormap (screen->dpy (),
						 screen->screenNum ());

    if (serverFrame)
	return false;

    XSync (dpy, false);
    XGrabServer (dpy);

    if (!XGetWindowAttributes (dpy, id, &wa))
    {
	XUngrabServer (dpy);
	XSync (dpy, false);
	return false;
    }

    if (wa.override_redirect)
	return false;

    /* Don't ever reparent windows which have ended up
     * reparented themselves on the server side but not
     * on the client side */

    XQueryTree (dpy, id, &root_return, &parent_return, &children, &nchildren);

    if (parent_return != root_return)
    {
	XFree (children);
	XUngrabServer (dpy);
	XSync (dpy, false);
	return false;
    }

    XFree (children);

    XQueryTree (dpy, root_return, &root_return, &parent_return, &children, &nchildren);

    XChangeSaveSet (dpy, id, SetModeInsert);

    /* Force border width to 0 */
    xwc.border_width = 0;
    XConfigureWindow (dpy, id, CWBorderWidth, &xwc);

    priv->serverGeometry.setBorder (0);

    mask = CWBorderPixel | CWColormap | CWBackPixmap | CWOverrideRedirect;

    if (wa.depth == 32)
    {
	cmap = wa.colormap;
	visual = wa.visual;
    }

    attr.background_pixmap = None;
    attr.border_pixel      = 0;
    attr.colormap          = cmap;
    attr.override_redirect = true;

    /* We need to know when the frame window is created
     * but that's all */
    XSelectInput (dpy, screen->root (), SubstructureNotifyMask);

    /* Awaiting a new frame to be given to us */
    frame = None;
    serverFrame = XCreateWindow (dpy, screen->root (), 0, 0,
				 wa.width, wa.height, 0, wa.depth,
				 InputOutput, visual, mask, &attr);

    /* Do not get any events from here on */
    XSelectInput (dpy, screen->root (), NoEventMask);

    wrapper = XCreateWindow (dpy, serverFrame, 0, 0,
			    wa.width, wa.height, 0, wa.depth,
			    InputOutput, visual, mask, &attr);

    xwc.stack_mode = Above;

    /* Look for the client in the current server side stacking
     * order and put the frame above what the client is above
     */
    if (nchildren &&
	children[0] == id)
    {
	/* client at the bottom */
	xwc.stack_mode = Below;
	xwc.sibling = id;
    }
    else
    {
	for (unsigned int i = 0; i < nchildren; i++)
	{
	    if (i < nchildren - 1)
	    {
		if (children[i + 1] == id)
		{
		    xwc.sibling = children[i];
		    break;
		}
	    }
	    else /* client on top */
		xwc.sibling = children[i];
	}
    }

    XFree (children);

    /* Make sure the frame is underneath the client */
    XConfigureWindow (dpy, serverFrame, CWSibling | CWStackMode, &xwc);

    /* Wait for the restacking to finish */
    XSync (dpy, false);

    /* Always need to have the wrapper window mapped */
    XMapWindow (dpy, wrapper);

    /* Reparent the client into the wrapper window */
    XReparentWindow (dpy, id, wrapper, 0, 0);

    /* Restore events */
    attr.event_mask = wa.your_event_mask;

    /* We don't care about client events on the frame, and listening for them
     * will probably end up fighting the client anyways, so disable them */

    attr.do_not_propagate_mask = KeyPressMask | KeyReleaseMask |
				 ButtonPressMask | ButtonReleaseMask |
				 EnterWindowMask | LeaveWindowMask |
				 PointerMotionMask | PointerMotionHintMask |
				 Button1MotionMask | Button2MotionMask |
				 Button3MotionMask | Button4MotionMask |
				 Button5MotionMask | ButtonMotionMask |
				 KeymapStateMask | ExposureMask |
				 VisibilityChangeMask | StructureNotifyMask |
				 ResizeRedirectMask | SubstructureNotifyMask |
				 SubstructureRedirectMask | FocusChangeMask |
				 PropertyChangeMask | ColormapChangeMask |
				 OwnerGrabButtonMask;

    XChangeWindowAttributes (dpy, id, CWEventMask | CWDontPropagate, &attr);

    if (wa.map_state == IsViewable || shaded)
	XMapWindow (dpy, serverFrame);

    attr.event_mask = SubstructureRedirectMask |
		      SubstructureNotifyMask | EnterWindowMask |
		      LeaveWindowMask;

    serverFrameGeometry = serverGeometry;

    XMoveResizeWindow (dpy, serverFrame, serverFrameGeometry.x (), serverFrameGeometry.y (),
		       serverFrameGeometry.width (), serverFrameGeometry.height ());

    XChangeWindowAttributes (dpy, serverFrame, CWEventMask, &attr);
    XChangeWindowAttributes (dpy, wrapper, CWEventMask, &attr);

    XSelectInput (dpy, screen->root (),
		  SubstructureRedirectMask |
		  SubstructureNotifyMask   |
		  StructureNotifyMask      |
		  PropertyChangeMask       |
		  LeaveWindowMask          |
		  EnterWindowMask          |
		  KeyPressMask             |
		  KeyReleaseMask           |
		  ButtonPressMask          |
		  ButtonReleaseMask        |
		  FocusChangeMask          |
		  ExposureMask);

    XUngrabServer (dpy);
    XSync (dpy, false);

    window->windowNotify (CompWindowNotifyReparent);

    return true;
}

void
PrivateWindow::unreparent ()
{
    Display        	 *dpy = screen->dpy ();
    XEvent         	 e;
    bool           	 alive = true;
    XWindowChanges 	 xwc = XWINDOWCHANGES_INIT;
    unsigned int         nchildren;
    Window		 *children = NULL, root_return, parent_return;
    XWindowAttributes	 wa;
    StackDebugger        *dbg = StackDebugger::Default ();

    if (!serverFrame)
	return;

    XSync (dpy, false);

    if (XCheckTypedWindowEvent (dpy, id, DestroyNotify, &e))
    {
        XPutBackEvent (dpy, &e);
        alive = false;
    }
    else
    {
	if (!XGetWindowAttributes (dpy, id, &wa))
	    alive = false;
    }

    /* Also don't reparent back into root windows that have ended up
     * reparented into other windows (and as such we are unmanaging them) */

    if (alive)
    {
	XQueryTree (dpy, id, &root_return, &parent_return, &children, &nchildren);

	if (parent_return != wrapper)
	    alive = false;
    }

    if ((!destroyed) && alive)
    {
	XGrabServer (dpy);

        XChangeSaveSet (dpy, id, SetModeDelete);
	XSelectInput (dpy, serverFrame, NoEventMask);
	XSelectInput (dpy, wrapper, NoEventMask);
	XSelectInput (dpy, id, NoEventMask);
	XSelectInput (dpy, screen->root (), NoEventMask);
	XReparentWindow (dpy, id, screen->root (), 0, 0);

	/* Wait for the reparent to finish */
	XSync (dpy, false);

	xwc.x = serverGeometry.xMinusBorder ();
	xwc.y = serverGeometry.yMinusBorder ();
	xwc.width = serverGeometry.widthIncBorders ();
	xwc.height = serverGeometry.heightIncBorders ();

	XConfigureWindow (dpy, serverFrame, CWX | CWY | CWWidth | CWHeight, &xwc);


	xwc.stack_mode = Below;
	xwc.sibling    = serverFrame;
	XConfigureWindow (dpy, id, CWSibling | CWStackMode, &xwc);

	/* Wait for the window to be restacked */
	XSync (dpy, false);

	XUnmapWindow (dpy, serverFrame);

	XSelectInput (dpy, id, wa.your_event_mask);

	XSelectInput (dpy, screen->root (),
		  SubstructureRedirectMask |
		  SubstructureNotifyMask   |
		  StructureNotifyMask      |
		  PropertyChangeMask       |
		  LeaveWindowMask          |
		  EnterWindowMask          |
		  KeyPressMask             |
		  KeyReleaseMask           |
		  ButtonPressMask          |
		  ButtonReleaseMask        |
		  FocusChangeMask          |
		  ExposureMask);

	XUngrabServer (dpy);
	XSync (dpy, false);

	XMoveWindow (dpy, id, serverGeometry.x (), serverGeometry.y ());
    }

    if (children)
	XFree (children);

    if (dbg)
	dbg->addDestroyedFrame (serverFrame);

    /* This is where things get tricky ... it is possible
     * to receive a ConfigureNotify relative to a frame window
     * for a destroyed window in case we process a ConfigureRequest
     * for the destroyed window and then a DestroyNotify for it directly
     * afterwards. In that case, we will receive the ConfigureNotify
     * for the XConfigureWindow request we made relative to that frame
     * window. Because of this, we must keep the frame window in the stack
     * as a new toplevel window so that the ConfigureNotify will be processed
     * properly until it too receives a DestroyNotify */

    if (serverFrame)
    {
	XWindowAttributes attrib;

	/* It's possible that the frame window was already destroyed because
	 * the client was unreparented before it was destroyed (eg
	 * UnmapNotify before DestroyNotify). In that case the frame window
	 * is going to be an invalid window but since we haven't received
	 * a DestroyNotify for it yet, it is possible that restacking
	 * operations could occurr relative to it so we need to hold it
	 * in the stack for now. Ensure that it is marked override redirect */
	XGetWindowAttributes (screen->dpy (), serverFrame, &attrib);

	/* Put the frame window "above" the client window
	 * in the stack */
	PrivateWindow::createCompWindow (id, id, attrib, serverFrame);
    }

    /* Issue a DestroyNotify */
    XDestroyWindow (screen->dpy (), serverFrame);
    XDestroyWindow (screen->dpy (), wrapper);

    /* This window is no longer "managed" in the
     * reparenting sense so clear its pending event
     * queue ... though maybe in future it would
     * be better to bookeep these events too and
     * handle the ReparentNotify */
    pendingConfigures.clear ();

    frame = None;
    wrapper = None;
    serverFrame = None;

    // Finally, (i.e. after updating state) notify the change
    window->windowNotify (CompWindowNotifyUnreparent);
}
