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
#ifndef _COMPIZ_GWD_MOCK_SETTINGS_STORAGE_H
#define _COMPIZ_GWD_MOCK_SETTINGS_STORAGE_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <glib-object.h>

typedef struct _GWDSettingsStorage GWDSettingsStorage;

class GWDMockSettingsStorageGMockInterface;

G_BEGIN_DECLS

#define GWD_TYPE_MOCK_SETTINGS_STORAGE (gwd_mock_settings_storage_get_type ())
GType gwd_mock_settings_storage_get_type ();

GWDSettingsStorage *
gwd_mock_settings_storage_new (GWDMockSettingsStorageGMockInterface *);

G_END_DECLS

class GWDMockSettingsStorageGMockInterface
{
    public:

	virtual ~GWDMockSettingsStorageGMockInterface () {}

	virtual gboolean updateUseTooltips () = 0;
	virtual gboolean updateDraggableBorderWidth () = 0;
	virtual gboolean updateAttachModalDialogs () = 0;
	virtual gboolean updateBlur () = 0;
	virtual gboolean updateMetacityTheme () = 0;
	virtual gboolean updateOpacity () = 0;
	virtual gboolean updateButtonLayout () = 0;
	virtual gboolean updateFont () = 0;
	virtual gboolean updateTitlebarActions () = 0;

	virtual void dispose () = 0;
	virtual void finalize () = 0;
};

class GWDMockSettingsStorageGMock :
    public GWDMockSettingsStorageGMockInterface
{
    public:

	MOCK_METHOD0 (updateUseTooltips, gboolean ());
	MOCK_METHOD0 (updateDraggableBorderWidth, gboolean ());
	MOCK_METHOD0 (updateAttachModalDialogs, gboolean ());
	MOCK_METHOD0 (updateBlur, gboolean ());
	MOCK_METHOD0 (updateMetacityTheme, gboolean ());
	MOCK_METHOD0 (updateOpacity, gboolean ());
	MOCK_METHOD0 (updateButtonLayout, gboolean ());
	MOCK_METHOD0 (updateFont, gboolean ());
	MOCK_METHOD0 (updateTitlebarActions, gboolean ());
	MOCK_METHOD0 (dispose, void ());
	MOCK_METHOD0 (finalize, void ());
};

#endif
