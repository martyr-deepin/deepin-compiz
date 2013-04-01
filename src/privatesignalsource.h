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

#include <boost/function.hpp>

#include <glib.h>

class CompSignalSource
{
    public:

	~CompSignalSource ();

	typedef boost::function <void (int)> callbackFunc;

	static
	CompSignalSource *
	create (int signum,
		const callbackFunc &);

    protected:

	explicit CompSignalSource (int signum, const callbackFunc &);

    private:

	static gboolean callback (gpointer user_data);
	static void     destroyed (gpointer user_data);

	callbackFunc mFunc;
	int          mSignal;
	gint         mSource;
};
