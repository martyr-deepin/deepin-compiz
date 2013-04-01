/*
 *
 * Compiz mouse position polling plugin
 *
 * Copyright : (C) 2008 by Dennis Kasprzyk
 * E-mail    : onestone@opencompositing.org
 *
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
 */

#ifndef _COMPIZ_MOUSEPOLL_H
#define _COMPIZ_MOUSEPOLL_H

#define COMPIZ_MOUSEPOLL_ABI 1

class MousePoller
{
    public:

	typedef boost::function<void (const CompPoint &)> CallBack;

	MousePoller ();
	~MousePoller ();

	void
	setCallback (CallBack callback);

	void
	start ();

	void
	stop ();

	bool
	active ();

	CompPoint
	getPosition ();

	static CompPoint
	getCurrentPosition ();

    private:

	bool 	  mActive;
	CompPoint mPoint;
	CallBack  mCallback;

    friend class MousepollScreen;
};

#endif
