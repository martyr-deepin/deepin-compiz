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

#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <core/screen.h>
#include <core/timer.h>

#include <fixx11h.h>

#include <QAbstractEventDispatcher>

class TimerObject;
class SocketObject;

class EventDispatcherCompiz : 
    public QAbstractEventDispatcher
{
    Q_OBJECT

    public:
	EventDispatcherCompiz (QObject *parent = 0);
	~EventDispatcherCompiz ();
	
	virtual void flush ();
	virtual bool hasPendingEvents ();
	virtual void interrupt ();
	virtual bool processEvents (QEventLoop::ProcessEventsFlags flags);
	virtual void registerSocketNotifier (QSocketNotifier *notifier);
	virtual void registerTimer (int timerId, int interval, QObject *object);
	virtual QList<QAbstractEventDispatcher::TimerInfo> registeredTimers (QObject *object) const;
	virtual void unregisterSocketNotifier (QSocketNotifier *notifier);
	virtual bool unregisterTimer (int timerId);
	virtual bool unregisterTimers (QObject *object);
	virtual void wakeUp ();
	
	void startingUp ();
	void closingDown ();
	
    private:
	void wakeUpEvent ();

	
    private:
	QList<TimerObject *>  mTimers;
	QList<SocketObject *> mSockets;
	
	QList<TimerObject *>  mDeleteTimers;
	QList<SocketObject *> mDeleteSockets;

	CompWatchFdHandle     mX11Handle;
	CompWatchFdHandle     mWakeUpHandle;

	QList<XEvent>         mQueuedEvents;
	
	int                   mWakeUpPipe[2];
	
	CompTimer             mEventTimer;
};

#endif
