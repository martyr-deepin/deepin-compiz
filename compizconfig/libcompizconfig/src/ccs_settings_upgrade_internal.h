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
#ifndef _COMPIZCONFIG_CCS_SETTINGS_UPGRADE_INTERNAL_H
#define _COMPIZCONFIG_CCS_SETTINGS_UPGRADE_INTERNAL_H

#include "ccs-defs.h"

COMPIZCONFIG_BEGIN_DECLS

typedef struct _CCSSettingList * CCSSettingList;

Bool
ccsUpgradeGetDomainNumAndProfile (const char   *name,
				  char         **domain,
				  unsigned int *num,
				  char         **profile);

int
ccsUpgradeNameFilter (const char *name);

void
ccsUpgradeClearValues (CCSSettingList clearSettings);

void
ccsUpgradeAddValues (CCSSettingList addSettings);

void
ccsUpgradeReplaceValues (CCSSettingList replaceFromValueSettings,
			 CCSSettingList replaceToValueSettings);

COMPIZCONFIG_END_DECLS

#endif
