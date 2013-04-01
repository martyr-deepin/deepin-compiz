/*
 * Copyright (c) 2009 Dennis Kasprzyk <onestone@compiz.org>
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

#ifndef KDE_H_
#define KDE_H_

#include "dispatcher.h"
#include <core/screen.h>
#include <core/timer.h>
#include <core/pluginclasshandler.h>

#include <KApplication>

class KdeScreen :
    public PluginClassHandler <KdeScreen, CompScreen>
{
    public:

	KdeScreen (CompScreen *);
	virtual ~KdeScreen ();

	void sendGlibNotify ();

    private:
	KApplication          *mApp;
	EventDispatcherCompiz *mEventDispatcher;

	char        *argv[1];
	int         argc;
};

class KdePluginVTable :
    public CompPlugin::VTableForScreen <KdeScreen>
{
    public:

	bool init ();
};

#endif
