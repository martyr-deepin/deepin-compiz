/*
 * Copyright © 2008 Dennis Kasprzyk
 * Copyright © 2010 Canonical Ltd.
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

#include "privatetimeouthandler.h"
#include "core/timer.h"

#include <boost/scoped_ptr.hpp>
#include <algorithm>

namespace
{
  static boost::scoped_ptr<TimeoutHandler> gDefault;
}

TimeoutHandler::TimeoutHandler () :
    priv (new PrivateTimeoutHandler ())
{
}

TimeoutHandler::~TimeoutHandler ()
{
    delete priv;
}

void
TimeoutHandler::addTimer (CompTimer *timer)
{
    std::list<CompTimer *>::iterator it;

    it = std::find (priv->mTimers.begin (), priv->mTimers.end (), timer);

    if (it != priv->mTimers.end ())
	return;

    for (it = priv->mTimers.begin (); it != priv->mTimers.end (); ++it)
    {
	if (timer->minTime () < (*it)->minLeft ())
	    break;
    }

    timer->setExpiryTimes (timer->minTime (), timer->maxTime ());

    priv->mTimers.insert (it, timer);
}

void
TimeoutHandler::removeTimer (CompTimer *timer)
{
    std::list<CompTimer *>::iterator it;

    it = std::find (priv->mTimers.begin (), priv->mTimers.end (), timer);

    if (it == priv->mTimers.end ())
	return;

    priv->mTimers.erase (it);
}

std::list <CompTimer *> &
TimeoutHandler::timers ()
{
    return priv->mTimers;
}

TimeoutHandler *
TimeoutHandler::Default ()
{
    return gDefault.get();
}

void
TimeoutHandler::SetDefault (TimeoutHandler *instance)
{
    gDefault.reset(instance);
}
