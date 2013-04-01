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

/* FIXME: Define CompWindowList somewhere else
 * so that this can be come a truly standalone
 * object */
#include <core/core.h>
#include <list>

#ifndef _COMPIZ_PRIVATESTACKDEBUGGER_H
#define _COMPIZ_PRIVATESTACKDEBUGGER_H

class FetchXEventInterface;

class StackDebugger
{
    public:

	typedef std::vector<XEvent> eventList;

	StackDebugger (Display *, Window, FetchXEventInterface *fetchXEvent);
	~StackDebugger ();

	void loadStack (CompWindowList &serverWindows, bool wait = false);
	void windowsChanged (bool change) { mWindowsChanged = change; };
	void serverWindowsChanged (bool change) { mServerWindowsChanged = change; };
	bool windowsChanged () { return mWindowsChanged; }
	bool serverWindowsChanged () { return mServerWindowsChanged; }
	void overrideRedirectRestack (Window toplevel, Window sibling);
	void removeServerWindow (Window);
	void addDestroyedFrame (Window);
	void removeDestroyedFrame (Window);
	bool stackChange ();
	bool cmpStack (CompWindowList &windows,
		       CompWindowList &serverWindows,
		       bool verbose = false);
	bool timedOut ();
	bool getNextEvent (XEvent &);

	bool checkSanity (CompWindowList &serverWindows, bool verbose = false);

	static StackDebugger * Default ();
	static void SetDefault (StackDebugger *);

    private:

	std::list <Window> mDestroyedFrames;

	unsigned int mServerNChildren;
	Window       *mServerChildren;
	bool         mWindowsChanged;
	bool         mServerWindowsChanged;
	Window       mRoot;
	Display      *mDpy;
	FetchXEventInterface *mFetchXEvent;
	bool         mTimeoutRequired;
	CompWindowList mLastServerWindows;
	std::list <XEvent> mEvents;
};

#endif
