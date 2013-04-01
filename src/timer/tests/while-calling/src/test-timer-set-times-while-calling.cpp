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

#include <pthread.h>

class CompTimerTestSetTimes: public CompTimerTest
{
protected:

    int mlastTimerTriggered;

    static void* run (void* cb)
    {
	if (cb == NULL)
	{
	    return NULL;
	}
	static_cast<CompTimerTestSetTimes*>(cb)->ml->run();
	return NULL;
    }

    pthread_t mmainLoopThread;
    std::list<int> mtriggeredTimers;

    void recordTimers ()
    {
	for (std::list<CompTimer *>::iterator it =
		TimeoutHandler::Default()->timers().begin();
        	it != TimeoutHandler::Default()->timers().end(); ++it)
	{
	    CompTimer *t = (*it);
	    RecordProperty("minLeft", t->minLeft());
	    RecordProperty("maxLeft", t->maxLeft());
	    RecordProperty("minTime", t->minTime());
	    RecordProperty("maxTime", t->maxTime());
	}
    }

    bool cb (int timernum, CompTimer* t1, CompTimer* t2, CompTimer* t3) {
	cb_(timernum,t1,t2,t3);
	return(true);
    }
    void cb_ (int timernum, CompTimer* t1, CompTimer* t2, CompTimer* t3)
    {
	recordTimers();
	if (mlastTimerTriggered == 0 && timernum == 1)
	{
	    /* Change the timeout time of the second timer to be after the third */
	    t2->setTimes(4000, 4100);

	    recordTimers();

	    /* Check if it is now at the back of the timeout list */
	    ASSERT_EQ( TimeoutHandler::Default()->timers().back(), t2 );
	}
	else if (mlastTimerTriggered == 1 && timernum == 2)
	{
	    recordTimers();
	    FAIL() << "timer with a higher timeout time got triggered "
		    "before a timer with a lower timeout time";
	}
	else if (mlastTimerTriggered == 2 && timernum != 1)
	{

	    recordTimers();
	    FAIL() << "timer with higher timeout time didn't get "
	       	    "triggered after a lower timeout time";
	}

	mlastTimerTriggered = timernum;
    }

    void SetUp ()
    {
	CompTimerTest::SetUp();
	mlastTimerTriggered = 0;
	CompTimer *t1, *t2, *t3;

	t1 = new CompTimer();
	t2 = new CompTimer();
	t3 = new CompTimer();

	timers.push_back(t1);
	timers.push_back(t2);
	timers.push_back(t3);

	t1->setCallback(
		boost::bind(&CompTimerTestSetTimes::cb, this, 1, t1, t2, t3));
	t1->setTimes(1000, 1100);
	t1->start();
	t2->setCallback(
		boost::bind(&CompTimerTestSetTimes::cb, this, 2, t1, t2, t3));
	t2->setTimes(2000, 2100);
	t2->start();
	t3->setCallback(
		boost::bind(&CompTimerTestSetTimes::cb, this, 3, t1, t2, t3));
	t3->setTimes(3000, 3100);
	t3->start();

	ASSERT_EQ(
		0,
		pthread_create(&mmainLoopThread, NULL, CompTimerTestSetTimes::run, this));
    }

    void TearDown ()
    {
	ml->quit();
	pthread_join(mmainLoopThread, NULL);

	CompTimerTest::TearDown();
    }
};

TEST_F(CompTimerTestSetTimes, SetTimesWhileCalling)
{
    ::sleep(4);
    // Just a dummy forcing instantiation of fixture.
}
