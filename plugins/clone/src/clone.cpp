/*
 * Copyright © 2006 Novell, Inc.
 *
 * clone.cpp
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
 * Ported to Compiz 0.9 by:
 * Copyright (c) 2009 Sam Spilsbury <smspillaz@gmail.com>
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include "clone.h"

COMPIZ_PLUGIN_20090315 (clone, ClonePluginVTable);

static void togglePaintFunctions (CloneScreen *cs, bool enabled)
{
    screen->handleEventSetEnabled (cs, enabled);
    cs->cScreen->preparePaintSetEnabled (cs, enabled);
    cs->gScreen->glPaintOutputSetEnabled (cs, enabled);
    cs->cScreen->donePaintSetEnabled (cs, enabled);

    foreach (CompWindow *w, screen->windows ())
    {
	CLONE_WINDOW (w);

	cw->gWindow->glPaintSetEnabled (cw, enabled);
    }
}

void
CloneScreen::finish ()
{
    grab = false;

    if (src != dst)
    {
	Clone *fClone = NULL;
	/* check if we should replace current clone */
	foreach (Clone *iClone, clones)
	{
	    if (iClone->dst == dst)
	    {
		fClone = iClone;
		break;
	    }
	}

	/* no existing clone for this destination, we must allocate one */
	if (!fClone)
	{
	    fClone = new Clone ();

	    XSetWindowAttributes attr;
	    int	 x, y;

	    attr.override_redirect = true;

	    x = (int) screen->outputDevs ()[dst].x1 ();
	    y = (int) screen->outputDevs ()[dst].y1 ();

	    fClone->input = 			
		    XCreateWindow (screen->dpy (),
				   screen->root (), x, y,
				   (int) screen->outputDevs ()[dst].width (),
				   (int) screen->outputDevs ()[dst].height (),
				   0, 0, InputOnly, CopyFromParent,
				   CWOverrideRedirect, &attr);
		    XMapRaised (screen->dpy (), fClone->input);

	    clones.push_back (fClone);
	}

	if (fClone)
	{
	    fClone->src = src;
	    fClone->dst = dst;
	}
    }

    if (grabbedOutput != dst)
    {
	/* remove clone */
	foreach (Clone *iClone, clones)
	{
	    if (iClone->dst == grabbedOutput)
	    {
		XDestroyWindow (screen->dpy (), iClone->input);
		clones.remove (iClone);
		delete iClone;
		break;
	    }
	}
    }
}

void
CloneScreen::preparePaint (int msSinceLastPaint)
{
    if (grab)
    {
	if (grabHandle)
	{
	    offset -= msSinceLastPaint * 0.005f;
	    if (offset < 0.0f)
		offset = 0.0f;
	}
	else
	{
	    offset += msSinceLastPaint * 0.005f;
	    if (offset >= 1.0f)
		offset = 1.0f;
	}
    }

    cScreen->preparePaint (msSinceLastPaint);

    foreach (Clone *iClone, clones)
    {
	CompOutput *srcOutput = &(screen->outputDevs () [iClone->src]);
	CompOutput *dstOutput = &(screen->outputDevs () [iClone->dst]);
	CompRegion dstOutputRegion (*dstOutput);
	CompRegion srcOutputRegion (*srcOutput);
	int	   dx, dy;

	dx = dstOutput->x1 () - srcOutput->x1 ();
	dy = dstOutput->y1 () - srcOutput->y1 ();

	if (cScreen->damageMask () & COMPOSITE_SCREEN_DAMAGE_REGION_MASK)
	{
	    if (srcOutput->width () != dstOutput->width () ||
	        srcOutput->height () != dstOutput->height ())
	    {
		cScreen->damageRegion (dstOutputRegion);
		iClone->region = srcOutputRegion;		
	    }
	    else
	    {
	    	CompRegion currentDamageRegion = cScreen->currentDamage ();
		iClone->region = currentDamageRegion - dstOutputRegion;
		iClone->region.translate (dx, dy);
		currentDamageRegion = iClone->region + currentDamageRegion;
		cScreen->damageRegion (currentDamageRegion);
		iClone->region = currentDamageRegion - srcOutputRegion;
		iClone->region.translate (-dx, -dy);
	    }
	}
	else
	{
	    iClone->region = srcOutputRegion;
	}
    }
}

void
CloneScreen::donePaint ()
{
    if (grab)
    {
	if (offset == 1.0f)
	    finish ();

	cScreen->damageScreen ();
    }

    cScreen->donePaint ();

    if (!grab && clones.empty ())
        togglePaintFunctions (this, false);
}

bool
CloneScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
		            const GLMatrix		   &transform,
		            const CompRegion	   &region,
		            CompOutput	   *output,
		            unsigned int mask)

{
    bool status;
    unsigned int  dstForThisOutput, outputId = 0;
    CompRegion sRegion = region;

    dstForThisOutput = outputId =
    			 ((unsigned int) output->id () != (unsigned int) ~0) ?
    							      output->id () : 0;

    if (!grab || (unsigned int) grabbedOutput != outputId)
    {
	foreach (Clone *iClone, clones)
	{
	    if ((unsigned int) iClone->dst == outputId)
	    {
		sRegion = iClone->region;
		dstForThisOutput    = (unsigned int) iClone->src;

		if (screen->outputDevs ()[dstForThisOutput].width ()  !=
		    screen->outputDevs ()[outputId].width () ||
		    screen->outputDevs ()[dstForThisOutput].height () != 
		    screen->outputDevs ()[outputId].height ())
		    transformed = true;
		else
		    transformed = false;

		break;
	    }
	}
    }

    if (output->id () != (unsigned int) ~0)
	status = gScreen->glPaintOutput (attrib, transform, sRegion,
				&screen->outputDevs ()[dstForThisOutput], mask);
    else
	status = 
	    gScreen->glPaintOutput (attrib, transform, sRegion, output, mask);

    if (grab)
    {
	GLMatrix      sTransform = transform;
	GLenum	      filter;
	float         zoom1, zoom2x, zoom2y, x1, y1, x2, y2;
	float         zoomX, zoomY;
	int           dx, dy;

	zoom1 = 160.0f / screen->outputDevs ()[src].height ();

	x1 = x - (screen->outputDevs ()[src].x1 () * zoom1);
	y1 = y - (screen->outputDevs ()[src].y1 () * zoom1);

	x1 -= (screen->outputDevs ()[src].width ()  * zoom1) / 2;
	y1 -= (screen->outputDevs ()[src].height () * zoom1) / 2;

	if (grabHandle)
	{
	    x2 = screen->outputDevs ()[grabbedOutput].x1 () -
		screen->outputDevs ()[src].x1 ();
	    y2 = screen->outputDevs ()[grabbedOutput].y1 () -
		screen->outputDevs ()[src].y1 ();

	    zoom2x = (float) screen->outputDevs ()[grabbedOutput].width () /
		screen->outputDevs ()[src].width ();
	    zoom2y = (float) screen->outputDevs ()[grabbedOutput].height () /
		screen->outputDevs ()[src].height ();
	}
	else
	{
	    x2 = screen->outputDevs ()[dst].x1 () -
		screen->outputDevs ()[src].x1 ();
	    y2 = screen->outputDevs ()[dst].y1 () -
		screen->outputDevs ()[src].y1 ();

	    zoom2x = (float) screen->outputDevs ()[dst].width () /
		screen->outputDevs ()[src].width ();
	    zoom2y = (float) screen->outputDevs ()[dst].height () /
		screen->outputDevs ()[src].height ();
	}

	/* XXX: hmm.. why do I need this.. */
	if (x2 < 0.0f)
	    x2 *= zoom2x;
	if (y2 < 0.0f)
	    y2 *= zoom2y;

	dx = x1 * (1.0f - offset) + x2 * offset;
	dy = y1 * (1.0f - offset) + y2 * offset;

	zoomX = zoom1 * (1.0f - offset) + zoom2x * offset;
	zoomY = zoom1 * (1.0f - offset) + zoom2y * offset;

	sTransform.translate (-0.5f, -0.5f, -DEFAULT_Z_CAMERA);
	sTransform.scale (1.0f  / screen->outputDevs ()[outputId].width (),
		     -1.0f / screen->outputDevs ()[outputId].height (),
		     1.0f);
	sTransform.translate (dx - screen->outputDevs ()[outputId].x1 (),
			 dy - screen->outputDevs ()[outputId].y2 (),
			 0.0f);
	sTransform.scale (zoomX, zoomY, 1.0f);

	filter = gScreen->textureFilter ();

	if (offset == 0.0f)
	    gScreen->setTextureFilter (GL_LINEAR_MIPMAP_LINEAR);

	CompRegion srcOutputRegion (screen->outputDevs ()[src]);

	foreach (CompWindow *w, screen->windows ())
	{
	    GLMatrix gTransform = transform;
	    gTransform.translate (-100, 0, 0);
	    CLONE_WINDOW (w);
	    if (w->destroyed ())
		continue;

	    if (!w->shaded ())
	    {
		if (!w->isViewable () || !cw->cWindow->damaged ())
		    continue;
	    }
	    
	    cw->gWindow->glPaint (cw->gWindow->paintAttrib (), sTransform,
				  srcOutputRegion,
				  PAINT_WINDOW_ON_TRANSFORMED_SCREEN_MASK);
	}

	gScreen->setTextureFilter (filter);
    }

    return status;
}

bool
CloneWindow::glPaint (const GLWindowPaintAttrib &attrib,
		      const GLMatrix		&transform,
		      const CompRegion		&region,
		      unsigned int		mask)
{
    CLONE_SCREEN (screen);

    if (!cs->clones.empty () && cs->transformed)
	mask |= PAINT_WINDOW_ON_TRANSFORMED_SCREEN_MASK;

    return gWindow->glPaint (attrib, transform, region, mask);

}

bool
CloneScreen::initiate (CompAction         *action,
		       CompAction::State  state,
		       CompOption::Vector options)
{
    std::list <Clone *>::iterator it = clones.begin ();
    if (grab || screen->otherGrabExist ("clone", NULL))
        return false;

    if (!grabHandle)
        grabHandle = screen->pushGrab (None, "clone");

    grab = true;

    x = CompOption::getIntOptionNamed (options, "x", 0);
    y = CompOption::getIntOptionNamed (options, "y", 0);

    src = grabbedOutput = screen->outputDeviceForPoint (x, y);

    /* trace source */
    while (it != clones.end ())
    {
        if ((*it)->dst == src)
        {
	    src = (*it)->src;
	    it = clones.begin ();
        }
        else
        {
	    ++it;
        }
    }

    togglePaintFunctions (this, true);

    if (state & CompAction::StateInitButton)
        action->setState (action->state () | CompAction::StateTermButton);

   return true;

}

bool
CloneScreen::terminate (CompAction         *action,
		        CompAction::State  state,
		        CompOption::Vector options)
{
    if (grabHandle)
    {
        int	x, y;

        screen->removeGrab (grabHandle, NULL);
	grabHandle = NULL;

        x = CompOption::getIntOptionNamed (options, "x", 0);
        y = CompOption::getIntOptionNamed (options, "y", 0);

        dst = screen->outputDeviceForPoint (x, y);

        cScreen->damageScreen ();
    }

    action->setState (action->state () & ~(CompAction::StateTermKey |
					   CompAction::StateTermButton));

    return false;
}

void
CloneScreen::setStrutsForCloneWindow (Clone *clone)
{
    CompOutput *output = &screen->outputDevs ()[clone->dst];
    XRectangle *rect = NULL;
    CompStruts *struts;
    CompStruts *wStruts;
    CompWindow *w;

    w = screen->findWindow (clone->input);
    if (!w)
	return;

    struts = new CompStruts ();
    if (!struts)
	return;

    wStruts = w->struts ();
    if (wStruts)
	delete wStruts;

    struts->left.x	= 0;
    struts->left.y	= 0;
    struts->left.width  = 0;
    struts->left.height = screen->height ();

    struts->right.x      = screen->width ();
    struts->right.y      = 0;
    struts->right.width  = 0;
    struts->right.height = screen->height ();

    struts->top.x      = 0;
    struts->top.y      = 0;
    struts->top.width  = screen->width ();
    struts->top.height = 0;

    struts->bottom.x      = 0;
    struts->bottom.y      = screen->height ();
    struts->bottom.width  = screen->width ();
    struts->bottom.height = 0;

    /* create struts relative to a screen edge that this output is next to */
    if (output->x1 () == 0)
	rect = &struts->left;
    else if (output->x2 () == screen->width ())
	rect = &struts->right;
    else if (output->y1 () == 0)
	rect = &struts->top;
    else if (output->y2 () == screen->height ())
	rect = &struts->bottom;

    if (rect)
    {
	rect->x	     = output->x1 ();
	rect->y	     = output->y1 ();
	rect->width  = output->width ();
	rect->height = output->height ();
    }

    wStruts = struts;
}

void
CloneScreen::handleMotionEvent (CompPoint &p)
{
    if (grabHandle)
    {
	x = p.x ();
	y = p.y ();

	cScreen->damageScreen ();
    }
}

void
CloneScreen::handleEvent (XEvent *event)
{
    switch (event->type)
    {
	case MotionNotify:
	{
	    CompPoint p (pointerX, pointerY);
	    handleMotionEvent (p);
	}
	break;
	case EnterNotify:
	case LeaveNotify:
	{
	    CompPoint p (pointerX, pointerY);
	    handleMotionEvent (p);
	}
	default:
	    break;

    }

    screen->handleEvent (event);

    switch (event->type)
    {
	case CreateNotify:
	{
	    foreach (Clone *iClone, clones)
	    {
		if (event->xcreatewindow.window == iClone->input)
		    setStrutsForCloneWindow (iClone);
	    }
	}
	default:
	    break;
    }
}

void
CloneScreen::outputChangeNotify ()
{
    std::list <Clone *>::iterator it;

    for (it = clones.begin (); it != clones.end (); ++it)
    {
	if ((unsigned int) (*it)->dst >= screen->outputDevs ().size () ||
	    (unsigned int) (*it)->src >= screen->outputDevs ().size ())
	{
	    clones.erase (it);
	    it = clones.begin ();
	    continue;
	}
    }

    screen->outputChangeNotify ();
}

CloneScreen::CloneScreen (CompScreen *screen) :
    PluginClassHandler <CloneScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    grabHandle (NULL),
    grab (false),
    offset (1.0f),
    transformed (false),
    src (0)
{
    ScreenInterface::setHandler (screen, false);
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);

    optionSetInitiateButtonInitiate (boost::bind (&CloneScreen::initiate, this,
						  _1, _2, _3));
    optionSetInitiateButtonTerminate (boost::bind (&CloneScreen::terminate, this,
						   _1, _2, _3));
}

CloneWindow::CloneWindow (CompWindow *window) :
    PluginClassHandler <CloneWindow, CompWindow> (window),
    cWindow (CompositeWindow::get (window)),
    gWindow (GLWindow::get (window))
{
    GLWindowInterface::setHandler (gWindow, false);
}
    
CloneScreen::~CloneScreen ()
{
    while (!clones.empty ())
    {
	clones.pop_front ();
    }
}

bool
ClonePluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
	!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

   return true;
}
