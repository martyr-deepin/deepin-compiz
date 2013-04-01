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

/*
 * Spring model implemented by Kristian Hogsberg.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <composite/composite.h>
#include <opengl/opengl.h>

#include "wobbly_options.h"

#define SNAP_WINDOW_TYPE (CompWindowTypeNormalMask  | \
			  CompWindowTypeToolbarMask | \
			  CompWindowTypeMenuMask    | \
			  CompWindowTypeUtilMask)

#define GRID_WIDTH 4
#define GRID_HEIGHT 4

#define MODEL_MAX_SPRINGS (GRID_WIDTH * GRID_HEIGHT * 2)

#define MASS 15.0f

#define NorthEdgeMask (1L << 0)
#define SouthEdgeMask (1L << 1)
#define WestEdgeMask  (1L << 2)
#define EastEdgeMask  (1L << 3)

#define EDGE_DISTANCE 25.0f
#define EDGE_VELOCITY 13.0f

typedef enum
{
    North = 0,
    South,
    West,
    East
} Direction;

#define WobblyInitialMask  (1L << 0)
#define WobblyForceMask    (1L << 1)
#define WobblyVelocityMask (1L << 2)

class WobblyWindow;

typedef struct _xy_pair {
    float x, y;
} Point, Vector;

typedef struct _Edge
{
    float next, prev;

    float start;
    float end;

    float attract;
    float velocity;

    bool  snapped;
} Edge;

class Object
{
public:
    Vector	 force;
    Point	 position;
    Vector	 velocity;
    float	 theta;
    bool	 immobile;
    unsigned int edgeMask;
    Edge	 vertEdge;
    Edge	 horzEdge;

    void init (float  positionX,
	       float  positionY,
	       float  velocityX,
	       float  velocityY);
    void applyForce (float  fx,
		     float  fy);
    float distanceToPoint (float  x,
			   float  y);
};

class Spring
{
public:
    Object *a;
    Object *b;
    Vector offset;

    void init (Object *newA,
	       Object *newB,
	       float  newOffsetX,
		   float  newOffsetY);
    void exertForces (float k);
};

class Model
{
public:
    Model (int          x,
	   int          y,
	   int          width,
	   int          height,
	   unsigned int edgeMask);
    ~Model ();

    void calcBounds ();
    void addSpring (Object *a,
		    Object *b,
		    float  offsetX,
		    float  offsetY);
    void setMiddleAnchor (int x,
			  int y,
			  int width,
			  int height);
    void setTopAnchor (int x,
		       int y,
		       int width);
    void addEdgeAnchors (int x,
			 int y,
			 int width,
			 int height);
    void removeEdgeAnchors (int x,
			    int y,
			    int width,
			    int height);
    void adjustObjectPosition (Object *object,
			       int    x,
			       int    y,
			       int    width,
			       int    height);
    void initObjects (int x,
		      int y,
		      int width,
		      int height);
    void initSprings (int x,
		      int y,
		      int width,
		      int height);
    void reduceEdgeEscapeVelocity ();
    bool disableSnapping ();
    void adjustObjectsForShiver (int   x,
				 int   y,
				 int   width,
				 int   height);
    void move (float tx,
	       float ty);
    void bezierPatchEvaluate (float u,
			      float v,
			      float *patchX,
			      float *patchY);
    Object * findNearestObject (float x,
				float y);

    Object	 *objects;
    int		 numObjects;
    Spring	 springs[MODEL_MAX_SPRINGS];
    int		 numSprings;
    Object	 *anchorObject;
    float	 steps;
    Point	 topLeft;
    Point	 bottomRight;
    unsigned int edgeMask;
    unsigned int snapCnt[4];
};

class WobblyScreen :
    public PluginClassHandler<WobblyScreen, CompScreen>,
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public WobblyOptions
{
public:
    WobblyScreen (CompScreen *s);

    bool enableSnapping ();
    bool disableSnapping ();
    bool shiver (CompOption::Vector &options);

    /// Start given window's wobbling
    void startWobbling (WobblyWindow *ww);

    // ScreenInterface methods
    void handleEvent (XEvent *event);

    // CompositeScreenInterface methods
    void preparePaint (int);
    void donePaint ();

    // GLScreenInterface methods
    bool glPaintOutput (const GLScreenPaintAttrib &,
			const GLMatrix &,
			const CompRegion &,
			CompOutput *,
			unsigned int);

    static void snapKeyChanged (CompOption *opt);
    void snapInvertedChanged (CompOption *opt);

    CompositeScreen *cScreen;
    GLScreen *gScreen;

    unsigned int wobblingWindowsMask;

    unsigned int grabMask;
    CompWindow	 *grabWindow;
    bool         moveWindow;

    bool snapping;

    bool           yConstrained;
    const CompRect *constraintBox;
};

class WobblyWindow :
    public PluginClassHandler<WobblyWindow, CompWindow>,
    public WindowInterface,
    public CompositeWindowInterface,
    public GLWindowInterface
{
public:
    WobblyWindow (CompWindow *);
    ~WobblyWindow ();

    void findNextWestEdge (Object *object);
    void findNextEastEdge (Object *object);
    void findNextNorthEdge (Object *object);
    void findNextSouthEdge (Object *object);
    void updateModelSnapping ();
    bool objectReleaseWestEastEdge (Object    *object,
				    Direction dir);
    bool objectReleaseNorthSouthEdge (Object    *object,
				      Direction dir);
    float modelStepObject (Object *object,
			   float  friction,
			   float  *force);
    unsigned int modelStep (float friction,
			    float k,
			    float time);
    bool ensureModel ();
    bool isWobblyWin ();
    void enableWobbling (bool enabling);
    void initiateMapEffect ();

    // WindowInterface methods
    void resizeNotify (int dx, int dy, int dwidth, int dheight);
    void moveNotify (int dx, int dy, bool immediate);
    void grabNotify (int x, int y, unsigned int state, unsigned int mask);
    void ungrabNotify ();
    void windowNotify (CompWindowNotify n);

    // CompositeWindowInterface methods
    bool damageRect (bool, const CompRect &);

    // GLWindowInterface methods
    bool glPaint (const GLWindowPaintAttrib &, const GLMatrix &,
		  const CompRegion &, unsigned int);
    void glAddGeometry (const GLTexture::MatrixList &,
			const CompRegion &, const CompRegion &,
			unsigned int = MAXSHORT, unsigned int = MAXSHORT);

    WobblyScreen     *wScreen;
    CompWindow       *window;
    CompositeWindow  *cWindow;
    GLWindow         *gWindow;

    Model	 *model;
    unsigned int wobblingMask;
    bool	 grabbed;
    bool	 velocity;
    unsigned int state;
};

class WobblyPluginVTable :
    public CompPlugin::VTableForScreenAndWindow<WobblyScreen, WobblyWindow>
{
    public:
	bool init ();
};

/* TODO rm
#define WIN_X(w) ((w)->attrib.x - (w)->output.left)
#define WIN_Y(w) ((w)->attrib.y - (w)->output.top)
#define WIN_W(w) ((w)->width + (w)->output.left + (w)->output.right)
#define WIN_H(w) ((w)->height + (w)->output.top + (w)->output.bottom)


#define WOBBLY_SCREEN_OPTION_FRICTION	        0
#define WOBBLY_SCREEN_OPTION_SPRING_K	        1
#define WOBBLY_SCREEN_OPTION_GRID_RESOLUTION    2
#define WOBBLY_SCREEN_OPTION_MIN_GRID_SIZE      3
#define WOBBLY_SCREEN_OPTION_MAP_EFFECT	        4
#define WOBBLY_SCREEN_OPTION_FOCUS_EFFECT       5
#define WOBBLY_SCREEN_OPTION_MAP_WINDOW_MATCH   6
#define WOBBLY_SCREEN_OPTION_FOCUS_WINDOW_MATCH 7
#define WOBBLY_SCREEN_OPTION_GRAB_WINDOW_MATCH  8
#define WOBBLY_SCREEN_OPTION_MOVE_WINDOW_MATCH  9
#define WOBBLY_SCREEN_OPTION_MAXIMIZE_EFFECT    10
#define WOBBLY_SCREEN_OPTION_NUM	        11


static CompMetadata wobblyMetadata;

static int displayPrivateIndex;

#define WOBBLY_DISPLAY_OPTION_SNAP_KEY      0
#define WOBBLY_DISPLAY_OPTION_SNAP_INVERTED 1
#define WOBBLY_DISPLAY_OPTION_SHIVER        2
#define WOBBLY_DISPLAY_OPTION_NUM           3


#define WOBBLY_EFFECT_NONE   0
#define WOBBLY_EFFECT_SHIVER 1
#define WOBBLY_EFFECT_LAST   WOBBLY_EFFECT_SHIVER


#define GET_WOBBLY_DISPLAY(d)					    \
    ((WobblyDisplay *) (d)->base.privates[displayPrivateIndex].ptr)

#define WOBBLY_DISPLAY(d)		       \
    WobblyDisplay *wd = GET_WOBBLY_DISPLAY (d)

#define GET_WOBBLY_SCREEN(s, wd)					\
    ((WobblyScreen *) (s)->base.privates[(wd)->screenPrivateIndex].ptr)

#define WOBBLY_SCREEN(s)						      \
    WobblyScreen *ws = GET_WOBBLY_SCREEN (s, GET_WOBBLY_DISPLAY (s->display))

#define GET_WOBBLY_WINDOW(w, ws)					\
    ((WobblyWindow *) (w)->base.privates[(ws)->windowPrivateIndex].ptr)

#define WOBBLY_WINDOW(w)				         \
    WobblyWindow *ww = GET_WOBBLY_WINDOW  (w,		         \
		       GET_WOBBLY_SCREEN  (w->screen,	         \
		       GET_WOBBLY_DISPLAY (w->screen->display)))

#define NUM_OPTIONS(s) (sizeof ((s)->opt) / sizeof (CompOption))
*/
