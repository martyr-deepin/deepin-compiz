/*
 * Copyright © 2012 Canonical, Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Canonical, Ltd. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Canonical, Ltd. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * CANONICAL LTD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL Canonical, Ltd. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Sam Spilsbury <sam.spilsbury@canonical.com>
 */

#include "privatesignalsource.h"

/* Add missing decls: https://bugzilla.gnome.org/show_bug.cgi?id=663880 */
#include <glib.h>
G_BEGIN_DECLS
#include <glib-unix.h>
G_END_DECLS

CompSignalSource *
CompSignalSource::create (int signum, const callbackFunc &f)
{
    return new CompSignalSource (signum, f);
}

CompSignalSource::CompSignalSource (int signum, const callbackFunc &f) :
    mFunc (f),
    mSignal (signum),
    mSource (g_unix_signal_add_full (G_PRIORITY_HIGH,
				     signum,
				     CompSignalSource::callback,
				     this,
				     CompSignalSource::destroyed))
{
}

gboolean
CompSignalSource::callback (gpointer user_data)
{
    CompSignalSource *self = static_cast <CompSignalSource *> (user_data);
    self->mFunc (self->mSignal);
    return TRUE;
}

void
CompSignalSource::destroyed (gpointer user_data)
{
    CompSignalSource *self = static_cast <CompSignalSource *> (user_data);
    self->mSource = 0;
}

CompSignalSource::~CompSignalSource ()
{
    if (mSource)
	g_source_remove (mSource);
}
