/*
 * Compiz configuration system library
 *
 * Copyright (C) 2012 Canonical Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Authored By:
 * Sam Spilsbury <sam.spilsbury@canonical.com>
 */
#ifndef _COMPIZCONFIG_CCS_MOCK_GNOME_INTEGRATED_SETTING_COMPOSITION_H
#define _COMPIZCONFIG_CCS_MOCK_GNOME_INTEGRATED_SETTING_COMPOSITION_H

COMPIZCONFIG_BEGIN_DECLS

typedef struct _CCSIntegratedSetting CCSIntegratedSetting;
typedef struct _CCSGNOMEIntegratedSettingInfo CCSGNOMEIntegratedSettingInfo;
typedef struct _CCSIntegratedSettingInfo CCSIntegratedSettingInfo;
typedef struct _CCSObjectAllocationInterface CCSObjectAllocationInterface;

CCSIntegratedSetting *
ccsMockCompositionIntegratedSettingNew (CCSIntegratedSetting          *integratedSetting,
					CCSGNOMEIntegratedSettingInfo *gnomeInfo,
					CCSIntegratedSettingInfo      *settingInfo,
					CCSObjectAllocationInterface  *allocator);

COMPIZCONFIG_END_DECLS

#endif

