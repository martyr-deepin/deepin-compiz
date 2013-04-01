/*
 * Compiz XOrg GTest, window stacking
 *
 * Copyright (C) 2012 Canonical Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Authored By:
 * Sam Spilsbury <sam.spilsbury@canonical.com>
 */
#include <list>
#include <string>
#include <stdexcept>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <xorg/gtest/xorg-gtest.h>
#include <compiz-xorg-gtest.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

using ::testing::MatchResultListener;
using ::testing::MakeMatcher;
using ::testing::Matcher;

namespace ct = compiz::testing;

class CompizXorgSystemStackingTest :
    public compiz::testing::XorgSystemTest
{
};

namespace
{
    const int          WINDOW_X = 0;
    const int          WINDOW_Y = 0;
    const unsigned int WINDOW_WIDTH = 640;
    const unsigned int WINDOW_HEIGHT = 480;
    const unsigned int WINDOW_BORDER = 0;
    const unsigned int WINDOW_DEPTH = CopyFromParent;
    const unsigned int WINDOW_CLASS = InputOutput;
    Visual             *WINDOW_VISUAL = CopyFromParent;


    const long                 WINDOW_ATTRIB_VALUE_MASK = 0;

    void MakeDock (Display *dpy, Window w)
    {
	Atom _NET_WM_WINDOW_TYPE = XInternAtom (dpy, "_NET_WM_WINDOW_TYPE", false);
	Atom _NET_WM_WINDOW_TYPE_DOCK = XInternAtom (dpy, "_NET_WM_WINDOW_TYPE_DOCK", false);

	XChangeProperty (dpy,
			 w,
			 _NET_WM_WINDOW_TYPE,
			 XA_ATOM,
			 32,
			 PropModeReplace,
			 reinterpret_cast <const unsigned char *> (&_NET_WM_WINDOW_TYPE_DOCK),
			 1);
    }

    Window CreateNormalWindow (Display *dpy)
    {
	XSetWindowAttributes WINDOW_ATTRIB;
	Window w = XCreateWindow (dpy,
				  DefaultRootWindow (dpy),
				  WINDOW_X,
				  WINDOW_Y,
				  WINDOW_WIDTH,
				  WINDOW_HEIGHT,
				  WINDOW_BORDER,
				  WINDOW_DEPTH,
				  WINDOW_CLASS,
				  WINDOW_VISUAL,
				  WINDOW_ATTRIB_VALUE_MASK,
				  &WINDOW_ATTRIB);

	XSelectInput (dpy, w, StructureNotifyMask);
	return w;
    }

    class PropertyNotifyXEventMatcher :
	public ct::XEventMatcher
    {
	public:

	    PropertyNotifyXEventMatcher (Display *dpy,
					 const std::string &propertyName) :
		mPropertyName (propertyName),
		mProperty (XInternAtom (dpy, propertyName.c_str (), false))
	    {
	    }

	    virtual bool MatchAndExplain (const XEvent &event, MatchResultListener *listener) const
	    {
		const XPropertyEvent *propertyEvent = reinterpret_cast <const XPropertyEvent *> (&event);

		if (mProperty == propertyEvent->atom)
		    return true;
		else
		    return false;
	    }

	    virtual void DescribeTo (std::ostream *os) const
	    {
		*os << "Is property identified by " << mPropertyName;
	    }

	    virtual void DescribeNegationTo (std::ostream *os) const
	    {
		*os << "Is not a property identified by" << mPropertyName;
	    }

	private:

	    std::string mPropertyName;
	    Atom	mProperty;
    };
}

TEST_F (CompizXorgSystemStackingTest, TestSetup)
{
}

TEST_F (CompizXorgSystemStackingTest, TestCreateWindowsAndRestackRelativeToEachOther)
{
    ::Display *dpy = Display ();

    Window w1 = CreateNormalWindow (dpy);
    Window w2 = CreateNormalWindow (dpy);

    XMapRaised (dpy, w1);
    XMapRaised (dpy, w2);

    /* Both reparented and both mapped */
    ASSERT_TRUE (ct::WaitForEventOfTypeOnWindow (dpy, w1, ReparentNotify, -1, -1));
    ASSERT_TRUE (ct::WaitForEventOfTypeOnWindow (dpy, w1, MapNotify, -1, -1));
    ASSERT_TRUE (ct::WaitForEventOfTypeOnWindow (dpy, w2, ReparentNotify, -1, -1));
    ASSERT_TRUE (ct::WaitForEventOfTypeOnWindow (dpy, w2, MapNotify, -1, -1));

    PropertyNotifyXEventMatcher matcher (dpy, "_NET_CLIENT_LIST_STACKING");

    /* Wait for property change notify on the root window to happen twice */
    ASSERT_TRUE (ct::WaitForEventOfTypeOnWindowMatching (dpy, DefaultRootWindow (dpy), PropertyNotify, -1, -1, matcher));
    ASSERT_TRUE (ct::WaitForEventOfTypeOnWindowMatching (dpy, DefaultRootWindow (dpy), PropertyNotify, -1, -1, matcher));

    /* Check the client list to see that w2 > w1 */
    std::list <Window> clientList = ct::NET_CLIENT_LIST_STACKING (dpy);

    ASSERT_EQ (clientList.size (), 2);
    EXPECT_EQ (clientList.front (), w1);
    EXPECT_EQ (clientList.back (), w2);
}

TEST_F (CompizXorgSystemStackingTest, TestCreateWindowsAndRestackRelativeToEachOtherDockAlwaysOnTop)
{
    ::Display *dpy = Display ();
    PropertyNotifyXEventMatcher matcher (dpy, "_NET_CLIENT_LIST_STACKING");

    Window dock = CreateNormalWindow (dpy);

    /* Make it a dock */
    MakeDock (dpy, dock);

    /* Immediately map the dock window and clear the event queue for it */
    XMapRaised (dpy, dock);

    ASSERT_TRUE (ct::WaitForEventOfTypeOnWindow (dpy, dock, ReparentNotify, -1, -1));
    ASSERT_TRUE (ct::WaitForEventOfTypeOnWindow (dpy, dock, MapNotify, -1, -1));

    /* Dock window needs to be in the client list */
    ASSERT_TRUE (ct::WaitForEventOfTypeOnWindowMatching (dpy, DefaultRootWindow (dpy), PropertyNotify, -1, -1, matcher));

    std::list <Window> clientList = ct::NET_CLIENT_LIST_STACKING (dpy);
    ASSERT_EQ (clientList.size (), 1);

    Window w1 = CreateNormalWindow (dpy);
    Window w2 = CreateNormalWindow (dpy);

    XSelectInput (dpy, w2, StructureNotifyMask);

    XMapRaised (dpy, w1);
    XMapRaised (dpy, w2);

    /* Both reparented and both mapped */
    ASSERT_TRUE (ct::WaitForEventOfTypeOnWindow (dpy, w1, ReparentNotify, -1, -1));
    ASSERT_TRUE (ct::WaitForEventOfTypeOnWindow (dpy, w1, MapNotify, -1, -1));
    ASSERT_TRUE (ct::WaitForEventOfTypeOnWindow (dpy, w2, ReparentNotify, -1, -1));
    ASSERT_TRUE (ct::WaitForEventOfTypeOnWindow (dpy, w2, MapNotify, -1, -1));

    /* Wait for property change notify on the root window to happen twice */
    ASSERT_TRUE (ct::WaitForEventOfTypeOnWindowMatching (dpy, DefaultRootWindow (dpy), PropertyNotify, -1, -1, matcher));
    ASSERT_TRUE (ct::WaitForEventOfTypeOnWindowMatching (dpy, DefaultRootWindow (dpy), PropertyNotify, -1, -1, matcher));

    clientList = ct::NET_CLIENT_LIST_STACKING (dpy);

    /* Check the client list to see that dock > w2 > w1 */
    ASSERT_EQ (clientList.size (), 3);

    std::list <Window>::iterator it = clientList.begin ();

    EXPECT_EQ (*it++, w1); /* first should be the bottom normal window */
    EXPECT_EQ (*it++, w2); /* second should be the top normal window */
    EXPECT_EQ (*it++, dock); /* dock must always be on top */
}
