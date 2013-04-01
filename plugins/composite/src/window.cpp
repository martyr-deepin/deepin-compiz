/*
 * Copyright © 2008 Dennis Kasprzyk
 * Copyright © 2007 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Dennis Kasprzyk not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Dennis Kasprzyk makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DENNIS KASPRZYK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DENNIS KASPRZYK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Dennis Kasprzyk <onestone@compiz-fusion.org>
 *          David Reveman <davidr@novell.com>
 */

#include "privates.h"

template class PluginClassHandler<CompositeWindow, CompWindow, COMPIZ_COMPOSITE_ABI>;

CompositeWindow::CompositeWindow (CompWindow *w) :
    PluginClassHandler<CompositeWindow, CompWindow, COMPIZ_COMPOSITE_ABI> (w),
    priv (new PrivateCompositeWindow (w, this))
{
    CompScreen *s = screen;

    if (w->windowClass () != InputOnly)
    {
	priv->damage = XDamageCreate (s->dpy (), w->id (),
				      XDamageReportBoundingBox);
    }
    else
    {
	priv->damage = None;
    }

    priv->opacity = OPAQUE;
    if (!(w->type () & CompWindowTypeDesktopMask))
	priv->opacity = s->getWindowProp32 (w->id (),
					    Atoms::winOpacity, OPAQUE);

    priv->brightness = s->getWindowProp32 (w->id (),
					   Atoms::winBrightness, BRIGHT);

    priv->saturation = s->getWindowProp32 (w->id (),
					   Atoms::winSaturation, COLOR);

    if (w->isViewable ())
	priv->damaged = true;
}

CompositeWindow::~CompositeWindow ()
{

    if (priv->damage)
	XDamageDestroy (screen->dpy (), priv->damage);

     if (!priv->redirected)
    {
	priv->cScreen->overlayWindowCount ()--;

	if (priv->cScreen->overlayWindowCount () < 1)
	    priv->cScreen->showOutputWindow ();
    }

    release ();

    addDamage ();

    if (lastDamagedWindow == priv->window)
	lastDamagedWindow = NULL;

    delete priv;
}

void
PrivateCompositeWindow::setNewPixmapReadyCallback (const PixmapBinding::NewPixmapReadyCallback &cb)
{
    mPixmapBinding.setNewPixmapReadyCallback (cb);
}

void
CompositeWindow::setNewPixmapReadyCallback (const PixmapBinding::NewPixmapReadyCallback &cb)
{
    priv->setNewPixmapReadyCallback (cb);
}

void
PrivateCompositeWindow::allowFurtherRebindAttempts ()
{
    mPixmapBinding.allowFurtherRebindAttempts ();
}

PrivateCompositeWindow::PrivateCompositeWindow (CompWindow      *w,
						CompositeWindow *cw) :
    window (w),
    cWindow (cw),
    cScreen (CompositeScreen::get (screen)),
    mPixmapBinding (boost::function <void ()> (),
		     this,
		     this,
		     this,
		     screen->serverGrabInterface ()),
    damage (None),
    damaged (false),
    redirected (cScreen->compositingActive ()),
    overlayWindow (false),
    opacity (OPAQUE),
    brightness (BRIGHT),
    saturation (COLOR),
    damageRects (0),
    sizeDamage (0),
    nDamage (0)
{
    WindowInterface::setHandler (w);
}

PrivateCompositeWindow::~PrivateCompositeWindow ()
{

    if (sizeDamage)
	free (damageRects);
}



bool
PrivateCompositeWindow::bind ()
{
    return mPixmapBinding.bind ();
}

bool
CompositeWindow::bind ()
{
    if (!priv->cScreen->compositingActive ())
	return false;

    redirect ();
    return priv->bind ();
}

void
PrivateCompositeWindow::release ()
{
    mPixmapBinding.release ();
}

void
CompositeWindow::release ()
{
    return priv->release ();
}

Pixmap
PrivateCompositeWindow::pixmap () const
{
    return mPixmapBinding.pixmap ();
}

WindowPixmapInterface::Ptr
PrivateCompositeWindow::getPixmap ()
{
    Pixmap pixmap = XCompositeNameWindowPixmap (screen->dpy (), ROOTPARENT (window));
    WindowPixmapInterface::Ptr p (new X11WindowPixmap (screen->dpy (), pixmap));
    return p;
}

bool
PrivateCompositeWindow::getAttributes (XWindowAttributes &attr)
{
    if (XGetWindowAttributes (screen->dpy (),
			      ROOTPARENT (window), &attr))
	return true;

    return false;
}

bool
PrivateCompositeWindow::frozen ()
{
    /* keep old pixmap for windows that are unmapped on the client side,
     * but not yet on our side as it's pretty likely that plugins are
     * currently using it for animations
     */

    bool pendingUnmap = !window->mapNum () && window->isViewable ();
    bool hidden = window->state () & CompWindowStateHiddenMask;
    bool animated = window->hasUnmapReference ();

    return (pendingUnmap || hidden) && animated;
}

Pixmap
CompositeWindow::pixmap ()
{
    return priv->pixmap ();
}

const CompSize &
PrivateCompositeWindow::size () const
{
    return mPixmapBinding.size ();
}

const CompSize &
CompositeWindow::size ()
{
    return priv->size ();
}

void
CompositeWindow::redirect ()
{
    if (priv->redirected || !priv->cScreen->compositingActive ())
	return;

    XCompositeRedirectWindow (screen->dpy (),
			      ROOTPARENT (priv->window),
			      CompositeRedirectManual);

    priv->redirected = true;

    if (priv->overlayWindow)
    {
	priv->cScreen->overlayWindowCount ()--;
	priv->overlayWindow = false;
    }

    if (priv->cScreen->overlayWindowCount () < 1)
	priv->cScreen->showOutputWindow ();
    else
	priv->cScreen->updateOutputWindow ();
}

void
CompositeWindow::unredirect ()
{
    if (!priv->redirected || !priv->cScreen->compositingActive ())
	return;

    release ();

    priv->redirected   = false;
    priv->overlayWindow = true;
    priv->cScreen->overlayWindowCount ()++;

    if (priv->cScreen->overlayWindowCount () > 0)
	priv->cScreen->updateOutputWindow ();

    XCompositeUnredirectWindow (screen->dpy (),
                               ROOTPARENT (priv->window),
                               CompositeRedirectManual);


}

bool
CompositeWindow::redirected ()
{
    return priv->redirected;
}

bool
CompositeWindow::overlayWindow ()
{
    return priv->overlayWindow;
}

bool
CompositeWindow::frozen ()
{
    return priv->frozen ();
}

void
CompositeWindow::damageTransformedRect (float          xScale,
					float          yScale,
					float          xTranslate,
					float          yTranslate,
					const CompRect &rect)
{
    int x1, x2, y1, y2;

    x1 = (short) (rect.x1 () * xScale) - 1;
    y1 = (short) (rect.y1 () * yScale) - 1;
    x2 = (short) (rect.x2 () * xScale + 0.5f) + 1;
    y2 = (short) (rect.y2 () * yScale + 0.5f) + 1;

    x1 += (short) xTranslate;
    y1 += (short) yTranslate;
    x2 += (short) (xTranslate + 0.5f);
    y2 += (short) (yTranslate + 0.5f);

    if (x2 > x1 && y2 > y1)
    {
	const CompWindow::Geometry &geom = priv->window->serverGeometry ();

	x1 += geom.x () + geom.border ();
	y1 += geom.y () + geom.border ();
	x2 += geom.x () + geom.border ();
	y2 += geom.y () + geom.border ();

	priv->cScreen->damageRegion (CompRegion (CompRect (x1, y1, x2 - x1, y2 - y1)));
    }
}

void
CompositeWindow::damageOutputExtents ()
{
    if (priv->cScreen->damageMask () & COMPOSITE_SCREEN_DAMAGE_ALL_MASK)
	return;

    if (priv->window->shaded () ||
	(priv->window->isViewable ()))
    {
	int x1, x2, y1, y2;

	const CompWindow::Geometry &geom = priv->window->geometry ();
	const CompWindowExtents &output  = priv->window->output ();

	/* top */
	x1 = -output.left - geom.border ();
	y1 = -output.top - geom.border ();
	x2 = priv->window->size ().width () + output.right;
	y2 = -geom.border ();

	if (x1 < x2 && y1 < y2)
	    addDamageRect (CompRect (x1, y1, x2 - x1, y2 - y1));

	/* bottom */
	y1 = priv->window->size ().height ();
	y2 = y1 + output.bottom - geom.border ();

	if (x1 < x2 && y1 < y2)
	    addDamageRect (CompRect (x1, y1, x2 - x1, y2 - y1));

	/* left */
	x1 = -output.left - geom.border ();
	y1 = -geom.border ();
	x2 = -geom.border ();
	y2 = priv->window->size ().height ();

	if (x1 < x2 && y1 < y2)
	    addDamageRect (CompRect (x1, y1, x2 - x1, y2 - y1));

	/* right */
	x1 = priv->window->size ().width ();
	x2 = x1 + output.right - geom.border ();

	if (x1 < x2 && y1 < y2)
	    addDamageRect (CompRect (x1, y1, x2 - x1, y2 - y1));
    }
}

void
CompositeWindow::addDamageRect (const CompRect &rect)
{
    if (priv->cScreen->damageMask () & COMPOSITE_SCREEN_DAMAGE_ALL_MASK)
	return;

    if (!damageRect (false, rect))
    {
	int x, y;

	x = rect.x ();
	y = rect.y ();

	const CompWindow::Geometry &geom = priv->window->geometry ();
	x += geom.x () + geom.border ();
	y += geom.y () + geom.border ();

	priv->cScreen->damageRegion (CompRegion (CompRect (x, y,
							   rect.width (),
							   rect.height ())));
    }
}

void
CompositeWindow::addDamage (bool force)
{
    if (priv->cScreen->damageMask () & COMPOSITE_SCREEN_DAMAGE_ALL_MASK)
	return;

    if (priv->window->shaded () || force ||
	(priv->window->isViewable ()))
    {
	int    border = priv->window->serverGeometry ().border ();

	int x1 = -MAX (priv->window->output ().left,
		       priv->window->input ().left) - border;
	int y1 = -MAX (priv->window->output ().top,
		       priv->window->input ().top) - border;
	int x2 = priv->window->size ().width () +
		 MAX (priv->window->output ().right,
		      priv->window->input ().right) ;
	int y2 = priv->window->size ().height () +
		 MAX (priv->window->output ().bottom,
		      priv->window->input ().bottom) ;
	CompRect r (x1, y1, x2 - x1, y2 - y1);

	addDamageRect (r);
    }
}

bool
CompositeWindow::damaged ()
{
    return priv->damaged;
}

void
CompositeWindow::processDamage (XDamageNotifyEvent *de)
{
    if (priv->window->syncWait ())
    {
	if (priv->nDamage == priv->sizeDamage)
	{
	    priv->damageRects = (XRectangle *) realloc (priv->damageRects,
				 (priv->sizeDamage + 1) *
				 sizeof (XRectangle));
	    priv->sizeDamage += 1;
	}

	priv->damageRects[priv->nDamage].x      = de->area.x;
	priv->damageRects[priv->nDamage].y      = de->area.y;
	priv->damageRects[priv->nDamage].width  = de->area.width;
	priv->damageRects[priv->nDamage].height = de->area.height;
	priv->nDamage++;
    }
    else
    {
        priv->handleDamageRect (this, de->area.x, de->area.y,
				de->area.width, de->area.height);
    }
}

void
PrivateCompositeWindow::handleDamageRect (CompositeWindow *w,
					  int             x,
					  int             y,
					  int             width,
					  int             height)
{
    bool   initial = false;

    if (!w->priv->redirected)
	return;

    if (!w->priv->damaged)
    {
	w->priv->damaged = initial = true;
    }

    if (!w->damageRect (initial, CompRect (x, y, width, height)))
    {
	const CompWindow::Geometry &geom = w->priv->window->geometry ();

	x += geom.x () + geom.border ();
	y += geom.y () + geom.border ();

	w->priv->cScreen->damageRegion (CompRegion (CompRect (x, y, width, height)));
    }

    if (initial)
	w->damageOutputExtents ();
}

void
CompositeWindow::updateOpacity ()
{
    unsigned short opacity;

    if (priv->window->type () & CompWindowTypeDesktopMask)
	return;

    opacity = screen->getWindowProp32 (priv->window->id (),
					     Atoms::winOpacity, OPAQUE);

    if (opacity != priv->opacity)
    {
	priv->opacity = opacity;
	addDamage ();
    }
}

void
CompositeWindow::updateBrightness ()
{
    unsigned short brightness;

    brightness = screen->getWindowProp32 (priv->window->id (),
						Atoms::winBrightness, BRIGHT);

    if (brightness != priv->brightness)
    {
	priv->brightness = brightness;
	addDamage ();
    }
}

void
CompositeWindow::updateSaturation ()
{
    unsigned short saturation;

    saturation = screen->getWindowProp32 (priv->window->id (),
						Atoms::winSaturation, COLOR);

    if (saturation != priv->saturation)
    {
	priv->saturation = saturation;
	addDamage ();
    }
}

unsigned short
CompositeWindow::opacity ()
{
    return priv->opacity;
}

unsigned short
CompositeWindow::brightness ()
{
    return priv->brightness;
}

unsigned short
CompositeWindow::saturation ()
{
    return priv->saturation;
}

bool
CompositeWindow::damageRect (bool           initial,
			     const CompRect &rect)
{
    WRAPABLE_HND_FUNCTN_RETURN (bool, damageRect, initial, rect)
    return false;
}

void
PrivateCompositeWindow::windowNotify (CompWindowNotify n)
{
    switch (n)
    {
	case CompWindowNotifyMap:
	    allowFurtherRebindAttempts ();
	    damaged = false;
	    break;
	case CompWindowNotifyUnmap:
	    cWindow->addDamage (true);
	    cWindow->release ();

	    if (!redirected && cScreen->compositingActive ())
		cWindow->redirect ();
	    break;
	case CompWindowNotifyRestack:
	case CompWindowNotifyHide:
	case CompWindowNotifyShow:
	case CompWindowNotifyAliveChanged:
	    cWindow->addDamage (true);
	    break;
	case CompWindowNotifyReparent:
	case CompWindowNotifyUnreparent:
	    if (redirected)
	    {
		cWindow->release ();
	    }
	    cScreen->damageScreen ();
	    cWindow->addDamage (true);
	    break;
	case CompWindowNotifyFrameUpdate:
	    cWindow->release ();
	    break;
	case CompWindowNotifySyncAlarm:
	{
	    XRectangle *rects;

	    rects   = damageRects;
	    while (nDamage--)
	    {
		PrivateCompositeWindow::handleDamageRect (cWindow,
							  rects[nDamage].x,
							  rects[nDamage].y,
							  rects[nDamage].width,
							  rects[nDamage].height);
	    }
	    break;
	}
	default:
	    break;
    }

    window->windowNotify (n);
}

void
PrivateCompositeWindow::resizeNotify (int dx, int dy, int dwidth, int dheight)
{
    window->resizeNotify (dx, dy, dwidth, dheight);

    if (window->shaded () || (window->isViewable ()))
    {
	int x, y, x1, x2, y1, y2;

	x = window->geometry ().x ();
	y = window->geometry ().y ();

	x1 = x - window->output ().left - dx;
	y1 = y - window->output ().top - dy;
	x2 = x + window->size ().width () +
	     window->output ().right - dx - dwidth;
	y2 = y + window->size ().height () +
	     window->output ().bottom - dy - dheight;

	cScreen->damageRegion (CompRegion (CompRect (x1, y1, x2 - x1, y2 - y1)));
    }

    cWindow->release ();
    cWindow->addDamage ();
}

void
PrivateCompositeWindow::moveNotify (int dx, int dy, bool now)
{
    if (window->shaded () || (window->isViewable ()))
    {
	int x, y, x1, x2, y1, y2;

	x = window->geometry ().x ();
	y = window->geometry ().y ();

	x1 = x - window->output ().left - dx;
	y1 = y - window->output ().top - dy;
	x2 = x + window->geometry ().width () +
	     window->output ().right - dx;
	y2 = y + window->geometry ().height () +
	     window->output ().bottom - dy;

	cScreen->damageRegion (CompRegion (CompRect (x1, y1, x2 - x1, y2 - y1)));
    }
    cWindow->addDamage ();

    window->moveNotify (dx, dy, now);
}

bool
CompositeWindowInterface::damageRect (bool initial, const CompRect &rect)
    WRAPABLE_DEF (damageRect, initial, rect)
