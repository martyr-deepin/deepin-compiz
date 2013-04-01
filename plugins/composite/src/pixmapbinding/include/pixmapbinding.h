/*
 * Copyright © 2012 Canonical Ltd.
 * Copyright © 2008 Dennis Kasprzyk
 * Copyright © 2007 Novell, Inc.
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
 *          David Reveman <davidr@novell.com>
 *          Sam Spilsbury <sam.spilsbury@canonical.com>
 */

#ifndef _COMPOSITE_PIXMAP_REBIND_H
#define _COMPOSITE_PIXMAP_REBIND_H

#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <core/size.h>

#include <X11/Xlib.h>

class ServerGrabInterface;

class CompositePixmapRebindInterface
{
    public:

	virtual ~CompositePixmapRebindInterface () {}

	virtual Pixmap pixmap () const = 0;
	virtual bool bind () = 0;
	virtual const CompSize & size () const = 0;
	virtual void release () = 0;

	/* This isn't great API, but probably necessary
	 * unless we make it a requirement that the
	 * renderer sets the strategy for the rebinder */
	virtual void setNewPixmapReadyCallback (const boost::function <void ()> &) = 0;

	/* Also don't like this either */
	virtual void allowFurtherRebindAttempts () = 0;
};

class PixmapFreezerInterface
{
    public:

	virtual ~PixmapFreezerInterface () {}

	virtual bool frozen () = 0;
};

class WindowAttributesGetInterface
{
    public:

	virtual ~WindowAttributesGetInterface () {}

	virtual bool getAttributes (XWindowAttributes &) = 0;
};

class WindowPixmapInterface
{
    public:

	virtual ~WindowPixmapInterface () {}

	typedef boost::shared_ptr <WindowPixmapInterface> Ptr;

	virtual Pixmap pixmap () const = 0;
	virtual void releasePixmap ()  = 0;
};

class X11WindowPixmap :
    public WindowPixmapInterface
{
    public:

	X11WindowPixmap (Display *d, Pixmap p) :
	    mDisplay (d),
	    mPixmap (p)
	{
	}

	Pixmap pixmap () const
	{
	    return mPixmap;
	}

	void releasePixmap ()
	{
	    if (mPixmap)
		XFreePixmap (mDisplay, mPixmap);

	    mPixmap = None;
	}

    private:

	Display *mDisplay;
	Pixmap  mPixmap;
};

class WindowPixmap
{
    public:

	WindowPixmap () :
	    mPixmap ()
	{
	}

	WindowPixmap (WindowPixmapInterface::Ptr &pm) :
	    mPixmap (pm)
	{
	}

	Pixmap pixmap () const
	{
	    if (mPixmap)
		return mPixmap->pixmap ();

	    return None;
	}

	~WindowPixmap ()
	{
	    if (mPixmap)
		mPixmap->releasePixmap ();
	}
    private:

	WindowPixmapInterface::Ptr mPixmap;
};

class WindowPixmapGetInterface
{
    public:

	virtual ~WindowPixmapGetInterface () {}

	virtual WindowPixmapInterface::Ptr getPixmap () = 0;
};

class PixmapBinding :
    public CompositePixmapRebindInterface
{
    public:

	typedef boost::function <void ()> NewPixmapReadyCallback;

	PixmapBinding (const NewPixmapReadyCallback &,
			WindowPixmapGetInterface *,
			WindowAttributesGetInterface *,
			PixmapFreezerInterface *,
			ServerGrabInterface *);

	~PixmapBinding ();

	Pixmap pixmap () const;
	bool bind ();
	const CompSize & size () const;
	void release ();
	void setNewPixmapReadyCallback (const boost::function <void ()> &);
	void allowFurtherRebindAttempts ();

    private:

	std::auto_ptr <WindowPixmap>  mPixmap;
	CompSize      mSize;
	bool	      needsRebind;
	bool          bindFailed;
	NewPixmapReadyCallback newPixmapReadyCallback;

	WindowPixmapGetInterface *windowPixmapRetreiver;
	WindowAttributesGetInterface *windowAttributesRetreiver;
	PixmapFreezerInterface        *pixmapFreezer;
	ServerGrabInterface *serverGrab;

};

#endif
