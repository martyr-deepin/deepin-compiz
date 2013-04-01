/*
 * Copyright © 2005 Novell, Inc.
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
 */

#include "core/plugin.h"

/* XXX: This dependency needs to go away */
#include "privatescreen.h"

#include <boost/scoped_array.hpp>
#include <boost/foreach.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <sys/types.h>

#include <algorithm>
#include <set>

#define foreach BOOST_FOREACH

static const char here[] = "core";

CompPlugin::Map pluginsMap;
CompPlugin::List plugins;

class CorePluginVTable : public CompPlugin::VTable
{
    public:

	bool init ();

	CompOption::Vector & getOptions ();

	bool setOption (const CompString  &name,
			CompOption::Value &value);
};

COMPIZ_PLUGIN_20090315 (core, CorePluginVTable)

CompPlugin::VTable * getCoreVTable ()
{
    if (!coreVTable)
    {
	return getCompPluginVTable20090315_core ();
    }

    return coreVTable;
}

bool
CorePluginVTable::init ()
{
    return true;
}

CompOption::Vector &
CorePluginVTable::getOptions ()
{
    return screen->getOptions ();
}

bool
CorePluginVTable::setOption (const CompString  &name,
			     CompOption::Value &value)
{
    return screen->setOption (name, value);
}

static bool
cloaderLoadPlugin (CompPlugin *p,
		   const char *path,
		   const char *name)
{
    if (path)
	return false;

    if (strcmp (name, getCoreVTable ()->name ().c_str ()))
	return false;

    p->vTable	      = getCoreVTable ();
    p->devPrivate.ptr = NULL;
    p->devType	      = "cloader";

    return true;
}

static void
cloaderUnloadPlugin (CompPlugin *p)
{
    delete p->vTable;
}

static bool
dlloaderLoadPlugin (CompPlugin *p,
		    const char *path,
		    const char *name)
{
    CompString  file;
    void        *dlhand;
    bool        loaded = false;

    if (cloaderLoadPlugin (p, path, name))
	return true;

    if (path)
    {
	file  = path;
	file += "/";
    }

    file += "lib";
    file += name;
    file += ".so";

    compLogMessage (here, CompLogLevelDebug,
                    "Trying to load %s from: %s", name, file.c_str ());
    
    int open_flags = RTLD_NOW;
#ifdef DEBUG
    // Do not unload the library during dlclose.
    open_flags |= RTLD_NODELETE;
    // Make the symbols available globally
    open_flags |= RTLD_GLOBAL;
#endif
    dlhand = dlopen (file.c_str (), open_flags);
    if (dlhand)
    {
	PluginGetInfoProc getInfo;
	char		  *error;
	char              sym[1024];

	compLogMessage (here, CompLogLevelDebug,
	                "Opened library: %s", file.c_str ());
	dlerror ();

	snprintf (sym, 1024, "getCompPluginVTable20090315_%s", name);
	getInfo = (PluginGetInfoProc) dlsym (dlhand, sym);

	error = dlerror ();
	if (error)
	{
	    compLogMessage (here, CompLogLevelError, "dlsym: %s", error);
	    getInfo = 0;
	}

	if (getInfo)
	{
	    p->vTable = (*getInfo) ();
	    if (!p->vTable)
	    {
		compLogMessage (here, CompLogLevelError,
				"Couldn't get vtable from '%s' plugin",
				file.c_str ());
	    }
	    else
	    {
		p->devPrivate.ptr = dlhand;
		p->devType	  = "dlloader";
		loaded            = true;
		compLogMessage (here, CompLogLevelDebug,
		                "Loaded plugin %s from: %s",
		                name, file.c_str ());
	    }
	}
    }
    else
    {
	compLogMessage (here, CompLogLevelDebug,
			"dlopen failed: %s", dlerror ());
    }

    if (!loaded && dlhand)
	dlclose (dlhand);

    return loaded;
}

static void
dlloaderUnloadPlugin (CompPlugin *p)
{
    if (p->devType == "dlloader")
    {
	const char *name = p->vTable->name ().c_str ();
	compLogMessage (here, CompLogLevelDebug, "Closing library: %s", name);
	delete p->vTable;
	dlclose (p->devPrivate.ptr);
    }
    else
	cloaderUnloadPlugin (p);
}

LoadPluginProc   loaderLoadPlugin   = dlloaderLoadPlugin;
UnloadPluginProc loaderUnloadPlugin = dlloaderUnloadPlugin;

bool
CompManager::initPlugin (CompPlugin *p)
{
    const char *name = p->vTable->name ().c_str ();
    if (!p->vTable->init ())
    {
	compLogMessage (here, CompLogLevelError,
	                "Plugin init failed: %s", name);
	return false;
    }

    if (screen && screen->displayInitialised())
    {
	if (!p->vTable->initScreen (screen))
	{
	    compLogMessage (here, CompLogLevelError,
	                    "Plugin initScreen failed: %s", name);
	    p->vTable->fini ();
	    return false;
	}
	if (!screen->initPluginForScreen (p))
	{
	    compLogMessage (here, CompLogLevelError,
	                    "initPluginForScreen failed: %s", name);
	    p->vTable->fini ();
	    return false;
	}
    }

    return true;
}

void
CompManager::finiPlugin (CompPlugin *p)
{

    if (screen)
    {
	screen->finiPluginForScreen (p);
	p->vTable->finiScreen (screen);
    }

    p->vTable->fini ();
}

bool
CompScreen::initPluginForScreen (CompPlugin *p)
{
    WRAPABLE_HND_FUNCTN_RETURN (bool, initPluginForScreen, p)
    return _initPluginForScreen (p);
}

bool
CompScreenImpl::_initPluginForScreen (CompPlugin *p)
{
    using compiz::private_screen::WindowManager;

    bool status               = true;
    WindowManager::iterator it, fail;
    CompWindow               *w;

    it = fail = windowManager.begin ();
    for (;it != windowManager.end (); ++it)
    {
	w = *it;
	if (!p->vTable->initWindow (w))
	{
	    const char *name = p->vTable->name ().c_str ();
	    compLogMessage (here, CompLogLevelError,
	                    "initWindow failed for %s", name);
            fail   = it;
            status = false;
            break;
	}
    }

    it = windowManager.begin ();
    for (;it != fail; ++it)
    {
	w = *it;
	p->vTable->finiWindow (w);
    }

    return status;
}

void
CompScreen::finiPluginForScreen (CompPlugin *p)
{
    WRAPABLE_HND_FUNCTN (finiPluginForScreen, p)
    _finiPluginForScreen (p);
}

void
CompScreenImpl::_finiPluginForScreen (CompPlugin *p)
{
    windowManager.forEachWindow(boost::bind(&CompPlugin::VTable::finiWindow, p->vTable, _1));
}

bool
CompPlugin::screenInitPlugins (CompScreen *s)
{
    CompPlugin::List::reverse_iterator it = plugins.rbegin ();

    CompPlugin *p = NULL;

    /* Plugins is a btf list, so iterate it in reverse */
    while (it != plugins.rend ())
    {
	p = (*it);

	if (p->vTable->initScreen (s))
	    s->initPluginForScreen (p);

	++it;
    }

    return true;
}

void
CompPlugin::screenFiniPlugins (CompScreen *s)
{
    foreach (CompPlugin *p, plugins)
    {
	s->finiPluginForScreen (p);
	p->vTable->finiScreen (s);
    }

}

bool
CompPlugin::windowInitPlugins (CompWindow *w)
{
    bool status = true;

    for (List::reverse_iterator rit = plugins.rbegin ();
         rit != plugins.rend (); ++rit)
    {
	status &= (*rit)->vTable->initWindow (w);
    }

    return status;
}

void
CompPlugin::windowFiniPlugins (CompWindow *w)
{
    foreach (CompPlugin *p, plugins)
    {
	p->vTable->finiWindow (w);
    }
}


CompPlugin *
CompPlugin::find (const char *name)
{
    CompPlugin::Map::iterator it = pluginsMap.find (name);

    if (it != pluginsMap.end ())
        return it->second;

    return NULL;
}

void
CompPlugin::unload (CompPlugin *p)
{
    if (p->vTable)
    {
	const char *name = p->vTable->name ().c_str ();
	compLogMessage (here, CompLogLevelInfo, "Unloading plugin: %s", name);
    }
    loaderUnloadPlugin (p);
    delete p;
}

CompPlugin *
CompPlugin::load (const char *name)
{
    std::auto_ptr<CompPlugin>p(new CompPlugin ());

    p->devPrivate.uval = 0;
    p->devType	       = "";
    p->vTable	       = 0;

    compLogMessage (here, CompLogLevelInfo, "Loading plugin: %s", name);

    if (char* home = getenv ("HOME"))
    {
        boost::scoped_array<char> plugindir(new char [strlen (home) + strlen (HOME_PLUGINDIR) + 3]);
        sprintf (plugindir.get(), "%s/%s", home, HOME_PLUGINDIR);

        if (loaderLoadPlugin (p.get(), plugindir.get(), name))
            return p.release();
    }

    if (loaderLoadPlugin (p.get(), PLUGINDIR, name))
        return p.release();

    if (loaderLoadPlugin (p.get(), NULL, name))
        return p.release();

    compLogMessage (here, CompLogLevelError, "Failed to load plugin: %s", name);

    return 0;
}

bool
CompPlugin::push (CompPlugin *p)
{
    const char *name = p->vTable->name ().c_str ();

    std::pair<CompPlugin::Map::iterator, bool> insertRet =
        pluginsMap.insert (std::pair<const char *, CompPlugin *> (name, p));

    if (!insertRet.second)
    {
	compLogMessage (here, CompLogLevelWarn,
			"Plugin '%s' already active",
			p->vTable->name ().c_str ());

	return false;
    }

    plugins.push_front (p);

    compLogMessage (here, CompLogLevelInfo, "Starting plugin: %s", name);
    if (CompManager::initPlugin (p))
    {
	compLogMessage (here, CompLogLevelDebug, "Started plugin: %s", name);
    }
    else
    {
	compLogMessage (here, CompLogLevelError,
	    "Failed to start plugin: %s", name);

        pluginsMap.erase (name);
	plugins.pop_front ();

	return false;
    }

    return true;
}

CompPlugin *
CompPlugin::pop (void)
{
    if (plugins.empty ())
	return NULL;

    CompPlugin *p = plugins.front ();

    if (!p)
	return 0;

    const char *name = p->vTable->name ().c_str ();
    pluginsMap.erase (name);

    compLogMessage (here, CompLogLevelInfo, "Stopping plugin: %s", name);
    CompManager::finiPlugin (p);
    compLogMessage (here, CompLogLevelDebug, "Stopped plugin: %s", name);

    plugins.pop_front ();

    return p;
}

CompPlugin::List &
CompPlugin::getPlugins (void)
{
    return plugins;
}

int
CompPlugin::getPluginABI (const char *name)
{
    CompPlugin *p = find (name);
    CompString s = name;

    if (!p)
	return 0;

    s += "_ABI";

    if (!screen->hasValue (s))
	return 0;

    return screen->getValue (s).uval;
}

bool
CompPlugin::checkPluginABI (const char *name,
			    int        abi)
{
    int pluginABI;

    pluginABI = getPluginABI (name);
    if (!pluginABI)
    {
	compLogMessage (here, CompLogLevelError,
			"Plugin '%s' not loaded.\n", name);
	return false;
    }
    else if (pluginABI != abi)
    {
	compLogMessage (here, CompLogLevelError,
			"Plugin '%s' has ABI version '%d', expected "
			"ABI version '%d'.\n",
			name, pluginABI, abi);
	return false;
    }

    return true;
}

CompPlugin::VTable::VTable () :
    mName (""),
    mSelf (NULL)
{
}

CompPlugin::VTable::~VTable ()
{
    if (mSelf)
	*mSelf = NULL;
}

void
CompPlugin::VTable::initVTable (CompString         name,
				CompPlugin::VTable **self)
{
    mName = name;
    if (self)
    {
	mSelf = self;
	*mSelf = this;
    }
}

const CompString
CompPlugin::VTable::name () const
{
    return mName;
}

void
CompPlugin::VTable::fini ()
{
}

bool
CompPlugin::VTable::initScreen (CompScreen *)
{
    return true;
}

void
CompPlugin::VTable::finiScreen (CompScreen *)
{
}

bool
CompPlugin::VTable::initWindow (CompWindow *)
{
    return true;
}

void
CompPlugin::VTable::finiWindow (CompWindow *)
{
}

CompOption::Vector &
CompPlugin::VTable::getOptions ()
{
    return noOptions ();
}

bool
CompPlugin::VTable::setOption (const CompString  &name,
			       CompOption::Value &value)
{
    return false;
}
