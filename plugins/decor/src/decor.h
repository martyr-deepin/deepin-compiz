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

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/make_shared.hpp>
#include <core/window.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>
#include <core/atoms.h>
#include <core/windowextents.h>

#include <clip-groups.h>
#include <pixmap-requests.h>

#include "decor_options.h"

#define DECOR_SCREEN(s) DecorScreen *ds = DecorScreen::get(s)
#define DECOR_WINDOW(w) DecorWindow *dw = DecorWindow::get(w)

struct Vector {
    int	dx;
    int	dy;
    int	x0;
    int	y0;
};

/* FIXME: Remove */
#define DECOR_BARE	0
#define DECOR_ACTIVE	1
#define DECOR_NUM	2

using namespace compiz::decor;

class MatchedDecorClipGroup :
    public DecorClipGroupInterface
{
    public:

	MatchedDecorClipGroup (const CompMatch &match);

    private:

	bool doPushClippable (DecorClippableInterface *dc);
	bool doPopClippable (DecorClippableInterface *dc) { return mClipGroupImpl.popClippable (dc); }
	void doRegenerateClipRegion () { return mClipGroupImpl.regenerateClipRegion (); }
	const CompRegion & getClipRegion () { return mClipGroupImpl.clipRegion (); }
	void doUpdateAllShadows () { return mClipGroupImpl.updateAllShadows (); }

	impl::GenericDecorClipGroup		mClipGroupImpl;
	CompMatch                               mMatch;
};

class DecorTexture {

    public:
	DecorTexture (DecorPixmapInterface::Ptr pixmap);
	~DecorTexture ();

    public:
	bool            status;
	int             refCount;
	DecorPixmapInterface::Ptr pixmap;
	Damage          damage;
	GLTexture::List textures;
};

class DecorWindow;

class Decoration :
    public DecorationInterface
{

    public:

	typedef boost::shared_ptr <Decoration> Ptr;

	static const unsigned int UpdateRequested = 1 << 0;
	static const unsigned int UpdatesPending = 1 << 1;

	static Decoration::Ptr create (Window        id,
				       long          *prop,
				       unsigned int  size,
				       unsigned int  type,
				       unsigned int  nOffset,
				       DecorPixmapRequestorInterface *requestor);

	Decoration (int   type,
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
		    Window owner,
		    DecorPixmapRequestorInterface *);

	~Decoration ();

	DecorPixmapReceiverInterface & receiverInterface ();

	unsigned int getFrameType () const;
	unsigned int getFrameState () const;
	unsigned int getFrameActions () const;

    public:
	int                       refCount;
	DecorTexture              *texture;
	CompWindowExtents         output;
	CompWindowExtents         border;
	CompWindowExtents	  input;
	CompWindowExtents         maxBorder;
	CompWindowExtents	  maxInput;
	int                       minWidth;
	int                       minHeight;
	unsigned int		  frameType;
	unsigned int		  frameState;
	unsigned int		  frameActions;
	boost::shared_array <decor_quad_t> quad;
	int                       nQuad;
	int                       type;

	unsigned int              updateState;
	X11DecorPixmapReceiver    mPixmapReceiver;
};

class DecorationList :
    public DecorationListFindMatchingInterface
{
    public:
	bool updateDecoration  (Window id, Atom decorAtom, DecorPixmapRequestorInterface *requestor);
	DecorationInterface::Ptr findMatchingDecoration(unsigned int frameType,
								 unsigned int frameState,
								 unsigned int frameActions);
	const Decoration::Ptr & findMatchingDecoration (CompWindow *w, bool sizeCheck);
        void clear ()
        {
	    mList.clear ();
        };

        DecorationList ();

	std::list <Decoration::Ptr> mList;
};

struct ScaledQuad {
    GLTexture::Matrix matrix;
    BoxRec            box;
    float             sx;
    float             sy;
};

class WindowDecoration {
    public:
	static WindowDecoration * create (const Decoration::Ptr &d);
	static void destroy (WindowDecoration *);

    public:
	Decoration::Ptr decor;
	ScaledQuad *quad;
	int	   nQuad;
};

class DecorWindow;

class DecorScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public PluginClassHandler<DecorScreen,CompScreen>,
    public DecorOptions
{
    public:
	DecorScreen (CompScreen *s);
	~DecorScreen ();

	bool setOption (const CompString &name, CompOption::Value &value);

	void handleEvent (XEvent *event);
	void matchPropertyChanged (CompWindow *);
	void addSupportedAtoms (std::vector<Atom>&);

	DecorTexture * getTexture (Pixmap);
	void releaseTexture (DecorTexture *);

	void checkForDm (bool);
	bool decoratorStartTimeout ();

	void updateDefaultShadowProperty ();

	bool registerPaintHandler (compiz::composite::PaintHandler *pHnd);
	void unregisterPaintHandler ();

    public:

	CompositeScreen *cScreen;

	std::list<DecorTexture *> textures;

	Atom supportingDmCheckAtom;
	Atom winDecorAtom;
	Atom decorAtom[DECOR_NUM];
	Atom inputFrameAtom;
	Atom outputFrameAtom;
	Atom decorTypeAtom;
	Atom decorTypePixmapAtom;
	Atom decorTypeWindowAtom;
	Atom requestFrameExtentsAtom;
	Atom shadowColorAtom;
	Atom shadowInfoAtom;
	Atom decorSwitchWindowAtom;
	Atom decorPendingAtom;
	Atom decorRequestAtom;

	Window dmWin;
	int    dmSupports;

	bool cmActive;

	DecorationList decor[DECOR_NUM];
	Decoration::Ptr     windowDefault;

	std::map<Window, DecorWindow *> frames;

	CompTimer decoratorStart;

	MatchedDecorClipGroup mMenusClipGroup;
	X11DecorPixmapRequestor   mRequestor;
};

class DecorWindow :
    public WindowInterface,
    public CompositeWindowInterface,
    public GLWindowInterface,
    public PluginClassHandler<DecorWindow,CompWindow>,
    public DecorClippableInterface
{
    public:
	DecorWindow (CompWindow *w);
	~DecorWindow ();

	void getOutputExtents (CompWindowExtents&);
	void resizeNotify (int, int, int, int);
	void moveNotify (int, int, bool);
	void stateChangeNotify (unsigned int);
	void updateFrameRegion (CompRegion &region);

	bool damageRect (bool, const CompRect &);

	bool glDraw (const GLMatrix &, const GLWindowPaintAttrib &,
		     const CompRegion &, unsigned int);
	void glDecorate (const GLMatrix &, const GLWindowPaintAttrib &,
		         const CompRegion &, unsigned int);

	void windowNotify (CompWindowNotify n);

	void updateDecoration ();

	void setDecorationMatrices ();

	void updateDecorationScale ();

	void updateFrame ();
	void updateInputFrame ();
	void updateOutputFrame ();
	void updateWindowRegions ();

	bool checkSize (const Decoration::Ptr &decor);

	int shiftX ();
	int shiftY ();

	bool update (bool);

	bool resizeTimeout ();

	void updateSwitcher ();
	void updateHandlers ();

	static bool matchType (CompWindow *w, unsigned int decorType);
	static bool matchState (CompWindow *w, unsigned int decorState);
	static bool matchActions (CompWindow *w, unsigned int decorActions);

    private:

	void doUpdateShadow (const CompRegion &);
	void doSetOwner (DecorClipGroupInterface *i);
	bool doMatches (const CompMatch &m);
	const CompRegion & getOutputRegion ();
	const CompRegion & getInputRegion ();
	void doUpdateGroupShadows ();

    public:

	CompWindow      *window;
	GLWindow        *gWindow;
	CompositeWindow *cWindow;
	DecorScreen     *dScreen;

	WindowDecoration *wd;
	DecorationList	 decor;

	CompRegion frameRegion;
	CompRegion shadowRegion;
	CompRegion tmpRegion;

	Window inputFrame;
	Window outputFrame;
	Damage frameDamage;

	int    oldX;
	int    oldY;
	int    oldWidth;
	int    oldHeight;

	bool pixmapFailed;

	CompRegion::Vector regions;
	bool               updateReg;
	bool		   updateMatrix;

	CompTimer resizeUpdate;
	CompTimer moveUpdate;

	bool	  unshading;
	bool	  shading;
	bool	  isSwitcher;

	bool      frameExtentsRequested;

	DecorClipGroupInterface *mClipGroup;
	CompRegion		mOutputRegion;
	CompRegion              mInputRegion;

	X11DecorPixmapRequestor   mRequestor;
};

class DecorPluginVTable :
    public CompPlugin::VTableForScreenAndWindow<DecorScreen, DecorWindow>
{
    public:

	bool init ();
};

