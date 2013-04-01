/*
 * Copyright © 2011 Canonical Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Canonical Ltd. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Canonical Ltd. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * CANONICAL, LTD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL CANONICAL, LTD. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authored by: Sam Spilsbury <sam.spilsbury@canonical.com>
 */

#include "privatestackdebugger.h"
#include "privatescreen.h"
#include "privatewindow.h"
#include <poll.h>

namespace
{
    StackDebugger * gStackDebugger = NULL;
}

StackDebugger *
StackDebugger::Default ()
{
    return gStackDebugger;
}

void
StackDebugger::SetDefault (StackDebugger *dbg)
{
    if (gStackDebugger)
	delete gStackDebugger;

    gStackDebugger = dbg;
}

StackDebugger::StackDebugger (Display *dpy, Window root, FetchXEventInterface *fetchXEvent) :
    mServerNChildren (0),
    mServerChildren (NULL),
    mWindowsChanged (false),
    mServerWindowsChanged (false),
    mRoot (root),
    mDpy (dpy),
    mFetchXEvent (fetchXEvent)
{
}

StackDebugger::~StackDebugger ()
{
    if (mServerChildren)
    {
	XFree (mServerChildren);
	mServerChildren = NULL;
	mServerNChildren = 0;
    }
}

bool
StackDebugger::timedOut ()
{
    return mTimeoutRequired;
}

void
StackDebugger::removeServerWindow (Window id)
{
    /* Find the toplevel window in the list and remove it */
    for (CompWindowList::iterator it = mLastServerWindows.begin ();
	 it != mLastServerWindows.end ();
	 ++it)
    {
	if ((*it)->id () == id)
	{
	    mLastServerWindows.erase (it);
	    break;
	}
    }
}

void
StackDebugger::overrideRedirectRestack (Window toplevel, Window sibling)
{
    CompWindow *tl = screen->findWindow (toplevel);

    removeServerWindow (toplevel);

    /* Find the sibling of this window and insert above it or at
     * the bottom if the sibling is 0 */

    if (sibling)
    {
	for (CompWindowList::iterator it = mLastServerWindows.begin ();
	     it != mLastServerWindows.end ();
	     ++it)
	{
	    if (sibling == (*it)->id () ||
		sibling == (*it)->frame ())
	    {
		mLastServerWindows.insert (++it, tl);
		break;
	    }
	}
    }
    else
	mLastServerWindows.push_front (tl);
}

bool
StackDebugger::getNextEvent (XEvent &ev)
{
    if (mEvents.empty ())
	return false;

    ev = mEvents.front ();

    mEvents.pop_front ();

    return true;
}

void
StackDebugger::loadStack (CompWindowList &serverWindows, bool wait)
{
    Window rootRet, parentRet;

    if (mServerChildren)
	XFree (mServerChildren);

    XSync (mDpy, FALSE);
    XGrabServer (mDpy);
    XQueryTree (mDpy, mRoot, &rootRet, &parentRet,
		&mServerChildren, &mServerNChildren);

    unsigned int n = XEventsQueued (mDpy, QueuedAfterFlush);
    mEvents.clear ();
    mEvents.resize (n);
    std::list <XEvent>::iterator it = mEvents.begin ();

    while (it != mEvents.end ())
    {
	mFetchXEvent->getNextXEvent ((*it));
	++it;
    }

    XSync (mDpy, FALSE);

    /* It is possible that X might not be keeping up with us, so
     * we should give it about 300 ms in case the stacks are out of sync
     * in order to deliver any more events that might be pending */

    mTimeoutRequired = false;
    mLastServerWindows = serverWindows;

    if (mServerNChildren != serverWindows.size () && wait)
    {
	struct pollfd pfd;

	pfd.events = POLLIN;
	pfd.revents = 0;
	pfd.fd = ConnectionNumber (mDpy);

	poll (&pfd, 1, 300);

	XEvent e;

	while (mFetchXEvent->getNextXEvent (e))
	    mEvents.push_back (e);

	mTimeoutRequired = true;
    }

    mDestroyedFrames.clear ();

    XUngrabServer (mDpy);
    XSync (mDpy, FALSE);
}

void
StackDebugger::addDestroyedFrame (Window f)
{
    mDestroyedFrames.push_back (f);
}

void
StackDebugger::removeDestroyedFrame (Window f)
{
    mDestroyedFrames.remove (f);
}

bool
StackDebugger::cmpStack (CompWindowList &windows,
			 CompWindowList &serverWindows,
			 bool           verbose)
{
    std::vector <Window>             serverSideWindows;
    CompWindowList::reverse_iterator lrrit = windows.rbegin ();
    CompWindowList::reverse_iterator lsrit = mLastServerWindows.rbegin ();
    unsigned int                     i = 0;
    bool                             err = false;

    for (unsigned int n = 0; n < mServerNChildren; n++)
    {
	if (std::find (mDestroyedFrames.begin (),
		       mDestroyedFrames.end (), mServerChildren[n])
		== mDestroyedFrames.end ())
	    serverSideWindows.push_back (mServerChildren[n]);
    }

    if (verbose)
	compLogMessage ("core", CompLogLevelDebug, "sent       | recv       | server     |");

    for (;(lrrit != windows.rend () ||
	   lsrit != mLastServerWindows.rend () ||
	   i != serverSideWindows.size ());)
    {
	Window lrXid = 0;
	Window lsXid = 0;
	Window sXid = 0;

	if (lrrit != windows.rend ())
	    lrXid = (*lrrit)->priv->frame ? (*lrrit)->priv->frame : (*lrrit)->id ();

	if (lsrit != mLastServerWindows.rend ())
	    lsXid = (*lsrit)->priv->frame ? (*lsrit)->priv->frame : (*lsrit)->id ();

	if (i != serverSideWindows.size ())
	    sXid = serverSideWindows[serverSideWindows.size () - (i + 1)];

	if (verbose)
	    compLogMessage ("core", CompLogLevelDebug, "id 0x%x id 0x%x id 0x%x %s",
		     (unsigned int) lsXid, (unsigned int) lrXid,
		     (unsigned int) sXid, (lrXid != sXid) ? "  /!\\ " : "");

	if (lrXid != sXid)
	    err = true;

	if (lrrit != windows.rend ())
	    ++lrrit;

	if (lsrit != mLastServerWindows.rend())
	    ++lsrit;

	if (i != serverSideWindows.size ())
	    i++;
    }

    return err;
}

/* Checks the sanity of the list of windows last sent to the server.
 *
 * There are a few stacking "layers" here. From top to bottom:
 * - 1) Docks stacked above toplevel windows which are stacked
 *      above fullscreen windows
 * - 2) "Keep above" toplevel windows above fullscreen windows
 *      where a toplevel is in focus
 * - 3) Toplevel windows in focus above fullscreen windows
 * - 4) Fullscreen windows
 * - 5) Dock windows
 * - 6) Keep above windows
 * - 7) Toplevel windows
 * - 8) Docks which are marked "Keep Below"
 * - 9) "Keep Below" windows
 * - 10) Desktop windows
 *
 * There are also a few rules which apply here:
 * - 1) Dock windows should always be above normal windows
 *      except if marked keep below on any layer.
 * - 2) Dock windows should ONLY be on one layer at a time,
 *      eg if they are on layer 1 then there cannot
 *      also be dock windows on layer 5 (except in the
 *      case of below dock windows on layer 8)
 * - 3) Fullscreen windows must always be above docks when in
 *      focus, no matter if there is another window with "Keep Above"
 * - 4) Focused windows take priority over fullscreen windows and
 *      docks must always be above them (see rule 1)
 *
 * As we pass through each layer, this function flags each one from
 * lowest being the most bits set to highest being the least bits
 * set. If a window violates this it raises a warning */

#define DOCKS_ABOVE_TOPLEVELS_ABOVE_FULLSCREEN 0xffffffff >> 1
#define KEEP_ABOVE_TOPLEVELS_ABOVE_FULLSCREEN 0xffffffff >> 2
#define TOPLEVELS_ABOVE_FULLSCREEN 0xffffffff >> 3
#define FULLSCREEN 0xffffffff >> 4
#define DOCKS 0xffffffff >> 5
#define KEEP_ABOVE 0xffffffff >> 6
#define TOPLEVELS 0xffffffff >> 7
#define DOCKS_BELOW 0xffffffff >> 8
#define KEEP_BELOW 0xffffffff >> 9
#define DESKTOP 0xffffffff >> 10

namespace
{
    bool setCurrentLayer (Window requestingWindow, int layer, int &current)
    {
	bool ret = false;
	/* Only allow steps down */
	if ((current & layer) != layer)
	{
	    ret = true;
	    compLogMessage ("stackdebugger", CompLogLevelWarn, "0x%x requested invalid layer 0x%x",
			    requestingWindow, layer, current);
	}

	current = layer;

	return ret;
    }
}

bool
StackDebugger::checkSanity (CompWindowList &serverWindows, bool verbose)
{
    int current = 0xffffffff;
    int oldCurrent = current;
    bool err = false;

    if (verbose)
	compLogMessage ("stackdebugger", CompLogLevelDebug, "processing new stack --------");

    /* go backwards down the stack */
    for (CompWindowList::reverse_iterator rit = serverWindows.rbegin ();
	 rit != serverWindows.rend (); ++rit)
    {
	CompWindow *w = (*rit);

	/* Override redirect windows set all kinds
	 * of crazy stuff and are required to stack
	 * themselves so skip those */
	if (w->overrideRedirect ())
	    continue;

	/* ignore non-override redirect unmanaged windows */
	if (!w->managed ())
	    continue;

	/* ignore any windows that just got created */
	if (!w->mapNum ())
	    continue;

	/* determine the current layer */
	if (w->type () == CompWindowTypeDockMask)
	{
	    if ((current & DOCKS_ABOVE_TOPLEVELS_ABOVE_FULLSCREEN) ==
			   DOCKS_ABOVE_TOPLEVELS_ABOVE_FULLSCREEN)
	    {
		bool fullscreenWindow = false;

		/* search down the stack to check if there is a fullscreen
		 * window, otherwise we are not on the fullscreen layer */
		for (CompWindow *rw = w->prev; rw; rw = rw->prev)
		{
		    if (rw->type () & CompWindowTypeFullscreenMask)
		    {
			fullscreenWindow = true;
			break;
		    }
		}

		/* if there is no fullscreen window, change the layer */
		if (!fullscreenWindow)
		    err |= setCurrentLayer (w->id (), DOCKS, current);
		else
		    err |= setCurrentLayer (w->id (), DOCKS_ABOVE_TOPLEVELS_ABOVE_FULLSCREEN, current);
	    }
	    else if (w->state () & CompWindowStateBelowMask)
		err |= setCurrentLayer (w->id (), DOCKS_BELOW, current);
	    else
		err |= setCurrentLayer (w->id (), DOCKS, current);
	}
	else if (w->type () == CompWindowTypeFullscreenMask)
	{
	    err |= setCurrentLayer (w->id (), FULLSCREEN, current);
	}
	else if (w->type () == CompWindowTypeDesktopMask)
	{
	    err |= setCurrentLayer (w->id (), DESKTOP, current);
	}
	/* everything else that is not a fullscreen window or a desktop */
	else
	{
	    if (w->state () & CompWindowStateAboveMask)
	    {
		if ((current & KEEP_ABOVE_TOPLEVELS_ABOVE_FULLSCREEN) ==
			       KEEP_ABOVE_TOPLEVELS_ABOVE_FULLSCREEN)
		{
		    bool fullscreenWindow = false;

		    /* search down the stack to check if there is a fullscreen
		     * window, otherwise we are not on the fullscreen layer */
		    for (CompWindow *rw = w->prev; rw; rw = rw->prev)
		    {
			if (rw->type () & CompWindowTypeFullscreenMask)
			{
			    fullscreenWindow = true;
			    break;
			}
		    }

		    if (!fullscreenWindow)
			err |= setCurrentLayer (w->id (), KEEP_ABOVE, current);
		    else
			err |= setCurrentLayer (w->id (), KEEP_ABOVE_TOPLEVELS_ABOVE_FULLSCREEN, current);
		}
		else
		    err |= setCurrentLayer (w->id (), KEEP_ABOVE, current);
	    }
	    else if (w->state () & CompWindowStateBelowMask)
		err |= setCurrentLayer (w->id (), KEEP_BELOW, current);
	    else
	    {
		if ((current & TOPLEVELS_ABOVE_FULLSCREEN) ==
			       TOPLEVELS_ABOVE_FULLSCREEN)
		{
		    bool fullscreenWindow = false;

		    /* search down the stack to check if there is a fullscreen
		     * window, otherwise we are not on the fullscreen layer */
		    for (CompWindow *rw = w->prev; rw; rw = rw->prev)
		    {
			if (rw->type () & CompWindowTypeFullscreenMask)
			{
			    fullscreenWindow = true;
			    break;
			}
		    }

		    if (!fullscreenWindow)
			err |= setCurrentLayer (w->id (), TOPLEVELS, current);
		    else
			err |= setCurrentLayer (w->id (), TOPLEVELS_ABOVE_FULLSCREEN, current);
		}
		else
		    err |= setCurrentLayer (w->id (), TOPLEVELS, current);
	    }
	}

	if ((current & DOCKS_ABOVE_TOPLEVELS_ABOVE_FULLSCREEN) ==
		       DOCKS_ABOVE_TOPLEVELS_ABOVE_FULLSCREEN)
	{
	    if (verbose && current != oldCurrent)
		compLogMessage ("stackdebugger", CompLogLevelDebug, "on layer DOCKS_ABOVE_TOPLEVELS_ABOVE_FULLSCREEN");
	}
	else if ((current & KEEP_ABOVE_TOPLEVELS_ABOVE_FULLSCREEN) ==
			    KEEP_ABOVE_TOPLEVELS_ABOVE_FULLSCREEN)
	{
	    if (verbose && current != oldCurrent)
		compLogMessage ("stackdebugger", CompLogLevelDebug, "on layer KEEP_ABOVE_TOPLEVELS_ABOVE_FULLSCREEN");
	}
	else if ((current & TOPLEVELS_ABOVE_FULLSCREEN) == TOPLEVELS_ABOVE_FULLSCREEN)
	{
	    if (verbose && current != oldCurrent)
		compLogMessage ("stackdebugger", CompLogLevelDebug, "on layer TOPLEVELS_ABOVE_FULLSCREEN");
	}
	else if ((current & FULLSCREEN) == FULLSCREEN)
	{
	    if (verbose && current != oldCurrent)
		compLogMessage ("stackdebugger", CompLogLevelDebug, "on layer FULLSCREEN");
	}
	else if ((current & DOCKS) == DOCKS)
	{
	    if (verbose && current != oldCurrent)
		compLogMessage ("stackdebugger", CompLogLevelDebug, "on layer DOCKS");
	}
	else if ((current & KEEP_ABOVE) == KEEP_ABOVE)
	{
	    if (verbose && current != oldCurrent)
		compLogMessage ("stackdebugger", CompLogLevelDebug, "on layer KEEP_ABOVE");
	}
	else if ((current & TOPLEVELS) == TOPLEVELS)
	{
	    if (verbose && current != oldCurrent)
		compLogMessage ("stackdebugger", CompLogLevelDebug, "on layer TOPLEVELS");
	}
	else if ((current & DOCKS_BELOW) == DOCKS_BELOW)
	{
	    if (verbose && current != oldCurrent)
		compLogMessage ("stackdebugger", CompLogLevelDebug, "on layer DOCKS_BELOW");
	}
	else if ((current & KEEP_BELOW) == KEEP_BELOW)
	{
	    if (verbose && current != oldCurrent)
		compLogMessage ("stackdebugger", CompLogLevelDebug, "on layer KEEP_BELOW");
	}
	else if ((current & DESKTOP) == DESKTOP)
	{
	    if (verbose && current != oldCurrent)
		compLogMessage ("stackdebugger", CompLogLevelDebug, "on layer DESKTOP");
	}

	oldCurrent = current;
    }

    return err;
}
