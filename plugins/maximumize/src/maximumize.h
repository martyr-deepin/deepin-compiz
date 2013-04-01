/*
 * Compiz Fusion Maximumize plugin
 *
 * Copyright 2007-2008 Kristian Lyngstøl <kristian@bohemians.org>
 * Copyright 2008 Eduardo Gurgel Pinho <edgurgel@gmail.com>
 * Copyright 2008 Marco Diego Aurelio Mesquita <marcodiegomesquita@gmail.com>
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
 * Author(s):
 * Kristian Lyngstøl <kristian@bohemians.org>
 * Eduardo Gurgel <edgurgel@gmail.com>
 * Marco Diego Aurélio Mesquita <marcodiegomesquita@gmail.com>
 *
 * Description:
 *
 * Maximumize resizes a window so it fills as much of the free space in any
 * direction as possible without overlapping with other windows.
 */

#include <core/core.h>
#include <core/pluginclasshandler.h>

#include "maximumize_options.h"

/* Convenience constants to make the code more readable (hopefully) */
extern const short	    REDUCE;
extern const unsigned short INCREASE;

typedef struct
{
    bool    left;
    bool    right;
    bool    up;
    bool    down;
    bool    shrink; // Shrink and grow can both be executed.
    bool    grow;   // Shrink will run first.
} MaxSet;

class MaximumizeScreen :
    public PluginClassHandler <MaximumizeScreen, CompScreen>,
    public MaximumizeOptions
{
    public:

	MaximumizeScreen (CompScreen *);

	bool
	triggerGeneral (CompAction         *action,
			CompAction::State  state,
			CompOption::Vector &options,
			bool		   grow);

	bool
	triggerDirection (CompAction         *action,
			  CompAction::State  state,
			  CompOption::Vector &options,
			  bool		     left,
			  bool		     right,
			  bool		     up,
			  bool		     down,
			  bool		     grow);
    private:
	typedef enum {
	    X1,
	    X2,
	    Y1,
	    Y2
	} Corner;

	bool
	substantialOverlap (const CompRect& a,
			    const CompRect& b);

	CompRegion
	findEmptyRegion (CompWindow      *window,
			 const CompRect& output);

	bool
	boxCompare (const CompRect& a,
		    const CompRect& b);

	void
	growGeneric (CompWindow        *w,
		     CompRect&         tmp,
		     const CompRegion& r,
		     Corner            corner,
		     const short       inc);

	inline void
	addToCorner (CompRect&   rect,
		     Corner      corner,
		     const short inc);

	inline void
	growWidth (CompWindow        *w,
		   CompRect&         tmp,
		   const CompRegion& r,
		   const MaxSet&     mset);

	inline void
	growHeight (CompWindow         *w,
		    CompRect&          tmp,
		    const CompRegion&  r,
		    const MaxSet&      mset);

	CompRect
	extendBox (CompWindow        *w,
		   const CompRect&   tmp,
		   const CompRegion& r,
		   bool	             xFirst,
		   const MaxSet&     mset);

	void
	setBoxWidth (CompRect&     box,
		     const int     width,
		     const MaxSet& mset);

	void
	setBoxHeight (CompRect&      box,
		      const int      height,
		      const MaxSet&  mset);

	CompRect
	minimumize (CompWindow      *w,
		    const CompRect& box,
		    const MaxSet&   mset);

	CompRect
	findRect (CompWindow        *w,
		  const CompRegion& r,
		  const MaxSet&     mset);

	unsigned int
	computeResize (CompWindow     *w,
		       XWindowChanges *xwc,
		       const MaxSet&  mset);
};

#define MAX_SCREEN(s)							       \
    MaximumizeScreen *ms = MaximumizeScreen::get (s);

class MaximumizePluginVTable :
    public CompPlugin::VTableForScreen <MaximumizeScreen>
{
    public:

	bool init ();
};



