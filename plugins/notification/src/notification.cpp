/*
 * Notification plugin for compiz
 *
 * notification.cpp
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

#include "notification.h"

COMPIZ_PLUGIN_20090315 (notification, NotificationPluginVTable);

const std::string IMAGE_DIR(".compiz/images");

/* libnotify 0.7 introduced proper NOTIFY_CHECK_VERSION macro */
#if defined(NOTIFY_CHECK_VERSION) && !defined(HAVE_LIBNOTIFY_0_6_1)
#if NOTIFY_CHECK_VERSION(0,6,1)
#define HAVE_LIBNOTIFY_0_6_1
#endif
#endif

void
NotificationScreen::logMessage (const char   *component,
			  	CompLogLevel level,
			  	const char   *message)
{
    NotifyNotification *n;
    char               *logLevel, *homeDir;
    CompString         iconUri;
    int                timeout;
    NotifyUrgency      urgency;

    if (level > optionGetMaxLogLevel ())
    {
	screen->logMessage (component, level, message);
	return;
    }

    homeDir = getenv ("HOME");
    if (!homeDir)
	return;

    /* FIXME: when not installing manually, the image will likely reside
              in $PREFIX/share/compiz, not in the home dir */
    iconUri   = "file://";
    iconUri += homeDir;
    iconUri += "/" + IMAGE_DIR + "/compiz.png";
    logLevel = (char *) logLevelToString (level);

    n = notify_notification_new (logLevel, message,
				 iconUri.c_str ()
#ifndef HAVE_LIBNOTIFY_0_6_1
				 , NULL
#endif				 
				 );

    timeout = optionGetTimeout ();
    if (timeout > 0)
	timeout *= 1000;

    notify_notification_set_timeout (n, timeout);

    if (level == CompLogLevelFatal || level == CompLogLevelError)
	urgency = NOTIFY_URGENCY_CRITICAL;
    else if (level == CompLogLevelWarn)
	urgency = NOTIFY_URGENCY_NORMAL;
    else
	urgency = NOTIFY_URGENCY_LOW;

    notify_notification_set_urgency (n, urgency);

    notify_notification_show (n, NULL);
    g_object_unref (G_OBJECT (n));

    screen->logMessage (component, level, message);
}

NotificationScreen::NotificationScreen (CompScreen *screen) :
    PluginClassHandler <NotificationScreen, CompScreen> (screen),
    NotificationOptions ()
{
    ScreenInterface::setHandler (screen);
}

bool
NotificationPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    return true;
}
