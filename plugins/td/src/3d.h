/*
 *
 * Compiz 3d plugin
 *
 * 3d.c
 *
 * Copyright : (C) 2006 by Roi Cohen
 * E-mail    : roico12@gmail.com
 *
 * Modified by : Dennis Kasprzyk <onestone@opencompositing.org>
 *               Danny Baumann <maniac@opencompositing.org>
 *               Robert Carr <racarr@beryl-project.org>
 *               Diogo Ferreira <diogo@underdev.org>
 *		 Kevin Lange <klange@ogunderground.com>
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

/** TODO:
  1. Add 3d shadows / projections.
  2. Add an option to select z-order of windows not only by viewports,
     but also by screens.
*/

#include <core/core.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>
#include <opengl/opengl.h>

#include <cube/cube.h>

#include <cmath>

#include "td_options.h"

extern const double PI;

class TdScreen :
    public PluginClassHandler <TdScreen, CompScreen>,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public CubeScreenInterface,
    public TdOptions
{
    public:
    
	TdScreen (CompScreen *);
	~TdScreen ();
	
	CompositeScreen *cScreen;
	GLScreen	*gScreen;
	CubeScreen	*cubeScreen;
	
	void
	preparePaint (int);
	
	bool
	glPaintOutput (const GLScreenPaintAttrib &,
		       const GLMatrix		 &,
		       const CompRegion		 &,
		       CompOutput		 *,
		       unsigned int		   );
	
	void
	donePaint ();
	
	void
	glApplyTransform (const GLScreenPaintAttrib &,
			  CompOutput		    *,
			  GLMatrix		    *);
	
	void
	cubePaintViewport (const GLScreenPaintAttrib &,
			   const GLMatrix	     &,
			   const CompRegion	     &,
			   CompOutput		     *,
			   unsigned int			);
			   
	bool
	cubeShouldPaintViewport (const GLScreenPaintAttrib &,
				 const GLMatrix		   &,
				 CompOutput		   *,
				 PaintOrder		    );
	
	bool
	cubeShouldPaintAllViewports ();
				 
	bool mActive;
	bool mPainting3D;
	float mCurrentScale;
	float mBasicScale;
	float mMaxDepth;
	bool mDamage;
	
	bool mWithDepth;
	
	GLMatrix mBTransform;
};

#define TD_SCREEN(s)							       \
    TdScreen *tds = TdScreen::get (s)

class TdWindow :
    public PluginClassHandler <TdWindow, CompWindow>,
    public GLWindowInterface
{
    public:
    
	TdWindow (CompWindow *);
	~TdWindow ();
	
	CompWindow *window;
	CompositeWindow *cWindow;
	GLWindow   *gWindow;
	
	bool
	glPaint (const GLWindowPaintAttrib &,
		 const GLMatrix		   &,
		 const CompRegion	   &,
		 unsigned int		     );

	bool
	glPaintWithDepth (const GLWindowPaintAttrib &,
		 	  const GLMatrix	    &,
		 	  const CompRegion	    &,
		 	  unsigned int		     );
		 
	bool
	is3D ();
	
	bool mIs3D;
	bool mFtb;
	
	float mDepth;
};

#define TD_WINDOW(w)							       \
    TdWindow *tdw = TdWindow::get (w)

class TdPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <TdScreen, TdWindow>
{
    public:
    
	bool init ();
};
