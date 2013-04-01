/*
 *
 * Compiz application switcher plugin
 *
 * staticswitcher.h
 *
 * Copyright : (C) 2008 by Danny Baumann
 * E-mail    : maniac@compiz-fusion.org
 *
 * Based on switcher.c:
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <X11/Xatom.h>

#include <compiztoolbox/compiztoolbox.h>

#include <core/pluginclasshandler.h>

#include "staticswitcher_options.h"

class StaticSwitchScreen :
    public BaseSwitchScreen,
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public PluginClassHandler<StaticSwitchScreen,CompScreen>,
    public StaticswitcherOptions
{
    public:
	StaticSwitchScreen (CompScreen *screen);
	~StaticSwitchScreen ();

	void preparePaint (int);
	void donePaint ();

	bool glPaintOutput (const GLScreenPaintAttrib &,
			    const GLMatrix &, const CompRegion &,
			    CompOutput *, unsigned int);

	void updatePopupWindow ();
	void updateWindowList ();
	void createWindowList ();
	bool getPaintRectangle (CompWindow *w,
				CompRect   &rect,
				int        *opacity);
	void doWindowDamage (CompWindow *w);
	void handleSelectionChange (bool toNext, int nextIdx);
	void createPopup ();
	bool showPopup ();
	Cursor getCursor (bool mouseSelectOn);
	void initiate (SwitchWindowSelection selection,
		       bool                  shouldShowPopup);
	void windowRemove (CompWindow *w);
	int getRowXOffset (int y);
	void getWindowPosition (unsigned int index,
				int          *x,
				int          *y);
	CompWindow *findWindowAt (int x,
				  int y);
	void handleEvent (XEvent *event);
	bool adjustVelocity ();
	void paintRect (const GLMatrix &transform,
			CompRect &box,
			int offset,
			unsigned short *color,
			unsigned short opacity);
	void paintSelectionRect (const GLMatrix &transform,
	                         int             x,
				 int          y,
				 float        dx,
				 float        dy,
				 unsigned int opacity);
	void getMinimizedAndMatch (bool &minimizedOption,
				   CompMatch *&match);
	bool getMipmap ();

	Window    lastActiveWindow;

	CompTimer popupDelayTimer;

	Window clientLeader;

	int previewWidth;
	int previewHeight;
	int previewBorder;
	int xCount;

	bool switching;

	GLfloat mVelocity;

	float pos;
	float move;

	bool mouseSelect;
};

class StaticSwitchWindow :
    public BaseSwitchWindow,
    public CompositeWindowInterface,
    public GLWindowInterface,
    public PluginClassHandler<StaticSwitchWindow,CompWindow>
{
    public:
	StaticSwitchWindow (CompWindow *window);

	bool isSwitchWin (bool removing = false);
	bool damageRect (bool initial, const CompRect &rect);

	bool glPaint (const GLWindowPaintAttrib &, const GLMatrix &,
		      const CompRegion &, unsigned int);

	void paintThumb (const GLWindowPaintAttrib &, const GLMatrix &,
		         unsigned int, int, int);
	void updateIconTexturedWindow (GLWindowPaintAttrib  &sAttrib,
				       int                  &wx,
				       int                  &wy,
				       int                  x,
				       int                  y,
				       GLTexture            *icon);
	void updateIconNontexturedWindow (GLWindowPaintAttrib  &sAttrib,
					  int                  &wx,
					  int                  &wy,
					  float                &width,
					  float                &height,
					  int                  x,
					  int                  y,
					  GLTexture            *icon);
	void updateIconPos (int   &wx,
			    int   &wy,
			    int   x,
			    int   y,
			    float width,
			    float height);

	IconMode getIconMode ();

	StaticSwitchScreen    *sScreen;
};

#define MAX_ICON_SIZE 256

extern const unsigned short PREVIEWSIZE;
extern const unsigned short BORDER;

#define SWITCH_SCREEN(s) \
    StaticSwitchScreen *ss = StaticSwitchScreen::get (s)

#define SWITCH_WINDOW(w) \
    StaticSwitchWindow *sw = StaticSwitchWindow::get (w)

class StaticSwitchPluginVTable :
    public CompPlugin::VTableForScreenAndWindow<StaticSwitchScreen, StaticSwitchWindow>
{
    public:

	bool init ();
};

