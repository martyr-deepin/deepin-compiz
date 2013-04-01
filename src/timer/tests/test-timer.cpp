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

CompTimerTest::CompTimerTest () :
	mc(Glib::MainContext::get_default()),
		ml(Glib::MainLoop::create(mc, false)),
		ts(CompTimeoutSource::create(mc)),
		lastTimerTriggered(0)
{
}

CompTimerTest::~CompTimerTest ()
{
    while (timers.size())
    {
	CompTimer *t = timers.front();

	timers.pop_front();
	delete t;
    }

    TimeoutHandler::SetDefault (NULL); 

    ts->destroy (); // calls delete this. Seriously.
}

class ForceGSliceMalloc
{
    public:

	ForceGSliceMalloc ()
	{
	    setenv ("G_SLICE", "always-malloc", 1);
	}

	~ForceGSliceMalloc ()
	{
	    unsetenv ("G_SLICE");
	}
};

namespace
{
    ForceGSliceMalloc fgm;
}

void CompTimerTest::SetUp ()
{
    TimeoutHandler *th = new TimeoutHandler();
    TimeoutHandler::SetDefault(th);
}
