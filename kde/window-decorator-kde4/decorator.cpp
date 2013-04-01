/*
 * Copyright © 2008 Dennis Kasprzyk <onestone@opencompositing.org>
 * Copyright © 2006 Novell, Inc.
 * Copyright © 2006 Volker Krause <vkrause@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 * Author: David Reveman <davidr@novell.com>
 */

#include <KDE/KCmdLineArgs>
#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KGlobal>
#include <kwindowsystem.h>
#include <KDE/KLocale>
#include <KDE/Plasma/Theme>
#include <kcommondecoration.h>
#include <kwindowsystem.h>

#include <QPoint>
#include <QList>
#include <QX11Info>

#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>

#include "decorator.h"
#include "options.h"
#include "utils.h"

#include "kwinadaptor.h"

#include <stdio.h>

const unsigned int ROOT_OFF_X = 8192;
const unsigned int ROOT_OFF_Y = 8192;

const unsigned short BLUR_TYPE_NONE     = 0;
const unsigned short BLUR_TYPE_TITLEBAR = 1;
const unsigned short BLUR_TYPE_ALL      = 2;

static const float SHADOW_RADIUS    = 8.0f;
static const float SHADOW_OPACITY   = 0.5f;
static const unsigned short  SHADOW_OFFSET_X = 1;
static const unsigned short  SHADOW_OFFSET_Y = 1;
#define SHADOW_COLOR_RED   0x0000
#define SHADOW_COLOR_GREEN 0x0000
#define SHADOW_COLOR_BLUE  0x0000

int    blurType = BLUR_TYPE_NONE;

decor_shadow_t *KWD::Decorator::mNoBorderShadow = 0;
KWD::PluginManager *KWD::Decorator::mPlugins = 0;
KWD::Options *KWD::Decorator::mOptions = 0;
NETRootInfo *KWD::Decorator::mRootInfo;
WId KWD::Decorator::mActiveId;
decor_shadow_options_t KWD::Decorator::mActiveShadowOptions;
decor_shadow_options_t KWD::Decorator::mInactiveShadowOptions;

KWD::Window *KWD::Decorator::mDecorNormal = NULL;
KWD::Window *KWD::Decorator::mDecorActive = NULL;
KWD::Decorator *KWD::Decorator::mSelf = NULL;

struct _cursor cursors[3][3] = {
    { C (top_left_corner), C (top_side), C (top_right_corner) },
    { C (left_side), C (left_ptr), C (right_side) },
    { C (bottom_left_corner), C (bottom_side), C (bottom_right_corner) }
};

KWD::PluginManager::PluginManager (KSharedConfigPtr config):
    KWD::KDecorationPlugins (config)
{
    if (QPixmap::defaultDepth () > 8)
	defaultPlugin = "kwin3_oxygen";
    else
	defaultPlugin = "kwin3_plastik";
}


KWD::Decorator::Decorator () :
    KApplication (),
    mConfig (0),
    mCompositeWindow (0),
    mSwitcher (0)
{
    XSetWindowAttributes attr;
    int			 i, j;

    mSelf = this;

    mRootInfo = new NETRootInfo (QX11Info::display (), 0);

    mActiveId = 0;

    KConfigGroup cfg (KSharedConfig::openConfig ("plasmarc"),
						 QString ("Theme"));
    Plasma::Theme::defaultTheme ()->setThemeName (cfg.readEntry ("name"));

    Atoms::init ();

    new KWinAdaptor (this);

    mConfig = new KConfig ("kwinrc");

    mOptions = new KWD::Options (mConfig);
    mPlugins = new PluginManager (KSharedConfig::openConfig ("kwinrc"));

    mActiveShadowOptions.shadow_radius   = SHADOW_RADIUS;
    mActiveShadowOptions.shadow_opacity  = SHADOW_OPACITY;
    mActiveShadowOptions.shadow_offset_x = SHADOW_OFFSET_X;
    mActiveShadowOptions.shadow_offset_y = SHADOW_OFFSET_Y;
    mActiveShadowOptions.shadow_color[0] = SHADOW_COLOR_RED;
    mActiveShadowOptions.shadow_color[1] = SHADOW_COLOR_GREEN;
    mActiveShadowOptions.shadow_color[2] = SHADOW_COLOR_BLUE;

    mInactiveShadowOptions.shadow_radius   = SHADOW_RADIUS;
    mInactiveShadowOptions.shadow_opacity  = SHADOW_OPACITY;
    mInactiveShadowOptions.shadow_offset_x = SHADOW_OFFSET_X;
    mInactiveShadowOptions.shadow_offset_y = SHADOW_OFFSET_Y;
    mInactiveShadowOptions.shadow_color[0] = SHADOW_COLOR_RED;
    mInactiveShadowOptions.shadow_color[1] = SHADOW_COLOR_GREEN;
    mInactiveShadowOptions.shadow_color[2] = SHADOW_COLOR_BLUE;

    updateShadowProperties (QX11Info::appRootWindow ());

    for (i = 0; i < 3; i++)
    {
	for (j = 0; j < 3; j++)
	{
	    if (cursors[i][j].shape != XC_left_ptr)
		cursors[i][j].cursor =
		    XCreateFontCursor (QX11Info::display (),
				       cursors[i][j].shape);
	}
    }

    attr.override_redirect = True;

    mCompositeWindow = XCreateWindow (QX11Info::display (),
				      QX11Info::appRootWindow (),
				      -ROOT_OFF_X, -ROOT_OFF_Y, 1, 1, 0,
				      CopyFromParent,
				      CopyFromParent,
				      CopyFromParent,
				      CWOverrideRedirect, &attr);

    long data = 1;
    XChangeProperty (QX11Info::display(), mCompositeWindow, Atoms::enlightmentDesktop,
		      XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &data, 1);

    XCompositeRedirectSubwindows (QX11Info::display (), mCompositeWindow,
				  CompositeRedirectManual);

    XMapWindow (QX11Info::display (), mCompositeWindow);
}

KWD::Decorator::~Decorator (void)
{
    QMap <WId, KWD::Window *>::ConstIterator it;

    for (it = mClients.begin (); it != mClients.end (); it++)
	delete (*it);

    if (mDecorNormal)
	delete mDecorNormal;

    if (mDecorActive)
	delete mDecorActive;

    if (mSwitcher)
	delete mSwitcher;

    XDestroyWindow (QX11Info::display (), mCompositeWindow);

    delete mOptions;
    delete mPlugins;
    delete mConfig;
    delete mRootInfo;
}

bool
KWD::Decorator::enableDecorations (Time timestamp)
{
    QList <WId>::ConstIterator it;
    unsigned int nchildren;
    WId       *children;
    WId       root, parent;
    long int  select;

    mDmSnTimestamp = timestamp;

    if (!pluginManager ()->loadPlugin (""))
	return false;

    updateAllShadowOptions ();

    KWD::trapXError ();
    (void) QApplication::desktop (); // trigger creation of desktop widget
    KWD::popXError ();

    updateShadow ();

    /* FIXME: Implement proper decoration lists and remove this */
    mDecorNormal = new KWD::Window (mCompositeWindow,
                                    QX11Info::appRootWindow (),
                                    0, Window::Default);
    mDecorActive = new KWD::Window (mCompositeWindow,
				    QX11Info::appRootWindow (),
				    0, Window::DefaultActive);

    mActiveId = KWindowSystem::activeWindow ();

    connect (KWindowSystem::self (), SIGNAL (windowAdded (WId)),
	     SLOT (handleWindowAdded (WId)));
    connect (KWindowSystem::self (), SIGNAL (windowRemoved (WId)),
	     SLOT (handleWindowRemoved (WId)));
    connect (KWindowSystem::self (), SIGNAL (activeWindowChanged (WId)),
	     SLOT (handleActiveWindowChanged (WId)));
    connect (KWindowSystem::self (),
	     SIGNAL (windowChanged (WId, const unsigned long *)),
	     SLOT (handleWindowChanged (WId, const unsigned long *)));

    foreach (WId id, KWindowSystem::windows ())
	handleWindowAdded (id);

    /* Find the switcher and add it too
     * FIXME: Doing XQueryTree and then
     * XGetWindowProperty on every window
     * like this is really expensive, surely
     * there is a better way to do this */

    XQueryTree (QX11Info::display (), QX11Info::appRootWindow (),
                &root, &parent, &children, &nchildren);

    for (unsigned int i = 0; i < nchildren; i++)
    {
        if (KWD::readWindowProperty (children[i],
                                     Atoms::switchSelectWindow, &select))
        {
            handleWindowAdded(children[i]);
            break;
        }
    }

    connect (Plasma::Theme::defaultTheme (), SIGNAL (themeChanged ()),
	     SLOT (plasmaThemeChanged ()));

    // select for client messages
    XSelectInput (QX11Info::display (), QX11Info::appRootWindow (),
                  SubstructureNotifyMask |
                  StructureNotifyMask |
                  PropertyChangeMask);

    return true;
}

void
KWD::Decorator::updateAllShadowOptions (void)
{
    updateShadowProperties (QX11Info::appRootWindow ());
}

void
KWD::Decorator::changeShadowOptions (decor_shadow_options_t *aopt, decor_shadow_options_t *iopt)
{
    bool changed = false;

    if (memcmp (aopt, &mActiveShadowOptions, sizeof (decor_shadow_options_t)))
    {
	mActiveShadowOptions = *aopt;
	changed = true;
    }

    if (memcmp (aopt, &mInactiveShadowOptions, sizeof (decor_shadow_options_t)))
    {
	mInactiveShadowOptions = *iopt;
	changed = true;
    }

    if (changed)
	updateShadow ();
}

void
KWD::Decorator::updateShadow (void)
{
    Display	    *xdisplay = QX11Info::display ();
    Screen	    *xscreen;
    decor_context_t context;

    xscreen = ScreenOfDisplay (xdisplay, QX11Info::appScreen ());

    if (mNoBorderShadow)
	decor_shadow_destroy (xdisplay, mNoBorderShadow);

    mNoBorderShadow = decor_shadow_create (xdisplay,
					   xscreen,
					   1, 1,
					   0,
					   0,
					   0,
					   0,
					   0, 0, 0, 0,
					   &mActiveShadowOptions,
					   &context,
					   decor_draw_simple,
					   0);

    if (mNoBorderShadow)
    {
	decor_extents_t extents = { 0, 0, 0, 0 };
	long	        *data;
	unsigned int    n = 1, frame_type = 0, frame_state = 0, frame_actions = 0;
	decor_quad_t    quads[N_QUADS_MAX];
	int	        nQuad;
	decor_layout_t  layout;

	decor_get_default_layout (&context, 1, 1, &layout);

	nQuad = decor_set_lSrStSbS_window_quads (quads, &context, &layout);

	data = decor_alloc_property (n, WINDOW_DECORATION_TYPE_PIXMAP);
	decor_quads_to_property (data, n - 1, mNoBorderShadow->pixmap,
				 &extents, &extents, &extents, &extents,
				 0, 0, quads, nQuad, frame_type, frame_state, frame_actions);

	KWD::trapXError ();
	XChangeProperty (QX11Info::display (), QX11Info::appRootWindow (),
			 Atoms::netWindowDecorBare,
			 XA_INTEGER,
			 32, PropModeReplace, (unsigned char *) data,
			 PROP_HEADER_SIZE + BASE_PROP_SIZE + QUAD_PROP_SIZE * N_QUADS_MAX);
	KWD::popXError ();

        free (data);
    }
}

void
KWD::Decorator::updateShadowProperties (WId id)
{
    int nItems;
    long *data;
    double aradius, aopacity;
    int    axOffset, ayOffset;
    double iradius, iopacity;
    int    ixOffset, iyOffset;
    QVector<QString> shadowColor;

    if (id != QX11Info::appRootWindow ())
	return;

    void *propData = KWD::readXProperty (id,
					  Atoms::compizShadowInfo,
					  XA_INTEGER,
					  &nItems);

    if (nItems != 4)
	return;

    data = reinterpret_cast <long *> (propData);

    aradius = data[0];
    aopacity = data[1];

    /* We multiplied by 1000 in compiz to keep
      * precision, now divide by that much */

    aradius /= 1000;
    aopacity /= 1000;

    axOffset = data[2];
    ayOffset = data[3];

    iradius = data[4];
    iopacity = data[5];

    /* We multiplied by 1000 in compiz to keep
     * precision, now divide by that much */

    iradius /= 1000;
    iopacity /= 1000;

    ixOffset = data[6];
    iyOffset = data[7];


    shadowRadiusChanged (aradius, iradius);
    shadowOpacityChanged (aopacity, iopacity);
    shadowXOffsetChanged (axOffset, ixOffset);
    shadowYOffsetChanged (ayOffset, iyOffset);

    shadowColor = KWD::readPropertyString (id, Atoms::compizShadowColor);

    if (shadowColor.size () == 2)
	shadowColorChanged (shadowColor.at (0), shadowColor.at (1));

    XFree (propData);
}

bool
KWD::Decorator::x11EventFilter (XEvent *xevent)
{
    KWD::Window *client;
    int		status;

    switch (xevent->type) {
    case ConfigureNotify: {
	XConfigureEvent *xce = reinterpret_cast <XConfigureEvent *> (xevent);

	if (mFrames.contains (xce->window))
	    mFrames[xce->window]->updateFrame (xce->window);

    } break;
    case SelectionRequest:
	decor_handle_selection_request (QX11Info::display (),
					xevent, mDmSnTimestamp);
	break;
    case SelectionClear:
	status = decor_handle_selection_clear (QX11Info::display (),
					       xevent, 0);
	if (status == DECOR_SELECTION_GIVE_UP)
	    KApplication::exit (0);

        break;
    case CreateNotify:
        /* We only care about windows that aren't managed here */
        if (!KWindowSystem::hasWId (xevent->xcreatewindow.window))
        {
            WId select;

            KWD::trapXError ();
            XSelectInput (QX11Info::display (), xevent->xcreatewindow.window,
                          StructureNotifyMask | PropertyChangeMask);
            KWD::popXError ();

            if (KWD::readWindowProperty (xevent->xcreatewindow.window,
                                         Atoms::switchSelectWindow,
                                         (long *) &select))
                handleWindowAdded (xevent->xcreatewindow.window);
        }

    case PropertyNotify:
	if (xevent->xproperty.atom == Atoms::netInputFrameWindow)
	{
	    handleWindowAdded (xevent->xproperty.window);
	}
	else if (xevent->xproperty.atom == Atoms::netOutputFrameWindow)
	{
	    handleWindowAdded (xevent->xproperty.window);
	}
	else if (xevent->xproperty.atom == Atoms::compizShadowInfo ||
		 xevent->xproperty.atom == Atoms::compizShadowColor)
	{
	    updateShadowProperties (xevent->xproperty.window);
	}
	else if (xevent->xproperty.atom == Atoms::switchSelectWindow)
	{
	    WId id = xevent->xproperty.window;

	    if (!mSwitcher || mSwitcher->xid () != id)
		handleWindowAdded (id);
	    mSwitcher->update ();
	}
	else if (xevent->xproperty.atom == Atoms::netWmWindowOpacity)
	{
	    if (mClients.contains (xevent->xproperty.window))
		mClients[xevent->xproperty.window]->updateOpacity ();
	}
	break;
    case EnterNotify:
    {
	XCrossingEvent *xce = reinterpret_cast <XCrossingEvent *> (xevent);
	QWidget	       *child;

	if (!mFrames.contains (xce->window))
	    break;

	client = mFrames[xce->window];

	if (!client->decorWidget ())
	    break;

	child = client->childAt (xce->x, xce->y);
	if (child)
	{
	    QEvent qe (QEvent::Enter);

	    QApplication::sendEvent (child, &qe);

	    client->setActiveChild (child);
	    client->updateCursor (QPoint (xce->x, xce->y));
	}
    } break;
    case LeaveNotify:
    {
	XCrossingEvent *xce = reinterpret_cast <XCrossingEvent *> (xevent);

	if (mFrames.contains (xce->window))
	{
	    QEvent qe (QEvent::Leave);

	    client = mFrames[xce->window];

	    if (client->activeChild ())
		QApplication::sendEvent (client->activeChild (), &qe);

	    XUndefineCursor (QX11Info::display (), client->frameId ());
	}
    } break;
    case MotionNotify:
    {
	XMotionEvent *xme = reinterpret_cast <XMotionEvent *> (xevent);
	QWidget	     *child;

	if (!mFrames.contains (xme->window))
	    break;

	client = mFrames[xme->window];

	if (!client->decorWidget ())
	    break;

	child = client->childAt (xme->x, xme->y);

	if (child)
	{
	    QPoint qp (xme->x, xme->y);

	    if (child != client->activeChild ())
	    {
		QEvent qee (QEvent::Enter);
		QEvent qle (QEvent::Leave);

		if (client->activeChild ())
		    QApplication::sendEvent (client->activeChild (), &qle);

		QApplication::sendEvent (child, &qee);

		client->setActiveChild (child);
	    }

	    if (client->decorWidget () != child)
		qp = child->mapFrom (client->decorWidget (), qp);

	    QMouseEvent qme (QEvent::MouseMove, qp, Qt::NoButton,
			     Qt::NoButton, Qt::NoModifier);

	    QApplication::sendEvent (child, &qme);

	    client->updateCursor (QPoint (xme->x, xme->y));
	}
    } break;
    case ButtonPress:
    case ButtonRelease:
    {
	XButtonEvent *xbe = reinterpret_cast <XButtonEvent *> (xevent);
	QWidget	     *child;

	if (!mFrames.contains (xbe->window))
	    break;

	client = mFrames[xbe->window];

	if (!client->decorWidget ())
	    break;

	child = client->childAt (xbe->x, xbe->y);

	if (child)
	{
	    XButtonEvent xbe2 = *xbe;
	    xbe2.window = child->winId ();
	    QPoint p;

	    p = client->mapToChildAt (QPoint (xbe->x, xbe->y));
	    xbe2.x = p.x ();
	    xbe2.y = p.y ();

	    p = child->mapToGlobal(p);
	    xbe2.x_root = p.x ();
	    xbe2.y_root = p.y ();

	    client->setFakeRelease (false);
	    QApplication::x11ProcessEvent ((XEvent *) &xbe2);

	    /* We won't get a button release event, because of the screengrabs
	       in compiz */
	    if (client->getFakeRelease () && xevent->type == ButtonPress)
	    {
		xbe2.type = ButtonRelease;
		QApplication::x11ProcessEvent ((XEvent *) &xbe2);
	    }

	    return true;
	}
    } break;
    case ClientMessage:
	if (xevent->xclient.message_type == Atoms::toolkitActionAtom)
	{
	    unsigned long action;

	    action = xevent->xclient.data.l[0];
	    if (action == Atoms::toolkitActionWindowMenuAtom)
	    {
		if (mClients.contains (xevent->xclient.window))
		{
		    QPoint pos;

		    client = mClients[xevent->xclient.window];

		    if (xevent->xclient.data.l[2])
		    {
			pos = QPoint (xevent->xclient.data.l[3],
				      xevent->xclient.data.l[4]);
		    }
		    else
		    {
			pos = client->clientGeometry ().topLeft ();
		    }

		    client->showWindowMenu (pos);
		}
	    }
	    else if (action == Atoms::toolkitActionForceQuitDialogAtom)
	    {
		if (mClients.contains (xevent->xclient.window))
		{
		    Time timestamp = xevent->xclient.data.l[1];

		    client = mClients[xevent->xclient.window];

		    if (xevent->xclient.data.l[2])
			client->showKillProcessDialog (timestamp);
		    else
			client->hideKillProcessDialog ();
		}
	    }
	}
	break;
    default:
	break;
    }

    return KApplication::x11EventFilter (xevent);
}

void
KWD::Decorator::reconfigure (void)
{
    unsigned long changed;

    mConfig->reparseConfiguration ();

    changed = mOptions->updateSettings ();
    if (mPlugins->reset (changed))
    {
	QMap < WId, KWD::Window * >::ConstIterator it;

	updateShadow ();

	mDecorNormal->reloadDecoration ();
	mDecorActive->reloadDecoration ();

	for (it = mClients.constBegin (); it != mClients.constEnd (); it++)
	    it.value ()->reloadDecoration ();

	mPlugins->destroyPreviousPlugin ();
    }
}

void
KWD::Decorator::handleWindowAdded (WId id)
{
    QMap <WId, KWD::Window *>::ConstIterator it;
    KWD::Window				     *client = 0;
    WId					     select, frame = 0;
    WId					     oframe = 0, iframe = 0;
    KWD::Window::Type			     type = KWD::Window::Normal;
    QWidgetList				     widgets;
    QRect                                    geometry;

    /* avoid adding any of our own top level windows */
    foreach (QWidget *widget, QApplication::topLevelWidgets ()) {
        if (widget->winId () == id)
	    return;
    }

    if (KWD::readWindowProperty (id, Atoms::switchSelectWindow,
				 (long *) &select))
    {
	if (!mSwitcher)
            mSwitcher = new Switcher (mCompositeWindow, id);
        if (mSwitcher->xid () != id)
        {
            delete mSwitcher;
            mSwitcher = new Switcher (mCompositeWindow, id);
        }

	geometry = mSwitcher->geometry ();
	frame = None;
    }
    else
    {
        KWindowInfo wInfo;

        KWD::trapXError ();
        wInfo = KWindowSystem::windowInfo (id, NET::WMGeometry);
        if (KWD::popXError ())
            return;

        if (!wInfo.valid ())
            return;

        KWD::readWindowProperty (id, Atoms::netInputFrameWindow, (long *) &iframe);
        KWD::readWindowProperty (id, Atoms::netOutputFrameWindow, (long *) &oframe);

        geometry = wInfo.geometry ();

        wInfo = KWindowSystem::windowInfo (id, NET::WMWindowType, 0);

	switch (wInfo.windowType (~0)) {
	case NET::Normal:
	case NET::Dialog:
	case NET::Toolbar:
	case NET::Menu:
	case NET::Utility:
	case NET::Splash:
	case NET::Unknown:
	    /* decorate these window types */
	    break;
	default:
	    return;
	}

	if (iframe)
	{
	    type = KWD::Window::Normal;
	    frame = iframe;
	}
	else
	{
	    type = KWD::Window::Normal2D;
	    frame = oframe;
	}
    }

    KWD::trapXError ();
    XSelectInput (QX11Info::display (), id,
		  StructureNotifyMask | PropertyChangeMask);
    KWD::popXError ();

    if (frame)
    {
	XWindowAttributes attr;
	KWD::trapXError ();
	XGetWindowAttributes (QX11Info::display (), frame, &attr);
	if (KWD::popXError ())
	    frame = None;
    }
    if (frame)
    {
	if (!mClients.contains (id))
	{
	    client = new KWD::Window (mCompositeWindow, id, frame, type,
				      geometry);

	    mClients.insert (id, client);
	    mFrames.insert (frame, client);
	}
	else
	{
	    client = mClients[id];
	    mFrames.remove (client->frameId ());
	    mFrames.insert (frame, client);

	    client->updateFrame (frame);
	}
    }
    else
    {
	if (mClients.contains (id))
	    client = mClients[id];

	if (client)
	{
	    mClients.remove (client->windowId ());
	    mFrames.remove (client->frameId ());

	    delete client;
	}
    }
}

void
KWD::Decorator::handleWindowRemoved (WId id)
{
    KWD::Window *window = 0;

    if (mClients.contains (id))
	window = mClients[id];
    else if (mFrames.contains (id))
	window = mFrames[id];

    if (window)
    {
	mClients.remove (window->windowId ());
	mFrames.remove (window->frameId ());
	delete window;
    }

    if (mSwitcher && mSwitcher->xid () == id)
    {
	delete mSwitcher;
	mSwitcher = NULL;
    }
}

void
KWD::Decorator::handleActiveWindowChanged (WId id)
{
    if (id != mActiveId)
    {
	KWD::Window *newActiveWindow = 0;
	KWD::Window *oldActiveWindow = 0;

	if (mClients.contains (id))
	    newActiveWindow = mClients[id];

	if (mClients.contains (mActiveId))
	    oldActiveWindow = mClients[mActiveId];

	mActiveId = id;

	if (oldActiveWindow)
	    oldActiveWindow->handleActiveChange ();

	if (newActiveWindow)
	    newActiveWindow->handleActiveChange ();
    }
}

void
KWD::Decorator::handleWindowChanged (WId		 id,
				     const unsigned long *properties)
{
    KWD::Window *client;

    if (mSwitcher && mSwitcher->xid () == id)
    {
	if (properties[0] & NET::WMGeometry)
	    mSwitcher->updateGeometry ();
	return;
    }

    if (!mClients.contains (id))
	return;

    client = mClients[id];

    if (properties[0] & NET::WMName)
	client->updateName ();
    if (properties[0] & NET::WMVisibleName)
	client->updateName ();
    if (properties[0] & NET::WMState)
	client->updateState ();
    if (properties[0] & NET::WMIcon)
	client->updateIcons ();
    if (properties[0] & NET::WMGeometry)
	client->updateWindowGeometry ();
}

void
KWD::Decorator::sendClientMessage (WId  eventWid,
				   WId  wid,
				   Atom atom,
				   Atom value,
				   long data1,
				   long data2,
				   long data3)
{
    XEvent ev;
    long   mask = 0;

    memset (&ev, 0, sizeof (ev));

    ev.xclient.type	    = ClientMessage;
    ev.xclient.window	    = wid;
    ev.xclient.message_type = atom;
    ev.xclient.format       = 32;

    ev.xclient.data.l[0] = value;
    ev.xclient.data.l[1] = QX11Info::appTime ();
    ev.xclient.data.l[2] = data1;
    ev.xclient.data.l[3] = data2;
    ev.xclient.data.l[4] = data3;

    if (eventWid == QX11Info::appRootWindow ())
	mask = SubstructureRedirectMask | SubstructureNotifyMask;

    KWD::trapXError ();
    XSendEvent (QX11Info::display (), eventWid, false, mask, &ev);
    KWD::popXError ();
}

void
KWD::Decorator::shadowRadiusChanged (double value_active, double value_inactive)
{
    decor_shadow_options_t aopt = *activeShadowOptions ();
    decor_shadow_options_t iopt = *inactiveShadowOptions ();

    aopt.shadow_radius = value_active;
    iopt.shadow_radius = value_inactive;

    changeShadowOptions (&aopt, &iopt);
}

void
KWD::Decorator::shadowOpacityChanged (double value_active, double value_inactive)
{
    decor_shadow_options_t aopt = *activeShadowOptions ();
    decor_shadow_options_t iopt = *inactiveShadowOptions ();

    aopt.shadow_opacity = value_active;
    iopt.shadow_opacity = value_inactive;

    changeShadowOptions (&aopt, &iopt);
}

void
KWD::Decorator::shadowXOffsetChanged (int value_active, int value_inactive)
{
    decor_shadow_options_t aopt = *activeShadowOptions ();
    decor_shadow_options_t iopt = *inactiveShadowOptions ();

    aopt.shadow_offset_x = value_active;
    iopt.shadow_offset_x = value_inactive;

    changeShadowOptions (&aopt, &iopt);
}

void
KWD::Decorator::shadowYOffsetChanged (int value_active, double value_inactive)
{
    decor_shadow_options_t aopt = *activeShadowOptions ();
    decor_shadow_options_t iopt = *inactiveShadowOptions ();

    aopt.shadow_offset_y = value_active;
    iopt.shadow_offset_y = value_inactive;

    changeShadowOptions (&aopt, &iopt);
}

void
KWD::Decorator::shadowColorChanged (QString value_active, QString value_inactive)
{
    decor_shadow_options_t aopt = *activeShadowOptions ();
    decor_shadow_options_t iopt = *inactiveShadowOptions ();

    int c[4];

    if (sscanf (value_active.toAscii ().data (), "#%2x%2x%2x%2x",
	        &c[0], &c[1], &c[2], &c[3]) == 4)
    {
	aopt.shadow_color[0] = c[0] << 8 | c[0];
	aopt.shadow_color[1] = c[1] << 8 | c[1];
	aopt.shadow_color[2] = c[2] << 8 | c[2];
    }

    if (sscanf (value_inactive.toAscii ().data (), "#%2x%2x%2x%2x",
		&c[0], &c[1], &c[2], &c[3]) == 4)
    {
	iopt.shadow_color[0] = c[0] << 8 | c[0];
	iopt.shadow_color[1] = c[1] << 8 | c[1];
	iopt.shadow_color[2] = c[2] << 8 | c[2];
    }

    changeShadowOptions (&aopt, &iopt);
}

void
KWD::Decorator::plasmaThemeChanged ()
{
    if (mSwitcher)
    {
	WId win = mSwitcher->xid ();
	delete mSwitcher;
	mSwitcher = new Switcher (mCompositeWindow, win);
    }
}
