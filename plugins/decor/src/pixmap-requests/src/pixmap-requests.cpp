#include "pixmap-requests.h"
#include <boost/foreach.hpp>
#include <algorithm>

#ifndef foreach
#define foreach BOOST_FOREACH
#endif

DecorPixmap::DecorPixmap (Pixmap pixmap, DecorPixmapDeletionInterface::Ptr d) :
    mPixmap (pixmap),
    mDeletor (d)
{
}

DecorPixmap::~DecorPixmap ()
{
    mDeletor->postDeletePixmap (mPixmap);
}

Pixmap
DecorPixmap::getPixmap ()
{
    return mPixmap;
}

X11DecorPixmapReceiver::X11DecorPixmapReceiver (DecorPixmapRequestorInterface *requestor,
						DecorationInterface *decor) :
    mUpdateState (0),
    mDecorPixmapRequestor (requestor),
    mDecoration (decor)
{
}

void
X11DecorPixmapReceiver::pending ()
{
    if (mUpdateState & X11DecorPixmapReceiver::UpdateRequested)
	mUpdateState |= X11DecorPixmapReceiver::UpdatesPending;
    else
    {
	mUpdateState |= X11DecorPixmapReceiver::UpdateRequested;

	mDecorPixmapRequestor->postGenerateRequest (mDecoration->getFrameType (),
						    mDecoration->getFrameState (),
						    mDecoration->getFrameActions ());
    }
}

void X11DecorPixmapReceiver::update ()
{
    if (mUpdateState & X11DecorPixmapReceiver::UpdatesPending)
	mDecorPixmapRequestor->postGenerateRequest (mDecoration->getFrameType (),
						    mDecoration->getFrameState (),
						    mDecoration->getFrameActions ());

    mUpdateState = 0;
}

X11DecorPixmapRequestor::X11DecorPixmapRequestor (Display *dpy,
						  Window  window,
						  DecorationListFindMatchingInterface *listFinder) :
    mDpy (dpy),
    mWindow (window),
    mListFinder (listFinder)
{
}

int
X11DecorPixmapRequestor::postGenerateRequest (unsigned int frameType,
					      unsigned int frameState,
					      unsigned int frameActions)
{
    return decor_post_generate_request (mDpy,
					 mWindow,
					 frameType,
					 frameState,
					 frameActions);
}

void
X11DecorPixmapRequestor::handlePending (long *data)
{
    DecorationInterface::Ptr d = mListFinder->findMatchingDecoration (static_cast <unsigned int> (data[0]),
								      static_cast <unsigned int> (data[1]),
								      static_cast <unsigned int> (data[2]));

    if (d)
	d->receiverInterface ().pending ();
    else
	postGenerateRequest (static_cast <unsigned int> (data[0]),
			     static_cast <unsigned int> (data[1]),
			     static_cast <unsigned int> (data[2]));
}
