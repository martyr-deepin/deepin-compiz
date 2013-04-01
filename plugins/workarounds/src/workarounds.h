/*
 * Copyright (C) 2007 Andrew Riedi <andrewriedi@gmail.com>
 *
 * Sticky window handling and OpenGL fixes:
 * Copyright (c) 2007 Dennis Kasprzyk <onestone@opencompositing.org>
 *
 * Ported to Compiz 0.9:
 * Copyright (c) 2008 Sam Spilsbury <smspillaz@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * This plug-in for Metacity-like workarounds.
 */

#include <string.h>
#include <limits.h>

#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <core/atoms.h>
#include <core/propertywriter.h>
#include <X11/Xatom.h>
#include <X11/extensions/shape.h>

#include "workarounds_options.h"

extern bool haveOpenGL;

#ifndef USE_GLES
typedef void (*GLProgramParameter4dvProc) (GLenum         target,
					   GLuint         index,
					   const GLdouble *data);
#endif

class WorkaroundsScreen :
    public PluginClassHandler <WorkaroundsScreen, CompScreen>,
    public ScreenInterface,
    public GLScreenInterface,
    public CompositeScreenInterface,
    public WorkaroundsOptions
{
    public:

	WorkaroundsScreen (CompScreen *);
	~WorkaroundsScreen ();

	CompositeScreen *cScreen;
	GLScreen	*gScreen;

	Atom		roleAtom;
	std::list <Window> mfwList;
	CompWindowList  minimizingWindows;
	bool		skipTransients;

	PropertyWriter	inputDisabledAtom;

#ifndef USE_GLES
	GL::GLProgramParameter4fProc origProgramEnvParameter4f;
	GLProgramParameter4dvProc    programEnvParameter4dv;

	GL::GLXGetVideoSyncProc      origGetVideoSync;
	GL::GLXWaitVideoSyncProc     origWaitVideoSync;

	GL::GLXCopySubBufferProc     origCopySubBuffer;
#endif

	void
	handleEvent (XEvent *);

	void
	preparePaint (int);

	bool
	glPaintOutput (const GLScreenPaintAttrib &,
		       const GLMatrix		 &,
		       const CompRegion	   	 &,
		       CompOutput		 *,
		       unsigned int		   );

	void
	handleCompizEvent (const char 	      *pluginName,
			   const char 	      *eventName,
			   CompOption::Vector &o);

	void
	addToFullscreenList (CompWindow *w);

	void
	removeFromFullscreenList (CompWindow *w);

	void
	updateParameterFix ();

	void
	updateVideoSyncFix ();

	void
	optionChanged (CompOption		      *opt,
		       WorkaroundsOptions::Options    num);

	void
	checkFunctions (bool window, bool screen);

	void
	setWindowState (unsigned int state, Window id);


};

#define WORKAROUNDS_SCREEN(s)						       \
    WorkaroundsScreen *ws = WorkaroundsScreen::get (s)

class WorkaroundsWindow :
    public PluginClassHandler <WorkaroundsWindow, CompWindow>,
    public WindowInterface,
    public CompositeWindowInterface,
    public GLWindowInterface
{
    public:

	typedef struct _HideInfo {
	    Window shapeWindow;

	    unsigned long skipState;
	    unsigned long shapeMask;

	    XRectangle *inputRects;
	    int        nInputRects;
	    int        inputRectOrdering;

	} HideInfo;

    public:

	WorkaroundsWindow (CompWindow *);
	~WorkaroundsWindow ();

	CompWindow 	*window;
	CompositeWindow *cWindow;
	GLWindow	*gWindow;

	bool adjustedWinType;
	bool madeSticky;
	bool madeFullscreen;
	bool isFullscreen;
	bool madeDemandAttention;
	bool isMinimized;

	unsigned int oldWmType;

	HideInfo *windowHideInfo;

	bool
	isGroupTransient (Window);

	void
	resizeNotify (int, int, int, int);

	void
	getAllowedActions (unsigned int &,
			   unsigned int &);

	void minimize ();
	void unminimize ();
	bool minimized ();

	bool glPaint (const GLWindowPaintAttrib &, const GLMatrix &,
		      const CompRegion &, unsigned int);

	void setVisibility (bool);
	void restoreInputShape (HideInfo *);
	void clearInputShape (HideInfo *);

	void
	removeSticky ();

	CompString
	getRoleAtom ();

	void
	updateSticky ();

	void
	updateUrgencyState ();

	void
	fixupFullscreen ();

	void
	updateFixedWindow (unsigned int newWmType);

	unsigned int
	getFixedWindowType ();

	bool
	damageRect (bool initial, const CompRect &rect);

};

#define WORKAROUNDS_WINDOW(w)						       \
    WorkaroundsWindow *ww = WorkaroundsWindow::get (w)

class WorkaroundsPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <WorkaroundsScreen,
						 WorkaroundsWindow>
{
    public:

	bool init ();
};
