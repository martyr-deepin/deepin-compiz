/*
 * Copyright © 2006 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include "inotify.h"

#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <sys/inotify.h>


COMPIZ_PLUGIN_20090315 (inotify, InotifyPluginVTable)

InotifyScreen::InotifyScreen (CompScreen *screen) :
    PluginClassHandler<InotifyScreen, CompScreen> (screen)
{
    fd = inotify_init ();

    fdHandle = screen->addWatchFd (fd,
				   POLLIN | POLLPRI | POLLHUP | POLLERR,
				   boost::bind (&InotifyScreen::processEvents,
						this));

    ScreenInterface::setHandler (screen, true);

    const CompFileWatchList           &watchList = screen->getFileWatches ();
    CompFileWatchList::const_iterator iter;

    for (iter = watchList.begin (); iter != watchList.end (); ++iter)
	fileWatchAdded (*iter);
}

InotifyScreen::~InotifyScreen ()
{
    const CompFileWatchList           &watchList = screen->getFileWatches ();
    CompFileWatchList::const_iterator iter;

    for (iter = watchList.begin (); iter != watchList.end (); ++iter)
	fileWatchRemoved (*iter);

    screen->removeWatchFd (fdHandle);

    close (fd);
}

void
InotifyScreen::processEvents ()
{
    char buf[256 * (sizeof (struct inotify_event) + 16)];
    int	 len;

    len = read (fd, buf, sizeof (buf));
    if (len < 0)
    {
	perror ("read");
    }
    else
    {
	struct inotify_event              *event;
	int		                  i = 0;
	WatchList::iterator               iter;
	const CompFileWatchList           &list = screen->getFileWatches ();
	CompFileWatchList::const_iterator wIter;

	while (i < len)
	{
	    event = (struct inotify_event *) &buf[i];

	    for (iter = watches.begin (); iter != watches.end (); ++iter)
		if ((*iter).wd == event->wd)
		    break;

	    if (iter != watches.end ())
	    {
		for (wIter = list.begin (); wIter != list.end (); ++iter)
		    if ((*iter).handle == (*wIter)->handle)
			break;

		if (wIter != list.end ())
		{
		    const char *name = (event->len) ? event->name : NULL;
		    (*wIter)->callBack (name);
		}
	    }

	    i += sizeof (*event) + event->len;
	}
    }
}

static unsigned int
inotifyMask (CompFileWatch *watch)
{
    unsigned int mask = 0;

    if (watch->mask & NOTIFY_CREATE_MASK)
	mask |= IN_CREATE;

    if (watch->mask & NOTIFY_DELETE_MASK)
	mask |= IN_DELETE;

    if (watch->mask & NOTIFY_MOVE_MASK)
	mask |= IN_MOVE;

    if (watch->mask & NOTIFY_MODIFY_MASK)
	mask |= IN_MODIFY;

    return mask;
}

void
InotifyScreen::fileWatchAdded (CompFileWatch *fileWatch)
{
    InotifyWatch iw;

    iw.handle = fileWatch->handle;
    iw.wd     = inotify_add_watch (fd, fileWatch->path.c_str (),
				   inotifyMask (fileWatch));
    if (iw.wd < 0)
    {
	perror ("inotify_add_watch");
	return;
    }

    watches.push_back (iw);
}

void
InotifyScreen::fileWatchRemoved (CompFileWatch *fileWatch)
{
    WatchList::iterator iter;

    for (iter = watches.begin (); iter != watches.end (); ++iter)
    {
	if ((*iter).handle == fileWatch->handle)
	{
	    if (inotify_rm_watch (fd, (*iter).wd))
		perror ("inotify_rm_watch");
	    watches.erase (iter);
	    break;
	}
    }
}

bool
InotifyPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION))
	return false;

    return true;
}

