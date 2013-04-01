/*
 * Copyright © 2006 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 *
 * Ported to Compiz 0.9 by:
 * Copyright (C) 2009 Sam Spilsbury <smspillaz@gmail.com>
 */

#include <core/screen.h>
#include <core/pluginclasshandler.h>

#include <cstring>
#include <vector>
#include <poll.h>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <libxml/xmlwriter.h>

#define COMPIZ_DBUS_SERVICE_NAME	            "org.freedesktop.compiz"
#define COMPIZ_DBUS_INTERFACE			    "org.freedesktop.compiz"
#define COMPIZ_DBUS_ROOT_PATH			    "/org/freedesktop/compiz"

#define COMPIZ_DBUS_ACTIVATE_MEMBER_NAME            "activate"
#define COMPIZ_DBUS_DEACTIVATE_MEMBER_NAME          "deactivate"
#define COMPIZ_DBUS_SET_MEMBER_NAME                 "set"
#define COMPIZ_DBUS_GET_MEMBER_NAME                 "get"
#define COMPIZ_DBUS_GET_METADATA_MEMBER_NAME	    "getMetadata"
#define COMPIZ_DBUS_LIST_MEMBER_NAME		    "list"
#define COMPIZ_DBUS_GET_PLUGINS_MEMBER_NAME	    "getPlugins"
#define COMPIZ_DBUS_GET_PLUGIN_METADATA_MEMBER_NAME "getPluginMetadata"

#define COMPIZ_DBUS_CHANGED_SIGNAL_NAME		    "changed"
#define COMPIZ_DBUS_PLUGINS_CHANGED_SIGNAL_NAME	    "pluginsChanged"

#define DBUS_FILE_WATCH_CURRENT	0
#define DBUS_FILE_WATCH_PLUGIN	1
#define DBUS_FILE_WATCH_HOME	2
#define DBUS_FILE_WATCH_NUM	3

class DbusScreen :
    public PluginClassHandler <DbusScreen, CompScreen>,
    public ScreenInterface
{
    public:

	DbusScreen (CompScreen *);
	~DbusScreen ();

	CompFileWatchHandle fileWatch[DBUS_FILE_WATCH_NUM];

	DBusConnection    *connection;
	CompWatchFdHandle watchFdHandle;

	bool
	setOptionForPlugin (const char *plugin,
			    const char *name,
			    CompOption::Value &v);

	bool
	initPluginForScreen (CompPlugin *p);

	CompOption::Vector &
	getOptionsFromPath (const std::vector<CompString>& path);

	bool
	handleActionMessage (DBusConnection                 *connection,
			     DBusMessage                    *message,
			     const std::vector<CompString>& path,
			     bool           activate);

	bool
	tryGetValueWithType (DBusMessageIter *iter,
			     int	     type,
			     void 	     *value);

	bool
	getOptionValue (DBusMessageIter   *iter,
			CompOption::Type  type,
			CompOption::Value &value);

	bool
	handleSetOptionMessage (DBusConnection                 *connection,
				DBusMessage                    *message,
				const std::vector<CompString>& path);

	void
	appendSimpleOptionValue (DBusMessage       *message,
				 CompOption::Type  type,
				 CompOption::Value &value);

	void
	appendListOptionValue (DBusMessage       *message,
			       CompOption::Type  type,
			       CompOption::Value &value);

	void
	appendOptionValue (DBusMessage       *message,
			   CompOption::Type  type,
			   CompOption::Value &value);

	bool
	handleGetOptionMessage (DBusConnection                 *connection,
				DBusMessage                    *message,
				const std::vector<CompString>& path);

	bool
	handleListMessage (DBusConnection                 *connection,
			   DBusMessage                    *message,
			   const std::vector<CompString>& path);

	DBusHandlerResult
	handleMessage (DBusConnection *connection,
		       DBusMessage    *message,
		       void           *userData);

	bool
	processMessages (short int);

	void
	sendChangeSignalForOption (CompOption       *o,
			           const CompString &plugin);

	bool
	getPathDecomposed (const char              *data,
			   std::vector<CompString> &path);

	bool
	registerOptions (DBusConnection *connection,
			 char	        *screenPath);

	bool
	unregisterOptions (DBusConnection *connection,
			   char           *screenPath);

	void
	registerPluginForScreen (DBusConnection *connection,
				 const char     *pluginName);

	void
	registerPluginsForScreen (DBusConnection *connection);

	void
	unregisterPluginForScreen (DBusConnection *connection,
				   const char     *pluginName);

	void
	unregisterPluginsForScreen (DBusConnection *connection);

	void
	sendPluginsChangedSignal (const char *name);
};

class DbusPluginVTable :
    public CompPlugin::VTableForScreen <DbusScreen>
{
    public:

	bool init ();
};

#define DBUS_SCREEN(s)							       \
     DbusScreen *ds = DbusScreen::get (s)
