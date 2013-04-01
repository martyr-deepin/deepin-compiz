/*
 *
 * Compiz thumbnail plugin
 *
 * thumbnail.cpp
 *
 * Copyright : (C) 2007 by Dennis Kasprzyk
 * E-mail    : onestone@beryl-project.org
 *
 * Ported to Compiz 0.9
 * Copyright : (C) 2009 by Sam Spilsbury
 * E-mail    : smspillaz@gmail.com
 *
 * Based on thumbnail.c:
 * Copyright : (C) 2007 Stjepan Glavina
 * E-mail    : stjepang@gmail.com
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


/* TODO:
	* - Make Thumbnail it's own class with methods
	* - Make a Thumbnail container class - this is where the window /
	    glow texture is drawn
	* - Set KDE Thumbnail property
	* - Set Compiz Thumbnail property (for plugins like peek)
 */

#include <cmath>

#include <core/core.h>
#include <core/atoms.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <text/text.h>
#include <mousepoll/mousepoll.h>

#include "thumbnail_options.h"

#define THUMB_SCREEN(s)						      \
    ThumbScreen *ts = ThumbScreen::get (s)

#define THUMB_WINDOW(w)						      \
    ThumbWindow *tw = ThumbWindow::get (w)

#define WIN_X(w) ((w)->x () - (w)->border ().left)
#define WIN_Y(w) ((w)->y () - (w)->border ().top)
#define WIN_W(w) ((w)->width () + (w)->border ().left + (w)->border ().right)
#define WIN_H(w) ((w)->height () + (w)->border ().top + (w)->border ().bottom)

extern const unsigned short TEXT_DISTANCE;

bool textPluginLoaded;

typedef struct _Thumbnail
{
    int   x;
    int   y;
    int   width;
    int   height;
    float scale;
    float opacity;
    int   offset;

    CompWindow *win;
    CompWindow *dock;

    CompText   *text;
    bool       textValid;
} Thumbnail;

class ThumbScreen:
	public PluginClassHandler <ThumbScreen, CompScreen>,
	public ScreenInterface,
	public GLScreenInterface,
	public CompositeScreenInterface,
	public ThumbnailOptions
{
    public:

	ThumbScreen (CompScreen *sceen);
	~ThumbScreen ();

	void handleEvent (XEvent *);

	void preparePaint (int);

	bool
	glPaintOutput (const GLScreenPaintAttrib &,
		       const GLMatrix &,
		       const CompRegion &,
		       CompOutput *,
		       unsigned int);

	void
	donePaint ();

	void
	glPaintTransformedOutput (const GLScreenPaintAttrib &,
			          const GLMatrix &,
			          const CompRegion &,
			          CompOutput *,
				  unsigned int);

	void
	freeThumbText (Thumbnail  *t);

	void
	renderThumbText (Thumbnail  *t,
		 	 bool       freeThumb);

	void
	damageThumbRegion (Thumbnail  *t);

	void
	thumbUpdateThumbnail ();

	bool
	thumbShowThumbnail ();

	bool
	checkPosition (CompWindow *w);

	void
	positionUpdate (const CompPoint &pos);

	void
	paintTexture (const GLMatrix &transform,
	              GLushort       *color,
	              int             wx,
		      int wy,
		      int width,
		      int height,
		      int off);

	void
	thumbPaintThumb (Thumbnail           *t,
		 	 const GLMatrix *transform);


	GLScreen   *gScreen;
	CompositeScreen *cScreen;

	CompWindow *dock;
	CompWindow *pointedWin;

	bool      showingThumb;
	Thumbnail thumb;
	Thumbnail oldThumb;
	bool      painted;

	CompTimer displayTimeout;

	GLTexture::List glowTexture;
	GLTexture::List windowTexture;

	int x;
	int y;

	MousePoller poller;
};

class ThumbWindow :
	public PluginClassHandler <ThumbWindow, CompWindow>,
	public WindowInterface,
	public CompositeWindowInterface,
	public GLWindowInterface
{
    public:

	ThumbWindow (CompWindow *window);
	~ThumbWindow ();

	CompWindow *window;
	CompositeWindow *cWindow;
	GLWindow *gWindow;

	bool
	glPaint (const GLWindowPaintAttrib &attrib,
		const GLMatrix		&transform,
		const CompRegion 		&region,
		unsigned int		mask);

	void
	resizeNotify (int dx,
		      int dy,
		      int dwidth,
		      int dheight);

	bool
	damageRect (bool initial,
		    const CompRect &rect);
};

class ThumbPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <ThumbScreen, ThumbWindow>
{
    public:
	bool init ();
};
