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
#ifndef _COMPIZ_GWD_SETTINGS_NOTIFIED_H
#define _COMPIZ_GWD_SETTINGS_NOTIFIED_H

#include <glib-object.h>

typedef struct _GWDSettingsNotified GWDSettingsNotified;
typedef struct _WnckScreen	    WnckScreen;

G_BEGIN_DECLS

#define GWD_TYPE_SETTINGS_NOTIFIED (gwd_settings_notified_impl_get_type ())
GType gwd_settings_notified_impl_get_type ();

GWDSettingsNotified *
gwd_settings_notified_impl_new (WnckScreen *screen);

G_END_DECLS

#endif
