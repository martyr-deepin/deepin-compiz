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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include <core/abiversion.h>
#include <decoration.h>
#include "decor.h"

#include <X11/Xatom.h>
#include <X11/extensions/shape.h>

COMPIZ_PLUGIN_20090315 (decor, DecorPluginVTable)

MatchedDecorClipGroup::MatchedDecorClipGroup (const CompMatch &match) :
    mMatch (match)
{
}

bool
MatchedDecorClipGroup::doPushClippable (DecorClippableInterface *dc)
{
    if (dc->matches (mMatch))
	return mClipGroupImpl.pushClippable (dc);

    return false;
}

void
DecorWindow::doUpdateShadow (const CompRegion &reg)
{
    shadowRegion = outputRegion () - (reg - inputRegion ());
}

void
DecorWindow::doSetOwner (DecorClipGroupInterface *i)
{
    mClipGroup = i;
}

bool
DecorWindow::doMatches (const CompMatch &m)
{
    return const_cast <CompMatch &> (m).evaluate (window) && !window->invisible ();
}

const CompRegion &
DecorWindow::getOutputRegion ()
{
    return mOutputRegion;
}

const CompRegion &
DecorWindow::getInputRegion ()
{
    return mInputRegion;
}

void
DecorWindow::doUpdateGroupShadows ()
{
    if (mClipGroup)
	mClipGroup->updateAllShadows ();
}

/* From core */

/*
 * DecorWindow::glDraw
 *
 * Handles the drawing of the actual decoration texture
 * (whether that be the bound pixmap for a window type
 *  decoration or a redirected scaled-quad pixmap for
 *  pixmap type decorations).
 *
 * For pixmap type decorations, we need to use the precomputed
 * shadow region for the clipping region of when we draw the
 * window decoration with glDrawTexture. We also need to add
 * geometry for each quad on the pixmap decoration so that it
 * stretches from its original size to the size of the actual
 * window.
 *
 * Usually, this works something like this
 *
 * -----------------------------------
 * | Q1 | Buttons ... Text (Q2) | Q3 |
 * -----------------------------------
 * | Q4 |                       | Q5 |
 * -----------------------------------
 * | Q6 |         Q7            | Q8 |
 * -----------------------------------
 *
 *
 * ---------------------------------------------------
 * | Q1 | Buttons ... Text (Q2)                 | Q3 |
 * ---------------------------------------------------
 * | Q4 |                                       | Q5 |
 * |    |                                       |    |
 * |    |                                       |    |
 * |    |                                       |    |
 * |    |                                       |    |
 * |    |                                       |    |
 * |    |                                       |    |
 * |    |                                       |    |
 * |    |                                       |    |
 * |    |                                       |    |
 * ---------------------------------------------------
 * | Q6 |         Q7                            | Q8 |
 * ---------------------------------------------------
 *
 * Note here that quads 2, 4, 5 and 7 are scaled according
 * to the matrix supplied on each quad structure.
 *
 * FIXME: This doesn't work with multiple textures, that
 * needs work to determine the quads for each separate texture
 *
 * For window type decorations we add geometry for the frame
 * region of the window.
 *
 * Next, we draw the actual texture with the saved window geometry
 *
 */

bool
DecorWindow::glDraw (const GLMatrix     &transform,
		     const GLWindowPaintAttrib &attrib,
		     const CompRegion   &region,
		     unsigned int       mask)
{
    bool status;

    status = gWindow->glDraw (transform, attrib, region, mask);

    /* Don't render dock decorations (shadows) on just any old window */
    if (!(window->type () & CompWindowTypeDockMask))
    {
	glDecorate (transform, attrib, region, mask);
	/* Render dock decorations (shadows) on desktop windows only */
	if (window->type () & CompWindowTypeDesktopMask)
	{
	    foreach (CompWindow *w, dScreen->cScreen->getWindowPaintList ())
	    {
		bool isDock = w->type () & CompWindowTypeDockMask;
		bool drawShadow = !(w->invisible () || w->destroyed ());

		if (isDock && drawShadow)
		{
		    DecorWindow *d = DecorWindow::get (w);

		    /* If the last mask was an occlusion pass, glPaint
		     * return value will mean something different, so
		     * remove it */
		    unsigned int pmask = d->gWindow->lastMask () &
				~(PAINT_WINDOW_OCCLUSION_DETECTION_MASK);

		    /* Check if the window would draw by seeing if glPaint
		     * returns true when using PAINT_NO_CORE_INSTANCE_MASK
		     */
		    pmask |= PAINT_WINDOW_NO_CORE_INSTANCE_MASK;

		    const GLWindowPaintAttrib &pAttrib (d->gWindow->paintAttrib ());

		    if (d->gWindow->glPaint (pAttrib,
					     transform,
					     region,
					     pmask))
			d->glDecorate (transform, pAttrib, region, mask);
		}
	    }
	}
    }

    return status;
}

void
DecorWindow::glDecorate (const GLMatrix     &transform,
			 const GLWindowPaintAttrib &attrib,
		         const CompRegion   &region,
		         unsigned int       mask)
{
    if (wd &&
	wd->decor->type == WINDOW_DECORATION_TYPE_PIXMAP)
    {
	CompRect box;
	GLTexture::MatrixList ml (1);
	mask |= PAINT_WINDOW_BLEND_MASK;

	gWindow->vertexBuffer ()->begin ();
	const CompRegion *preg = NULL;

	if ((mask & (PAINT_WINDOW_ON_TRANSFORMED_SCREEN_MASK |
		     PAINT_WINDOW_WITH_OFFSET_MASK)))
	    preg = &region;
	else if (mask & PAINT_WINDOW_TRANSFORMED_MASK)
	    preg = &infiniteRegion;
	else if (mClipGroup)
	{
	    tmpRegion = mOutputRegion;
	    tmpRegion &= region;
	    tmpRegion &= shadowRegion;
	    preg = &tmpRegion;
	}
	else
	    preg = &region;

	/* In case some plugin needs to paint us with an offset region */
	if (preg->isEmpty ())
	    preg = &region;

	const CompRegion &reg (*preg);

	if (updateMatrix)
	    updateDecorationScale ();

	for (int i = 0; i < wd->nQuad; i++)
	{
	    box.setGeometry (wd->quad[i].box.x1,
			     wd->quad[i].box.y1,
			     wd->quad[i].box.x2 - wd->quad[i].box.x1,
			     wd->quad[i].box.y2 - wd->quad[i].box.y1);

	    if (box.width () > 0 && box.height () > 0)
	    {
		ml[0] = wd->quad[i].matrix;
		const CompRegionRef boxRegion (box.region ());
		gWindow->glAddGeometry (ml, boxRegion, reg);
	    }
	}

	if (gWindow->vertexBuffer ()->end ())
	{
	    glEnable (GL_BLEND);
	    gWindow->glDrawTexture (wd->decor->texture->textures[0], transform,
				    attrib, mask);
	    glDisable (GL_BLEND);
	}
    }
    else if (wd && wd->decor->type == WINDOW_DECORATION_TYPE_WINDOW)
    {
	GLTexture::MatrixList ml (1);

	if (gWindow->textures ().empty ())
	    gWindow->bind ();
	if (gWindow->textures ().empty ())
	    return;

	if (updateMatrix)
	    updateDecorationScale ();

	glEnable (GL_BLEND);

	if (gWindow->textures ().size () == 1)
	{
	    ml[0] = gWindow->matrices ()[0];
	    gWindow->vertexBuffer ()->begin ();
	    gWindow->glAddGeometry (ml, window->frameRegion (), region);
	    if (gWindow->vertexBuffer ()->end ())
		gWindow->glDrawTexture (gWindow->textures ()[0], transform,
		                        attrib, mask);
	}
	else
	{
	    if (updateReg)
		updateWindowRegions ();
	    for (unsigned int i = 0; i < gWindow->textures ().size (); i++)
	    {
		ml[0] = gWindow->matrices ()[i];
		gWindow->vertexBuffer ()->begin ();
		gWindow->glAddGeometry (ml, regions[i], region);
		if (gWindow->vertexBuffer ()->end ())
		    gWindow->glDrawTexture (gWindow->textures ()[i], transform,
		                            attrib, mask);
	    }
	}

	glDisable (GL_BLEND);
    }
}

static bool bindFailed;
/*
 * DecorTexture::DecorTexture
 *
 * Creates a texture for a given pixmap in the property.
 * bindFailed is a global variable used to determine if
 * binding to a pixmap gave us more than one texture since
 * FIXME that isn't supported yet. Also sets damage handlers
 * for this pixmap
 */

DecorTexture::DecorTexture (DecorPixmapInterface::Ptr pixmap) :
    status (true),
    refCount (1),
    pixmap (pixmap),
    damage (None)
{
    unsigned int width, height, depth, ui;
    Window	 root;
    int		 i;

    if (!XGetGeometry (screen->dpy (), pixmap->getPixmap (), &root,
		       &i, &i, &width, &height, &ui, &depth))
    {
	status = false;
	return;
    }

    bindFailed = false;
    textures = GLTexture::bindPixmapToTexture (pixmap->getPixmap (),
					       width, height, depth,
					       compiz::opengl::ExternallyManaged);
    if (textures.size () != 1)
    {
	bindFailed = true;
	status = false;
	return;
    }

    if (!DecorScreen::get (screen)->optionGetMipmap ())
	textures[0]->setMipmap (false);

    damage = XDamageCreate (screen->dpy (), pixmap->getPixmap (),
			     XDamageReportBoundingBox);
}

/*
 * DecorTexture::~DecorTexture
 *
 * Remove damage handle on texture
 *
 */

DecorTexture::~DecorTexture ()
{
    if (damage)
	XDamageDestroy (screen->dpy (), damage);
}

/*
 * DecorScreen::getTexture
 *
 * Returns the texture for a given pixmap. Note
 * that if this particular pixmap was already found
 * in the list of decor textures, then the refcount
 * is increased and that one is returned instead of
 * binding a new texture.
 *
 */

DecorTexture *
DecorScreen::getTexture (Pixmap pixmap)
{
    if (!cmActive)
	return NULL;

    foreach (DecorTexture *t, textures)
	if (t->pixmap->getPixmap () == pixmap)
	{
	    t->refCount++;
	    return t;
	}

    X11PixmapDeletor::Ptr dl = boost::make_shared <X11PixmapDeletor> (screen->dpy ());
    DecorPixmap::Ptr pm = boost::make_shared <DecorPixmap> (pixmap, dl);

    DecorTexture *texture = new DecorTexture (boost::shared_static_cast <DecorPixmapInterface> (pm));

    if (!texture->status)
    {
	delete texture;
	return NULL;
    }

    textures.push_back (texture);

    return texture;
}

/*
 * DecorScreen::releaseTexture
 *
 * Unreferences the texture, deletes the texture from
 * the list of textures if its no longer in use
 */

void
DecorScreen::releaseTexture (DecorTexture *texture)
{
    texture->refCount--;
    if (texture->refCount)
	return;

    std::list<DecorTexture *>::iterator it =
	std::find (textures.begin (), textures.end (), texture);

    if (it == textures.end ())
	return;

    textures.erase (it);
    delete texture;
}

/*
 * computeQuadBox
 *
 * Determines a scaled quad box for the decor plugin
 * by determining the x2 and y2 points by either clamping
 * or stretching the box. Also applies gravity to determine
 * relative x1 and y1 points to the window
 *
 */

static void
computeQuadBox (decor_quad_t *q,
		int	     width,
		int	     height,
		int	     *return_x1,
		int	     *return_y1,
		int	     *return_x2,
		int	     *return_y2,
		float        *return_sx,
		float        *return_sy)
{
    int   x1, y1, x2, y2;
    float sx = 1.0f;
    float sy = 1.0f;

    decor_apply_gravity (q->p1.gravity, q->p1.x, q->p1.y, width, height,
			 &x1, &y1);
    decor_apply_gravity (q->p2.gravity, q->p2.x, q->p2.y, width, height,
			 &x2, &y2);

    if (q->clamp & CLAMP_HORZ)
    {
	if (x1 < 0)
	    x1 = 0;
	if (x2 > width)
	    x2 = width;
    }

    if (q->clamp & CLAMP_VERT)
    {
	if (y1 < 0)
	    y1 = 0;
	if (y2 > height)
	    y2 = height;
    }

    if (q->stretch & STRETCH_X)
    {
	sx = (float) q->max_width / ((float) (x2 - x1));
    }
    else if (q->max_width < x2 - x1)
    {
	if (q->align & ALIGN_RIGHT)
	    x1 = x2 - q->max_width;
	else
	    x2 = x1 + q->max_width;
    }

    if (q->stretch & STRETCH_Y)
    {
	sy = (float) q->max_height / ((float) (y2 - y1));
    }
    else if (q->max_height < y2 - y1)
    {
	if (q->align & ALIGN_BOTTOM)
	    y1 = y2 - q->max_height;
	else
	    y2 = y1 + q->max_height;
    }

    *return_x1 = x1;
    *return_y1 = y1;
    *return_x2 = x2;
    *return_y2 = y2;

    if (return_sx)
	*return_sx = sx;
    if (return_sy)
	*return_sy = sy;
}

/*
 * Decoration::create
 *
 * Factory function to create a Decoration *, takes a decoration
 * property data, size, type and offset inside that property data
 * to start reading from.
 *
 * This function determines the decoration type and reads the property
 * data appropriately and then uses the property data to determine the
 * quads and extents of the input windows and the hinted extents.
 *
 */

Decoration::Ptr
Decoration::create (Window        id,
		    long          *prop,
		    unsigned int  size,
		    unsigned int  type,
		    unsigned int  nOffset,
		    DecorPixmapRequestorInterface *requestor)
{
    unsigned int    frameType, frameState, frameActions;
    Pixmap	    pixmap = None;
    decor_extents_t border;
    decor_extents_t input;
    decor_extents_t maxBorder;
    decor_extents_t maxInput;
    int		    minWidth;
    int		    minHeight;
    int             nQuad = N_QUADS_MAX;
    boost::shared_array <decor_quad_t> quad (new decor_quad_t[nQuad]);

    if (type ==  WINDOW_DECORATION_TYPE_PIXMAP &&
	!DecorScreen::get (screen)->cmActive)
    {
	compLogMessage ("decor", CompLogLevelWarn, "requested a pixmap type decoration when compositing isn't available");
	throw std::exception ();
    }

    if (type == WINDOW_DECORATION_TYPE_PIXMAP)
    {
	nQuad = decor_pixmap_property_to_quads (prop, nOffset, size, &pixmap, &input,
						&border, &maxInput,
						&maxBorder, &minWidth, &minHeight,
						&frameType, &frameState, &frameActions, quad.get ());

	if (!nQuad)
	    throw std::exception ();
    }
    else if (type == WINDOW_DECORATION_TYPE_WINDOW)
    {
	if (!decor_window_property (prop, nOffset, size, &input, &maxInput,
				    &minWidth, &minHeight, &frameType, &frameState, &frameActions))
	{
	    compLogMessage ("decor", CompLogLevelWarn, "malformed decoration - not a window");
	    throw std::exception ();
	}

	border = input;
	maxBorder = maxInput;
    }
    else
    {
	compLogMessage ("decor", CompLogLevelWarn, "malformed decoration - undetermined type");
	throw std::exception ();
    }

    return Decoration::Ptr (new Decoration (type, border, input, maxBorder, maxInput, frameType, frameState, frameActions, minWidth, minHeight, pixmap, quad, nQuad, id, requestor));
}

Decoration::Decoration (int   type,
			const decor_extents_t &border,
			const decor_extents_t &input,
			const decor_extents_t &maxBorder,
			const decor_extents_t &maxInput,
			unsigned int frameType,
			unsigned int frameState,
			unsigned int frameActions,
			unsigned int minWidth,
			unsigned int minHeight,
			Pixmap       pixmap,
			const boost::shared_array <decor_quad_t> &quad,
			unsigned int nQuad,
			Window       owner,
			DecorPixmapRequestorInterface *requestor) :
    refCount (0),
    texture (DecorScreen::get (screen)->getTexture (pixmap)),
    border (border.left, border.right, border.top, border.bottom),
    input (input.left, input.right, input.top, input.bottom),
    maxBorder (maxBorder.left, maxBorder.right, maxBorder.top, maxBorder.bottom),
    maxInput (maxInput.left, maxInput.right, maxInput.top, maxInput.bottom),
    minWidth (minWidth),
    minHeight (minHeight),
    frameType (frameType),
    frameState (frameState),
    frameActions (frameActions),
    quad (quad),
    nQuad (nQuad),
    type (type),
    updateState (0),
    mPixmapReceiver (requestor, this)
{
    int		    x1, y1, x2, y2;

    if (!texture && type == WINDOW_DECORATION_TYPE_PIXMAP)
    {
        compLogMessage ("decor", CompLogLevelWarn, "failed to bind pixmap to texture");
	throw std::exception ();
    }

    if (type == WINDOW_DECORATION_TYPE_PIXMAP)
    {
	int left = 0, right = minWidth, top = 0, bottom = minHeight;

	for (unsigned int i = 0; i  < nQuad; i++)
	{
	    computeQuadBox (&(quad[i]), minWidth, minHeight, &x1, &y1, &x2, &y2,
			    NULL, NULL);

	    if (x1 < left)
		left = x1;
	    if (y1 < top)
		top = y1;
	    if (x2 > right)
		right = x2;
	    if (y2 > bottom)
		bottom = y2;
	}

	this->output.left   = -left;
	this->output.right  = right - minWidth;
	this->output.top    = -top;
	this->output.bottom = bottom - minHeight;
    }
    else
    {
	this->output.left   = MAX (input.left, maxInput.left);
	this->output.right  = MAX (input.right, maxInput.right);
	this->output.top    = MAX (input.top, maxInput.top);
	this->output.bottom = MAX (input.bottom, maxInput.bottom);
    }
}

/*
 * Decoration::~Decoration
 *
 * Unreferences the decoration, also unreferences the texture
 * if this decoration is about to be destroyed
 *
 */

Decoration::~Decoration ()
{
    if (texture)
	DecorScreen::get (screen)->releaseTexture (texture);
}

DecorPixmapReceiverInterface &
Decoration::receiverInterface ()
{
    return mPixmapReceiver;
}

unsigned int
Decoration::getFrameType () const
{
    return frameType;
}

unsigned int
Decoration::getFrameState () const
{
    return frameState;
}

unsigned int
Decoration::getFrameActions () const
{
    return frameActions;
}

/*
 * DecorationList is a class which allows multiple decorations
 * to be stored in a list and read from a window property, which
 * allows caching to be done for decorations so that we aren't
 * always querying another process for decorations
 *
 */

DecorationList::DecorationList () :
    mList (0)
{
}

/*
 * DecorationList::updateDecoration
 *
 * Fetches the window property for a particular
 * window for this decoration list on a particular
 * decoration atom (whether that be the default
 * atom on the root window or the _COMPIZ_WINDOW_DECOR
 * atom on the client)
 *
 * Note that due to the way XGetWindowProperty works,
 * we only cache what is appropriate and then get the
 * rest from the property in case we didn't read enough
 * data. It would be awesome if XGetWindowProperty allowed
 * you to read as much as you wanted.
 *
 * FIXME: Currently this function refreshes all decorations.
 * That's slow and should be fixed
 *
 */

bool
DecorationList::updateDecoration (Window   id,
				  Atom     decorAtom,
				  DecorPixmapRequestorInterface *requestor)
{
    unsigned long   n, nleft;
    unsigned char   *data;
    long	    *prop;
    Atom	    actual;
    int		    result, format;
    unsigned int    type;

    /* Dispatch any new updates */
    foreach (const Decoration::Ptr &d, mList)
    {
	d->mPixmapReceiver.update ();
    }

    result = XGetWindowProperty (screen->dpy (), id,
                                 decorAtom, 0L,
                                 PROP_HEADER_SIZE + 6 * (BASE_PROP_SIZE +
                                                         QUAD_PROP_SIZE * N_QUADS_MAX),
                                 false, XA_INTEGER, &actual, &format,
				 &n, &nleft, &data);

    if (result != Success || !n || !data)
        return false;

    /* Attempted to read the reasonable amount of
     * around 6 pixmap decorations, if there are more, we need
     * an additional roundtrip to read everything else
     */
    if (nleft)
    {
        XFree (data);

        result = XGetWindowProperty (screen->dpy (), id, decorAtom, 0L,
                                     n + nleft, false, XA_INTEGER, &actual, &format,
                                     &n, &nleft, &data);

        if (result != Success || !n || !data)
            return false;
    }

    prop = (long *) data;

    if (decor_property_get_version (prop) != decor_version ())
    {
	compLogMessage ("decoration", CompLogLevelWarn,
			"Property ignored because "
			"version is %d and decoration plugin version is %d\n",
			decor_property_get_version (prop), decor_version ());

	XFree (data);
	return false;
    }

    type = decor_property_get_type (prop);

    std::list <Decoration::Ptr> remove;
    std::list <int> skip;

    /* only recreate decorations if they need to be updated */
    foreach (const Decoration::Ptr &d, mList)
    {
	decor_extents_t input, border, maxInput, maxBorder;

	input.left = d->input.left;
	input.right = d->input.right;
	input.top = d->input.top;
	input.bottom = d->input.bottom;

	border.left = d->border.left;
	border.right = d->border.right;
	border.top = d->border.top;
	border.bottom = d->border.bottom;

	maxInput.left = d->maxInput.left;
	maxInput.right = d->maxInput.right;
	maxInput.top = d->maxInput.top;
	maxInput.bottom = d->maxInput.bottom;

	maxBorder.left = d->maxBorder.left;
	maxBorder.right = d->maxBorder.right;
	maxBorder.top = d->maxBorder.top;
	maxBorder.bottom = d->maxBorder.bottom;

	Pixmap pm = d->texture->pixmap->getPixmap ();

	int num = decor_match_pixmap (prop, n, &pm, &input, &border, &maxInput, &maxBorder,
				     d->minWidth, d->minHeight, d->frameType, d->frameState, d->frameActions,
				    d->quad.get (), d->nQuad);
	if (num != -1)
	    skip.push_back (num);
	else
	    remove.push_back (d);
    }

    /* Create a new decoration for each individual item on the property */
    for (int i = 0; i < decor_property_get_num (prop); i++)
    {
	if (std::find (skip.begin (), skip.end (), i) != skip.end ())
	    continue;

	try
	{
	    std::list <Decoration::Ptr>::iterator it = mList.begin ();
	    Decoration::Ptr d = Decoration::create (id, prop, n, type, i, requestor);

	    /* Try to replace an existing decoration */
	    for (; it != mList.end (); ++it)
	    {
		if ((*it)->frameType == d->frameType &&
		    (*it)->frameState == d->frameState &&
		    (*it)->frameActions == d->frameActions)
		{
		    remove.remove ((*it));
		    (*it) = d;
		    break;
		}
	    }

	    if (it == mList.end ())
		mList.push_back (d);
	}
	catch (...)
	{
	    /* Creating a new decoration failed ... see if we can use
	     * the old one */

	    unsigned int    frameType, frameState, frameActions;
	    Pixmap	    pixmap = None;
	    decor_extents_t border;
	    decor_extents_t input;
	    decor_extents_t maxBorder;
	    decor_extents_t maxInput;
	    int		    minWidth;
	    int		    minHeight;
	    int             ok = 0;
	    boost::shared_array <decor_quad_t> quad (new decor_quad_t[N_QUADS_MAX]);

	    if (type == WINDOW_DECORATION_TYPE_PIXMAP)
	    {
		ok = decor_pixmap_property_to_quads (prop, i, n, &pixmap, &input,
							&border, &maxInput,
							&maxBorder, &minWidth, &minHeight,
							&frameType, &frameState, &frameActions, quad.get ());
	    }
	    else if (type == WINDOW_DECORATION_TYPE_WINDOW)
	    {
		ok = decor_window_property (prop, i, n, &input, &maxInput,
					    &minWidth, &minHeight, &frameType, &frameState, &frameActions);
	    }

	    if (ok)
	    {
		std::list <Decoration::Ptr>::iterator it = mList.begin ();

		/* Use an existing decoration */
		for (; it != mList.end (); ++it)
		{
		    if ((*it)->frameType == frameType &&
			(*it)->frameState == frameState &&
			(*it)->frameActions == frameActions)
		    {
			remove.remove ((*it));
			break;
		    }
		}
	    }
	}
    }

    foreach (const Decoration::Ptr &d, remove)
	mList.remove (d);

    XFree (data);

    return true;
}

/*
 * DecorWindow::updateDecoration
 *
 * Updates the decoration list on this window
 */

void
DecorWindow::updateDecoration ()
{
    bindFailed = false;

    decor.updateDecoration (window->id (), dScreen->winDecorAtom, &mRequestor);
    if (bindFailed)
	pixmapFailed = true;
    else
	pixmapFailed = false;
}

/*
 * WindowDecoration::create
 *
 * Factory function for WindowDecoration, creates
 * a window specific decoration for this window,
 * not to be confused with window /type/ decorations
 * which are a different matter.
 *
 * Decorations can indeed be re-used, and that is what
 * WindowDecoration is for.
 *
 */
WindowDecoration *
WindowDecoration::create (const Decoration::Ptr &d)
{
    WindowDecoration *wd;

    wd = new WindowDecoration ();
    if (!wd)
	return NULL;

    if (d->type == WINDOW_DECORATION_TYPE_PIXMAP)
    {
	wd->quad = new ScaledQuad[d->nQuad];

	if (!wd->quad)
	{
	    delete wd;
	    return NULL;
	}
    }
    else
	wd->quad = NULL;

    d->refCount++;

    wd->decor = d;
    wd->nQuad = d->nQuad;

    return wd;
}

/*
 * WindowDecoration::destroy
 *
 * Unreferences the bound decoration
 * and frees quads
 */
void
WindowDecoration::destroy (WindowDecoration *wd)
{
    delete [] wd->quad;
    delete wd;
}

/*
 * DecorWindow::setDecorationMatrices
 *
 * Statically update the quad display matrices
 * (2x3 matrix) each time the window is moved
 * or resized
 *
 * For this to work, we need to multiply the
 * scaled quad and decor quad matrices together
 * to get the final scaled transformation
 *
 * Translation (x0, y0) is based on the actual box
 * position in decoration co-ordinate space multiplied
 * by the scaled transformation matrix
 */
void
DecorWindow::setDecorationMatrices ()
{
    float	      x0, y0;
    decor_matrix_t    a;
    GLTexture::Matrix b;

    if (!wd)
	return;

    for (int i = 0; i < wd->nQuad; i++)
    {
	/* Set the quad matrix to the texture matrix */
	wd->quad[i].matrix = wd->decor->texture->textures[0]->matrix ();

	/* Initial translation point is based on existing translation point */
	x0 = wd->decor->quad[i].m.x0;
	y0 = wd->decor->quad[i].m.y0;

	a = wd->decor->quad[i].m;
	b = wd->quad[i].matrix;

	/* Multiply wd->quad[i].matrix (decoration matrix)
	 * and the scaled quad matrix */
	wd->quad[i].matrix.xx = a.xx * b.xx + a.yx * b.xy;
	wd->quad[i].matrix.yx = a.xx * b.yx + a.yx * b.yy;
	wd->quad[i].matrix.xy = a.xy * b.xx + a.yy * b.xy;
	wd->quad[i].matrix.yy = a.xy * b.yx + a.yy * b.yy;
	wd->quad[i].matrix.x0 = x0 * b.xx + y0 * b.xy + b.x0;
	wd->quad[i].matrix.y0 = x0 * b.yx + y0 * b.yy + b.y0;

	wd->quad[i].matrix.xx *= wd->quad[i].sx;
	wd->quad[i].matrix.yx *= wd->quad[i].sx;
	wd->quad[i].matrix.xy *= wd->quad[i].sy;
	wd->quad[i].matrix.yy *= wd->quad[i].sy;

	/* Align translation points to the right
	 * in the scaled quad (window) space */
	if (wd->decor->quad[i].align & ALIGN_RIGHT)
	    x0 = wd->quad[i].box.x2 - wd->quad[i].box.x1;
	else
	    x0 = 0.0f;

	/* Align translation points to the bottom
	 * in the scaled quad (window) space */
	if (wd->decor->quad[i].align & ALIGN_BOTTOM)
	    y0 = wd->quad[i].box.y2 - wd->quad[i].box.y1;
	else
	    y0 = 0.0f;

	wd->quad[i].matrix.x0 -=
	    x0 * wd->quad[i].matrix.xx +
	    y0 * wd->quad[i].matrix.xy;

	wd->quad[i].matrix.y0 -=
	    y0 * wd->quad[i].matrix.yy +
	    x0 * wd->quad[i].matrix.yx;

	wd->quad[i].matrix.x0 -=
	    wd->quad[i].box.x1 * wd->quad[i].matrix.xx +
	    wd->quad[i].box.y1 * wd->quad[i].matrix.xy;

	wd->quad[i].matrix.y0 -=
	    wd->quad[i].box.y1 * wd->quad[i].matrix.yy +
	    wd->quad[i].box.x1 * wd->quad[i].matrix.yx;
    }

    updateMatrix = false;
}

/*
 * DecorWindow::updateDecorationScale
 *
 * Update the scaled quads box for this
 * window. Do that by determining
 * the scaled quad box for the window size
 * and then translating each quad box by
 * the window position
 *
 */

void
DecorWindow::updateDecorationScale ()
{
    int		     x1, y1, x2, y2;
    float            sx, sy;

    if (!wd)
	return;

    for (int i = 0; i < wd->nQuad; i++)
    {
	int x, y;
	unsigned int width = window->size ().width ();
	unsigned int height = window->size ().height ();

	if (window->shaded ())
	{
	    if (dScreen->cScreen &&
		dScreen->cScreen->compositingActive ())
	    {
		if (!cWindow->pixmap ())
		    height = 0;
	    }
	    else
		height = 0;
	}

	computeQuadBox (&wd->decor->quad[i], width, height,
			&x1, &y1, &x2, &y2, &sx, &sy);

	/* Translate by x and y points of this window */
	x = window->geometry ().x ();
	y = window->geometry ().y ();

	wd->quad[i].box.x1 = x1 + x;
	wd->quad[i].box.y1 = y1 + y;
	wd->quad[i].box.x2 = x2 + x;
	wd->quad[i].box.y2 = y2 + y;

	wd->quad[i].sx     = sx;
	wd->quad[i].sy     = sy;
    }

    setDecorationMatrices ();
}

/*
 * DecorWindow::checkSize
 *
 * Convenience function to check if this decoration
 * is going to display laerger than the window size
 * itself, in that case we can't use it since it
 * would like the decoration was larger than the window
 * (or trying to compress it anymore would result in
 * eg, distorted text or buttons). In that case
 * we'd use default decorations
 *
 */
bool
DecorWindow::checkSize (const Decoration::Ptr &decoration)
{
    return (decoration->minWidth <= (int) window->size ().width () &&
	    decoration->minHeight <= (int) window->size ().height ());
}

/*
 * decorOffsetMove
 *
 * Function called on a timer (to avoid calling configureXWindow from
 * within a ::moveNotify) which actually moves the window by the offset
 * specified in the xwc. Also sends a notification that the window
 * was decorated
 *
 */
static bool
decorOffsetMove (CompWindow *w, XWindowChanges xwc, unsigned int mask)
{
    CompOption::Vector o (1);

    o.at (0).setName ("window", CompOption::TypeInt);
    o.at (0).value ().set ((int) w->id ());

    xwc.x += w->serverGeometry ().x ();
    xwc.y += w->serverGeometry ().y ();

    w->configureXWindow (mask, &xwc);
    screen->handleCompizEvent ("decor", "window_decorated", o);
    return false;
}

/*
 * DecorWindow::matchType
 *
 * Converts libdecoration window types packed
 * into the property into Compiz window types
 *
 */
bool
DecorWindow::matchType (CompWindow *w,
                        unsigned int decorType)
{
    const unsigned int nTypeStates = 5;
    struct typestate {
        unsigned int compFlag;
        unsigned int decorFlag;
    } typeStates[] =
    {
        { CompWindowTypeNormalMask, DECOR_WINDOW_TYPE_NORMAL },
        { CompWindowTypeDialogMask, DECOR_WINDOW_TYPE_DIALOG },
        { CompWindowTypeModalDialogMask, DECOR_WINDOW_TYPE_MODAL_DIALOG },
        { CompWindowTypeMenuMask, DECOR_WINDOW_TYPE_MENU },
        { CompWindowTypeUtilMask, DECOR_WINDOW_TYPE_UTILITY}
    };

    for (unsigned int i = 0; i < nTypeStates; i++)
    {
        if ((decorType & typeStates[i].decorFlag) && (w->type () & typeStates[i].compFlag))
            return true;
    }

    return false;
}

/*
 * DecorWindow::matchType
 *
 * Converts libdecoration window states packed
 * into the property into Compiz window states
 *
 * Since there is no _NET_WM_STATE_ACTIVE
 * we need to determine that ourselves from
 * _NET_ACTIVE_WINDOW on the root window
 *
 */
bool
DecorWindow::matchState (CompWindow   *w,
                         unsigned int decorState)
{
    const unsigned int nStateStates = 3;
    struct statestate {
        unsigned int compFlag;
        unsigned int decorFlag;
    } stateStates[] =
    {
        { CompWindowStateMaximizedVertMask, DECOR_WINDOW_STATE_MAXIMIZED_VERT },
        { CompWindowStateMaximizedHorzMask, DECOR_WINDOW_STATE_MAXIMIZED_HORZ },
        { CompWindowStateShadedMask, DECOR_WINDOW_STATE_SHADED }
    };

    /* Active is a separate check */
    if (screen->activeWindow () == w->id ())
        decorState &= ~(DECOR_WINDOW_STATE_FOCUS);

    for (unsigned int i = 0; i < nStateStates; i++)
    {
        if ((decorState & stateStates[i].decorFlag) && (w->state () & stateStates[i].compFlag))
            decorState &= ~(stateStates[i].decorFlag);
    }

    return (decorState == 0);
}

/*
 * DecorWindow::matchActions
 *
 * Converts libdecoration window actions packed
 * into the property into Compiz window types
 *
 */
bool
DecorWindow::matchActions (CompWindow   *w,
                           unsigned int decorActions)
{
    const unsigned int nActionStates =16;
    struct actionstate {
        unsigned int compFlag;
        unsigned int decorFlag;
    } actionStates[] =
    {
        { DECOR_WINDOW_ACTION_RESIZE_HORZ, CompWindowActionResizeMask },
        { DECOR_WINDOW_ACTION_RESIZE_VERT, CompWindowActionResizeMask },
        { DECOR_WINDOW_ACTION_CLOSE, CompWindowActionCloseMask },
        { DECOR_WINDOW_ACTION_MINIMIZE, CompWindowActionMinimizeMask },
        { DECOR_WINDOW_ACTION_UNMINIMIZE,CompWindowActionMinimizeMask },
        { DECOR_WINDOW_ACTION_MAXIMIZE_HORZ, CompWindowActionMaximizeHorzMask },
        { DECOR_WINDOW_ACTION_MAXIMIZE_VERT, CompWindowActionMaximizeVertMask },
        { DECOR_WINDOW_ACTION_UNMAXIMIZE_HORZ, CompWindowActionMaximizeHorzMask },
        { DECOR_WINDOW_ACTION_UNMAXIMIZE_VERT, CompWindowActionMaximizeVertMask },
        { DECOR_WINDOW_ACTION_SHADE, CompWindowActionShadeMask },
        { DECOR_WINDOW_ACTION_UNSHADE, CompWindowActionShadeMask },
        { DECOR_WINDOW_ACTION_STICK, CompWindowActionStickMask },
        { DECOR_WINDOW_ACTION_UNSTICK, CompWindowActionStickMask },
        { DECOR_WINDOW_ACTION_FULLSCREEN, CompWindowActionFullscreenMask },
        { DECOR_WINDOW_ACTION_ABOVE, CompWindowActionAboveMask },
        { DECOR_WINDOW_ACTION_BELOW, CompWindowActionBelowMask },
    };

    for (unsigned int i = 0; i < nActionStates; i++)
    {
        if ((decorActions & actionStates[i].decorFlag) && (w->type () & actionStates[i].compFlag))
            decorActions &= ~(actionStates[i].decorFlag);
    }

    return (decorActions == 0);
}

DecorationInterface::Ptr
DecorationList::findMatchingDecoration (unsigned int frameType,
					unsigned int frameState,
					unsigned int frameActions)
{
    foreach (const Decoration::Ptr &d, mList)
    {
	if (d->frameType == frameType &&
	    d->frameState == frameState &&
	    d->frameActions == frameActions)
	    return boost::shared_static_cast <DecorationInterface> (d);
    }

    return DecorationInterface::Ptr ();
}

/*
 * DecorationList::findMatchingDecoration
 *
 * Searches a decoration list for a decoration
 * that actually matches this window, or at least
 * comes close to it.
 * 
 * There is an order of preference when matching
 * decorations here.
 *
 * Type:State:Actions
 *
 * If a property before another one is matched, that
 * decoration is "locked" so if a decoration is found
 * that has the correct matching property but does not
 * match the locked property, then it is not matched
 *
 */
const Decoration::Ptr &
DecorationList::findMatchingDecoration (CompWindow *w,
                                        bool       sizeCheck)
{
    std::list <Decoration::Ptr>::iterator cit = mList.end ();
    DECOR_WINDOW (w);

    if (!mList.empty ())
    {
        const unsigned int typeMatch = (1 << 0);
        const unsigned int stateMatch = (1 << 1);
        const unsigned int actionsMatch = (1 << 2);

        unsigned int currentDecorState = 0;

	if (sizeCheck)
	    if (dw->checkSize (mList.front ()))
		cit = mList.begin ();

	for (std::list <Decoration::Ptr>::iterator it = mList.begin ();
	     it != mList.end (); ++it)
	{
	    const Decoration::Ptr &d = *it;

	    /* Must always match type */
	    if (DecorWindow::matchType (w, d->frameType))
	    {
		/* Use this decoration if the type matched */
		if (!(typeMatch & currentDecorState) && (!sizeCheck || dw->checkSize (d)))
		{
		    cit = it;
		    currentDecorState |= typeMatch;
		}

		/* Must always match state if type is already matched */
		if (DecorWindow::matchState (w, d->frameState) && (!sizeCheck || dw->checkSize (d)))
		{
		    /* Use this decoration if the type and state match */
		    if (!(stateMatch & currentDecorState))
		    {
			cit = it;
			currentDecorState |= stateMatch;
		    }

		    /* Must always match actions if state and type are already matched */
		    if (DecorWindow::matchActions (w, d->frameActions) && (!sizeCheck || dw->checkSize (d)))
		    {
			/* Use this decoration if the requested actions match */
			if (!(actionsMatch & currentDecorState))
			{
			    cit = it;
			    currentDecorState |= actionsMatch;

			    /* Perfect match, no need to continue searching */
			    break;
			}
		    }
		}
	    }
	}
    }

    if (cit == mList.end ())
	throw std::exception ();

    return *cit;
}

/*
 * DecorWindow::update
 * This is the master function for managing decorations on windows
 *
 * The first part of this function determines if we want to actually
 * decorate a particular window. This only passes if the window
 * matches the decorated type match and it is also capable of being
 * decorated (eg, has a frame window and not override redirect) and
 * also has appropriate _MOTIF_WM_HINTS set on it (specifically 0x3
 * and 0x6)
 *
 * The next part of the function attempts to find a matching decoration
 * has has been created by the decorators. If it can't find one, the
 * window gets a default decoration (until the decorators have "caught
 * up" with us and given this window an actual decoration.
 *
 * Windows that we've marked not to decorate get shadows around them
 * at least. This is the "bare" type decoration
 *
 * If an appropriate decoration is found for this window, a WindowDecoration
 * (which is a window specific class determining how that decoration
 *  should operate on /this particular window/) is created.
 *
 * At this point we also update the "frame extents" in core (for
 * _NET_REQUEST_FRAME_EXTENTS) and also the actual frame geometry
 * since we might need a larger space on the frame window (which is
 * shaped to accomadate decorations) for things like, eg grab areas
 * which shouldn't be represented to clients as actual visible
 * decoration space
 *
 * FIXME: There are a bunch of hacks in here to allow override redirect
 * windows which have "special" switcher type decorations to be decorated
 * without being reparented. Ideally, these shouldn't be handled by 
 * the decor plugin
 *
 */
bool
DecorWindow::update (bool allowDecoration)
{
    Decoration::Ptr  old, decoration;
    bool	     decorate = false;
    bool	     shadowOnly = true;
    CompPoint        oldShift, movement;

    if (wd)
	old = wd->decor;
    else
	old.reset ();

    /* Only want to decorate windows which have a frame or are in the process
     * of waiting for an animation to be unmapped (in which case we can give
     * them a new pixmap type frame since we don't actually need an input
     * window to go along with that
     *
     * FIXME: That's not going to play nice with reparented decorations in core
     * since the window gets reparented right away before plugins are done
     * with it */

    /* Unconditionally decorate switchers */
    if (!isSwitcher)
    {
        switch (window->type ()) {
	    case CompWindowTypeDialogMask:
	    case CompWindowTypeModalDialogMask:
	    case CompWindowTypeUtilMask:
	    case CompWindowTypeMenuMask:
	    case CompWindowTypeNormalMask:
		if (window->mwmDecor () & (MwmDecorAll | MwmDecorTitle))
		    shadowOnly = false;
	    default:
		break;
	}

	if (window->overrideRedirect ())
	    shadowOnly = true;

	if (window->wmType () & (CompWindowTypeDockMask | CompWindowTypeDesktopMask))
	    shadowOnly = true;

	if (!shadowOnly)
	{
	    if (!dScreen->optionGetDecorationMatch ().evaluate (window))
		shadowOnly = true;
	}
    }
    else
	shadowOnly = false;

    decorate = ((window->frame () ||
		 window->hasUnmapReference ()) && !shadowOnly) ||
		 isSwitcher;

    if (decorate || frameExtentsRequested)
    {
        /* Attempt to find a matching decoration */
	try
	{
	    decoration = decor.findMatchingDecoration (window, true);
	}
	catch (...)
	{
	    /* Find an appropriate default decoration to use */
	    if (dScreen->dmSupports & WINDOW_DECORATION_TYPE_PIXMAP &&
	        dScreen->cmActive &&
		!(dScreen->dmSupports & WINDOW_DECORATION_TYPE_WINDOW &&
		  pixmapFailed))
	    {
		try
		{
		    decoration = dScreen->decor[DECOR_ACTIVE].findMatchingDecoration (window, false);
		}
		catch (...)
		{
		    compLogMessage ("decor", CompLogLevelWarn, "No default decoration found, placement will not be correct");
		    decoration.reset ();
		}
	    }
	    else if (dScreen->dmSupports & WINDOW_DECORATION_TYPE_WINDOW)
		decoration = dScreen->windowDefault;
	}

	/* Do not allow windows which are later undecorated
	 * to have a set _NET_FRAME_EXTENTS */
	if (decorate)
	    frameExtentsRequested = false;
    }
    else
    {
	/* This window isn't "decorated" but it still gets a shadow as long
	 * as it isn't shaped weirdly, since the shadow is just a quad rect */
	if (dScreen->optionGetShadowMatch ().evaluate (window))
	{
	    if (window->region ().numRects () == 1 && !window->alpha () && dScreen->decor[DECOR_BARE].mList.size ())
		decoration = dScreen->decor[DECOR_BARE].mList.front ();

	    if (decoration)
	    {
		if (!checkSize (decoration))
		    decoration.reset ();
	    }
	}
    }

    /* Don't allow the windows to be decorated if
     * we're tearing down or if a decorator isn't running
     * (nobody owns the selection window) 
     */
    if (!dScreen->dmWin || !allowDecoration)
	decoration.reset ();

    /* Don't bother going any further if
     * this window is going to get the same
     * decoration, just use the old one */
    if (decoration == old)
	return false;

    /* We need to damage the current output extents
     * and recompute the shadow region if a compositor
     * is running
     */
    if (dScreen->cmActive)
    {
	cWindow->damageOutputExtents ();
	updateGroupShadows ();
    }

    /* Determine how much we moved the window for the old
     * decoration and save that, also destroy the old
     * WindowDecoration */
    if (old)
    {
	oldShift = compiz::window::extents::shift (window->border (), window->sizeHints ().win_gravity);

	WindowDecoration::destroy (wd);

	wd = NULL;
    }

    /* If a decoration was found for this window, create
     * a new WindowDecoration for it and set the frame
     * extents accordingly. We should also move the
     * window by the distance the new decoration provides
     * in case that actually changed
     */
    if (decoration)
    {
	wd = WindowDecoration::create (decoration);
	if (!wd)
	    return false;

	/* Set extents based on maximize/unmaximize state
	 * FIXME: With the new type system, this should be
	 * removed */
	if ((window->state () & MAXIMIZE_STATE))
	    window->setWindowFrameExtents (&wd->decor->maxBorder,
					   &wd->decor->maxInput);
	else if (!window->hasUnmapReference ())
	    window->setWindowFrameExtents (&wd->decor->border,
					   &wd->decor->input);

	movement = compiz::window::extents::shift (window->border (), window->sizeHints ().win_gravity);
	movement -= oldShift;

	/* Update the input and output frame */
	if (decorate)
	    updateFrame ();
	window->updateWindowOutputExtents ();

	updateReg = true;
	updateMatrix = true;
	mOutputRegion = CompRegion (window->outputRect ());
	updateGroupShadows ();
	if (dScreen->cmActive)
	    cWindow->damageOutputExtents ();
	updateDecorationScale ();
    }
    else
    {
	CompWindowExtents emptyExtents;
	wd = NULL;

	/* Undecorated windows need to have the
	 * input and output frame removed and the
	 * frame window geometry reset */
	updateFrame ();

	memset (&emptyExtents, 0, sizeof (CompWindowExtents));

	window->setWindowFrameExtents (&emptyExtents, &emptyExtents);

	movement -= oldShift;
    }

    /* Need to actually move the window */
    if (window->placed () && !window->overrideRedirect () &&
	(movement.x () || movement.y ()))
    {
	XWindowChanges xwc;
	unsigned int   mask = CWX | CWY;

	memset (&xwc, 0, sizeof (XWindowChanges));

	/* Grab the geometry last sent to server at configureXWindow
	 * time and not here since serverGeometry may be updated by
	 * the time that we do call configureXWindow */
	xwc.x = movement.x ();
	xwc.y = movement.y ();

	/* Except if it's fullscreen, maximized or such */
	if (window->state () & CompWindowStateFullscreenMask)
	    mask &= ~(CWX | CWY);

	if (window->state () & CompWindowStateMaximizedHorzMask)
	    mask &= ~CWX;

	if (window->state () & CompWindowStateMaximizedVertMask)
	    mask &= ~CWY;

	if (window->saveMask () & CWX)
	    window->saveWc ().x += movement.x ();

	if (window->saveMask () & CWY)
	    window->saveWc ().y += movement.y ();

	if (mask)
	{
	    /* allowDecoration is only false in the case of
	     * the destructor calling the update function so since it
	     * is not safe to put the function in a timer (since
	     * it will get unref'd on the vtable destruction) we
	     * need to do it immediately
	     *
	     * FIXME: CompTimer should really be PIMPL and allow
	     * refcounting in case we need to keep it alive
	     */
	    if (!allowDecoration)
		decorOffsetMove (window, xwc, mask);
	    else
		moveUpdate.start (boost::bind (decorOffsetMove, window, xwc, mask), 0);
	}
    }

    return true;
}

/*
 * DecorWindow::updateFrame
 *
 * Updates the actual frame window which is
 * used for either displaying the decoration in the case of window
 * type reparented decorations, or handling input events in the case
 * of pixmap decorations
 */
void
DecorWindow::updateFrame ()
{
    /* Destroy the input and output frames in case the window can't
     * actually be decorated */
    if (!wd || !(window->border ().left || window->border ().right ||
		 window->border ().top || window->border ().bottom) ||
        (wd->decor->type == WINDOW_DECORATION_TYPE_PIXMAP && outputFrame) ||
        (wd->decor->type == WINDOW_DECORATION_TYPE_WINDOW && inputFrame))
    {
	if (inputFrame)
	{
	    XDeleteProperty (screen->dpy (), window->id (),
			     dScreen->inputFrameAtom);

	    if (window->frame ())
		XDestroyWindow (screen->dpy (), inputFrame);

	    inputFrame = None;
	    frameRegion = CompRegion ();

	    oldX = 0;
	    oldY = 0;
	    oldWidth  = 0;
	    oldHeight = 0;
	}
	if (outputFrame)
	{
	    XDamageDestroy (screen->dpy (), frameDamage);
	    XDeleteProperty (screen->dpy (), window->id (),
			     dScreen->outputFrameAtom);

	    if (window->frame ())
		XDestroyWindow (screen->dpy (), outputFrame);
	    dScreen->frames.erase (outputFrame);

	    outputFrame = None;
	    frameRegion = CompRegion ();

	    oldX = 0;
	    oldY = 0;
	    oldWidth  = 0;
	    oldHeight = 0;
	}
    }
    /* If the window can be decorated, update the frames */
    if (wd && (window->border ().left || window->border ().right ||
	       window->border ().top || window->border ().bottom))
    {
	if (wd->decor->type == WINDOW_DECORATION_TYPE_PIXMAP)
	    updateInputFrame ();
	else if (wd->decor->type == WINDOW_DECORATION_TYPE_WINDOW)
	    updateOutputFrame ();
    }
}

/*
 * DecorWindow::updateInputFrame
 *
 * Actually creates an input frame if there isn't
 * one, otherwise sets the shape regions on it so that
 * if the decoration inside the parent window ever
 * gets stacked on top of the client, it won't obstruct
 * it
 *
 * This also sets the _COMPIZ_WINDOW_DECOR_INPUT_FRAME
 * atom on the window. Decorators should listen for this
 * to determine if a window is to be decorated, and when
 * they get a PropertyNotify indicating that this the case
 * should draw a decoration and set the _COMPIZ_WINDOW_DECOR
 * atom in response, otherwise this window is going to
 * be stuck with default decorations
 *
 */
void
DecorWindow::updateInputFrame ()
{
    XRectangle           rects[4];
    int                  x, y, width, height;
    CompWindow::Geometry server = window->serverGeometry ();
    CompWindowExtents	 input;
    CompWindowExtents    border;
    Window		 parent;

    /* Switchers are special, we need to put input frames
     * there in the root window rather than in the frame
     * window that this window is reparented into */
    if (isSwitcher)
	parent = screen->root ();
    else
	parent = window->frame ();

    /* Determine frame extents */
    if ((window->state () & MAXIMIZE_STATE))
    {
	border = wd->decor->maxBorder;
	input = wd->decor->maxInput;
    }
    else
    {
	border = wd->decor->border;
	input = wd->decor->input;
    }

    x      = window->border ().left - border.left;
    y      = window->border ().top - border.top;
    width  = server.widthIncBorders () + input.left + input.right;
    height = server.heightIncBorders ()+ input.top  + input.bottom ;

    /* Non switcher windows are rooted relative to the frame window of the client
     * and switchers need to be offset by the window geometry of the client */
    if (isSwitcher)
    {
	x += window->x ();
	y += window->y ();
    }

    /* Shaded windows automatically have no height */
    if (window->shaded ())
	height = input.top + input.bottom;

    /* Since we're reparenting windows here, we need to grab the server
     * which sucks, but its necessary */
    XGrabServer (screen->dpy ());

    if (!inputFrame)
    {
	XSetWindowAttributes attr;

	attr.event_mask	   = StructureNotifyMask;
	attr.override_redirect = true;

	inputFrame = XCreateWindow (screen->dpy (), parent,
				    x, y, width, height, 0, CopyFromParent,
				    InputOnly, CopyFromParent,
				    CWOverrideRedirect | CWEventMask,
				    &attr);

	XGrabButton (screen->dpy (), AnyButton, AnyModifier, inputFrame,
		     true, ButtonPressMask | ButtonReleaseMask |
		     ButtonMotionMask, GrabModeSync, GrabModeSync, None,
		     None);

	XMapWindow (screen->dpy (), inputFrame);

	/* Notify the decorators that an input frame has been created on
	 * this window so that they can react by actually create a decoration
	 * for it (while we use the default decorations) */
	XChangeProperty (screen->dpy (), window->id (),
			 dScreen->inputFrameAtom, XA_WINDOW, 32,
			 PropModeReplace, (unsigned char *) &inputFrame, 1);

	if (screen->XShape ())
	    XShapeSelectInput (screen->dpy (), inputFrame, ShapeNotifyMask);

	/* invalidate the decoration so that it gets shaped */
	oldX = 0;
	oldY = 0;
	oldWidth  = 0;
	oldHeight = 0;
    }

    if (x != oldX || y != oldY || width != oldWidth || height != oldHeight)
    {
	int    i = 0;
	oldX = x;
	oldY = y;
	oldWidth  = width;
	oldHeight = height;

	XMoveResizeWindow (screen->dpy (), inputFrame, x, y,
			   width, height);

	/* Non switcher decorations need to be lowered in
	 * in the frame to ensure that they go below
	 * the window contents (so that our set input shape
	 * works correctly */
	if (!isSwitcher)
	    XLowerWindow (screen->dpy (), inputFrame);

	rects[i].x	= 0;
	rects[i].y	= 0;
	rects[i].width  = width;
	rects[i].height = input.top;

	if (rects[i].width && rects[i].height)
	    i++;

	rects[i].x	= 0;
	rects[i].y	= input.top;
	rects[i].width  = input.left;
	rects[i].height = height - input.top - input.bottom;

	if (rects[i].width && rects[i].height)
	    i++;

	rects[i].x	= width - input.right;
	rects[i].y	= input.top;
	rects[i].width  = input.right;
	rects[i].height = height - input.top - input.bottom;

	if (rects[i].width && rects[i].height)
	    i++;

	rects[i].x	= 0;
	rects[i].y	= height - input.bottom;
	rects[i].width  = width;
	rects[i].height = input.bottom;

	if (rects[i].width && rects[i].height)
	    i++;

	XShapeCombineRectangles (screen->dpy (), inputFrame,
				 ShapeInput, 0, 0, rects, i,
				 ShapeSet, YXBanded);

	frameRegion = CompRegion ();
    }

    XUngrabServer (screen->dpy ());
}

/*
 * DecorWindow::updateOutputFrame
 *
 * Actually creates an output frame if there isn't
 * one, otherwise sets the shape regions on it so that
 * if the decoration inside the parent window ever
 * gets stacked on top of the client, it won't obstruct
 * it
 *
 * This also sets the _COMPIZ_WINDOW_DECOR_OUTPUT_FRAME
 * atom on the window. Decorators should listen for this
 * to determine if a window is to be decorated, and when
 * they get a PropertyNotify indicating that this the case
 * should draw a decoration inside the window and set the
 * _COMPIZ_WINDOW_DECOR atom in response, otherwise this
 * window is going to be stuck with default decorations
 *
 */
void
DecorWindow::updateOutputFrame ()
{
    XRectangle           rects[4];
    int                  x, y, width, height;
    CompWindow::Geometry server = window->serverGeometry ();
    CompWindowExtents	 input;

    /* Determine frame extents */
    if ((window->state () & MAXIMIZE_STATE))
	input = wd->decor->maxInput;
    else
	input = wd->decor->input;

    x      = window->input ().left - input.left;
    y      = window->input ().top - input.top;
    width  = server.widthIncBorders () + input.left + input.right;
    height = server.heightIncBorders ()+ input.top  + input.bottom;

    if (window->shaded ())
	height = input.top + input.bottom;

    /* Since we're reparenting windows here, we need to grab the server
     * which sucks, but its necessary */
    XGrabServer (screen->dpy ());

    if (!outputFrame)
    {
	XSetWindowAttributes attr;

	attr.background_pixel  = 0x0;
	attr.event_mask        = StructureNotifyMask;
	attr.override_redirect = true;

	outputFrame = XCreateWindow (screen->dpy (), window->frame (),
				     x, y, width, height, 0, CopyFromParent,
				     InputOutput, CopyFromParent,
				     CWOverrideRedirect | CWEventMask,
				     &attr);

	XGrabButton (screen->dpy (), AnyButton, AnyModifier, outputFrame,
			true, ButtonPressMask | ButtonReleaseMask |
			ButtonMotionMask, GrabModeSync, GrabModeSync, None,
			None);

	XMapWindow (screen->dpy (), outputFrame);

	/* Notify the decorators that an input frame has been created on
	 * this window so that they can react by actually create a decoration
	 * for it (while we use the default decorations) */
	XChangeProperty (screen->dpy (), window->id (),
			 dScreen->outputFrameAtom, XA_WINDOW, 32,
			 PropModeReplace, (unsigned char *) &outputFrame, 1);

	if (screen->XShape ())
	    XShapeSelectInput (screen->dpy (), outputFrame,
			       ShapeNotifyMask);

	/* invalidate the decoration so that it gets shaped */
	oldX = 0;
	oldY = 0;
	oldWidth  = 0;
	oldHeight = 0;

	frameDamage = XDamageCreate (screen->dpy (), outputFrame,
			             XDamageReportBoundingBox);

	dScreen->frames[outputFrame] = this;
    }

    if (x != oldX || y != oldY || width != oldWidth || height != oldHeight)
    {
	int    i = 0;
	oldX = x;
	oldY = y;
	oldWidth  = width;
	oldHeight = height;

	XMoveResizeWindow (screen->dpy (), outputFrame, x, y, width, height);
	XLowerWindow (screen->dpy (), outputFrame);


	rects[i].x	= 0;
	rects[i].y	= 0;
	rects[i].width  = width;
	rects[i].height = input.top;

	if (rects[i].width && rects[i].height)
	    i++;

	rects[i].x	= 0;
	rects[i].y	= input.top;
	rects[i].width  = input.left;
	rects[i].height = height - input.top - input.bottom;

	if (rects[i].width && rects[i].height)
	    i++;

	rects[i].x	= width - input.right;
	rects[i].y	= input.top;
	rects[i].width  = input.right;
	rects[i].height = height - input.top - input.bottom;

	if (rects[i].width && rects[i].height)
	    i++;

	rects[i].x	= 0;
	rects[i].y	= height - input.bottom;
	rects[i].width  = width;
	rects[i].height = input.bottom;

	if (rects[i].width && rects[i].height)
	    i++;

	XShapeCombineRectangles (screen->dpy (), outputFrame,
				 ShapeBounding, 0, 0, rects, i,
				 ShapeSet, YXBanded);

	frameRegion = CompRegion ();
    }

    XUngrabServer (screen->dpy ());
}

/*
 * DecorScreen::checkForDm
 *
 * Checks for a running decoration manager on the root window
 * and also checks to see what decoration modes it supports
 *
 * dmWin is set based on the window on supportingDmCheckAtom
 * on the root window. That's a window which is owned by
 * the decorator, so if it changes we need to invalidate
 * all the decorations
 */

void
DecorScreen::checkForDm (bool updateWindows)
{
    Atom	  actual;
    int		  result, format, dmSupports = 0;
    unsigned long n, left;
    unsigned char *data;
    Window	  dmWin = None;

    result = XGetWindowProperty (screen->dpy (), screen->root (),
				 supportingDmCheckAtom, 0L, 1L, false,
				 XA_WINDOW, &actual, &format,
				 &n, &left, &data);

    if (result == Success && n && data)
    {
	XWindowAttributes attr;

	memcpy (&dmWin, data, sizeof (Window));
	XFree (data);

	CompScreen::checkForError (screen->dpy ());

	XGetWindowAttributes (screen->dpy (), dmWin, &attr);

	if (CompScreen::checkForError (screen->dpy ()))
	    dmWin = None;
	else
	{
	    result = XGetWindowProperty (screen->dpy (), dmWin,
					 decorTypeAtom, 0L, 2L, false,
					 XA_ATOM, &actual, &format,
					 &n, &left, &data);
	    if (result == Success && n && data)
	    {
		Atom *ret = (Atom *) data;

		for (unsigned long i = 0; i < n; i++)
		{
		    if (ret[i] == decorTypePixmapAtom)
			dmSupports |= WINDOW_DECORATION_TYPE_PIXMAP;
		    else if (ret[i] == decorTypeWindowAtom)
			dmSupports |= WINDOW_DECORATION_TYPE_WINDOW;
		}

		if (!dmSupports)
		    dmWin = None;

		XFree (data);
	    }
	    else
		dmWin = None;
	}
    }

    /* Different decorator became active, update all decorations */
    if (dmWin != this->dmWin)
    {
	int i;

	this->dmSupports = dmSupports;

	/* Create new default decorations */
	screen->updateSupportedWmHints ();

	if (dmWin)
	{
	    for (i = 0; i < DECOR_NUM; i++)
	    {
		decor[i].updateDecoration (screen->root (), decorAtom[i], &mRequestor);
	    }
	}
	else
	{
	    /* No decorator active, destroy all decorations */
	    for (i = 0; i < DECOR_NUM; i++)
	    {
		decor[i].clear ();

		foreach (CompWindow *w, screen->windows ())
		    DecorWindow::get (w)->decor.mList.clear ();
	    }
	}

	this->dmWin = dmWin;

	if (updateWindows)
	{
	    foreach (CompWindow *w, screen->windows ())
		if (w->shaded () || w->isViewable ())
		    DecorWindow::get (w)->update (true);
	}
    }
}

/*
 * DecorWindow::updateFrameRegion
 *
 * Shapes the toplevel frame region according to the rects
 * in the decoration that we have. This is a wrapped function
 *
 */
void
DecorWindow::updateFrameRegion (CompRegion &region)
{
    window->updateFrameRegion (region);
    if (wd)
    {
	if (!frameRegion.isEmpty ())
	{
	    int x, y;

	    x = window->geometry (). x ();
	    y = window->geometry (). y ();

	    region += frameRegion.translated (x - wd->decor->input.left,
					      y - wd->decor->input.top);
	}
	else
	{
	    region += infiniteRegion;
	}
    }

    updateReg = true;
    updateMatrix = true;
}

/*
 * DecorWindow::updateWindowRegions
 *
 * Used to update the region that the window type
 * decorations occupty when the window is moved */
void
DecorWindow::updateWindowRegions ()
{
    const CompRect &input (window->inputRect ());

    if (regions.size () != gWindow->textures ().size ())
	regions.resize (gWindow->textures ().size ());

    for (unsigned int i = 0; i < gWindow->textures ().size (); i++)
    {
	regions[i] = CompRegion (*gWindow->textures ()[i]);
	regions[i].translate (input.x (), input.y ());
	regions[i] &= window->frameRegion ();
    }

    updateReg = false;
}

/*
 * DecorWindow::windowNotify
 *
 * Window event notification handler. On various
 * events on windows we need to update the decorations
 *
 */
void
DecorWindow::windowNotify (CompWindowNotify n)
{
    switch (n)
    {
	case CompWindowNotifyMap:

	    /* When the switcher is mapped, it has no frame window
	     * so the frame window for it needs to mapped manually */
	    if (isSwitcher)
	    {
		update (true);
		XMapWindow (screen->dpy (), inputFrame);
		break;
	    }

	    /* For non-switcher windows we need to update the decoration
	     * anyways, since the window is unmapped. Also need to
	     * update the shadow clip regions for panels and other windows */
	    update (true);
	    updateDecorationScale ();
	    if (dScreen->mMenusClipGroup.pushClippable (this))
		updateGroupShadows ();

	    break;

	case CompWindowNotifyUnmap:
	{

	    /* When the switcher is unmapped, it has no frame window
	     * so the frame window for it needs to unmapped manually */
	    if (isSwitcher)
	    {
		update (true);
		XUnmapWindow (screen->dpy (), inputFrame);
		break;
	    }

	    /* For non-switcher windows we need to update the decoration
	     * anyways, since the window is unmapped. Also need to
	     * update the shadow clip regions for panels and other windows */
	    update (true);
	    updateDecorationScale ();
	    /* Preserve the group shadow update ptr */
	    DecorClipGroupInterface *clipGroup = mClipGroup;

	    if (dScreen->mMenusClipGroup.popClippable (this))
		if (clipGroup)
		    clipGroup->updateAllShadows ();
	    break;
	}
	case CompWindowNotifyUnreparent:
	{
	    /* Compiz detaches the frame window from
	     * the client on unreparent, so we must
	     * destroy our frame windows by forcing
	     * this window to have no decorations */

	    update (false);
	    
	    break;
	}
	case CompWindowNotifyReparent:
	    /* Always update decorations when a window gets reparented */
	    update (true);
	    break;
	case CompWindowNotifyShade:
	    /* We get the notification for shade before the window is
	     * actually resized which means that calling update ->
	     * damageOutputExtents here will not do anything useful for us
	     * so we need to track when windows are (un)shading and then wait
	     * for the following resize notification to actually
	     * update their decoration (since at this point they would have
	     * been resized)
	     */
	    shading = true;
	    unshading = false;
	    break;
	case CompWindowNotifyUnshade:
	    unshading = true;
	    shading = false;
	    break;
	default:
	    break;
    }

    window->windowNotify (n);
}

/*
 * DecorWindow::updateSwitcher
 *
 * Check this window to see if it is a switcher,
 * if so, update the switcher flag 
 */
void
DecorWindow::updateSwitcher ()
{
    Atom	  actualType;
    int	      	  actualFmt;
    unsigned long nitems, nleft;
    unsigned long *data;

    DECOR_SCREEN (screen);

    if (XGetWindowProperty (screen->dpy (), window->id (),
		    	    ds->decorSwitchWindowAtom, 0L, 1024L,
		    	    false, XA_WINDOW, &actualType, &actualFmt,
		    	    &nitems, &nleft, (unsigned char **) &data) == Success)
    {
	if (data)
	    XFree (data);

	if (nitems == 1)
	{
	    isSwitcher = true;
	    return;
	}
    }

    isSwitcher = false;
}


/*
 * DecorScreen::handleEvent
 *
 * Handles X11 events
 */
void
DecorScreen::handleEvent (XEvent *event)
{
    Window  activeWindow = screen->activeWindow ();
    CompWindow *w;

    switch (event->type) {
	case DestroyNotify:
	    /* When a decorator selection owner window is destroyed
	     * it means that this decorator went away, so we need
	     * to account for this */
	    w = screen->findWindow (event->xdestroywindow.window);
	    if (w)
	    {
		if (w->id () == dmWin)
		    checkForDm (true);
	    }
	    break;
	case ClientMessage:
	    /* Update decorations whenever someone requests frame extents
	     * so that core doesn't reply with the wrong extents when
	     * when handleEvent is passed to core
	     */
	    if (event->xclient.message_type == requestFrameExtentsAtom)
	    {
		w = screen->findWindow (event->xclient.window);
		if (w)
		    DecorWindow::get (w)->update (true);
	    }
	    /* A decoration is pending creation, allow it to be created */
	    if (event->xclient.message_type == decorPendingAtom)
	    {
		CompWindow *w = screen->findWindow (event->xclient.window);

		if (w)
		{
		    DecorWindow *dw = DecorWindow::get (w);

		    dw->mRequestor.handlePending (event->xclient.data.l);
		}
	    }
	    break;
	default:
	    /* Check for damage events. If the output or input window
	     * or a texture is updated then damage output extents.
	     */
	    if (cmActive &&
		event->type == cScreen->damageEvent () + XDamageNotify)
	    {
		XDamageNotifyEvent *de = (XDamageNotifyEvent *) event;

		if (frames.find (de->drawable) != frames.end ())
		    frames[de->drawable]->cWindow->damageOutputExtents ();

		foreach (DecorTexture *t, textures)
		{
		    if (t->pixmap->getPixmap () == de->drawable)
		    {
			foreach (CompWindow *w, screen->windows ())
			{
			    if (w->shaded () || w->mapNum ())
			    {
				DECOR_WINDOW (w);

				if (dw->wd && dw->wd->decor->texture == t)
				    dw->cWindow->damageOutputExtents ();
			    }
			}
			break;
		    }
		}
	    }
	    break;
    }

    screen->handleEvent (event);

    /* If the active window changed, update the decoration,
     * as long as the decoration isn't animating out */
    if (screen->activeWindow () != activeWindow)
    {
	w = screen->findWindow (activeWindow);
	if (w && !w->hasUnmapReference ())
	    DecorWindow::get (w)->update (true);

	w = screen->findWindow (screen->activeWindow ());
	if (w)
	    DecorWindow::get (w)->update (true);
    }

    switch (event->type) {
	case PropertyNotify:
	    /* When the switcher atom changes we should probably
	     * update the switcher property on this window */
	    if (event->xproperty.atom == decorSwitchWindowAtom)
	    {
		CompWindow    *w = screen->findWindow (event->xproperty.window);

		if (w)
		{
		    DECOR_WINDOW (w);

		    if (dw->isSwitcher && !event->xproperty.state == PropertyDelete)
			dw->updateSwitcher ();
		}
	    }
	    /* Decorator has created or updated a decoration for this window,
	     * update the decoration */
	    else if (event->xproperty.atom == winDecorAtom)
	    {
		w = screen->findWindow (event->xproperty.window);
		if (w)
		{
		    DECOR_WINDOW (w);

		    dw->updateDecoration ();
		    dw->update (true);
		}
	    }
	    /* _MOTIF_WM_HINTS has been set on this window, it may not
	     * may need to be decorated */
	    else if (event->xproperty.atom == Atoms::mwmHints)
	    {
		w = screen->findWindow (event->xproperty.window);
		if (w)
		    DecorWindow::get (w)->update (true);
	    }
	    else
	    {
		if (event->xproperty.window == screen->root ())
		{
		    /* If the supportingDmCheckAtom changed on the root window
		     * it could mean that the decorator changed, so we need
		     * to update decorations */
		    if (event->xproperty.atom == supportingDmCheckAtom)
		    {
			checkForDm (true);
		    }
		    else
		    {
			/* A default decoration changed */
			for (int i = 0; i < DECOR_NUM; i++)
			{
			    if (event->xproperty.atom == decorAtom[i])
			    {
				decor[i].updateDecoration (screen->root (),
							   decorAtom[i],
							   &mRequestor);

				foreach (CompWindow *w, screen->windows ())
				    DecorWindow::get (w)->update (true);
			    }
			}
		    }
		}
	    }
	    break;
	/* Whenever a window is configured, we need to update the frame window */
	case ConfigureNotify:
	    w = screen->findTopLevelWindow (event->xconfigure.window);
	    if (w)
	    {
		DECOR_WINDOW (w);
		if (!w->hasUnmapReference () && dw->wd && dw->wd->decor)
		    dw->updateFrame ();
	    }
	    break;
	case DestroyNotify:
	    /* Only for when the client window gets destroyed */
	    w = screen->findTopLevelWindow (event->xproperty.window);
	    if (w)
	    {
		DECOR_WINDOW (w);
		if (dw->inputFrame &&
		    dw->inputFrame == event->xdestroywindow.window)
		{
		    XDeleteProperty (screen->dpy (), w->id (),
				     inputFrameAtom);
		    dw->inputFrame = None;
		}
		else if (dw->outputFrame &&
		         dw->outputFrame == event->xdestroywindow.window)
		{
		    XDeleteProperty (screen->dpy (), w->id (),
				     outputFrameAtom);
		    dw->outputFrame = None;
		}
	    }
	    break;
	/* Add new shape rects to frame region in case of a
	 * shape notification */
	default:
	    if (screen->XShape () && event->type ==
		screen->shapeEvent () + ShapeNotify)
	    {
		w = screen->findWindow (((XShapeEvent *) event)->window);
		if (w)
		    DecorWindow::get (w)->update (true);
		else
		{
		    foreach (w, screen->windows ())
		    {
			DECOR_WINDOW (w);
			if (dw->inputFrame ==
			    ((XShapeEvent *) event)->window)
			{
			    XRectangle *shapeRects = 0;
			    int order, n;

			    dw->frameRegion = CompRegion ();

			    shapeRects =
				XShapeGetRectangles (screen->dpy (),
				    dw->inputFrame, ShapeInput,
				    &n, &order);
			    if (!shapeRects || !n)
				break;

			    for (int i = 0; i < n; i++)
				dw->frameRegion +=
				    CompRegion (shapeRects[i].x,
					        shapeRects[i].y,
						shapeRects[i].width,
						shapeRects[i].height);

			    w->updateFrameRegion ();

			    XFree (shapeRects);
			}
			else if (dw->outputFrame ==
			         ((XShapeEvent *) event)->window)
			{
			    XRectangle *shapeRects = 0;
			    int order, n;

			    dw->frameRegion = CompRegion ();

			    shapeRects =
				XShapeGetRectangles (screen->dpy (),
				    dw->outputFrame, ShapeBounding,
				    &n, &order);
			    if (!n || !shapeRects)
				break;

			    for (int i = 0; i < n; i++)
				dw->frameRegion +=
				    CompRegion (shapeRects[i].x,
					        shapeRects[i].y,
						shapeRects[i].width,
						shapeRects[i].height);

			    w->updateFrameRegion ();

			    XFree (shapeRects);
			}
		    }
		}
	    }
	    break;
    }
}

/*
 * DecorWindow::damageRect
 *
 * When this window is first presented to the user, we need to update
 * the decoration since it is now visible
 *
 */

bool
DecorWindow::damageRect (bool initial, const CompRect &rect)
{
    if (initial)
	update (true);

    return cWindow->damageRect (initial, rect);
}

/*
 * DecorWindow::getOutputExtents
 *
 * Extend "output extents" (eg decoration + shadows) for this
 * window if necessary
 */

void
DecorWindow::getOutputExtents (CompWindowExtents& output)
{
    window->getOutputExtents (output);

    if (wd)
    {
	CompWindowExtents *e = &wd->decor->output;

	if (e->left > output.left)
	    output.left = e->left;
	if (e->right > output.right)
	    output.right = e->right;
	if (e->top > output.top)
	    output.top = e->top;
	if (e->bottom > output.bottom)
	    output.bottom = e->bottom;
    }
}

/*
 * DecorScreen::updateDefaultShadowProperty
 *
 * Set some default shadow options on the root
 * window in case the default decorator doesn't
 * use custom shadows
 */

void
DecorScreen::updateDefaultShadowProperty ()
{
    long data[8];
    CompOption *activeColorOption = CompOption::findOption (getOptions (), "active_shadow_color");
    CompOption *inactiveColorOption = CompOption::findOption (getOptions (), "inactive_shadow_color");
    char *colorString[2];
    XTextProperty xtp;

    if (!activeColorOption || !inactiveColorOption)
	return;

    colorString[0] = strdup (CompOption::colorToString (activeColorOption->value ().c ()).c_str ());
    colorString[1] = strdup (CompOption::colorToString (inactiveColorOption->value ().c ()).c_str ());

    /* 1) Active Shadow Radius
     * 2) Active Shadow Opacity
     * 3) Active Shadow Offset X
     * 4) Active Shadow Offset Y
     * 5) Inactive Shadow Radius
     * 6) Inactive Shadow Opacity
     * 7) Inactive Shadow Offset X
     * 8) Inactive Shadow Offset Y
     */

    /* the precision is 0.0001, so multiply by 1000 */
    data[0] = optionGetActiveShadowRadius () * 1000;
    data[1] = optionGetActiveShadowOpacity () * 1000;
    data[2] = optionGetActiveShadowXOffset ();
    data[3] = optionGetActiveShadowYOffset ();
    data[4] = optionGetInactiveShadowRadius () * 1000;
    data[5] = optionGetInactiveShadowOpacity () * 1000;
    data[6] = optionGetInactiveShadowXOffset ();
    data[7] = optionGetInactiveShadowYOffset ();


    XChangeProperty (screen->dpy (), screen->root (),
		      shadowInfoAtom, XA_INTEGER, 32,
		      PropModeReplace, (unsigned char *) data, 8);

    if (XStringListToTextProperty (colorString, 2, &xtp))
    {
	XSetTextProperty (screen->dpy (), screen->root (), &xtp, shadowColorAtom);
	XFree (xtp.value);
    }

    free (colorString[0]);
    free (colorString[1]);
}

bool
DecorScreen::setOption (const CompString  &name,
			CompOption::Value &value)
{
    unsigned int index;

    bool rv = DecorOptions::setOption (name, value);

    if (!rv || !CompOption::findOption (getOptions (), name, &index))
	return false;

    switch (index) {
	case DecorOptions::Command:
	    if (!dmWin)
		screen->runCommand (optionGetCommand ());
	    break;
	case DecorOptions::ShadowMatch:
	    {
		CompString matchString;

		/*
		Make sure RGBA matching is always present and disable shadows
		for RGBA windows by default if the user didn't specify an
		RGBA match.
		Reasoning for that is that shadows are desired for some RGBA
		windows (e.g. rectangular windows that just happen to have an
		RGBA colormap), while it's absolutely undesired for others
		(especially shaped ones) ... by enforcing no shadows for RGBA
		windows by default, we are flexible to user desires while still
		making sure we don't show ugliness by default
		*/

		matchString = optionGetShadowMatch ().toString ();
		if (matchString.find ("rgba=") == CompString::npos)
		{
		    CompMatch rgbaMatch ("rgba=0");
		    optionGetShadowMatch () &= rgbaMatch;
		}
	    }
	    /* fall-through intended */
	case DecorOptions::DecorationMatch:
	    foreach (CompWindow *w, screen->windows ())
		DecorWindow::get (w)->update (true);
	    break;
	case DecorOptions::ActiveShadowRadius:
	case DecorOptions::ActiveShadowOpacity:
	case DecorOptions::ActiveShadowColor:
	case DecorOptions::ActiveShadowXOffset:
	case DecorOptions::ActiveShadowYOffset:
	case DecorOptions::InactiveShadowRadius:
	case DecorOptions::InactiveShadowOpacity:
	case DecorOptions::InactiveShadowColor:
	case DecorOptions::InactiveShadowXOffset:
	case DecorOptions::InactiveShadowYOffset:
	    updateDefaultShadowProperty ();
	    break;
	default:
	    break;
    }

    return rv;
}

/*
 * DecorWindow::moveNotify
 *
 * Translate window scaled quad boxes for movement
 * and also recompute shadow clip regions
 * for panels and menus
 */
void
DecorWindow::moveNotify (int dx, int dy, bool immediate)
{
    if (wd)
    {
	for (int i = 0; i < wd->nQuad; i++)
	{
	    wd->quad[i].box.x1 += dx;
	    wd->quad[i].box.y1 += dy;
	    wd->quad[i].box.x2 += dx;
	    wd->quad[i].box.y2 += dy;
	}
    }

    updateReg = true;
    updateMatrix = true;

    mInputRegion.translate (dx, dy);
    mOutputRegion.translate (dx, dy);

    if (dScreen->cmActive && mClipGroup)
	updateGroupShadows ();

    window->moveNotify (dx, dy, immediate);
}

/*
 * DecorWindow::resizeNotify
 *
 * Called whenever a window is resized. Update scaled quads and 
 * recompute shadow region
 *
 */

void
DecorWindow::resizeNotify (int dx, int dy, int dwidth, int dheight)
{
    if (shading || unshading)
    {
	shading = false;
	unshading = false;
    }
    /* FIXME: we should not need a timer for calling decorWindowUpdate,
       and only call updateWindowDecorationScale if decorWindowUpdate
       returns false. Unfortunately, decorWindowUpdate may call
       updateWindowOutputExtents, which may call WindowResizeNotify. As
       we never should call a wrapped function that's currently
       processed, we need the timer for the moment. updateWindowOutputExtents
       should be fixed so that it does not emit a resize notification. */
    updateMatrix = true;
    updateReg = true;

    mInputRegion = CompRegion (window->inputRect ());
    mOutputRegion = CompRegion (window->outputRect ());
    if (dScreen->cmActive && mClipGroup)
	updateGroupShadows ();

    updateReg = true;

    window->resizeNotify (dx, dy, dwidth, dheight);
}


/*
 * DecorWindow::stateChangeNotify
 *
 * Called whenever a window state changes, we might need to use
 * different extents in case the decoration didn't change
 *
 */
void
DecorWindow::stateChangeNotify (unsigned int lastState)
{
    if (wd && wd->decor)
    {
	CompPoint oldShift = compiz::window::extents::shift (window->border (), window->sizeHints ().win_gravity);
	

	if ((window->state () & MAXIMIZE_STATE))
	    window->setWindowFrameExtents (&wd->decor->maxBorder,
					   &wd->decor->maxInput);
	else
	    window->setWindowFrameExtents (&wd->decor->border,
					   &wd->decor->input);

	/* Since we immediately update the frame extents, we must
	 * also update the stored saved window geometry in order
	 * to prevent the window from shifting back too far once
	 * unmaximized */

	CompPoint movement = compiz::window::extents::shift (window->border (), window->sizeHints ().win_gravity) - oldShift;

	if (window->saveMask () & CWX)
	    window->saveWc ().x += movement.x ();

	if (window->saveMask () & CWY)
	    window->saveWc ().y += movement.y ();

	updateFrame ();
    }

    window->stateChangeNotify (lastState);
}

void
DecorScreen::matchPropertyChanged (CompWindow *w)
{
    DecorWindow::get (w)->update (true);

    screen->matchPropertyChanged (w);
}

/*
 * DecorScreen::addSupportedAtoms
 *
 * _NET_REQUEST_FRAME_EXTENTS is only supported where
 * a decorator is running, so add that to _NET_WM_SUPPORTED
 * where that is the case
 *
 */
void
DecorScreen::addSupportedAtoms (std::vector<Atom> &atoms)
{
    screen->addSupportedAtoms (atoms);

    /* Don't support _NET_REQUEST_FRAME_EXTENTS
     * where there is no decorator running yet */
    if (dmWin)
	atoms.push_back (requestFrameExtentsAtom);
}

void
DecorWindow::updateHandlers ()
{
    if (dScreen->cmActive)
    {
	gWindow = GLWindow::get (window);
	cWindow = CompositeWindow::get (window);

	CompositeWindowInterface::setHandler (cWindow);
	GLWindowInterface::setHandler (gWindow);
    }
    else
    {
	CompositeWindowInterface::setHandler (cWindow, false);
	GLWindowInterface::setHandler (gWindow, false);

	gWindow = NULL;
	cWindow = NULL;
    }
}

/*
 * DecorScreen::decoratorStartTimeout
 *
 * Start a decorator in case there isn't one running
 *
 */
bool
DecorScreen::decoratorStartTimeout ()
{
    if (!dmWin)
	screen->runCommand (optionGetCommand ());

    /* Update all the windows */
    foreach (CompWindow *w, screen->windows ())
    {
	DECOR_WINDOW (w);

	dw->updateHandlers ();

	dw->updateSwitcher ();

	if (!w->overrideRedirect () || dw->isSwitcher)
	    dw->updateDecoration ();

	if (w->shaded () || w->isViewable ())
	    dw->update (true);
    }

    return false;
}

bool
DecorScreen::registerPaintHandler (compiz::composite::PaintHandler *p)
{
    cmActive = true;
    return cScreen->registerPaintHandler (p);
}

void
DecorScreen::unregisterPaintHandler ()
{
    cmActive = false;
    return cScreen->unregisterPaintHandler ();
}

/*
 * DecorScreen::DecorScreen
 *
 * Initialize atoms, and create a
 * default "window type" decoration
 * and ensure that _NET_REQUEST_FRAME_EXTENTS
 * gets added to _NET_WM_SUPPORTED
 *
 */
DecorScreen::DecorScreen (CompScreen *s) :
    PluginClassHandler<DecorScreen,CompScreen> (s),
    cScreen (CompositeScreen::get (s)),
    textures (),
    dmWin (None),
    dmSupports (0),
    cmActive (false),
    windowDefault (new Decoration (WINDOW_DECORATION_TYPE_WINDOW,
				   decor_extents_t (),
				   decor_extents_t (),
				   decor_extents_t (),
				   decor_extents_t (),
				   0,
				   0,
				   0,
				   0,
				   0,
				   None,
				   boost::shared_array <decor_quad_t> (NULL),
				   0,
				   screen->root (),
				   NULL)),
    mMenusClipGroup (CompMatch ("type=Dock | type=DropdownMenu | type=PopupMenu")),
    mRequestor (screen->dpy (), screen->root (), &(decor[DECOR_ACTIVE]))
{
    supportingDmCheckAtom =
	XInternAtom (s->dpy (), DECOR_SUPPORTING_DM_CHECK_ATOM_NAME, 0);
    winDecorAtom =
	XInternAtom (s->dpy (), DECOR_WINDOW_ATOM_NAME, 0);
    decorAtom[DECOR_BARE] =
        XInternAtom (s->dpy (), DECOR_BARE_ATOM_NAME, 0);
    decorAtom[DECOR_ACTIVE] =
	XInternAtom (s->dpy (), DECOR_ACTIVE_ATOM_NAME, 0);
    inputFrameAtom =
	XInternAtom (s->dpy (), DECOR_INPUT_FRAME_ATOM_NAME, 0);
    outputFrameAtom =
	XInternAtom (s->dpy (), DECOR_OUTPUT_FRAME_ATOM_NAME, 0);
    decorTypeAtom =
	XInternAtom (s->dpy (), DECOR_TYPE_ATOM_NAME, 0);
    decorTypePixmapAtom =
	XInternAtom (s->dpy (), DECOR_TYPE_PIXMAP_ATOM_NAME, 0);
    decorTypeWindowAtom =
	XInternAtom (s->dpy (), DECOR_TYPE_WINDOW_ATOM_NAME, 0);
    decorSwitchWindowAtom =
	XInternAtom (s->dpy (), DECOR_SWITCH_WINDOW_ATOM_NAME, 0);
    decorPendingAtom =
	XInternAtom (s->dpy (), "_COMPIZ_DECOR_PENDING", 0);
    decorRequestAtom =
	XInternAtom (s->dpy (), "_COMPIZ_DECOR_REQUEST", 0);
    requestFrameExtentsAtom =
        XInternAtom (s->dpy (), "_NET_REQUEST_FRAME_EXTENTS", 0);
    shadowColorAtom =
	XInternAtom (s->dpy (), "_COMPIZ_NET_CM_SHADOW_COLOR", 0);
    shadowInfoAtom =
	XInternAtom (s->dpy (), "_COMPIZ_NET_CM_SHADOW_PROPERTIES", 0);

    cmActive = (cScreen) ? cScreen->compositingActive () &&
               GLScreen::get (s) != NULL : false;

    checkForDm (false);

    decoratorStart.start (boost::bind (&DecorScreen::decoratorStartTimeout,
				       this),
			  0);

    ScreenInterface::setHandler (s);
    CompositeScreenInterface::setHandler (cScreen);
    screen->updateSupportedWmHints ();
}

/*
 * DecorScreen::~DecorScreen
 *
 * Ensure that _NET_REQUEST_FRAME_EXTENTS
 * is cleared from _NET_WM_SUPPORTED
 *
 */
DecorScreen::~DecorScreen ()
{
    for (unsigned int i = 0; i < DECOR_NUM; i++)
        decor[i].clear ();

    screen->addSupportedAtomsSetEnabled (this, false);
    screen->updateSupportedWmHints ();
}

DecorWindow::DecorWindow (CompWindow *w) :
    PluginClassHandler<DecorWindow,CompWindow> (w),
    window (w),
    gWindow (GLWindow::get (w)),
    cWindow (CompositeWindow::get (w)),
    dScreen (DecorScreen::get (screen)),
    wd (NULL),
    inputFrame (None),
    outputFrame (None),
    pixmapFailed (false),
    regions (),
    updateReg (true),
    updateMatrix (true),
    unshading (false),
    shading (false),
    isSwitcher (false),
    frameExtentsRequested (false),
    mClipGroup (NULL),
    mOutputRegion (window->outputRect ()),
    mInputRegion (window->inputRect ()),
    mRequestor (screen->dpy (), w->id (), &decor)
{
    WindowInterface::setHandler (window);

    /* FIXME :DecorWindow::update can call updateWindowOutputExtents
     * which will call a zero-diff resizeNotify. Since this window
     * might be part of a startup procedure, we can't assume that
     * all other windows in the list are necessarily safe to use
     * (since DecorWindow::DecorWindow might not have been called
     * for them) so we need to turn off resize notifications
     * and turn them back on once we're done updating the decoration
     */

    window->resizeNotifySetEnabled (this, false);

    if (!dScreen->decoratorStart.active ())
    {
	updateHandlers ();

	updateSwitcher ();

	if (!w->overrideRedirect () || isSwitcher)
	    updateDecoration ();

	if (w->shaded () || w->isViewable ())
	    update (true);
    }

    window->resizeNotifySetEnabled (this, true);

    if (!window->invisible ())
	if (dScreen->mMenusClipGroup.pushClippable (this))
	    updateGroupShadows ();
}

/* 
 * DecorWindow::~DecorWindow
 * 
 * On tear-down, we need to shift all windows
 * back to their original positions
 */
DecorWindow::~DecorWindow ()
{
    if (!window->destroyed ())
	update (false);

    if (wd)
	WindowDecoration::destroy (wd);

    if (mClipGroup)
	mClipGroup->popClippable (this);

    decor.mList.clear ();
}

bool
DecorPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	 return false;

    return true;
}

