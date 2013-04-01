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

#include "timer.h"
#include "boost/bind.hpp"

#include <QApplication>
#include <QTimerEvent>

TimerObject::TimerObject (int timerId, int interval, QObject *object) :
    mTimerInfo (QAbstractEventDispatcher::TimerInfo (timerId, interval)),
    mObject (object)
{
    mTimer.start (boost::bind (&TimerObject::execute, this), interval);

    mEnabled = true;
}

TimerObject::~TimerObject ()
{
    mTimer.stop ();
}

QAbstractEventDispatcher::TimerInfo 
TimerObject::timerInfo () const
{
    return mTimerInfo;
}

QObject *
TimerObject::object () const
{
    return mObject;
}

bool
TimerObject::execute ()
{
    if (!mEnabled)
	return false;

    QTimerEvent event (mTimerInfo.first);
    QApplication::sendEvent (mObject, &event);
    return mEnabled;
}

void 
TimerObject::disable ()
{
    mEnabled = false;
    mTimer.stop();
};