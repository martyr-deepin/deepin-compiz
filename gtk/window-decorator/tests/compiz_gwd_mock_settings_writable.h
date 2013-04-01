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
 *
 * Authored By: Sam Spilsbury <sam.spilsbury@canonical.com>
 */
#ifndef _COMPIZ_GWD_MOCK_SETTINGS_WRITABLE_H
#define _COMPIZ_GWD_MOCK_SETTINGS_WRITABLE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <glib-object.h>

typedef struct _GWDSettingsWritable GWDSettingsWritable;

class GWDMockSettingsWritableGMockInterface;

G_BEGIN_DECLS

#define GWD_TYPE_MOCK_SETTINGS_WRITABLE (gwd_mock_settings_writable_get_type ())
GType gwd_mock_settings_writable_get_type ();

GWDSettingsWritable *
gwd_mock_settings_writable_new (GWDMockSettingsWritableGMockInterface *);

G_END_DECLS

class GWDMockSettingsWritableGMockInterface
{
    public:

	virtual ~GWDMockSettingsWritableGMockInterface () {}

	virtual void freezeUpdates() = 0;
	virtual void thawUpdates () = 0;
	virtual gboolean shadowPropertyChanged (gdouble active_shadow_radius,
						gdouble active_shadow_opacity,
						gdouble active_shadow_offset_x,
						gdouble active_shadow_offset_y,
						const gchar   *active_shadow_color,
						gdouble inactive_shadow_radius,
						gdouble inactive_shadow_opacity,
						gdouble inactive_shadow_offset_x,
						gdouble inactive_shadow_offset_y,
						const gchar	*inactive_shadow_color) = 0;
	virtual gboolean useTooltipsChanged (gboolean newValue) = 0;
	virtual gboolean draggableBorderWidthChanged (gint newValue) = 0;
	virtual gboolean attachModalDialogsChanged (gboolean newValue) = 0;
	virtual gboolean blurChanged (const gchar *type) = 0;
	virtual gboolean metacityThemeChanged (gboolean useMetacityTheme, const gchar *metacityTheme) = 0;
	virtual gboolean opacityChanged (gdouble inactiveOpacity,
					 gdouble activeOpacity,
					 gboolean inactiveShadeOpacity,
					 gboolean activeShadeOpacity) = 0;
	virtual gboolean buttonLayoutChanged (const gchar *buttonLayout) = 0;
	virtual gboolean fontChanged (gboolean useSystemFont,
				      const gchar *titlebarFont) = 0;
	virtual gboolean titlebarActionsChanged (const gchar *doubleClickAction,
						 const gchar *middleClickAction,
						 const gchar *rightClickAction,
						 const gchar *mouseWheelAction) = 0;


	virtual void dispose () = 0;
	virtual void finalize () = 0;
};

class GWDMockSettingsWritableGMock :
    public GWDMockSettingsWritableGMockInterface
{
    public:

	MOCK_METHOD0 (freezeUpdates, void ());
	MOCK_METHOD0 (thawUpdates, void ());
	MOCK_METHOD10 (shadowPropertyChanged, gboolean (gdouble, gdouble, gdouble, gdouble, const gchar *,
							gdouble, gdouble, gdouble, gdouble, const gchar *));
	MOCK_METHOD1 (useTooltipsChanged, gboolean (gboolean));
	MOCK_METHOD1 (draggableBorderWidthChanged, gboolean (gint));
	MOCK_METHOD1 (attachModalDialogsChanged, gboolean (gboolean));
	MOCK_METHOD1 (blurChanged, gboolean (const gchar *));
	MOCK_METHOD2 (metacityThemeChanged, gboolean (gboolean, const gchar *));
	MOCK_METHOD4 (opacityChanged, gboolean (gdouble, gdouble, gboolean, gboolean));
	MOCK_METHOD1 (buttonLayoutChanged, gboolean (const gchar *));
	MOCK_METHOD2 (fontChanged, gboolean (gboolean, const gchar *));
	MOCK_METHOD4 (titlebarActionsChanged, gboolean (const gchar *,
							const gchar *,
							const gchar *,
							const gchar *));

	MOCK_METHOD0 (dispose, void ());
	MOCK_METHOD0 (finalize, void ());
};

#endif
