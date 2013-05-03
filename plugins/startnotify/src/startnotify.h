/*
 *
 * Copyright : (C) 2012, Linux Deepin Inc.
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

#include <cmath>

#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>
#include <mousepoll/mousepoll.h>

#include "startnotify_options.h"

#define	CURSOR_WIDTH	20
#define	CURSOR_HEIGHT	22
#define CURSOR_NUM	31

#define CURSOR_HOTSPOT_X 10
#define CURSOR_HOTSPOT_Y 10

#define CURSOR_FPS  (1.0 * CURSOR_NUM)

#define PLUGIN_NAME	"startnotify"
#define CURSOR_NAME "iconbusy.png"

class AnimCursor
{
    public:
	AnimCursor ();
	~AnimCursor ();

	bool	active;

	//GLuint		animTex;      //
    GLTexture::List animTex;

	GLuint  	animTexIndex; //current index into  animTex. count from 0.

    GLfloat     x, y;         //top-left of the sprite on the screen
    GLfloat     cursor_texcoords[12];
    GLfloat     cursor_vertices[18];

	void
	initAnimCursor ();	//used in preparePaint and screen initialization

	void
	drawAnimCursor (const GLMatrix &transform);	//only used in glPaintOutput

	void
	finiAnimCursor (); 	//only used in donePaint
};

class StartnotifyScreen :
    public PluginClassHandler <StartnotifyScreen, CompScreen>,
    public StartnotifyOptions,
    public CompositeScreenInterface,
    public GLScreenInterface
{
    public:

    int  count;//for testing
	StartnotifyScreen (CompScreen *);
	~StartnotifyScreen ();

	CompositeScreen *cScreen;
	GLScreen	*gScreen;

    CompScreen::GrabHandle  grabIndex;

	bool	       	active;

	//mouse  polling;
	CompPoint	mousePos;
	MousePoller    	pollHandle;
	void
	positionUpdate (const CompPoint &p);

	//rendering
	AnimCursor	animCursor;

	void
	preparePaint (int);

	bool
	glPaintOutput (const GLScreenPaintAttrib &,
		       const GLMatrix		 &,
		       const CompRegion		 &,
		       CompOutput		 *,
		       unsigned int		 );

	void
	donePaint ();

	void
	doDamageRegion ();

	//
	bool
	initiate (CompAction         *action,
		  CompAction::State  state,
		  CompOption::Vector options);

	bool
	terminate (CompAction         *action,
		   CompAction::State  state,
		   CompOption::Vector options);

};

#define STARTNOTIFY_SCREEN(s)						       \
    StartnotifyScreen *ss = StartnotifyScreen::get (s);

class StartnotifyPluginVTable :
    public CompPlugin::VTableForScreen <StartnotifyScreen>
{
    bool init ();
};
