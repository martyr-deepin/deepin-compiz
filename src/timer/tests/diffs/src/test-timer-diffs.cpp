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

class CompTimerTestDiffs: public CompTimerTest
{
protected:

    static void* run (void* cb)
    {
	if (cb == NULL)
	{
	    return NULL;
	}
	static_cast<CompTimerTestDiffs*>(cb)->ml->run();
	return NULL;
    }

    pthread_t mmainLoopThread;
    std::list<int> mtriggeredTimers;

    bool cb (int timernum, CompTimer* t1, CompTimer* t2, CompTimer* t3)
    {
	if (timernum == 1 || timernum == 2 || timernum == 3)
	{
	    RecordProperty("executing timer", timernum);
	    RecordProperty("t1->minLeft", t1->minLeft());
	    RecordProperty("t1->maxLeft", t1->maxLeft());
	    RecordProperty("t1->minTime", t1->minTime());
	    RecordProperty("t1->maxTime", t1->maxTime());

	    RecordProperty("t3->minLeft", t3->minLeft());
	    RecordProperty("t3->maxLeft", t3->maxLeft());
	    RecordProperty("t3->minTime", t3->minTime());
	    RecordProperty("t3->maxTime", t3->maxTime());

	    RecordProperty("t3->minLeft", t3->minLeft());
	    RecordProperty("t3->maxLeft", t3->maxLeft());
	    RecordProperty("t3->minTime", t3->minTime());
	    RecordProperty("t3->maxTime", t3->maxTime());

	}

	return false;
    }

    void SetUp ()
    {
	CompTimerTest::SetUp();
	mtriggeredTimers.clear();

	CompTimer *t1, *t2, *t3;

	t1 = new CompTimer();
	t2 = new CompTimer();
	t3 = new CompTimer();

	timers.push_back(t1);
	timers.push_back(t2);
	timers.push_back(t3);

	t1->setCallback(
		boost::bind(&CompTimerTestDiffs::cb, this, 1, t1, t2, t3));
	t1->setTimes(1000, 1100);
	t1->start();
	t2->setCallback(
		boost::bind(&CompTimerTestDiffs::cb, this, 2, t1, t2, t3));
	t2->setTimes(2000, 2100);
	t2->start();
	t3->setCallback(
		boost::bind(&CompTimerTestDiffs::cb, this, 3, t1, t2, t3));
	t3->setTimes(3000, 3100);
	t3->start();

	ASSERT_EQ(
		0,
		pthread_create(&mmainLoopThread, NULL, CompTimerTestDiffs::run, this));

	::sleep(4);
    }

    void TearDown ()
    {
	ml->quit();
	pthread_join(mmainLoopThread, NULL);

	CompTimerTest::TearDown();
    }
};

TEST_F(CompTimerTestDiffs,TimerDiffs) {}

