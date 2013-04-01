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

#ifndef _COMPIZ_COMPOSITE_H
#define _COMPIZ_COMPOSITE_H

#include <X11/extensions/Xcomposite.h>

#define COMPIZ_COMPOSITE_ABI 5

#include "core/pluginclasshandler.h"
#include "core/timer.h"
#include "core/output.h"
#include "core/screen.h"
#include "core/wrapsystem.h"

#define COMPOSITE_SCREEN_DAMAGE_PENDING_MASK (1 << 0)
#define COMPOSITE_SCREEN_DAMAGE_REGION_MASK  (1 << 1)
#define COMPOSITE_SCREEN_DAMAGE_ALL_MASK     (1 << 2)

#define OPAQUE 0xffff
#define COLOR  0xffff
#define BRIGHT 0xffff

/**
 * Used to indicate only part of the screen is being redrawn
 */
#define PAINT_SCREEN_REGION_MASK		   (1 << 0)
/**
 * Used to indicate that the whole screen is being redrawn
 */
#define PAINT_SCREEN_FULL_MASK			   (1 << 1)
/**
 * Used to indicate that every window on this screen will be
 * transformed, so non-painted areas should be
 * double-buffered
 */
#define PAINT_SCREEN_TRANSFORMED_MASK		   (1 << 2)
/**
 * Used to indicate that some windows on this screen will
 * be drawn transformed
 */
#define PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK (1 << 3)
/**
 * Used to indicate that nothing is being drawn on this pass
 */
#define PAINT_SCREEN_CLEAR_MASK			   (1 << 4)
/**
 * Used to indicate that occlusion detection is not in use
 * on this pass
 */
#define PAINT_SCREEN_NO_OCCLUSION_DETECTION_MASK   (1 << 5)
/**
 * Used to indicate that no background will be drawn on this
 * pass
 */
#define PAINT_SCREEN_NO_BACKGROUND_MASK            (1 << 6)


typedef enum
{
    CompositeFPSLimiterModeDisabled = 0,
    CompositeFPSLimiterModeDefault,
    CompositeFPSLimiterModeVSyncLike
} CompositeFPSLimiterMode;

class PrivateCompositeScreen;
class PrivateCompositeWindow;
class CompositeScreen;
class CompositeWindow;

namespace compiz
{
namespace composite
{
class PaintHandler {
public:
    virtual ~PaintHandler () {};

    virtual void paintOutputs (CompOutput::ptrList &outputs,
			       unsigned int        mask,
			       const CompRegion    &region) = 0;

    virtual bool hasVSync () { return false; };
    virtual bool requiredForcedRefreshRate () { return false; };

    virtual void prepareDrawing () {};
    virtual bool compositingActive () { return false; };
};
}
}

/**
 * Wrapable function interface for CompositeScreen
 */
class CompositeScreenInterface :
    public WrapableInterface<CompositeScreen, CompositeScreenInterface>
{
    public:
    
	/**
	 * Hook which activates just before the screen is painted,
	 * plugins should use this to calculate animation parameters
	 *
	 * @param msSinceLastPaint Describes how many milliseconds have passed
	 * since the last screen repaint
	 */
	virtual void preparePaint (int);
	
	/**
	 * Hook which activates right after the screen is painted,
	 * plugins should use this to run post-paint cleanup, damage handling
	 * and setting next paint variables
	 *
	 */
	virtual void donePaint ();
	
	/**
	 * Hookable function which dispatches painting of outputs
	 * to rendering plugins such as OpenGL. Hook this function
	 * to change which outputs are painted, or to paint them
	 * manually if you are rendering
	 */
	virtual void paint (CompOutput::ptrList &outputs, unsigned int);
	
	/**
	 * Hookable function which gets a list of windows that need to be 
	 * evaluated for repainting
	 */
	virtual const CompWindowList & getWindowPaintList ();

	/**
	 * Hookable function to register a new paint handler, overload
	 * and insert your own paint handler if you want to prevent
	 * another one from being loaded
	 */
	virtual bool registerPaintHandler (compiz::composite::PaintHandler *pHnd);

	/**
	 * Hookable function to notify unregistration of a paint handler
	 *
	 */
	virtual void unregisterPaintHandler ();

	/**
	 * Hookable function to damage regions directly
	 */
	virtual void damageRegion (const CompRegion &r);
};

extern template class PluginClassHandler<CompositeScreen, CompScreen, COMPIZ_COMPOSITE_ABI>;

class CompositeScreen :
    public WrapableHandler<CompositeScreenInterface, 7>,
    public PluginClassHandler<CompositeScreen, CompScreen, COMPIZ_COMPOSITE_ABI>,
    public CompOption::Class
{
    public:
	CompositeScreen (CompScreen *s);
	~CompositeScreen ();

	CompOption::Vector & getOptions ();
        bool setOption (const CompString &name, CompOption::Value &value);

	bool compositingActive ();

	/**
	 * Returns the value of an XDamage Extension event signature
	 */	
	int damageEvent ();

	/**
	 * Causes the entire screen to be redrawn on the next
	 * event loop
	 */
	void damageScreen ();

	void damagePending ();
	

	unsigned int damageMask ();
	const CompRegion & currentDamage () const;

	void showOutputWindow ();
	void hideOutputWindow ();
	void updateOutputWindow ();
	bool outputWindowChanged () const;

	Window overlay ();
	Window output ();

	int & overlayWindowCount ();

	void setWindowPaintOffset (int x, int y);
	CompPoint windowPaintOffset ();

	/**
	 * Limits the number of redraws per second
	 */	
	void setFPSLimiterMode (CompositeFPSLimiterMode newMode);
	CompositeFPSLimiterMode FPSLimiterMode ();

	int redrawTime ();
	int optimalRedrawTime ();

	bool handlePaintTimeout ();

	WRAPABLE_HND (0, CompositeScreenInterface, void, preparePaint, int);
	WRAPABLE_HND (1, CompositeScreenInterface, void, donePaint);
	WRAPABLE_HND (2, CompositeScreenInterface, void, paint,
		      CompOutput::ptrList &outputs, unsigned int);

	WRAPABLE_HND (3, CompositeScreenInterface, const CompWindowList &,
		      getWindowPaintList);

	WRAPABLE_HND (4, CompositeScreenInterface, bool, registerPaintHandler, compiz::composite::PaintHandler *);
	WRAPABLE_HND (5, CompositeScreenInterface, void, unregisterPaintHandler);

	/**
	 * Adds a specific region to be redrawn on the next
	 * event loop
	 */
	WRAPABLE_HND (6, CompositeScreenInterface, void, damageRegion, const CompRegion &);

	friend class PrivateCompositeDisplay;

    private:
	PrivateCompositeScreen *priv;

    public:
	static bool toggleSlowAnimations (CompAction         *action,
					  CompAction::State  state,
					  CompOption::Vector &options);
};

/*
  window paint flags

  bit 1-16 are used for read-only flags and they provide
  information that describe the screen rendering pass
  currently in process.

  bit 17-32 are writable flags and they provide information
  that is used to optimize rendering.
*/

/**
 * this flag is present when window is being painted
 * on a transformed screen.
 */
#define PAINT_WINDOW_ON_TRANSFORMED_SCREEN_MASK (1 << 0)

/**
 * this flag is present when window is being tested
 * for occlusion of other windows.
 */
#define PAINT_WINDOW_OCCLUSION_DETECTION_MASK   (1 << 1)

/**
 * this flag indicates that the window ist painted with
 * an offset
 */
#define PAINT_WINDOW_WITH_OFFSET_MASK           (1 << 2)

/**
 * flag indicate that window is translucent.
 */
#define PAINT_WINDOW_TRANSLUCENT_MASK           (1 << 16)

/**
 * flag indicate that window is transformed.
 */
#define PAINT_WINDOW_TRANSFORMED_MASK           (1 << 17)

/**
 * flag indicate that core PaintWindow function should
 * not draw this window.
 */
#define PAINT_WINDOW_NO_CORE_INSTANCE_MASK	(1 << 18)

/**
 * flag indicate that blending is required.
 */
#define PAINT_WINDOW_BLEND_MASK			(1 << 19)

class CompositeWindowInterface :
    public WrapableInterface<CompositeWindow, CompositeWindowInterface>
{
    public:

	/**
	 * Hookable function to determine which parts of the
	 * screen for this window to redraw on the next pass
	 *
	 * @param initial Indicates if this is the first time
	 * this window is being redrawn
	 * @param rect Reference to a rect which describes which
	 * parts of the screen need to be redrawn on next pass
	 */
	virtual bool damageRect (bool initial, const CompRect &rect);
};

extern template class PluginClassHandler<CompositeWindow, CompWindow, COMPIZ_COMPOSITE_ABI>;

class CompositeWindow :
    public WrapableHandler<CompositeWindowInterface, 1>,
    public PluginClassHandler<CompositeWindow, CompWindow, COMPIZ_COMPOSITE_ABI>
{
    public:

	CompositeWindow (CompWindow *w);
	~CompositeWindow ();

	/**
	 * Binds the window contents of this window to some offscreen pixmap
	 */
	bool bind ();
	
	/**
	 * Releases the pixmap data for this window with XFreePixmap.
	 */
	void release ();
	
	/**
	 * Returns the window pixmap
	 */
	Pixmap pixmap ();

	/**
	 * Pixmap size at the time the pixmap was last bound
	 */

	const CompSize & size ();

	/**
	 * Forces this window to be composited so that the X Server
	 * stops drawing it and all output is redirected to an
	 * offscreen pixmap
	 */
	void redirect ();
	
	/**
	 * Stops this window from being composited, so that the X Server
	 * draws the window on-screen normally and output is not redirected
	 * to an offscreen pixmap
	 */
	
	void unredirect ();
	
	/**
	 * Returns true if a window is redirected
	 */
	bool redirected ();
	bool overlayWindow ();

	/**
	 * Returns true if pixmap updates are frozen
	 */
	bool frozen ();

	void damageTransformedRect (float          xScale,
				    float          yScale,
				    float          xTranslate,
				    float          yTranslate,
				    const CompRect &rect);

	void damageOutputExtents ();

	/**
	 * Causes an area of the window to be redrawn on the
	 * next event loop
	 */
	void addDamageRect (const CompRect &);

	/**
	 * Causes the window to be redrawn on the next
	 * event loop
	 */
	void addDamage (bool force = false);

	/**
	 * Returns true if this window will be redrawn or
	 * partially redrawn on the next event loop
	 */
	bool damaged ();

	/**
	 * Sets screen redraw hints for "damaged" areas
	 * as stated by XDamageNotifyEvent
	 *
	 * @param de An XDamageNotifyEvent to be used to
	 * calculate areas to redraw on the next event loop
	 */
	void processDamage (XDamageNotifyEvent *de);

	void updateOpacity ();
	void updateBrightness ();
	void updateSaturation ();

	/**
	 * Returns the window opacity
	 */
	unsigned short opacity ();
	/**
	 * Returns the window brightness
	 */
	unsigned short brightness ();
	/**
	 * Returns the window saturation
	 */
	unsigned short saturation ();

	/**
	 * A function to call when a new pixmap is ready to
	 * be bound just before the old one is released
	 */
	void setNewPixmapReadyCallback (const boost::function <void ()> &cb);

	WRAPABLE_HND (0, CompositeWindowInterface, bool, damageRect,
		      bool, const CompRect &);

	friend class PrivateCompositeWindow;
	friend class CompositeScreen;

    private:
	PrivateCompositeWindow *priv;
};

#endif
