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

#ifndef _ATOMS_H
#define _ATOMS_H

#include <X11/Xlib-xcb.h>

namespace Atoms {
    extern Atom supported;
    extern Atom supportingWmCheck;

    extern Atom utf8String;

    extern Atom wmName;

    extern Atom winType;
    extern Atom winTypeDesktop;
    extern Atom winTypeDock;
    extern Atom winTypeToolbar;
    extern Atom winTypeMenu;
    extern Atom winTypeUtil;
    extern Atom winTypeSplash;
    extern Atom winTypeDialog;
    extern Atom winTypeNormal;
    extern Atom winTypeDropdownMenu;
    extern Atom winTypePopupMenu;
    extern Atom winTypeTooltip;
    extern Atom winTypeNotification;
    extern Atom winTypeCombo;
    extern Atom winTypeDnd;

    extern Atom winOpacity;
    extern Atom winBrightness;
    extern Atom winSaturation;
    extern Atom winActive;
    extern Atom winDesktop;

    extern Atom workarea;

    extern Atom desktopViewport;
    extern Atom desktopGeometry;
    extern Atom currentDesktop;
    extern Atom numberOfDesktops;

    extern Atom winState;
    extern Atom winStateModal;
    extern Atom winStateSticky;
    extern Atom winStateMaximizedVert;
    extern Atom winStateMaximizedHorz;
    extern Atom winStateShaded;
    extern Atom winStateSkipTaskbar;
    extern Atom winStateSkipPager;
    extern Atom winStateHidden;
    extern Atom winStateFullscreen;
    extern Atom winStateAbove;
    extern Atom winStateBelow;
    extern Atom winStateDemandsAttention;
    extern Atom winStateDisplayModal;
    extern Atom winStateFocused;

    extern Atom winActionMove;
    extern Atom winActionResize;
    extern Atom winActionStick;
    extern Atom winActionMinimize;
    extern Atom winActionMaximizeHorz;
    extern Atom winActionMaximizeVert;
    extern Atom winActionFullscreen;
    extern Atom winActionClose;
    extern Atom winActionShade;
    extern Atom winActionChangeDesktop;
    extern Atom winActionAbove;
    extern Atom winActionBelow;

    extern Atom wmAllowedActions;

    extern Atom wmStrut;
    extern Atom wmStrutPartial;

    extern Atom wmUserTime;

    extern Atom wmIcon;
    extern Atom wmIconGeometry;

    extern Atom clientList;
    extern Atom clientListStacking;

    extern Atom frameExtents;
    extern Atom frameWindow;

    extern Atom wmState;
    extern Atom wmChangeState;
    extern Atom wmProtocols;
    extern Atom wmClientLeader;

    extern Atom wmDeleteWindow;
    extern Atom wmTakeFocus;
    extern Atom wmPing;
    extern Atom wmSyncRequest;

    extern Atom wmSyncRequestCounter;

    extern Atom wmFullscreenMonitors;

    extern Atom closeWindow;
    extern Atom wmMoveResize;
    extern Atom moveResizeWindow;
    extern Atom restackWindow;

    extern Atom showingDesktop;

    extern Atom xBackground[2];

    extern Atom toolkitAction;
    extern Atom toolkitActionWindowMenu;
    extern Atom toolkitActionForceQuitDialog;

    extern Atom mwmHints;

    extern Atom xdndAware;
    extern Atom xdndEnter;
    extern Atom xdndLeave;
    extern Atom xdndPosition;
    extern Atom xdndStatus;
    extern Atom xdndDrop;

    extern Atom manager;
    extern Atom targets;
    extern Atom multiple;
    extern Atom timestamp;
    extern Atom version;
    extern Atom atomPair;

    extern Atom startupId;

    void init (Display *dpy);
};

#endif
