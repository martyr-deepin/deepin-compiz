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

#include <core/atoms.h>
#include <poll.h>

#include "dispatcher.h"
#include "timer.h"
#include "socket.h"


#include <QApplication>
#include <QX11Info>

#include "dispatcher.moc"

EventDispatcherCompiz::EventDispatcherCompiz (QObject *)
{
    mWakeUpPipe[0] = 0;
    mWakeUpPipe[1] = 0;

    mEventTimer.start (boost::bind (&EventDispatcherCompiz::processEvents, this, QEventLoop::AllEvents),
		       1, MAXSHORT);
}

EventDispatcherCompiz::~EventDispatcherCompiz ()
{
    foreach (TimerObject *timer, mTimers)
	delete timer;
    foreach (SocketObject *socket, mSockets)
	delete socket;
}
	
void 
EventDispatcherCompiz::flush ()
{
   XFlush (QX11Info::display());
}

bool 
EventDispatcherCompiz::hasPendingEvents ()
{
    extern Q_CORE_EXPORT uint qGlobalPostedEventsCount();
    return qGlobalPostedEventsCount() != 0;
}

void 
EventDispatcherCompiz::interrupt ()
{
    wakeUp ();
}

bool 
EventDispatcherCompiz::processEvents (QEventLoop::ProcessEventsFlags flags)
{
    emit awake ();

    Display *dpy = QX11Info::display();

    while (!(flags & QEventLoop::ExcludeUserInputEvents)
	    && !mQueuedEvents.isEmpty()) 
    {
	// process a pending user input event
	XEvent event = mQueuedEvents.takeFirst();
	// send through event filter
	if (filterEvent(&event))
	    continue;
	if (qApp->x11ProcessEvent(&event) == 1)
	    return true;
    }

    while (XPending (dpy)) 
    {
	XEvent event;

	// process events from the X server
	XNextEvent(dpy, &event);

	if (flags & QEventLoop::ExcludeUserInputEvents) {
	    // queue user input events
	    switch (event.type) {
		case ButtonPress:
		case ButtonRelease:
		case MotionNotify:
		case XKeyPress:
		case XKeyRelease:
		case EnterNotify:
		case LeaveNotify:
		    mQueuedEvents.append(event);
		    continue;

		case ClientMessage:
		    // only keep the wm_take_focus
		    // client messages
		    if (event.xclient.format == 32) {
			if (event.xclient.message_type == Atoms::wmProtocols ||
			    (Atom) event.xclient.data.l[0] == Atoms::wmTakeFocus) {
			    break;
			}
		    }
		    mQueuedEvents.append(event);
		    continue;

		default:
		    break;
	    }
	}
	
	// send through event filter
	if (filterEvent(&event))
	    continue;
	if (qApp->x11ProcessEvent(&event) == 1)
	    return true;
    }

    QApplication::sendPostedEvents();

    while (!mDeleteTimers.isEmpty ())
	delete mDeleteTimers.takeFirst ();
    while (!mDeleteSockets.isEmpty ())
	delete mDeleteSockets.takeFirst ();

    return true;
}

void
EventDispatcherCompiz::registerSocketNotifier (QSocketNotifier *notifier)
{
    mSockets.append (new SocketObject (notifier));
}

void 
EventDispatcherCompiz::registerTimer (int timerId, int interval, QObject *object)
{
    TimerObject *timer = new TimerObject (timerId, interval, object);
    mTimers.append (timer);
}

QList<QAbstractEventDispatcher::TimerInfo> 
EventDispatcherCompiz::registeredTimers (QObject *object) const
{
    QList<QAbstractEventDispatcher::TimerInfo> list;
    foreach (TimerObject *timer, mTimers)
	if (timer->object () == object)
	{
	    list.append (timer->timerInfo ());
	}
    return list;
}

void 
EventDispatcherCompiz::unregisterSocketNotifier (QSocketNotifier *notifier)
{
    SocketObject *sock = NULL;
    foreach (SocketObject *socket, mSockets)
	if (socket->notifier () == notifier)
	{
	    sock = socket;
	    break;
	}
	
    if (sock)
    {
	mSockets.removeAll (sock);
	mDeleteSockets.append (sock);
    }
}

bool 
EventDispatcherCompiz::unregisterTimer (int timerId)
{
    TimerObject *time = NULL;

    foreach (TimerObject *timer, mTimers)
	if (timer->timerInfo ().first == timerId)
	{
	    time = timer;
	    break;
	}

    if (time)
    {
	mTimers.removeAll (time);
	time->disable ();
	mDeleteTimers.append (time);
	return true;
    }
    return false;
}

bool 
EventDispatcherCompiz::unregisterTimers (QObject *object)
{
    QList<TimerObject *> list;
    foreach (TimerObject *timer, mTimers)
	if (timer->object () == object)
	{
	    list.append (timer);
	}

    if (!list.isEmpty ())
    {
	foreach (TimerObject *timer, list)
	{
	    mTimers.removeAll (timer);
	    timer->disable ();
	    mDeleteTimers.append (timer);
	}
	return true;
    }
    return false;
}

void 
EventDispatcherCompiz::wakeUp ()
{
    if (mWakeUpPipe[1])
	if (write (mWakeUpPipe[1], "w", 1) <= 0)
	    return;
}

void 
EventDispatcherCompiz::startingUp ()
{
    if (pipe (mWakeUpPipe) < 0)
	return;
    mWakeUpHandle = screen->addWatchFd (mWakeUpPipe [0],POLLIN,
				        boost::bind (&EventDispatcherCompiz::wakeUpEvent, this));
    mX11Handle = screen->addWatchFd (ConnectionNumber (QX11Info::display()),
				     POLLIN | POLLHUP | POLLERR, NULL);
}

void 
EventDispatcherCompiz::closingDown ()
{
    screen->removeWatchFd (mX11Handle);
    screen->removeWatchFd (mWakeUpPipe[0]);
}



void 
EventDispatcherCompiz::wakeUpEvent ()
{
    char buf[256];
    if (read (mWakeUpPipe[0], buf, 256) <= 0)
	return;
}
