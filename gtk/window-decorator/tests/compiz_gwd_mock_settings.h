/*
 * Copyright © 2012 Canonical Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _COMPIZ_GWD_MOCK_SETTINGS_H
#define _COMPIZ_GWD_MOCK_SETTINGS_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <glib-object.h>

class GWDMockSettingsGMockInterface;

typedef struct _GWDSettings GWDSettingsImpl;

G_BEGIN_DECLS

#define GWD_TYPE_MOCK_SETTINGS (gwd_mock_settings_get_type ())
GType gwd_mock_settings_get_type ();

GWDSettings *
gwd_mock_settings_new (GWDMockSettingsGMockInterface *);

enum
{
    GWD_MOCK_SETTINGS_PROPERTY_ACTIVE_SHADOW = 1,
    GWD_MOCK_SETTINGS_PROPERTY_INACTIVE_SHADOW = 2,
    GWD_MOCK_SETTINGS_PROPERTY_USE_TOOLTIPS = 3,
    GWD_MOCK_SETTINGS_PROPERTY_DRAGGABLE_BORDER_WIDTH = 4,
    GWD_MOCK_SETTINGS_PROPERTY_ATTACH_MODAL_DIALOGS = 5,
    GWD_MOCK_SETTINGS_PROPERTY_BLUR_CHANGED = 6,
    GWD_MOCK_SETTINGS_PROPERTY_METACITY_THEME = 7,
    GWD_MOCK_SETTINGS_PROPERTY_ACTIVE_OPACITY = 8,
    GWD_MOCK_SETTINGS_PROPERTY_INACTIVE_OPACITY = 9,
    GWD_MOCK_SETTINGS_PROPERTY_ACTIVE_SHADE_OPACITY = 10,
    GWD_MOCK_SETTINGS_PROPERTY_INACTIVE_SHADE_OPACITY = 11,
    GWD_MOCK_SETTINGS_PROPERTY_BUTTON_LAYOUT = 12,
    GWD_MOCK_SETTINGS_PROPERTY_TITLEBAR_ACTION_DOUBLE_CLICK = 13,
    GWD_MOCK_SETTINGS_PROPERTY_TITLEBAR_ACTION_MIDDLE_CLICK = 14,
    GWD_MOCK_SETTINGS_PROPERTY_TITLEBAR_ACTION_RIGHT_CLICK = 15,
    GWD_MOCK_SETTINGS_PROPERTY_MOUSE_WHEEL_ACTION = 16,
    GWD_MOCK_SETTINGS_PROPERTY_TITLEBAR_FONT = 17,
    GWD_MOCK_SETTINGS_PROPERTY_GMOCK_INTERFACE = 18
};

G_END_DECLS

class GWDMockSettingsGMockInterface
{
    public:

	virtual ~GWDMockSettingsGMockInterface () {}

	virtual void getProperty (guint property_id,
				  GValue *property_value,
				  GParamSpec *pspec) = 0;
	virtual void dispose () = 0;
	virtual void finalize () = 0;
};

class GWDMockSettingsGMock :
    public GWDMockSettingsGMockInterface
{
    public:

	MOCK_METHOD3 (getProperty, void (guint, GValue *, GParamSpec *));
	MOCK_METHOD0 (dispose, void ());
	MOCK_METHOD0 (finalize, void ());
};

#endif
