/*
 * Copyright © 2008 Dennis Kasprzyk
 * Copyright © 2011 Canonical Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Dennis Kasprzyk not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Dennis Kasprzyk makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DENNIS KASPRZYK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DENNIS KASPRZYK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Dennis Kasprzyk <onestone@compiz-fusion.org>
 */

#include <boost/foreach.hpp>
#include <cmath>

#include "privatetimeoutsource.h"
#include "privatetimer.h"

#define foreach BOOST_FOREACH

CompTimeoutSource::CompTimeoutSource (Glib::RefPtr <Glib::MainContext> &ctx) :
    Glib::Source ()
{
    /*
     * Compiz MUST use priority G_PRIORITY_DEFAULT because that's the same
     * as GDK_PRIORITY_EVENTS.
     *
     * Any higher priority than that and you will starve the glib event loop
     * of X events. That causes stuttering and even complete starvation
     * can cause compiz to hang inside X11/GLX functions indefinitely.
     * In the best case, GLX functions would be slowed down by their messages
     * being prioritized lower than us.
     *
     * Any lower priority than that and screen redraws can lag behind
     * important things like dragging windows around.
     *
     * We have an "unfair" scheduling algorithm in the glib main event loop
     * to thank for all this. Ideally, compiz should be threaded so it never
     * slows down the handling of X11/GLX messages in the glib main loop, or
     * vice-versa.
     */
    set_priority (G_PRIORITY_DEFAULT);
    attach (ctx);

    /* We have to unreference the source so that it is destroyed
     * when the main context destroys it */
    unreference ();

    connect (sigc::mem_fun <bool, CompTimeoutSource> (this, &CompTimeoutSource::callback));
}

CompTimeoutSource::~CompTimeoutSource ()
{
}

sigc::connection
CompTimeoutSource::connect (const sigc::slot <bool> &slot)
{
    return connect_generic (slot);
}

CompTimeoutSource *
CompTimeoutSource::create (Glib::RefPtr <Glib::MainContext> &ctx)
{
    return new CompTimeoutSource (ctx);
}

static const unsigned short COMPIZ_TIMEOUT_WAIT = 15;

bool
CompTimeoutSource::prepare (int &timeout)
{
    /* Determine time to wait */

    if (TimeoutHandler::Default ()->timers ().empty ())
    {
	/* This kind of sucks, but we have to do it, considering
	 * that glib provides us no safe way to remove the source -
	 * thankfully we shouldn't ever be hitting this case since
	 * we create the source after we start pingTimer
	 * and that doesn't stop until compiz does
	 */

	timeout = COMPIZ_TIMEOUT_WAIT;

	return true;
    }

    if (TimeoutHandler::Default ()->timers ().front ()->minLeft () > 0)
    {
	std::list<CompTimer *>::iterator it = TimeoutHandler::Default ()->timers ().begin ();

	CompTimer *t = (*it);
	timeout = t->maxLeft ();
	while (it != TimeoutHandler::Default ()->timers ().end ())
	{
	    t = (*it);
	    if (t->minLeft () >= (unsigned int) timeout)
		break;
	    if (t->maxLeft () < (unsigned int) timeout)
		timeout = (int) t->maxLeft ();
	    ++it;
	}
    }
    else
    {
	timeout = 0;
    }

    return timeout <= 0;
}

bool
CompTimeoutSource::check ()
{
    return (!TimeoutHandler::Default ()->timers ().empty () &&
	     TimeoutHandler::Default ()->timers ().front ()->minLeft () <= 0);
}

bool
CompTimeoutSource::dispatch (sigc::slot_base *slot)
{
    (*static_cast <sigc::slot <bool> *> (slot)) ();
    return true;
}

bool
CompTimeoutSource::callback ()
{
    TimeoutHandler *handler = TimeoutHandler::Default ();
    std::list<CompTimer*> requeue, &timers = handler->timers ();

    while (!timers.empty ())
    {
	CompTimer *t = timers.front ();
	if (t->minLeft () > 0)
	    break;
	timers.pop_front ();
	t->setActive (false);
	if (t->triggerCallback ())
	    requeue.push_back (t);
    }

    std::list<CompTimer*>::const_iterator i = requeue.begin ();
    for (; i != requeue.end (); ++i)
    {
	CompTimer *t = *i;
	handler->addTimer (t);
	t->setActive (true);
    }

    return !timers.empty ();
}

PrivateTimer::PrivateTimer () :
    mActive (false),
    mMinTime (0),
    mMaxTime (0),
    mMinDeadline (0),
    mMaxDeadline (0),
    mCallBack (NULL)
{
}

PrivateTimer::~PrivateTimer ()
{
}

CompTimer::CompTimer () :
    priv (new PrivateTimer ())
{
    assert (priv);
}

CompTimer::~CompTimer ()
{
    TimeoutHandler::Default ()->removeTimer (this);
    delete priv;
}

void
CompTimer::setTimes (unsigned int min, unsigned int max)
{
    bool wasActive = priv->mActive;
    if (priv->mActive)
	stop ();
    priv->mMinTime = min;
    priv->mMaxTime = (min <= max)? max : min;

    if (wasActive)
	start ();
}

void
CompTimer::setExpiryTimes (unsigned int min, unsigned int max)
{
    gint64 now = g_get_monotonic_time ();
    priv->mMinDeadline = now + (static_cast <gint64> (min) * 1000);
    priv->mMaxDeadline = now + (static_cast <gint64> (max >= min ? max : min) * 1000);
}

void
CompTimer::decrement (unsigned int diff)
{
    /* deprecated */
}

void
CompTimer::setActive (bool active)
{
    priv->mActive = active;
}

bool
CompTimer::triggerCallback ()
{
    return priv->mCallBack ();
}

void
CompTimer::setCallback (CompTimer::CallBack callback)
{
    bool wasActive = priv->mActive;
    if (priv->mActive)
	stop ();

    priv->mCallBack = callback;

    if (wasActive)
	start ();
}


void
CompTimer::start ()
{
    stop ();

    if (priv->mCallBack.empty ())
    {
/* FIXME: compLogMessage needs to be testable */
#if 0
	compLogMessage ("core", CompLogLevelWarn,
			"Attempted to start timer without callback.");
#endif
	return;
    }

    priv->mActive = true;
    TimeoutHandler::Default ()->addTimer (this);
}

void
CompTimer::start (unsigned int min, unsigned int max)
{
    setTimes (min, max);
    start ();
}

void
CompTimer::start (CompTimer::CallBack callback,
		  unsigned int min, unsigned int max)
{
    setTimes (min, max);
    setCallback (callback);
    start ();
}

void
CompTimer::stop ()
{
    priv->mActive = false;
    TimeoutHandler::Default ()->removeTimer (this);
}

unsigned int
CompTimer::minTime ()
{
    return priv->mMinTime;
}

unsigned int
CompTimer::maxTime ()
{
    return priv->mMaxTime;
}

unsigned int
CompTimer::minLeft ()
{
    gint64 now = g_get_monotonic_time ();
    return (priv->mMinDeadline > now) ?
	(unsigned int)(priv->mMinDeadline - now + 500) / 1000 : 0;
}

unsigned int
CompTimer::maxLeft ()
{
    gint64 now = g_get_monotonic_time ();
    return (priv->mMaxDeadline > now) ?
	(unsigned int)(priv->mMaxDeadline - now + 500) / 1000 : 0;
}

bool
CompTimer::active ()
{
    return priv->mActive;
}
