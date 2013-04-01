/*
 *
 * Compiz mouse position polling plugin
 *
 * mousepoll.c
 *
 * Copyright : (C) 2008 by Dennis Kasprzyk
 * E-mail    : onestone@opencompositing.org
 *
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
#include <core/timer.h>

#include <mousepoll/mousepoll.h>

#include "mousepoll_options.h"

typedef enum _MousepollOptions
{
    MP_DISPLAY_OPTION_MOUSE_POLL_INTERVAL,
    MP_DISPLAY_OPTION_NUM
} MousepollDisplayOptions;

extern const unsigned short MP_OPTION_MOUSE_POLL_INTERVAL;
extern const unsigned short MP_OPTION_NUM;

class MousepollScreen;
extern template class PluginClassHandler <MousepollScreen, CompScreen, COMPIZ_MOUSEPOLL_ABI>;

class MousepollScreen :
    public PluginClassHandler <MousepollScreen, CompScreen, COMPIZ_MOUSEPOLL_ABI>,
    public MousepollOptions
{
    public:

	MousepollScreen (CompScreen *screen);

	std::list<MousePoller *> pollers;
	CompTimer		 timer;

	CompPoint pos;

	bool
	updatePosition ();

	bool
	getMousePosition ();

	bool
	addTimer (MousePoller *poller);

	void
	removeTimer (MousePoller *poller);

	void updateTimer ();
};

#define MOUSEPOLL_SCREEN(s)						\
    MousepollScreen *ms = MousepollScreen::get (s)

#define NUM_OPTIONS(s) (sizeof ((s)->opt) / sizeof (CompOption))

class MousepollPluginVTable :
    public CompPlugin::VTableForScreen<MousepollScreen>
{
    public:

	bool init ();
	void fini ();
};

