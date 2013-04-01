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

#ifndef TIMER_H_
#define TIMER_H_

#include <core/timer.h>

#include <fixx11h.h>

#include <QPair>
#include <QAbstractEventDispatcher>


class TimerObject
{
    public:
	TimerObject (int timerId, int interval, QObject *object);
	~TimerObject ();
	
	QAbstractEventDispatcher::TimerInfo timerInfo () const;
	
	QObject *object () const;
	
	void disable ();
	
    private:
	bool execute ();
	
    private:
	QAbstractEventDispatcher::TimerInfo mTimerInfo;
	CompTimer                           mTimer;
	QObject                             *mObject;
	
	bool mEnabled;
};

#endif
