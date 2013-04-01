/*
 * Copyright (c) 2010 Dennis Kasprzyk <onestone@compiz.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "socket.h"

#include <poll.h>

#include <fixx11h.h>

#include <QSocketNotifier>
#include <QApplication>

SocketObject::SocketObject (QSocketNotifier *notifier) :
    mNotifier (notifier)
{
    short int mask;
    switch (notifier->type()) {
	case QSocketNotifier::Read:
	    mask = POLLIN | POLLPRI | POLLHUP | POLLERR;
	    break;
	case QSocketNotifier::Write:
	    mask = POLLOUT;
	    break;
	case QSocketNotifier::Exception:
	    mask = 0;
	    break;
	default:
	    return;
    }
    mHandle = screen->addWatchFd (notifier->socket (), mask, 
				  boost::bind (&SocketObject::callback, this));
}

SocketObject::~SocketObject ()
{
    screen->removeWatchFd (mHandle);
}

QSocketNotifier *
SocketObject::notifier () const
{
    return mNotifier;
}

void
SocketObject::callback ()
{
    QEvent event(QEvent::SockAct);
    QApplication::sendEvent(mNotifier, &event);
}
