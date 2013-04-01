/*
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
 * Authored by: Jason Smith <jason.smith@canonical.com>
 * 	      : Sam Spilsbury <sam.spilsbury@canonical.com>
 */

#include "privateeventsource.h"
#include "core/screen.h"

CompEventSource *
CompEventSource::create ()
{
    return new CompEventSource ();
}

sigc::connection
CompEventSource::connect (const sigc::slot <bool> &slot)
{
    return connect_generic (slot);
}

CompEventSource::CompEventSource () :
    Glib::Source (),
    mDpy (screen->dpy ()),
    mConnectionFD (ConnectionNumber (screen->dpy ()))
{
    mPollFD.set_fd (mConnectionFD);
    mPollFD.set_events (Glib::IO_IN);

    set_priority (G_PRIORITY_DEFAULT);
    add_poll (mPollFD);
    set_can_recurse (true);

    connect (sigc::mem_fun <bool, CompEventSource> (this, &CompEventSource::callback));
}

CompEventSource::~CompEventSource ()
{
}

bool
CompEventSource::callback ()
{
    screen->processEvents ();
    return true;
}

bool
CompEventSource::prepare (int &timeout)
{
    timeout = -1;
    return XPending (mDpy);
}

bool
CompEventSource::check ()
{
    if (mPollFD.get_revents () & Glib::IO_IN)
	return XPending (mDpy);

    return false;
}

bool
CompEventSource::dispatch (sigc::slot_base *slot)
{
    return (*static_cast <sigc::slot <bool> *> (slot)) ();
}
