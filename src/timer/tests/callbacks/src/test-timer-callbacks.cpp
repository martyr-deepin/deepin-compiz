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

#include "test-timer.h"
#include <ctime>
#include <boost/noncopyable.hpp>

using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::_;
using ::testing::AtLeast;

namespace
{

class CompTimerTestCallbackDispatchTable
{
public:

    CompTimerTestCallbackDispatchTable () {};
    virtual ~CompTimerTestCallbackDispatchTable () {};

    virtual bool callback1 (unsigned int num) = 0;
    virtual bool callback2 (unsigned int num) = 0;
    virtual bool callback3 (unsigned int num) = 0;
protected:

    
};

class MockCompTimerTestCallbackDispatchTable :
    public CompTimerTestCallbackDispatchTable,
    boost::noncopyable
{
public:

    MOCK_METHOD1 (callback1, bool (unsigned int));
    MOCK_METHOD1 (callback2, bool (unsigned int));
    MOCK_METHOD1 (callback3, bool (unsigned int));

    MockCompTimerTestCallbackDispatchTable (const Glib::RefPtr <Glib::MainLoop> &ml) :
	CompTimerTestCallbackDispatchTable (),
	mMainLoop (ml)
    {
	memset (&mCallsCounter, 0, sizeof (mCallsCounter));
	ON_CALL (*this, callback1 (_)).WillByDefault (Invoke (this, &MockCompTimerTestCallbackDispatchTable::QuitIfLast));
	ON_CALL (*this, callback2 (_)).WillByDefault (Invoke (this, &MockCompTimerTestCallbackDispatchTable::QuitIfLast));
	ON_CALL (*this, callback3 (_)).WillByDefault (Invoke (this, &MockCompTimerTestCallbackDispatchTable::QuitIfLast));
    };

    void setMax (unsigned int timerId, int maxCalls)
    {
	mCallsCounter[timerId].maxCalls = maxCalls;
    }

private:
    Glib::RefPtr <Glib::MainLoop> mMainLoop;

    class _counter
    {
	public:
	    unsigned int calls;
	    int maxCalls;
    } mCallsCounter[3];

    bool QuitIfLast (unsigned int num)
    {
	mCallsCounter[num].calls++;

	if (mCallsCounter[num].maxCalls < 0 ||
	    static_cast <unsigned int> (mCallsCounter[num].maxCalls) == mCallsCounter[num].calls)
	{
	    /* We are the last timer, quit the main loop */
	    if (TimeoutHandler::Default ()->timers ().size () == 0)
	    {
		mMainLoop->quit ();
		return false;
	    }
	    else if (mCallsCounter[num].maxCalls)
		return false;
	}

	return true;
    };
};

class CompTimerTestCallback: public CompTimerTest
{
public:
    CompTimerTestCallback () :
	mLastAdded (0),
	mDispatchTable (new MockCompTimerTestCallbackDispatchTable (ml))
    {
    }

    ~CompTimerTestCallback ()
    {
	delete mDispatchTable;
    }
protected:

    unsigned int    mLastAdded;
    MockCompTimerTestCallbackDispatchTable *mDispatchTable;

    void AddTimer (unsigned int min,
		   unsigned int max,
		   const boost::function <bool ()> &callback,
		   int maxAllowedCalls)
    {
	timers.push_back (new CompTimer ());
	timers.back ()->setTimes (min, max);
	timers.back ()->setCallback (callback);

	ASSERT_FALSE (callback.empty ());

	/* TimeoutHandler::timers should be empty */
	EXPECT_TRUE (TimeoutHandler::Default ()->timers ().empty ()) << "timers list is not empty";

	mDispatchTable->setMax (mLastAdded, maxAllowedCalls);

	mLastAdded++;
    }

    void Run ()
    {
	for (std::deque <CompTimer *>::iterator it = timers.begin ();
	     it != timers.end (); ++it)
	    (*it)->start ();

	/* TimeoutHandler::timers should have the timer that
	 * is going to trigger first at the front of the
	 * list and the last timer at the back */
	if (TimeoutHandler::Default ()->timers ().front () != timers.back ())
	{
	    RecordProperty ("TimeoutHandler::Default ().size",
		    TimeoutHandler::Default ()->timers ().size ());
	    RecordProperty ("TimeoutHandler::Default ().front->minLeft",
		    TimeoutHandler::Default ()->timers ().front ()->minLeft());
	    RecordProperty ("TimeoutHandler::Default ().front->maxLeft",
		    TimeoutHandler::Default ()->timers ().front ()->maxLeft());
	    RecordProperty ("TimeoutHandler::Default ().front->minTime",
		    TimeoutHandler::Default ()->timers ().front ()->minTime());
	    RecordProperty ("TimeoutHandler::Default ().front->maxTime",
		    TimeoutHandler::Default ()->timers ().front ()->maxTime());
	    RecordProperty ("TimeoutHandler::Default ().back->minLeft",
		    TimeoutHandler::Default ()->timers ().back ()->minLeft());
	    RecordProperty ("TimeoutHandler::Default ().back->maxLeft",
		    TimeoutHandler::Default ()->timers ().back ()->maxLeft());
	    RecordProperty ("TimeoutHandler::Default ().back->minTime",
		    TimeoutHandler::Default ()->timers ().back ()->minTime());
	    RecordProperty ("TimeoutHandler::Default ().back->maxTime",
		    TimeoutHandler::Default ()->timers ().back ()->maxTime());
	    FAIL () << "timer with the least time is not at the front";
	}

	if (TimeoutHandler::Default ()->timers ().back () != timers.front ())
	{
	    FAIL () << "timer with the most time is not at the back";
	}

	ml->run ();
    }

    void SetUp ()
    {
	CompTimerTest::SetUp ();

	::sleep (1);
    }

    void TearDown ()
    {
	CompTimerTest::TearDown ();
    }
};

TEST_F(CompTimerTestCallback, TimerOrder)
{
    AddTimer (1000, 1100, boost::bind (&MockCompTimerTestCallbackDispatchTable::callback1, mDispatchTable, 0), 3);
    AddTimer (500, 900, boost::bind (&MockCompTimerTestCallbackDispatchTable::callback2, mDispatchTable, 1), 6);
    AddTimer (0, 0, boost::bind (&MockCompTimerTestCallbackDispatchTable::callback3, mDispatchTable, 2), 10);

    /* TimeoutHandler::timers should be empty since no timers have started */
    ASSERT_TRUE (TimeoutHandler::Default ()->timers ().empty ()) << "timers list is not empty";

    InSequence s;

    EXPECT_CALL (*mDispatchTable, callback3 (2)).Times (10);
    EXPECT_CALL (*mDispatchTable, callback2 (1)).Times (1);
    EXPECT_CALL (*mDispatchTable, callback1 (0)).Times (1);
    EXPECT_CALL (*mDispatchTable, callback2 (1)).Times (1);
    EXPECT_CALL (*mDispatchTable, callback1 (0)).Times (1);
    EXPECT_CALL (*mDispatchTable, callback2 (1)).Times (1);
    EXPECT_CALL (*mDispatchTable, callback1 (0)).Times (1);
    EXPECT_CALL (*mDispatchTable, callback2 (1)).Times (1);
    EXPECT_CALL (*mDispatchTable, callback2 (1)).Times (1);
    EXPECT_CALL (*mDispatchTable, callback2 (1)).Times (1);

    Run ();
}

TEST_F(CompTimerTestCallback, NoZeroStarvation)
{
    AddTimer (100, 110, boost::bind (&MockCompTimerTestCallbackDispatchTable::callback1, mDispatchTable, 0), 1);
    AddTimer (50, 90, boost::bind (&MockCompTimerTestCallbackDispatchTable::callback2, mDispatchTable, 1), 1);
    AddTimer (0, 0, boost::bind (&MockCompTimerTestCallbackDispatchTable::callback3, mDispatchTable, 2), -1);

    EXPECT_CALL (*mDispatchTable, callback3 (2)).Times (AtLeast (1));
    EXPECT_CALL (*mDispatchTable, callback2 (1)).Times (1);
    EXPECT_CALL (*mDispatchTable, callback1 (0)).Times (1);

    Run ();
}

}
