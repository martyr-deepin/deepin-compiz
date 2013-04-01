/*
 * Notification plugin for compiz
 *
 * notification.h
 *
 * Copyright (C) 2007 Mike Dransfield (mike (at) blueroot.co.uk)
 * Maintained by Danny Baumann <dannybaumann (at) web.de>
 * Ported to compiz++ by Sam Spilsbury <smspillaz (at) gmail.com>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <core/core.h>
#include <core/pluginclasshandler.h>

#include <libnotify/notify.h>

#include "notification_options.h"

extern const std::string IMAGE_DIR;

class NotificationScreen :
    public PluginClassHandler <NotificationScreen, CompScreen>,
    public NotificationOptions,
    public ScreenInterface
{
    public:

	NotificationScreen (CompScreen *);

	void logMessage (const char *,
			 CompLogLevel,
			 const char *);
};

#define NOTIFICATION_SCREEN(s)						       \
    NotificationScreen *ns = NotificationScreen::get (s);

class NotificationPluginVTable :
    public CompPlugin::VTableForScreen <NotificationScreen>
{
    public:

	bool init ();
};
