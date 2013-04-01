/*
 *
 * Compiz KDE compatibility plugin
 *
 * kdecompat.cpp
 *
 * Copyright : (C) 2007 by Danny Baumann
 * E-mail    : maniac@opencompositing.org
 *
 * Based on scale.c and switcher.c:
 * Copyright : (C) 2007 David Reveman
 * E-mail    : davidr@novell.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
 
#include <core/core.h>
#include <core/pluginclasshandler.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <decoration.h>

#include <X11/Xatom.h>
#include <core/atoms.h>

#include "kdecompat_options.h"

#include <cmath>
 
class KDECompatScreen :
    public PluginClassHandler <KDECompatScreen, CompScreen>,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public ScreenInterface,
    public KdecompatOptions
{
    public:

	KDECompatScreen (CompScreen *);
	~KDECompatScreen ();

    public:

	void
	handleEvent (XEvent *);

	void
	advertiseSupport (Atom atom,
			  bool enable);

	void
	optionChanged (CompOption                *option,
		       KdecompatOptions::Options num);

	void
	preparePaint (int);
	
	bool
	glPaintOutput (const GLScreenPaintAttrib &attrib,
		       const GLMatrix		 &transform,
		       const CompRegion		 &region,
		       CompOutput		 *output,
		       unsigned int		 mask);

	void
	donePaint ();

	void
	handleCompizEvent (const char         *pluginName,
			   const char         *eventName,
			   CompOption::Vector &options);
	
	CompAction *
	getScaleAction (const char *name);

	bool
	scaleActivate ();
	
	void
	freeScaleTimeout ();

	inline void
	checkPaintFunctions ();

	CompositeScreen *cScreen;
	GLScreen	*gScreen;

	Atom mKdePreviewAtom;
	Atom mKdeSlideAtom;
	Atom mKdePresentGroupAtom;
	Atom mKdeBlurBehindRegionAtom;
	Atom mCompizWindowBlurAtom;

	bool mHasSlidingPopups;

	int  mDestroyCnt;
	int  mUnmapCnt;

	CompPlugin *mScaleHandle;
	bool	   mScaleActive;
	CompTimer  mScaleTimeout;

	bool	   mBlurLoaded;

	CompWindow          *mPresentWindow;
	std::vector<Window> mPresentWindowList;
};

#define KDECOMPAT_SCREEN(s)						       \
    KDECompatScreen *ks = KDECompatScreen::get (s)

class KDECompatWindow :
    public PluginClassHandler <KDECompatWindow, CompWindow>,
    public WindowInterface,
    public CompositeWindowInterface,
    public GLWindowInterface
{
    public:

	KDECompatWindow (CompWindow *);
	~KDECompatWindow ();
	
	CompWindow      *window;
	CompositeWindow *cWindow;
	GLWindow	*gWindow;

    public:

	typedef struct {
	    Window   id;
	    CompRect thumb;
	} Thumb;

	typedef enum {
	    West  = 0,
	    North = 1,
	    East  = 2,
	    South = 3
	} SlidePosition;

	typedef struct {
	    SlidePosition position;
	    int           start;
	    bool          appearing;
	    int           remaining;
	    int           duration;
	} SlideData;

	std::list<Thumb> mPreviews;
	bool		 mIsPreview;

	SlideData	 *mSlideData;
	int		 mDestroyCnt;
	int		 mUnmapCnt;

	bool		mBlurPropertySet;

    public:

	bool
	glPaint (const GLWindowPaintAttrib &,
		 const GLMatrix		   &,
		 const CompRegion	   &,
		 unsigned int);

	bool
	damageRect (bool,
		    const CompRect &);

	void
	updatePreviews ();

	void
	stopCloseAnimation ();

	void
	sendSlideEvent (bool start);

	void
	startSlideAnimation (bool appearing);

	void
	endSlideAnimation ();

	void
	updateSlidePosition ();

	void
	updateBlurProperty (bool enabled);

	void
	handleClose (bool);

	void
	presentGroup ();

	void
	windowNotify (CompWindowNotify n);
};

#define KDECOMPAT_WINDOW(w)						       \
    KDECompatWindow *kw = KDECompatWindow::get(w)

class KDECompatPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <KDECompatScreen,
						 KDECompatWindow>
{
    public:

	bool
	init ();
};
