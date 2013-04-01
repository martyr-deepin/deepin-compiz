/*
 * Copyright © 2008 Dennis Kasprzyk
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Dennis Kasprzyk not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Dennis Kasprzyk makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DENNIS KASPRZYK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DENNIS KASPRZYK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Dennis Kasprzyk <onestone@compiz-fusion.org>
 */

#ifndef _COMPPLUGINCLASSHANDLER_H
#define _COMPPLUGINCLASSHANDLER_H

#include <typeinfo>
#include <boost/preprocessor/cat.hpp>

#include <core/string.h>
#include <core/valueholder.h>
#include <core/pluginclasses.h>

/* Continuously increments every time a new
 * plugin class is added, guarunteed to be
 * the same as the pcIndex of the most up-to-date
 * PluginClassHandler index. Any index that
 * hold the same value this value is safe to
 * use to retreive the plugin class for a base
 * class (and as such we don't need to constantly
 * re-query ValueHolder for the index of this plugin
 * on the base class)
 */
extern unsigned int pluginClassHandlerIndex;

template<class Tp, class Tb, int ABI = 0>
class PluginClassHandler {
    public:
	PluginClassHandler (Tb *);
	~PluginClassHandler ();

	/**
	 * Changes this PluginClassHandler's state to "failed"
	 */ 
	void setFailed () { mFailed = true; };

	/**
	 * Checks to see if this plugin class failed to load for
	 * whatever reason, so that instantiation of the class
	 * can be aborted
	 */
	bool loadFailed () { return mFailed; };

	/**
	 * Returns the unique instance of the plugin class
	 * for this base class
	 */
	Tb * get () { return mBase; };
	static Tp * get (Tb *);

    private:
	/**
	 * Returns the unique string identifying this plugin type with it's
	 * ABI
	 */
	static CompString keyName ()
	{
	    return compPrintf ("%s_index_%lu", typeid (Tp).name (), ABI);
	}

	/**
	 * Actually initializes the index of this plugin class
	 * by allocating a new index for plugin class storage
	 * on this base class if there wasn't one already and then
	 * storing that index inside of ValueHolder
	 */
	static bool initializeIndex (Tb *base);
	static inline Tp * getInstance (Tb *base);

    private:
	bool mFailed;
	Tb   *mBase;

	static PluginClassIndex mIndex;
};

template<class Tp, class Tb, int ABI>
PluginClassIndex PluginClassHandler<Tp,Tb,ABI>::mIndex;

/**
 * Attaches a unique instance of the specified plugin class to a
 * unique instance of a specified base class
 */
template<class Tp, class Tb, int ABI>
PluginClassHandler<Tp,Tb,ABI>::PluginClassHandler (Tb *base) :
    mFailed (false),
    mBase (base)
{
    /* If allocating plugin class indices for this base has
     * already failed once don't bother to allocate more for
     * this plugin class, just instantly fail */
    if (mIndex.pcFailed)
    {
	mFailed = true;
    }
    else
    {
	/* The index for this plugin class hasn't been initiated
	 * so do that (done once per plugin) */
	if (!mIndex.initiated)
	    mFailed = !initializeIndex (base);

	/* Increase the reference count of this plugin class index
	 * for this plugin on this base object for each attached
	 * plugin class and set the index pointer */
	if (!mIndex.failed)
	{
	    mIndex.refCount++;
	    mBase->pluginClasses[mIndex.index] = static_cast<Tp *> (this);
	}
    }
}

template<class Tp, class Tb, int ABI>
bool
PluginClassHandler<Tp,Tb,ABI>::initializeIndex (Tb *base)
{
    /* Allocate a new storage space index in the array of CompPrivate's
     * specified in the base class */
    mIndex.index = base->allocPluginClassIndex ();
    if (mIndex.index != (unsigned)~0)
    {
	/* Allocation was successful, this is the most recently allocated
	 * plugin class index so it is fresh to use */
	mIndex.initiated = true;
	mIndex.failed    = false;
	mIndex.pcIndex = pluginClassHandlerIndex;

	CompPrivate p;
	p.uval = mIndex.index;

	/* Also store the index value inside of ValueHolder for this plugin-base
	 * combination so that we can fetch it later should the index location
	 * change */
	if (!ValueHolder::Default ()->hasValue (keyName ()))
	{
	    ValueHolder::Default ()->storeValue (keyName (), p);
	    pluginClassHandlerIndex++;
	}
	else
	{
	    compLogMessage ("core", CompLogLevelFatal,
		"Private index value \"%s\" already stored in screen.",
		keyName ().c_str ());
	}
	return true;
    }
    else
    {
	mIndex.index = 0;
	mIndex.failed = true;
	mIndex.initiated = false;
	mIndex.pcFailed = true;
	mIndex.pcIndex = pluginClassHandlerIndex;
	return false;
    }
}

template<class Tp, class Tb, int ABI>
PluginClassHandler<Tp,Tb,ABI>::~PluginClassHandler ()
{
    /* Only bother to destroy the static mIndex if plugin
     * class allocation was actually successful */
    if (!mIndex.pcFailed)
    {
	mIndex.refCount--;

	/* Once this index's reference count hits zero it
	 * means that there are no more plugin classes attached
	 * to the base class, so other plugins are free to use our
	 * index and we can erase our data from ValueHolder
	 * (also increment pluginClassHandlerIndex since after doing
	 *  this operation, all the other indexes with the same pcIndex
	 *  won't be fresh) */
	if (mIndex.refCount == 0)
	{
	    mBase->freePluginClassIndex (mIndex.index);
	    mIndex.initiated = false;
	    mIndex.failed = false;
	    mIndex.pcIndex = pluginClassHandlerIndex;
	    ValueHolder::Default ()->eraseValue (keyName ());
	    pluginClassHandlerIndex++;
	}
    }
}

template<class Tp, class Tb, int ABI>
Tp *
PluginClassHandler<Tp,Tb,ABI>::getInstance (Tb *base)
{
    /* Return the plugin class found at index if it exists
     * otherwise spawn a new one implicitly (objects should
     * never expect ::get () to fail, unless the plugin class
     * fails to instantiate or the underlying base object
     * does not take plugin classes of this type)
     */
    if (base->pluginClasses[mIndex.index])
	return static_cast<Tp *> (base->pluginClasses[mIndex.index]);
    else
    {
	/* mIndex.index will be implicitly set by
	 * the constructor */
	Tp *pc = new Tp (base);

	if (!pc)
	    return NULL;

	/* FIXME: If a plugin class fails to load for
	 * whatever reason, then ::get is going to return
	 * NULL, which is unsafe in cases that aren't
	 * initScreen and initWindow */
	if (pc->loadFailed ())
	{
	    delete pc;
	    return NULL;
	}

	return static_cast<Tp *> (base->pluginClasses[mIndex.index]);
    }
}

template<class Tp, class Tb, int ABI>
Tp *
PluginClassHandler<Tp,Tb,ABI>::get (Tb *base)
{
    /* Always ensure that the index is initialized before
     * calls to ::get */
    if (!mIndex.initiated)
	initializeIndex (base);
    /* If pluginClassHandlerIndex == mIndex.pcIndex it means that our
     * mIndex.index is fresh and can be used directly without needing
     * to fetch it from ValueHolder */
    if (mIndex.initiated && pluginClassHandlerIndex == mIndex.pcIndex)
	return getInstance (base);
    /* If allocating or getting the updated index failed at any point
     * then just return NULL we don't know where our private data is stored */
    if (mIndex.failed && pluginClassHandlerIndex == mIndex.pcIndex)
	return NULL;

    if (ValueHolder::Default ()->hasValue (keyName ()))
    {
	mIndex.index     = ValueHolder::Default ()->getValue (keyName ()).uval;
	mIndex.initiated = true;
	mIndex.failed    = false;
	mIndex.pcIndex = pluginClassHandlerIndex;

	return getInstance (base);
    }
    else
    {
	mIndex.initiated = false;
	mIndex.failed    = true;
	mIndex.pcIndex = pluginClassHandlerIndex;
	return NULL;
    }
}

#endif
