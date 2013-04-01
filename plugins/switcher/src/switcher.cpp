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

#include "switcher.h"

COMPIZ_PLUGIN_20090315 (switcher, SwitchPluginVTable)

const unsigned short WIDTH  = 212;
const unsigned short HEIGHT = 192;
const unsigned short SPACE  = 10;

const unsigned short BOX_WIDTH = 3;

#define XWINDOWCHANGES_INIT {0, 0, 0, 0, 0, None, 0}

static float _boxVertices[] =
{
    -(WIDTH >> 1), BOX_WIDTH, 0.0f,
     (WIDTH >> 1), BOX_WIDTH, 0.0f,
    -(WIDTH >> 1), 0.0f,      0.0f,
    -(WIDTH >> 1), 0.0f,      0.0f,
     (WIDTH >> 1), BOX_WIDTH, 0.0f,
     (WIDTH >> 1), 0.0f,      0.0f,

    -(WIDTH >> 1),             HEIGHT - BOX_WIDTH, 0.0f,
    -(WIDTH >> 1) + BOX_WIDTH, HEIGHT - BOX_WIDTH, 0.0f,
    -(WIDTH >> 1),             BOX_WIDTH,          0.0f,
    -(WIDTH >> 1),             BOX_WIDTH,          0.0f,
    -(WIDTH >> 1) + BOX_WIDTH, HEIGHT - BOX_WIDTH, 0.0f,
    -(WIDTH >> 1) + BOX_WIDTH, BOX_WIDTH,          0.0f,

     (WIDTH >> 1) - BOX_WIDTH, HEIGHT - BOX_WIDTH, 0.0f,
     (WIDTH >> 1),             HEIGHT - BOX_WIDTH, 0.0f,
     (WIDTH >> 1) - BOX_WIDTH, BOX_WIDTH,          0.0f,
     (WIDTH >> 1) - BOX_WIDTH, BOX_WIDTH,          0.0f,
     (WIDTH >> 1),             HEIGHT - BOX_WIDTH, 0.0f,
     (WIDTH >> 1),             BOX_WIDTH,          0.0f,

    -(WIDTH >> 1), HEIGHT,             0.0f,
     (WIDTH >> 1), HEIGHT,             0.0f,
    -(WIDTH >> 1), HEIGHT - BOX_WIDTH, 0.0f,
    -(WIDTH >> 1), HEIGHT - BOX_WIDTH, 0.0f,
     (WIDTH >> 1), HEIGHT,             0.0f,
     (WIDTH >> 1), HEIGHT - BOX_WIDTH, 0.0f,
};


void
SwitchScreen::updateWindowList (int count)
{
    int x, y;

    if (count > 1)
    {
	count -= (count + 1) & 1;
	if (count < 3)
	    count = 3;
    }

    pos  = ((count >> 1) - (int)windows.size ()) * WIDTH;
    move = 0;

    selectedWindow = windows.front ();

    x = screen->currentOutputDev ().x1 () +
	screen->currentOutputDev ().width () / 2;
    y = screen->currentOutputDev ().y1 () +
	screen->currentOutputDev ().height () / 2;

    if (popupWindow)
    {
	CompWindow *w = screen->findWindow (popupWindow);

	XWindowChanges xwc = XWINDOWCHANGES_INIT;
	unsigned int valueMask = 0;

	valueMask |= (CWX | CWY | CWWidth | CWHeight);

	xwc.x = x - WINDOW_WIDTH (count) / 2;
	xwc.y = y - WINDOW_HEIGHT / 2;
	xwc.width = WINDOW_WIDTH (count);
	xwc.height = WINDOW_HEIGHT;

	if (w)
	    w->configureXWindow (valueMask, &xwc);
	else
	    XConfigureWindow (screen->dpy (), popupWindow, valueMask, &xwc);
    }
}

void
SwitchScreen::createWindowList (int count)
{
    windows.clear ();

    foreach (CompWindow *w, screen->windows ())
    {
	SWITCH_WINDOW (w);

	if (sw->isSwitchWin ())
	{
	    windows.push_back (w);

	    sw->cWindow->damageRectSetEnabled (sw, true);
	}
    }

    windows.sort (BaseSwitchScreen::compareWindows);

    if (windows.size () == 2)
    {
	windows.push_back (windows.front ());
	windows.push_back ((*++windows.begin ()));
    }

    updateWindowList (count);
}

bool
SwitchWindow::damageRect (bool initial, const CompRect &rect)
{
    return BaseSwitchWindow::damageRect (initial, rect);
}

BaseSwitchWindow::IconMode
SwitchWindow::getIconMode ()
{
    if (sScreen->optionGetIconOnly ())
	return ShowIconOnly;
    if (!sScreen->optionGetIcon ())
	return HideIcon;

    return ShowIcon;
}

void
SwitchScreen::getMinimizedAndMatch (bool &minimizedOption,
				    CompMatch *&matchOption)
{
    minimizedOption = optionGetMinimized ();
    matchOption = &optionGetWindowMatch ();
}

bool
SwitchScreen::getMipmap ()
{
    return optionGetMipmap ();
}

void
SwitchScreen::switchToWindow (bool toNext)
{
    CompWindow *w =
	BaseSwitchScreen::switchToWindow (toNext, optionGetAutoRotate (), optionGetFocusOnSwitch ());
    if (w)
    {
	if (!zoomedWindow)
	    zoomedWindow = selectedWindow;
    }
}

void
SwitchScreen::handleSelectionChange (bool toNext, int nextIdx)
{
    if (toNext)
	move -= WIDTH;
    else
	move += WIDTH;

    moreAdjust = true;
}

int
SwitchScreen::countWindows ()
{
    int count = 0;

    foreach (CompWindow *w, screen->windows ())
	if (SwitchWindow::get (w)->isSwitchWin ())
	{
	    count++;
	    if (count == 5)
		break;
	}

    if (count == 5 && screen->width () <= WINDOW_WIDTH (5))
	count = 3;

    return count;
}

void
SwitchScreen::handleEvent (XEvent *event)
{
    BaseSwitchScreen::handleEvent (event);
}

void
SwitchScreen::initiate (SwitchWindowSelection selection,
			bool                  showPopup)
{
    int count;

    if (screen->otherGrabExist ("switcher", NULL))
	return;

    this->selection      = selection;
    selectedWindow       = NULL;

    count = countWindows ();
    if (count < 1)
	return;

    if (!popupWindow && showPopup)
    {
	Display		     *dpy = screen->dpy ();
	XSizeHints	     xsh;
	XWMHints	     xwmh;
	XClassHint           xch;
	Atom		     state[4];
	int		     nState = 0;
	XSetWindowAttributes attr;
	Visual		     *visual;

	visual = findArgbVisual (dpy, screen->screenNum ());
	if (!visual)
	    return;

	if (count > 1)
	{
	    count -= (count + 1) & 1;
	    if (count < 3)
		count = 3;
	}

	xsh.flags       = PSize | PPosition | PWinGravity;
	xsh.width       = WINDOW_WIDTH (count);
	xsh.height      = WINDOW_HEIGHT;
	xsh.win_gravity = StaticGravity;

	xwmh.flags = InputHint;
	xwmh.input = 0;

	xch.res_name  = (char *)"compiz";
	xch.res_class = (char *)"switcher-window";

	attr.background_pixel = 0;
	attr.border_pixel     = 0;
	attr.colormap	      = XCreateColormap (dpy, screen->root (), visual,
						 AllocNone);
	attr.override_redirect = true;

	popupWindow =
	    XCreateWindow (dpy, screen->root (),
			   screen->width () / 2 - xsh.width / 2,
			   screen->height () / 2 - xsh.height / 2,
			   (unsigned) xsh.width, (unsigned) xsh.height, 0,
			   32, InputOutput, visual,
			   CWBackPixel | CWBorderPixel | CWColormap | CWOverrideRedirect, &attr);

	XSetWMProperties (dpy, popupWindow, NULL, NULL,
			  programArgv, programArgc,
			  &xsh, &xwmh, &xch);

	state[nState++] = Atoms::winStateAbove;
	state[nState++] = Atoms::winStateSticky;
	state[nState++] = Atoms::winStateSkipTaskbar;
	state[nState++] = Atoms::winStateSkipPager;

	XChangeProperty (dpy, popupWindow,
			 Atoms::winState,
			 XA_ATOM, 32, PropModeReplace,
			 (unsigned char *) state, nState);

	XChangeProperty (dpy, popupWindow,
			 Atoms::winType,
			 XA_ATOM, 32, PropModeReplace,
			 (unsigned char *) &Atoms::winTypeUtil, 1);

	screen->setWindowProp (popupWindow, Atoms::winDesktop, 0xffffffff);

	setSelectedWindowHint (false);
    }

    if (!grabIndex)
	grabIndex = screen->pushGrab (screen->invisibleCursor (), "switcher");

    if (grabIndex)
    {
	if (!switching)
	{
	    lastActiveNum = screen->activeNum ();

	    createWindowList (count);

	    sTranslate = zoom;

	    if (popupWindow && showPopup)
	    {
		XMapWindow (screen->dpy (), popupWindow);

		setSelectedWindowHint (optionGetFocusOnSwitch ());
	    }

	    lastActiveWindow = screen->activeWindow ();
	    activateEvent (true);
	}

	cScreen->damageScreen ();

	switching  = true;
	moreAdjust = true;

	screen->handleEventSetEnabled (this, true);
	cScreen->preparePaintSetEnabled (this, true);
	cScreen->donePaintSetEnabled (this, true);
	gScreen->glPaintOutputSetEnabled (this, true);

	foreach (CompWindow *w, screen->windows ())
	{
	    SWITCH_WINDOW (w);

	    sw->gWindow->glPaintSetEnabled (sw, true);
	}
    }
}


static bool
switchTerminate (CompAction         *action,
	         CompAction::State  state,
	         CompOption::Vector &options)
{
    Window     xid;

    xid = (Window) CompOption::getIntOptionNamed (options, "root");

    if (action)
	action->setState (action->state () &
			  (unsigned)~(CompAction::StateTermKey |
			  	      CompAction::StateTermButton));

    if (xid && xid != screen->root ())
	return false;

    SWITCH_SCREEN (screen);

    if (ss->grabIndex)
    {
	if (ss->popupWindow)
	{
	    XUnmapWindow (screen->dpy (), ss->popupWindow);
	}

	ss->switching = false;

	if (state & CompAction::StateCancel)
	{
	    ss->selectedWindow = NULL;
	    ss->zoomedWindow   = NULL;

	    if (screen->activeWindow () != ss->lastActiveWindow)
	    {
		CompWindow *w = screen->findWindow (ss->lastActiveWindow);

		if (w)
		    w->moveInputFocusTo ();
	    }
	}

	if (state && ss->selectedWindow && !ss->selectedWindow->destroyed ())
	    screen->sendWindowActivationRequest (ss->selectedWindow->id ());

	screen->removeGrab (ss->grabIndex, 0);
	ss->grabIndex = NULL;

	if (!ss->popupWindow)
	    screen->handleEventSetEnabled (ss, false);

	if (!ss->zooming)
	{
	    ss->selectedWindow = NULL;
	    ss->zoomedWindow   = NULL;

	    ss->activateEvent (false);
	}
	else
	{
	    ss->moreAdjust = true;
	}

	ss->selectedWindow = NULL;
	ss->setSelectedWindowHint (false);

	ss->lastActiveNum = 0;

	ss->cScreen->damageScreen ();
    }


    return false;
}

static bool
switchInitiateCommon (CompAction            *action,
		      CompAction::State     state,
		      CompOption::Vector    &options,
		      SwitchWindowSelection selection,
		      bool                  showPopup,
		      bool                  nextWindow)
{
    Window     xid;

    xid = (Window) CompOption::getIntOptionNamed (options, "root");

    if (xid != screen->root ())
	return false;

    SWITCH_SCREEN (screen);

    if (!ss->switching)
    {
	ss->initiate (selection, showPopup);

	if (state & CompAction::StateInitKey)
	    action->setState (action->state () | CompAction::StateTermKey);

	if (state & CompAction::StateInitEdge)
	    action->setState (action->state () | CompAction::StateTermEdge);
	else if (state & CompAction::StateInitButton)
	    action->setState (action->state () | CompAction::StateTermButton);
    }

    ss->switchToWindow (nextWindow);

    return false;
}

void
SwitchScreen::windowRemove (CompWindow *w)
{
    if (w)
    {
	int count;

	CompWindow *selected;
	CompWindow *old;

	int allWindowsWidth;

	SWITCH_WINDOW (w);

	if (!sw->isSwitchWin (true))
	    return;

	sw->cWindow->damageRectSetEnabled (sw, false);
	sw->gWindow->glPaintSetEnabled (sw, false);

	old = selected = selectedWindow;

	CompWindowList::iterator it = std::find (windows.begin (),
						 windows.end (),
						 w);

	if (it == windows.end ())
	    return;

	if (w == selected)
	{
	    CompWindowList::iterator newSelected = it;

	    if (w == windows.back ())
		newSelected = windows.begin ();
	    else
		++newSelected;

	    selected = *newSelected;
	}

	windows.erase (it);
	count = windows.size ();

	if (count == 2)
	{
	    if (windows.front () == windows.back ())
	    {
		windows.pop_back ();
		count = 1;
	    }
	    else
	    {
		windows.push_back (windows.front ());
		windows.push_back (*++windows.begin ());
	    }
	}
	else if (count == 0)
	{
	    CompOption::Vector o (0);

	    o.push_back (CompOption ("root", CompOption::TypeInt));

	    o[0].value ().set ((int) screen->root ());

	    switchTerminate (NULL, 0, o);
	    return;
	}

	if (!grabIndex)
	    return;

	updateWindowList (count);

	allWindowsWidth = windows.size () * WIDTH;

	foreach (CompWindow *w, windows)
	{
	    selectedWindow = w;

	    if (selectedWindow == selected)
		break;

	    pos -= WIDTH;
	    if (pos < -allWindowsWidth)
		pos += allWindowsWidth;
	}

	if (popupWindow)
	{
	    CompWindow *popup;

	    popup = screen->findWindow (popupWindow);
	    if (popup)
		CompositeWindow::get (popup)->addDamage ();

	    setSelectedWindowHint (optionGetFocusOnSwitch ());
	}

	if (old != selectedWindow)
	{
	    zoomedWindow = selectedWindow;

	    CompositeWindow::get (selectedWindow)->addDamage ();
	    CompositeWindow::get (w)->addDamage ();

	    if (old && !old->destroyed ())
		CompositeWindow::get (old)->addDamage ();
	}
    }
}

bool
SwitchScreen::adjustVelocity ()
{
    float dx, adjust, amount;

    dx = move;

    adjust = dx * 0.15f;
    amount = fabs (dx) * 1.5f;
    if (amount < 0.2f)
	amount = 0.2f;
    else if (amount > 2.0f)
	amount = 2.0f;

    mVelocity = (amount * mVelocity + adjust) / (amount + 1.0f);

    if (zooming)
    {
	float dt, ds;

	if (switching)
	    dt = zoom - translate;
	else
	    dt = 0.0f - translate;

	adjust = dt * 0.15f;
	amount = fabs (dt) * 1.5f;
	if (amount < 0.2f)
	    amount = 0.2f;
	else if (amount > 2.0f)
	    amount = 2.0f;

	tVelocity = (amount * tVelocity + adjust) / (amount + 1.0f);

	if (selectedWindow == zoomedWindow)
	    ds = zoom - sTranslate;
	else
	    ds = 0.0f - sTranslate;

	adjust = ds * 0.5f;
	amount = fabs (ds) * 5.0f;
	if (amount < 1.0f)
	    amount = 1.0f;
	else if (amount > 6.0f)
	    amount = 6.0f;

	sVelocity = (amount * sVelocity + adjust) / (amount + 1.0f);

	if (selectedWindow == zoomedWindow)
	{
	    if (fabs (dx) < 0.1f   && fabs (mVelocity) < 0.2f   &&
		fabs (dt) < 0.001f && fabs (tVelocity) < 0.001f &&
		fabs (ds) < 0.001f && fabs (sVelocity) < 0.001f)
	    {
		mVelocity = tVelocity = sVelocity = 0.0f;
		return false;
	    }
	}
    }
    else
    {
	if (fabs (dx) < 0.1f  && fabs (mVelocity) < 0.2f)
	{
	    mVelocity = 0.0f;
	    return false;
	}
    }

    return true;
}

void
SwitchScreen::preparePaint (int msSinceLastPaint)
{
    if (moreAdjust)
    {
	int   steps, m;
	float amount, chunk;
	int   allWindowsWidth;

	allWindowsWidth = windows.size () * WIDTH;

	amount = msSinceLastPaint * 0.05f * optionGetSpeed ();
	steps  = amount / (0.5f * optionGetTimestep ());
	if (!steps) steps = 1;
	chunk  = amount / (float) steps;

	while (steps--)
	{
	    moreAdjust = adjustVelocity ();
	    if (!moreAdjust)
	    {
		pos += move;
		move = 0;

		if (zooming)
		{
		    if (switching)
		    {
			translate  = zoom;
			sTranslate = zoom;
		    }
		    else
		    {
			translate  = 0.0f;
			sTranslate = zoom;

			selectedWindow = NULL;
			zoomedWindow   = NULL;

			if (grabIndex)
			{
			    screen->removeGrab (grabIndex, 0);
			    grabIndex = 0;
			}

			activateEvent (false);
		    }
		}
		break;
	    }

	    m = mVelocity * chunk;
	    if (!m)
	    {
		if (mVelocity)
		    m = (move > 0) ? 1 : -1;
	    }

	    move -= m;
	    pos  += m;
	    if (pos < -allWindowsWidth)
		pos += allWindowsWidth;
	    else if (pos > 0)
		pos -= allWindowsWidth;

	    translate  += tVelocity * chunk;
	    sTranslate += sVelocity * chunk;

	    if (selectedWindow != zoomedWindow)
	    {
		if (sTranslate < 0.01f)
		    zoomedWindow = selectedWindow;
	    }
	}
    }

    cScreen->preparePaint (msSinceLastPaint);
}

bool
SwitchScreen::glPaintOutput (const GLScreenPaintAttrib &sAttrib,
			     const GLMatrix            &transform,
			     const CompRegion          &region,
			     CompOutput                *output,
			     unsigned int              mask)
{
    bool status;

    zoomMask = ZOOMED_WINDOW_MASK | NORMAL_WINDOW_MASK;

    if (grabIndex || (zooming && translate > 0.001f))
    {
	GLMatrix   sTransform (transform);
	CompWindow *zoomed;
	CompWindow *switcher;
	Window     zoomedAbove = None;

	if (zooming)
	{
	    mask &= (unsigned)~PAINT_SCREEN_REGION_MASK;
	    mask |= PAINT_SCREEN_TRANSFORMED_MASK | PAINT_SCREEN_CLEAR_MASK;

	    sTransform.translate (0.0f, 0.0f, -translate);

	    zoomMask = NORMAL_WINDOW_MASK;
	}

	if (optionGetBringToFront ())
	{
	    CompWindow *frontWindow = ::screen->clientList ().back ();

	    zoomed = zoomedWindow;
	    if (zoomed && !zoomed->destroyed () && zoomed != frontWindow)
	    {
		CompWindow *w;

		for (w = zoomed->prev; w && w->id () <= 1; w = w->prev)
		    ;
		zoomedAbove = (w) ? w->id () : None;

		screen->unhookWindow (zoomed);
		screen->insertWindow (zoomed, frontWindow->id ());
	    }
	    else
	    {
		zoomed = NULL;
	    }
	}
	else
	{
	    zoomed = NULL;
	}

	ignoreSwitcher = true;

	status = gScreen->glPaintOutput (sAttrib, sTransform, region, output,
					 mask);

	if (zooming)
	{
	    float zTranslate;

	    mask &= (unsigned)~PAINT_SCREEN_CLEAR_MASK;
	    mask |= PAINT_SCREEN_NO_BACKGROUND_MASK;

	    zoomMask = ZOOMED_WINDOW_MASK;

	    zTranslate = MIN (sTranslate, translate);
	    sTransform.translate (0.0f, 0.0f, zTranslate);

	    status = gScreen->glPaintOutput (sAttrib, sTransform, region,
					     output, mask);
	}

	if (zoomed)
	{
	    screen->unhookWindow (zoomed);
	    screen->insertWindow (zoomed, zoomedAbove);
	}

	ignoreSwitcher = false;

	switcher = screen->findWindow (popupWindow);

	if (switcher)
	{
	    SWITCH_WINDOW (switcher);

	    sTransform = transform;

	    sTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);

	    if (!switcher->destroyed () &&
		switcher->isViewable () &&
		sw->cWindow->damaged ())
	    {
		sw->gWindow->glPaint (sw->gWindow->paintAttrib (),
				      sTransform, infiniteRegion, 0);
	    }
	}
    }
    else
    {
	status = gScreen->glPaintOutput (sAttrib, transform, region, output,
					 mask);
    }

    return status;
}

void
SwitchScreen::donePaint ()
{
    if ((grabIndex || zooming) && moreAdjust)
    {
	if (zooming)
	{
	    cScreen->damageScreen ();
	}
	else
	{
	    CompWindow *w;

	    w = screen->findWindow (popupWindow);
	    if (w)
		CompositeWindow::get (w)->addDamage ();
	}
    }
    else if (!grabIndex && !(zooming && translate > 0.001f) && !moreAdjust)
    {
	cScreen->preparePaintSetEnabled (this, false);
	cScreen->donePaintSetEnabled (this, false);
	gScreen->glPaintOutputSetEnabled (this, false);

	foreach (CompWindow *w, screen->windows ())
	{
	    SWITCH_WINDOW (w);
	    sw->cWindow->damageRectSetEnabled (sw, false);
	    sw->gWindow->glPaintSetEnabled (sw, false);
	}
    }

    cScreen->donePaint ();
}

void
SwitchWindow::paintThumb (const GLWindowPaintAttrib &attrib,
			  const GLMatrix            &transform,
		          unsigned int              mask,
			  int                       x,
			  int                       y)
{
    BaseSwitchWindow::paintThumb (attrib,
    				  transform,
    				  mask,
    				  x,
    				  y,
    				  WIDTH  - (SPACE << 1),
    				  HEIGHT - (SPACE << 1),
    				  WIDTH  - (WIDTH  >> 2),
    				  HEIGHT - (HEIGHT >> 2));
}

void
SwitchWindow::updateIconTexturedWindow (GLWindowPaintAttrib  &sAttrib,
					int                  &wx,
					int                  &wy,
					int                  x,
					int                  y,
					GLTexture            *icon)
{
    sAttrib.xScale = (float) ICON_SIZE / icon->width ();
    sAttrib.yScale = (float) ICON_SIZE / icon->height ();
    if (sAttrib.xScale < sAttrib.yScale)
	sAttrib.yScale = sAttrib.xScale;
    else
	sAttrib.xScale = sAttrib.yScale;

    wx = x + WIDTH  - icon->width ()  * sAttrib.xScale - SPACE;
    wy = y + HEIGHT - icon->height () * sAttrib.yScale - SPACE;
}

void
SwitchWindow::updateIconNontexturedWindow (GLWindowPaintAttrib  &sAttrib,
					   int                  &wx,
					   int                  &wy,
					   float                &width,
					   float                &height,
					   int                  x,
					   int                  y,
					   GLTexture            *icon)
{
    float iw, ih;

    iw = width  - SPACE;
    ih = height - SPACE;

    sAttrib.xScale = iw / icon->width ();
    sAttrib.yScale = ih / icon->height ();

    if (sAttrib.xScale < sAttrib.yScale)
	sAttrib.yScale = sAttrib.xScale;
    else
	sAttrib.xScale = sAttrib.yScale;

    width  = icon->width ()  * sAttrib.xScale;
    height = icon->height () * sAttrib.yScale;

    wx = x + SPACE + ((WIDTH  - (SPACE << 1)) - width)  / 2;
    wy = y + SPACE + ((HEIGHT - (SPACE << 1)) - height) / 2;
}

void
SwitchWindow::updateIconPos (int   &wx,
			     int   &wy,
			     int   x,
			     int   y,
			     float width,
			     float height)
{
    wx = x + SPACE + ((WIDTH  - (SPACE << 1)) - width)  / 2;
    wy = y + SPACE + ((HEIGHT - (SPACE << 1)) - height) / 2;
}


/* Only for the popup window */
bool
SwitchWindow::managed ()
{
    return true;
}

bool
SwitchWindow::glPaint (const GLWindowPaintAttrib &attrib,
		       const GLMatrix            &transform,
		       const CompRegion          &region,
		       unsigned int              mask)
{
    GLVertexBuffer *streamingBuffer = GLVertexBuffer::streamingBuffer ();
    GLMatrix        wTransform (transform);
    int	       zoomType = NORMAL_WINDOW_MASK;
    bool       status;

    if (window->id () == sScreen->popupWindow)
    {
	int            x, y, x1, x2, cx, i;
	unsigned short color[4];

	CompWindow::Geometry &g = window->geometry ();

	if (mask & PAINT_WINDOW_OCCLUSION_DETECTION_MASK ||
	    sScreen->ignoreSwitcher)
	    return false;

	status = gWindow->glPaint (attrib, transform, region, mask);

	if (!(mask & PAINT_WINDOW_TRANSFORMED_MASK) && region.isEmpty ())
	    return true;

	x1 = g.x () + SPACE;
	x2 = g.x () + g.width () - SPACE;

	x = x1 + sScreen->pos;
	y = g.y () + SPACE;

	glEnable (GL_SCISSOR_TEST);
	glScissor (x1, 0, x2 - x1, screen->height ());

	foreach (CompWindow *w, sScreen->windows)
	{
	    if (x + WIDTH > x1)
		SwitchWindow::get (w)->paintThumb (gWindow->lastPaintAttrib (),
		                                   transform, mask, x, y);
	    x += WIDTH;
	}

	foreach (CompWindow *w, sScreen->windows)
	{
	    if (x > x2)
		break;

            SwitchWindow::get (w)->paintThumb (gWindow->lastPaintAttrib (),
	                                       transform, mask, x, y);
	    x += WIDTH;
	}

	glDisable (GL_SCISSOR_TEST);

	cx = g.x () + (g.width () >> 1);
	wTransform.translate (cx, y, 0.0f);

	glEnable (GL_BLEND);
	for (i = 0; i < 4; i++)
	{
	    color[i] = (unsigned int)sScreen->fgColor[i] *
		       gWindow->lastPaintAttrib ().opacity /
		       0xffff;
	}

	streamingBuffer->begin (GL_TRIANGLES);

	streamingBuffer->addColors (1, color);
	streamingBuffer->addVertices (24, _boxVertices);

	streamingBuffer->end ();
	streamingBuffer->render (wTransform, attrib);
	glDisable (GL_BLEND);
    }
    else if (window == sScreen->selectedWindow)
    {
	if (sScreen->optionGetBringToFront () &&
	    sScreen->selectedWindow == sScreen->zoomedWindow)
	    zoomType = ZOOMED_WINDOW_MASK;

	if (!(sScreen->zoomMask & zoomType))
	    return (mask & PAINT_WINDOW_OCCLUSION_DETECTION_MASK) ?
		false : true;

	status = gWindow->glPaint (attrib, transform, region, mask);
    }
    else if (sScreen->switching)
    {
	GLWindowPaintAttrib sAttrib (attrib);
	GLuint              value;

	value = (GLuint) sScreen->optionGetSaturation ();
	if (value != 100)
	    sAttrib.saturation = sAttrib.saturation * value / 100;

	value = (GLuint) sScreen->optionGetBrightness ();
	if (value != 100)
	    sAttrib.brightness = sAttrib.brightness * value / 100;

	if (window->wmType () & (unsigned)~(CompWindowTypeDockMask |
					    CompWindowTypeDesktopMask))
	{
	    value = (GLuint) sScreen->optionGetOpacity ();
	    if (value != 100)
		sAttrib.opacity = sAttrib.opacity * value / 100;
	}

	if (sScreen->optionGetBringToFront () &&
	    window == sScreen->zoomedWindow)
	    zoomType = ZOOMED_WINDOW_MASK;

	if (!(sScreen->zoomMask & zoomType))
	    return (mask & PAINT_WINDOW_OCCLUSION_DETECTION_MASK) ?
		false : true;

	status = gWindow->glPaint (sAttrib, transform, region, mask);
    }
    else
    {
	if (!(sScreen->zoomMask & zoomType))
	    return (mask & PAINT_WINDOW_OCCLUSION_DETECTION_MASK) ?
		false : true;

	status = gWindow->glPaint (attrib, transform, region, mask);
    }

    return status;
}

void
SwitchScreen::setZoom ()
{
    if (optionGetZoom () < 0.05f)
    {
	zooming = false;
	zoom    = 0.0f;
    }
    else
    {
	zooming = true;
	zoom    = optionGetZoom () / 30.0f;
    }

}

SwitchScreen::SwitchScreen (CompScreen *screen) :
    BaseSwitchScreen (screen),
    PluginClassHandler<SwitchScreen,CompScreen> (screen),
    lastActiveWindow (None),
    zoomedWindow (NULL),
    switching (false),
    zoomMask (~0),
    mVelocity (0.0),
    tVelocity (0.0),
    sVelocity (0.0),
    pos (0),
    move (0),
    translate (0.0),
    sTranslate (0.0)
{
    zoom = optionGetZoom () / 30.0f;

    zooming = (optionGetZoom () > 0.05f);

    optionSetZoomNotify (boost::bind (&SwitchScreen::setZoom, this));

#define SWITCHBIND(a,b,c) boost::bind (switchInitiateCommon, _1, _2, _3, a, b, c)

    optionSetNextButtonInitiate (SWITCHBIND (CurrentViewport, true, true));
    optionSetNextButtonTerminate (switchTerminate);
    optionSetNextKeyInitiate (SWITCHBIND (CurrentViewport, true, true));
    optionSetNextKeyTerminate (switchTerminate);
    optionSetPrevButtonInitiate (SWITCHBIND (CurrentViewport, true, false));
    optionSetPrevButtonTerminate (switchTerminate);
    optionSetPrevKeyInitiate (SWITCHBIND (CurrentViewport, true, false));
    optionSetPrevKeyTerminate (switchTerminate);

    optionSetNextAllButtonInitiate (SWITCHBIND (AllViewports, true, true));
    optionSetNextAllButtonTerminate (switchTerminate);
    optionSetNextAllKeyInitiate (SWITCHBIND (AllViewports, true, true));
    optionSetNextAllKeyTerminate (switchTerminate);
    optionSetPrevAllButtonInitiate (SWITCHBIND (AllViewports, true, false));
    optionSetPrevAllButtonTerminate (switchTerminate);
    optionSetPrevAllKeyInitiate (SWITCHBIND (AllViewports, true, false));
    optionSetPrevAllKeyTerminate (switchTerminate);

    optionSetNextNoPopupButtonInitiate (SWITCHBIND (CurrentViewport, false, true));
    optionSetNextNoPopupButtonTerminate (switchTerminate);
    optionSetNextNoPopupKeyInitiate (SWITCHBIND (CurrentViewport, false, true));
    optionSetNextNoPopupKeyTerminate (switchTerminate);
    optionSetPrevNoPopupButtonInitiate (SWITCHBIND (CurrentViewport, false, false));
    optionSetPrevNoPopupButtonTerminate (switchTerminate);
    optionSetPrevNoPopupKeyInitiate (SWITCHBIND (CurrentViewport, false, false));
    optionSetPrevNoPopupKeyTerminate (switchTerminate);

    optionSetNextPanelButtonInitiate (SWITCHBIND (Panels, false, true));
    optionSetNextPanelButtonTerminate (switchTerminate);
    optionSetNextPanelKeyInitiate (SWITCHBIND (Panels, false, true));
    optionSetNextPanelKeyTerminate (switchTerminate);
    optionSetPrevPanelButtonInitiate (SWITCHBIND (Panels, false, false));
    optionSetPrevPanelButtonTerminate (switchTerminate);
    optionSetPrevPanelKeyInitiate (SWITCHBIND (Panels, false, false));
    optionSetPrevPanelKeyTerminate (switchTerminate);

#undef SWITCHBIND

    ScreenInterface::setHandler (screen, false);
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);
}

SwitchScreen::~SwitchScreen ()
{
    if (popupWindow)
	XDestroyWindow (screen->dpy (), popupWindow);
}

SwitchWindow::SwitchWindow (CompWindow *window) :
    BaseSwitchWindow (dynamic_cast<BaseSwitchScreen *>
    		      (SwitchScreen::get (screen)), window),
    PluginClassHandler<SwitchWindow,CompWindow> (window),
    sScreen (SwitchScreen::get (screen))
{
    GLWindowInterface::setHandler (gWindow, false);
    CompositeWindowInterface::setHandler (cWindow, false);

    if (window->id () == sScreen->popupWindow)
	WindowInterface::setHandler (window, true);
    else
	WindowInterface::setHandler (window, false);

    if (sScreen->popupWindow && sScreen->popupWindow == window->id ())
	gWindow->glPaintSetEnabled (this, true);
}

bool
SwitchPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI) ||
        !CompPlugin::checkPluginABI ("compiztoolbox", COMPIZ_COMPIZTOOLBOX_ABI))
	 return false;

    return true;
}

