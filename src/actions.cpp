/*
 * Copyright © 2008 Dennis Kasprzyk
 * Copyright © 2007 Novell, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Dennis Kasprzyk not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Dennis Kasprzyk makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DENNIS KASPRZYK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DENNIS KASPRZYK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Dennis Kasprzyk <onestone@compiz-fusion.org>
 *          David Reveman <davidr@novell.com>
 */

#include <core/screen.h>
#include <core/window.h>
#include <core/atoms.h>
#include "privatescreen.h"
#include "privatewindow.h"

bool
CompScreenImpl::closeWin (CompAction         *action,
		      CompAction::State  state,
		      CompOption::Vector &options)
{
    CompWindow   *w;
    Window       xid;
    unsigned int time;

    xid  = CompOption::getIntOptionNamed (options, "window");
    time = CompOption::getIntOptionNamed (options, "time", CurrentTime);

    w = screen->findTopLevelWindow (xid);
    if (w && (w->priv->actions  & CompWindowActionCloseMask))
	w->close (time);

    return true;
}

bool
CompScreenImpl::unmaximizeWin (CompAction         *action,
			   CompAction::State  state,
			   CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");

    w = screen->findTopLevelWindow (xid);
    if (w)
	w->maximize (0);

    return true;
}

bool
CompScreenImpl::minimizeWin (CompAction         *action,
			 CompAction::State  state,
			 CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");

    w = screen->findTopLevelWindow (xid);
    if (w && (w->actions () & CompWindowActionMinimizeMask))
	w->minimize ();

    return true;
}

bool
CompScreenImpl::maximizeWin (CompAction         *action,
			 CompAction::State  state,
			 CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");

    w = screen->findTopLevelWindow (xid);
    if (w)
	w->maximize (MAXIMIZE_STATE);

    return true;
}

bool
CompScreenImpl::maximizeWinHorizontally (CompAction         *action,
				     CompAction::State  state,
				     CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");

    w = screen->findTopLevelWindow (xid);
    if (w)
	w->maximize (w->state () | CompWindowStateMaximizedHorzMask);

    return true;
}

bool
CompScreenImpl::maximizeWinVertically (CompAction         *action,
				   CompAction::State  state,
				   CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");

    w = screen->findTopLevelWindow (xid);
    if (w)
	w->maximize (w->state () | CompWindowStateMaximizedVertMask);

    return true;
}

bool
CompScreenImpl::showDesktop (CompAction         *action,
			 CompAction::State  state,
			 CompOption::Vector &options)
{
    if (screen->showingDesktopMask() == 0)
	screen->enterShowDesktopMode ();
    else
	screen->leaveShowDesktopMode (NULL);

    return true;
}

bool
CompScreenImpl::raiseWin (CompAction         *action,
		      CompAction::State  state,
		      CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");

    w = screen->findTopLevelWindow (xid);
    if (w)
	w->raise ();

    return true;
}

bool
CompScreenImpl::lowerWin (CompAction         *action,
		      CompAction::State  state,
		      CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");

    w = screen->findTopLevelWindow (xid);
    if (w)
	w->lower ();

    return true;
}

bool
CompScreenImpl::windowMenu (CompAction         *action,
			CompAction::State  state,
			CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");

    w = screen->findTopLevelWindow (xid);
    if (w && screen->grabsEmpty ())
    {
	int  x, y, button;
	Time time;

	time   = CompOption::getIntOptionNamed (options, "time", CurrentTime);
	button = CompOption::getIntOptionNamed (options, "button", 0);
	x      = CompOption::getIntOptionNamed (options, "x",
						w->geometry ().x ());
	y      = CompOption::getIntOptionNamed (options, "y",
						w->geometry ().y ());

	screen->toolkitAction (Atoms::toolkitActionWindowMenu,
			       time, w->id (), button, x, y);
    }

    return true;
}

bool
CompScreenImpl::toggleWinMaximized (CompAction         *action,
				CompAction::State  state,
				CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");

    w = screen->findTopLevelWindow (xid);
    if (w)
    {
	if ((w->priv->state & MAXIMIZE_STATE) == MAXIMIZE_STATE)
	    w->maximize (0);
	else
	    w->maximize (MAXIMIZE_STATE);
    }

    return true;
}

bool
CompScreenImpl::toggleWinMaximizedHorizontally (CompAction         *action,
					    CompAction::State  state,
					    CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");

    w = screen->findTopLevelWindow (xid);
    if (w)
	w->maximize (w->priv->state ^ CompWindowStateMaximizedHorzMask);

    return true;
}

bool
CompScreenImpl::toggleWinMaximizedVertically (CompAction         *action,
					  CompAction::State  state,
					  CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");

    w = screen->findTopLevelWindow (xid);
    if (w)
	w->maximize (w->priv->state ^ CompWindowStateMaximizedVertMask);

    return true;
}

bool
CompScreenImpl::shadeWin (CompAction         *action,
		      CompAction::State  state,
		      CompOption::Vector &options)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window");

    w = screen->findTopLevelWindow (xid);
    if (w && (w->priv->actions & CompWindowActionShadeMask))
    {
	w->priv->state ^= CompWindowStateShadedMask;
	w->updateAttributes (CompStackingUpdateModeNone);
    }

    return true;
}
