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

#ifndef _COMPOSITE_PRIVATES_H
#define _COMPOSITE_PRIVATES_H

#include <memory>
#include <boost/shared_ptr.hpp>

#include <composite/composite.h>
#include <core/atoms.h>
#include <map>

#include "pixmapbinding.h"
#include "composite_options.h"

extern CompPlugin::VTable *compositeVTable;

extern CompWindow *lastDamagedWindow;

class PrivateCompositeScreen :
    ScreenInterface,
    public CompositeOptions
{
    public:
	PrivateCompositeScreen (CompositeScreen *cs);
	~PrivateCompositeScreen ();

	bool setOption (const CompString &name, CompOption::Value &value);

	void outputChangeNotify ();

	void handleEvent (XEvent *event);

	void makeOutputWindow ();

	bool init ();

	void handleExposeEvent (XExposeEvent *event);

	void detectRefreshRate ();

	void scheduleRepaint ();

    public:

	CompositeScreen *cScreen;

	int compositeEvent, compositeError, compositeOpcode;
	int damageEvent, damageError;
	int fixesEvent, fixesError, fixesVersion;

	bool shapeExtension;
	int  shapeEvent, shapeError;

	bool randrExtension;
	int  randrEvent, randrError;

	CompRegion    damage;
	unsigned long damageMask;

	CompRegion    tmpRegion;

	Window	      overlay;
	Window	      output;

	std::list <CompRect> exposeRects;

	CompPoint windowPaintOffset;

	int overlayWindowCount;
	bool outputShapeChanged;

	struct timeval lastRedraw;
	int            redrawTime;
	int            optimalRedrawTime;
	bool           scheduled, painting, reschedule;

	bool slowAnimations;

	CompTimer paintTimer;

	compiz::composite::PaintHandler *pHnd;

	CompositeFPSLimiterMode FPSLimiterMode;

	CompWindowList withDestroyedWindows;

	Atom cmSnAtom;
	Window newCmSnOwner;

	/* Map Damage handle to its bounding box */
	std::map<Damage, XRectangle> damages;
};

class PrivateCompositeWindow :
    public WindowInterface,
    public CompositePixmapRebindInterface,
    public WindowPixmapGetInterface,
    public WindowAttributesGetInterface,
    public PixmapFreezerInterface
{
    public:
	PrivateCompositeWindow (CompWindow *w, CompositeWindow *cw);
	~PrivateCompositeWindow ();

	void windowNotify (CompWindowNotify n);
	void resizeNotify (int dx, int dy, int dwidth, int dheight);
	void moveNotify (int dx, int dy, bool now);

	Pixmap pixmap () const;
	bool   bind ();
	const CompSize & size () const;
	void release ();
	void setNewPixmapReadyCallback (const boost::function <void ()> &);
	void allowFurtherRebindAttempts ();
	bool frozen ();

	static void handleDamageRect (CompositeWindow *w,
				      int             x,
				      int             y,
				      int             width,
				      int             height);

    public:
	CompWindow      *window;
	CompositeWindow *cWindow;
	CompositeScreen *cScreen;

	PixmapBinding mPixmapBinding;

	Damage	      damage;

	bool	      damaged;
	bool	      redirected;
	bool          overlayWindow;

	unsigned short opacity;
	unsigned short brightness;
	unsigned short saturation;

	XRectangle *damageRects;
	int        sizeDamage;
	int        nDamage;

    private:

	bool getAttributes (XWindowAttributes &);
	WindowPixmapInterface::Ptr getPixmap ();
};

#endif
