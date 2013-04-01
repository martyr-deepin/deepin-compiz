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

#ifndef _DECORATOR_H
#define _DECORATOR_H

#include <kapplication.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>

#include <QTimer>

#include <fixx11h.h>
#include <KDE/KConfig>
#include <netwm.h>

#include <decoration.h>

#include "window.h"
#include "switcher.h"
#include "kdecoration_plugins.h"
#include "utils.h"

extern const unsigned int ROOT_OFF_X;
extern const unsigned int ROOT_OFF_Y;

#define C(name) { 0, XC_ ## name }

struct _cursor {
    Cursor       cursor;
    unsigned int shape;
};

extern struct _cursor cursors[3][3];

extern const unsigned short BLUR_TYPE_NONE;
extern const unsigned short BLUR_TYPE_TITLEBAR;
extern const unsigned short BLUR_TYPE_ALL;

extern int blurType;

class KConfig;
class KWindowSystem;

namespace KWD
{
    class Options;

class PluginManager:public KDecorationPlugins {
    public:
	PluginManager (KSharedConfigPtr config);
	virtual bool provides (Requirement)
	{
	    return false;
	}
    };


class Decorator:public KApplication {
    Q_OBJECT public:
#ifdef QT_45
	Decorator ();
#else
        Decorator (Display* display, Qt::HANDLE visual, Qt::HANDLE colormap);
#endif
	~Decorator (void);

	static NETRootInfo *rootInfo (void)
	{
	    return mRootInfo;
	}
	static PluginManager *pluginManager (void)
	{
	    return mPlugins;
	}
	static KWD::Options *options (void)
	{
	    return mOptions;
	}
	static WId activeId (void)
	{
	    return mActiveId;
	}
	static decor_shadow_options_t *activeShadowOptions (void)
	{
	    return &mActiveShadowOptions;
	}
	static decor_shadow_options_t *inactiveShadowOptions (void)
	{
	    return &mInactiveShadowOptions;
	}

	static KWD::Window *defaultNormal ()
	{
	    return mDecorNormal;
	}

	static KWD::Window *defaultActive ()
	{
	    return mDecorActive;
	}

	static KWD::Decorator *self ()
	{
	    return mSelf;
	}

	static void sendClientMessage (WId  eventWid,
				       WId  wid,
				       Atom atom,
				       Atom value,
				       long data1 = 0,
				       long data2 = 0,
				       long data3 = 0);
	
	bool enableDecorations (Time timestamp);
	bool x11EventFilter (XEvent *xevent);
	void changeShadowOptions (decor_shadow_options_t *aopt, decor_shadow_options_t *iopt);

    public slots:
	void reconfigure (void);

    private:
	WId fetchFrame (WId window);
	void updateShadow (void);
	void updateAllShadowOptions (void);
	void updateShadowProperties (WId id);

    private slots:
	void handleWindowAdded (WId id);
	void handleWindowRemoved (WId id);
	void handleActiveWindowChanged (WId id);
	void handleWindowChanged (WId		      id,
				  const unsigned long *properties);

	void shadowRadiusChanged (double value_active, double value_inactive);
	void shadowOpacityChanged (double value_active, double value_inactive);
	void shadowXOffsetChanged (int value_active, int value_inactive);
	void shadowYOffsetChanged (int value_active, double value_inactive);
	void shadowColorChanged (QString value_active, QString value_inactive);

	void plasmaThemeChanged ();

    private:
	static PluginManager *mPlugins;
	static KWD::Options *mOptions;
	static decor_shadow_t *mNoBorderShadow;
	static decor_shadow_options_t mActiveShadowOptions;
	static decor_shadow_options_t mInactiveShadowOptions;
	static NETRootInfo *mRootInfo;
	static WId mActiveId;

	static KWD::Window *mDecorNormal;
	static KWD::Window *mDecorActive;
	QMap <WId, KWD::Window *>mClients;
	QMap <WId, KWD::Window *>mFrames;
	KConfig *mConfig;
	Time mDmSnTimestamp;

	WId mCompositeWindow;

	Switcher *mSwitcher;

        static KWD::Decorator *mSelf; /* XXX: Remove */
    };
}

#endif
