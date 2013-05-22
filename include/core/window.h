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

#ifndef _COMPWINDOW_H
#define _COMPWINDOW_H

#include <boost/function.hpp>

#include <X11/Xlib-xcb.h>
#include <X11/Xutil.h>
#include <X11/Xregion.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/sync.h>

#include <core/action.h>
#include <core/pluginclasses.h>
#include <core/size.h>
#include <core/point.h>
#include <core/region.h>
#include <core/windowgeometry.h>
#include <core/windowextents.h>

#include <core/wrapsystem.h>

#include <map>

class CompWindow;
class CompIcon;
class PrivateWindow;
struct CompStartupSequence;

namespace compiz { namespace private_screen {
    class Ping;
    class GrabManager;
    class OutputDevices;
    class WindowManager;
    class StartupSequence;
}}

#define ROOTPARENT(x) (((x)->frame ()) ? (x)->frame () : (x)->id ())

#define CompWindowProtocolDeleteMask	  (1 << 0)
#define CompWindowProtocolTakeFocusMask	  (1 << 1)
#define CompWindowProtocolPingMask	  (1 << 2)
#define CompWindowProtocolSyncRequestMask (1 << 3)

#define CompWindowTypeDesktopMask      (1 << 0)
#define CompWindowTypeDockMask         (1 << 1)
#define CompWindowTypeToolbarMask      (1 << 2)
#define CompWindowTypeMenuMask         (1 << 3)
#define CompWindowTypeUtilMask         (1 << 4)
#define CompWindowTypeSplashMask       (1 << 5)
#define CompWindowTypeDialogMask       (1 << 6)
#define CompWindowTypeNormalMask       (1 << 7)
#define CompWindowTypeDropdownMenuMask (1 << 8)
#define CompWindowTypePopupMenuMask    (1 << 9)
#define CompWindowTypeTooltipMask      (1 << 10)
#define CompWindowTypeNotificationMask (1 << 11)
#define CompWindowTypeComboMask	       (1 << 12)
#define CompWindowTypeDndMask	       (1 << 13)
#define CompWindowTypeModalDialogMask  (1 << 14)
#define CompWindowTypeFullscreenMask   (1 << 15)
#define CompWindowTypeUnknownMask      (1 << 16)

#define NO_FOCUS_MASK (CompWindowTypeDesktopMask | \
		       CompWindowTypeDockMask    | \
		       CompWindowTypeSplashMask)

#define CompWindowStateModalMask	    (1 <<  0)
#define CompWindowStateStickyMask	    (1 <<  1)
#define CompWindowStateMaximizedVertMask    (1 <<  2)
#define CompWindowStateMaximizedHorzMask    (1 <<  3)
#define CompWindowStateShadedMask	    (1 <<  4)
#define CompWindowStateSkipTaskbarMask	    (1 <<  5)
#define CompWindowStateSkipPagerMask	    (1 <<  6)
#define CompWindowStateHiddenMask	    (1 <<  7)
#define CompWindowStateFullscreenMask	    (1 <<  8)
#define CompWindowStateAboveMask	    (1 <<  9)
#define CompWindowStateBelowMask	    (1 << 10)
#define CompWindowStateDemandsAttentionMask (1 << 11)
#define CompWindowStateDisplayModalMask	    (1 << 12)
#define CompWindowStateFocusedMask	    (1 << 13)

#define MAXIMIZE_STATE (CompWindowStateMaximizedHorzMask | \
			CompWindowStateMaximizedVertMask)

#define CompWindowActionMoveMask	  (1 << 0)
#define CompWindowActionResizeMask	  (1 << 1)
#define CompWindowActionStickMask	  (1 << 2)
#define CompWindowActionMinimizeMask      (1 << 3)
#define CompWindowActionMaximizeHorzMask  (1 << 4)
#define CompWindowActionMaximizeVertMask  (1 << 5)
#define CompWindowActionFullscreenMask	  (1 << 6)
#define CompWindowActionCloseMask	  (1 << 7)
#define CompWindowActionShadeMask	  (1 << 8)
#define CompWindowActionChangeDesktopMask (1 << 9)
#define CompWindowActionAboveMask	  (1 << 10)
#define CompWindowActionBelowMask	  (1 << 11)

#define MwmFuncAll      (1L << 0)
#define MwmFuncResize   (1L << 1)
#define MwmFuncMove     (1L << 2)
#define MwmFuncIconify  (1L << 3)
#define MwmFuncMaximize (1L << 4)
#define MwmFuncClose    (1L << 5)

#define MwmDecorHandle   (1L << 2)
#define MwmDecorTitle    (1L << 3)
#define MwmDecorMenu     (1L << 4)
#define MwmDecorMinimize (1L << 5)
#define MwmDecorMaximize (1L << 6)

#define MwmDecorAll      (1L << 0)
#define MwmDecorBorder   (1L << 1)
#define MwmDecorHandle   (1L << 2)
#define MwmDecorTitle    (1L << 3)
#define MwmDecorMenu     (1L << 4)
#define MwmDecorMinimize (1L << 5)
#define MwmDecorMaximize (1L << 6)

#define WmMoveResizeSizeTopLeft      0
#define WmMoveResizeSizeTop          1
#define WmMoveResizeSizeTopRight     2
#define WmMoveResizeSizeRight        3
#define WmMoveResizeSizeBottomRight  4
#define WmMoveResizeSizeBottom       5
#define WmMoveResizeSizeBottomLeft   6
#define WmMoveResizeSizeLeft         7
#define WmMoveResizeMove             8
#define WmMoveResizeSizeKeyboard     9
#define WmMoveResizeMoveKeyboard    10
#define WmMoveResizeCancel          11

/* EWMH source indication client types */
#define ClientTypeUnknown      0
#define ClientTypeApplication  1
#define ClientTypePager        2

#define CompWindowGrabKeyMask         (1 << 0)
#define CompWindowGrabButtonMask      (1 << 1)
#define CompWindowGrabMoveMask        (1 << 2)
#define CompWindowGrabResizeMask      (1 << 3)
#define CompWindowGrabExternalAppMask (1 << 4)

/**
 * Enumeration value which represents
 * how a window will be stacked by compiz
 */
enum CompStackingUpdateMode {
    CompStackingUpdateModeNone = 0,
    CompStackingUpdateModeNormal,
    CompStackingUpdateModeAboveFullscreen,
    CompStackingUpdateModeInitialMap,
    CompStackingUpdateModeInitialMapDeniedFocus
};

/**
 * Enumeration value used by CompWindow::windowNotify
 * which specifies the type of event that occured.
 */
enum CompWindowNotify {
   CompWindowNotifyMap,
   CompWindowNotifyUnmap,
   CompWindowNotifyRestack,
   CompWindowNotifyHide,
   CompWindowNotifyShow,
   CompWindowNotifyAliveChanged,
   CompWindowNotifySyncAlarm,
   CompWindowNotifyReparent,
   CompWindowNotifyUnreparent,
   CompWindowNotifyFrameUpdate,
   CompWindowNotifyFocusChange,
   CompWindowNotifyBeforeUnmap,
   CompWindowNotifyBeforeDestroy,
   CompWindowNotifyClose,
   CompWindowNotifyMinimize,
   CompWindowNotifyUnminimize,
   CompWindowNotifyShade,
   CompWindowNotifyUnshade,
   CompWindowNotifyEnterShowDesktopMode,
   CompWindowNotifyLeaveShowDesktopMode,
   CompWindowNotifyBeforeMap
};

/**
 * Specifies the left, right, top and bottom positions of a window's
 * geometry
 */
typedef compiz::window::extents::Extents CompWindowExtents;

namespace compiz
{
    namespace window
    {
        unsigned int fillStateData (unsigned int state, Atom *data);
    }
}

/**
 * Specifies the area of the screen taken up by strut windows
 */
struct CompStruts {
    XRectangle left;
    XRectangle right;
    XRectangle top;
    XRectangle bottom;
};

class WindowInterface;
extern template class WrapableInterface<CompWindow, WindowInterface>;

/**
 * Wrappable core window functions. Derive from this class
 * and overload these functions in order to have your function called
 * after a core CompWindow function is called with the same name.
 */
class WindowInterface : public WrapableInterface<CompWindow, WindowInterface>
{
    public:
	virtual void getOutputExtents (CompWindowExtents& output);

	virtual void getAllowedActions (unsigned int &setActions,
					unsigned int &clearActions);

	virtual bool focus ();
	virtual void activate ();
	virtual bool place (CompPoint &pos);

	virtual void validateResizeRequest (unsigned int   &mask,
					    XWindowChanges *xwc,
					    unsigned int   source);

	virtual void resizeNotify (int dx, int dy, int dwidth, int dheight);
	virtual void moveNotify (int dx, int dy, bool immediate);
	virtual void windowNotify (CompWindowNotify n);

	virtual void grabNotify (int x, int y,
				 unsigned int state, unsigned int mask);
	virtual void ungrabNotify ();

	virtual void stateChangeNotify (unsigned int lastState);

	virtual void updateFrameRegion (CompRegion &region);

	virtual void minimize ();
	virtual void unminimize ();
	virtual bool minimized () const;

	virtual bool alpha () const;
	virtual bool isFocussable () const;
	virtual bool managed () const;

	virtual bool focused () const;
};

/**
 * An Window object that wraps an X window. This handles snychronization of
 * window state, geometry, etc. between Compiz and the X server.
 */
class CompWindow :
    public WrapableHandler<WindowInterface, 20>,
    public PluginClassStorage
{
    public:

	typedef compiz::window::Geometry Geometry;
	typedef boost::function<void (CompWindow *)> ForEach;
	typedef std::map<Window, CompWindow *> Map;

    public:
	CompWindow *next;
	CompWindow *prev;

	CompWindow *serverNext;
	CompWindow *serverPrev;

    public:
	~CompWindow ();

	CompPoint::vector    deepinWindowViewports;

	void setDeepinWindowViewportsProp ();

	/**
	*  Geometry retrieved from the
	 * last ConfigureNotify event received
	 */
	const Geometry & geometry () const;

	int x () const;
	int y () const;
	CompPoint pos () const;

	/* With border */
	int width () const;
	int height () const;
	CompSize size () const;

	/**
	 * Geometry last sent to the server
         */
	const Geometry & serverGeometry () const;

	int serverX () const;
	int serverY () const;
	CompPoint serverPos () const;

	/* With border */
	int serverWidth () const;
	int serverHeight () const;
	const CompSize serverSize () const;

	/* effective decoration extents */
	CompRect borderRect () const;
	CompRect serverBorderRect () const;

	/* frame window geometry */
	CompRect inputRect () const;
	CompRect serverInputRect () const;

	/* includes decorations and shadows */
	CompRect outputRect () const;
	CompRect serverOutputRect () const;

	Window id () const;
	Window frame () const;

	CompString resName () const;

	const CompRegion & region () const;

	const CompRegion & frameRegion () const;

	void updateFrameRegion ();
	void setWindowFrameExtents (const CompWindowExtents *border,
				    const CompWindowExtents *frame = NULL);

	unsigned int & wmType () const;

	unsigned int type () const;

	unsigned int & state () const;

	unsigned int actions () const;

	unsigned int & protocols () const;

	void close (Time serverTime);

	bool inShowDesktopMode () const;

	void setShowDesktopMode (bool);

	bool grabbed () const;

	int pendingMaps () const;

	unsigned int activeNum () const;

	int mapNum () const;

	int & saveMask () const;

	XWindowChanges & saveWc ();

	void moveToViewportPosition (int x, int y, bool sync);

	const char * startupId () const;

	unsigned int desktop () const;

	Window clientLeader (bool checkAncestor = false) const;

	void changeState (unsigned int newState);

	void recalcActions ();

	void recalcType ();

	void updateWindowOutputExtents ();

	void destroy ();

	void sendConfigureNotify ();

	void sendSyncRequest ();

	XSyncAlarm syncAlarm () const;

	void map ();

	void unmap ();

	void incrementUnmapReference ();

	void incrementDestroyReference ();

	bool hasUnmapReference () const;

	bool resize (XWindowAttributes);

	bool resize (Geometry);

	bool resize (int x, int y, int width, int height,
		     int border = 0);

	void move (int dx, int dy, bool immediate = true);

	void syncPosition ();

	void moveInputFocusTo ();

	void moveInputFocusToOtherWindow ();

	/* wraps XConfigureWindow and updates serverGeometry */
	void configureXWindow (unsigned int valueMask,
			       XWindowChanges *xwc);

	void moveResize (XWindowChanges *xwc,
			 unsigned int   xwcm,
			 int            gravity,
			 unsigned int   source);

	void raise ();

	void lower ();

	void restackAbove (CompWindow *sibling);

	void restackBelow (CompWindow *sibling);

	void updateAttributes (CompStackingUpdateMode stackingMode);

	void hide ();

	void show ();

	void maximize (unsigned int state = 0);

	CompPoint defaultViewport () const;

	const CompPoint & initialViewport () const;

	CompIcon * getIcon (int width, int height);

	const CompRect & iconGeometry () const;

	int outputDevice () const;

	void setDesktop (unsigned int desktop);

	bool onCurrentDesktop () const;

	bool onAllViewports () const;

	CompPoint getMovementForOffset (const CompPoint &offset) const;

	Window transientFor () const;

	int pendingUnmaps () const;

	bool placed () const;

	bool shaded () const;

	const CompWindowExtents & border () const;
	const CompWindowExtents & input () const;
	const CompWindowExtents & output () const;

	// FIXME: This should return a const reference but grid needs fixing...
	XSizeHints & sizeHints () const;

	bool destroyed () const;

	bool invisible () const;

	bool syncWait () const;

	bool alive () const;

	bool overrideRedirect () const;

	bool isMapped () const;
	bool isViewable () const;

	int windowClass () const;

	unsigned int depth () const;

	unsigned int mwmDecor () const;
	unsigned int mwmFunc () const;

	bool constrainNewWindowSize (int width,
				     int height,
				     int *newWidth,
				     int *newHeight);

	static unsigned int constrainWindowState (unsigned int state,
						  unsigned int actions);

	static unsigned int allocPluginClassIndex ();
	static void freePluginClassIndex (unsigned int index);

	bool updateStruts ();
	const CompStruts *struts () const;

	WRAPABLE_HND (0, WindowInterface, void, getOutputExtents,
		      CompWindowExtents&);
	WRAPABLE_HND (1, WindowInterface, void, getAllowedActions,
		      unsigned int &, unsigned int &);

	WRAPABLE_HND (2, WindowInterface, bool, focus);
	WRAPABLE_HND (3, WindowInterface, void, activate);
	WRAPABLE_HND (4, WindowInterface, bool, place, CompPoint &);
	WRAPABLE_HND (5, WindowInterface, void, validateResizeRequest,
		      unsigned int &, XWindowChanges *, unsigned int);

	WRAPABLE_HND (6, WindowInterface, void, resizeNotify,
		      int, int, int, int);
	WRAPABLE_HND (7, WindowInterface, void, moveNotify, int, int, bool);
	WRAPABLE_HND (8, WindowInterface, void, windowNotify, CompWindowNotify);
	WRAPABLE_HND (9, WindowInterface, void, grabNotify, int, int,
		      unsigned int, unsigned int);
	WRAPABLE_HND (10, WindowInterface, void, ungrabNotify);
	WRAPABLE_HND (11, WindowInterface, void, stateChangeNotify,
		      unsigned int);

	WRAPABLE_HND (12, WindowInterface, void, updateFrameRegion,
		      CompRegion &);

	WRAPABLE_HND (13, WindowInterface, void, minimize);
	WRAPABLE_HND (14, WindowInterface, void, unminimize);
	WRAPABLE_HND_CONST (15, WindowInterface, bool, minimized);

	WRAPABLE_HND_CONST (16, WindowInterface, bool, alpha);
	WRAPABLE_HND_CONST (17, WindowInterface, bool, isFocussable);
	WRAPABLE_HND_CONST (18, WindowInterface, bool, managed);

	WRAPABLE_HND_CONST (19, WindowInterface, bool, focused);

	friend class PrivateWindow;
	friend class CompScreenImpl;
	friend class PrivateScreen;
	friend class compiz::private_screen::Ping;
	friend class compiz::private_screen::GrabManager;
	friend class compiz::private_screen::OutputDevices;
	friend class compiz::private_screen::WindowManager;
	friend class compiz::private_screen::StartupSequence;
	friend class ModifierHandler;
	friend class CoreWindow;
	friend class StackDebugger;

    private:

	CompWindow (Window	      aboveId,
		    Window	      aboveServerId,
		    XWindowAttributes &wa,
		    PrivateWindow     *priv);

	PrivateWindow *priv;
};

#endif
