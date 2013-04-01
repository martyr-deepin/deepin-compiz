/*
 * extrawm.h
 * Compiz extra WM actions plugins
 * Copyright: (C) 2007 Danny Baumann <maniac@beryl-project.org>
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

#include <X11/Xatom.h>

#include <core/core.h>
#include <core/pluginclasshandler.h>

#include <composite/composite.h>

#include "extrawm_options.h"

class ExtraWMScreen :
    public PluginClassHandler <ExtraWMScreen, CompScreen>,
    public ExtrawmOptions,
    public ScreenInterface
{
    public:

	std::list <CompWindow *> attentionWindows;

	ExtraWMScreen (CompScreen *);

	void
	handleEvent (XEvent *);

	void
	addAttentionWindow (CompWindow *w);

	void
	removeAttentionWindow (CompWindow *w);

	void
	updateAttentionWindow (CompWindow *w);

	void
	fullscreenWindow (CompWindow *w,
		  	  unsigned int state);

	static bool
	activateDemandsAttention (CompAction         *action,
				  CompAction::State  state,
				  CompOption::Vector &options);

	static bool
	activateWin (CompAction         *action,
		     CompAction::State  state,
		     CompOption::Vector &options);

	static bool
	toggleFullscreen (CompAction         *action,
			  CompAction::State  state,
			  CompOption::Vector &options);

	static bool
	toggleRedirect (CompAction         *action,
			CompAction::State  state,
			CompOption::Vector &options);

	static bool
	toggleAlwaysOnTop (CompAction         *action,
			   CompAction::State  state,
			   CompOption::Vector &options);

	static bool
	toggleSticky (CompAction         *action,
		      CompAction::State  state,
		      CompOption::Vector &options);
};

class ExtraWMWindow :
    public PluginClassHandler <ExtraWMWindow, CompWindow>,
    public WindowInterface
{
    public:

	ExtraWMWindow (CompWindow *);
	~ExtraWMWindow ();

	CompWindow *window;

	void
	stateChangeNotify (unsigned int);
};

#define EXTRAWM_SCREEN(s)						      \
    ExtraWMScreen *es = ExtraWMScreen::get (s);

#define EXTRAWM_WINDOW(w)						       \
    ExtraWMWindow *ew = ExtraWMWindow::get (w);

class ExtraWMPluginVTable :
    public CompPlugin::VTableForScreenAndWindow <ExtraWMScreen, ExtraWMWindow>
{
    public:

	bool init ();
};
