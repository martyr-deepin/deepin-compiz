/*
 * winrules plugin for compiz
 *
 * Copyright (C) 2007 Bellegarde Cedric (gnumdk (at) gmail.com)
 * Copyright (C) 2009 Sam Spilsbury (smspillaz@gmail.com)
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
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#include <core/core.h>
#include <core/pluginclasshandler.h>
#include <core/timer.h>
#include <core/atoms.h>
#include <X11/Xatom.h>
#include "winrules_options.h"


class WinrulesScreen :
    public PluginClassHandler <WinrulesScreen, CompScreen>,
    public WinrulesOptions,
    public ScreenInterface
{
    public:

	WinrulesScreen (CompScreen *screen);

	void
	handleEvent (XEvent *event);

	void
	matchExpHandlerChanged ();

	void
	matchPropertyChanged (CompWindow *w);

	void
	setProtocols (unsigned int protocols,
		      Window       id);

	void
	optionChanged (CompOption	        *option,
		       WinrulesOptions::Options num);	
};

#define WINRULES_SCREEN(screen)					       \
    WinrulesScreen *ws = WinrulesScreen::get(screen);

class WinrulesWindow :
    public PluginClassHandler <WinrulesWindow, CompWindow>,
    public WindowInterface
{
    public:

	WinrulesWindow (CompWindow *window);

	CompWindow *window;

	void
	getAllowedActions (unsigned int &,
			   unsigned int &);

	bool is ();

	void setNoFocus (int optNum);

	void setNoAlpha (int optNum);

	void
	updateState (int optNum,
		     int mask);

	void
	setAllowedActions (int        optNum,
			   int        action);

	bool
	matchSizeValue (CompOption::Value::Vector matches,
			CompOption::Value::Vector widthValues,
			CompOption::Value::Vector heightValues,
			int	   		  *width,
			int	   		  *height);

	bool
	matchSize (int	      *width,
		   int	      *height);

	void
	updateWindowSize (int        width,
			  int        height);

	bool applyRules ();

	bool alpha ();
	bool isFocussable ();
	bool focus ();

	unsigned int allowedActions;
	unsigned int stateSetMask;
	unsigned int protocolSetMask;
};

#define WINRULES_WINDOW(window)					       \
    WinrulesWindow *ww = WinrulesWindow::get(window);

class WinrulesPluginVTable:
    public CompPlugin::VTableForScreenAndWindow <WinrulesScreen, WinrulesWindow>
{
    public:

	bool init ();
};
