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

#ifndef _COMPIZ_DECOR_PIXMAP_REQUESTS_H
#define _COMPIZ_DECOR_PIXMAP_REQUESTS_H

#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/make_shared.hpp>
#include <decoration.h>

#include <X11/Xlib.h>

class DecorPixmapInterface
{
    public:

	typedef boost::shared_ptr <DecorPixmapInterface> Ptr;

	virtual ~DecorPixmapInterface () {};

	virtual Pixmap getPixmap () = 0;
};

class DecorPixmapReceiverInterface
{
    public:

	virtual ~DecorPixmapReceiverInterface () {}

	virtual void pending () = 0;
	virtual void update () = 0;
};

/* So far, nothing particularly interesting here
 * we just need a way to pass around pointers for
 * testing */
class DecorationInterface
{
    public:

	typedef boost::shared_ptr <DecorationInterface> Ptr;

	virtual ~DecorationInterface () {}

	virtual DecorPixmapReceiverInterface & receiverInterface () = 0;
	virtual unsigned int getFrameType () const = 0;
	virtual unsigned int getFrameState () const = 0;
	virtual unsigned int getFrameActions () const = 0;
};

class DecorPixmapDeletionInterface
{
    public:

	typedef boost::shared_ptr <DecorPixmapDeletionInterface> Ptr;

	virtual ~DecorPixmapDeletionInterface () {}

	virtual int postDeletePixmap (Pixmap pixmap) = 0;
};

class X11PixmapDeletor :
    public DecorPixmapDeletionInterface
{
    public:

	typedef boost::shared_ptr <X11PixmapDeletor> Ptr;

	X11PixmapDeletor (Display *dpy) :
	    mDisplay (dpy)
	{
	}

	int postDeletePixmap (Pixmap pixmap) { return decor_post_delete_pixmap (mDisplay, pixmap); }

    private:

	Display *mDisplay;
};

class DecorPixmap :
    public DecorPixmapInterface
{
    public:

	typedef boost::shared_ptr <DecorPixmap> Ptr;

	DecorPixmap (Pixmap p, DecorPixmapDeletionInterface::Ptr deletor);
	~DecorPixmap ();

	Pixmap getPixmap ();

    private:

	Pixmap mPixmap;
	DecorPixmapDeletionInterface::Ptr mDeletor;
};

class DecorPixmapRequestorInterface
{
    public:

	virtual ~DecorPixmapRequestorInterface () {}

	virtual int postGenerateRequest (unsigned int frameType,
					 unsigned int frameState,
					 unsigned int frameActions) = 0;

	virtual void handlePending (long *data) = 0;
};

class DecorationListFindMatchingInterface
{
    public:

	virtual ~DecorationListFindMatchingInterface () {}

	virtual DecorationInterface::Ptr findMatchingDecoration (unsigned int frameType,
								 unsigned int frameState,
								 unsigned int frameActions) = 0;
};

class X11DecorPixmapRequestor :
    public DecorPixmapRequestorInterface
{
    public:

	X11DecorPixmapRequestor (Display *dpy,
				 Window  xid,
				 DecorationListFindMatchingInterface *listFinder);

	int postGenerateRequest (unsigned int frameType,
				 unsigned int frameState,
				 unsigned int frameActions);

	void handlePending (long *data);

    private:

	Display *mDpy;
	Window  mWindow;
	DecorationListFindMatchingInterface *mListFinder;
};

class X11DecorPixmapReceiver :
    public DecorPixmapReceiverInterface
{
    public:

	static const unsigned int UpdateRequested = 1 << 0;
	static const unsigned int UpdatesPending = 1 << 1;

	X11DecorPixmapReceiver (DecorPixmapRequestorInterface *,
				DecorationInterface *decor);

	void pending ();
	void update ();
    private:

	unsigned int mUpdateState;
	DecorPixmapRequestorInterface *mDecorPixmapRequestor;
	DecorationInterface *mDecoration;
};

#endif
