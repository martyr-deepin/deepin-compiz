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

#include <ccs.h>
#include <ccs-backend.h>
#include <ccs_gnome_integrated_setting.h>
#include "compizconfig_ccs_mock_gnome_integrated_setting_composition.h"

typedef struct _CCSMockGNOMEIntegratedSettingCompositionPrivate
{
    CCSIntegratedSetting          *integratedSetting;
    CCSGNOMEIntegratedSettingInfo *gnomeIntegratedSettingInfo;
    CCSIntegratedSettingInfo      *integratedSettingInfo;
} CCSMockGNOMEIntegratedSettingCompositionPrivate;

static CCSIntegratedSetting *
allocateCCSIntegratedSetting (CCSObjectAllocationInterface *allocator)
{
    CCSIntegratedSetting *setting =
	    (*allocator->calloc_) (allocator->allocator,
				   1,
				   sizeof (CCSIntegratedSetting));

    ccsObjectInit (setting, allocator);

    return setting;
}

static CCSMockGNOMEIntegratedSettingCompositionPrivate *
allocatePrivate (CCSIntegratedSetting         *integratedSetting,
		 CCSObjectAllocationInterface *allocator)
{
    CCSMockGNOMEIntegratedSettingCompositionPrivate *priv =
	    (*allocator->calloc_) (allocator->allocator,
				   1,
				   sizeof (CCSMockGNOMEIntegratedSettingCompositionPrivate));

    if (!priv)
    {
	ccsObjectFinalize (integratedSetting);
	(*allocator->free_) (allocator->allocator, integratedSetting);
	return NULL;
    }

    return priv;
}

static SpecialOptionType
ccsMockCompositionIntegratedSettingGetSpecialOptionType (CCSGNOMEIntegratedSettingInfo *setting)
{
    CCSMockGNOMEIntegratedSettingCompositionPrivate *priv = GET_PRIVATE (CCSMockGNOMEIntegratedSettingCompositionPrivate, setting);

    return ccsGNOMEIntegratedSettingInfoGetSpecialOptionType (priv->gnomeIntegratedSettingInfo);
}

static const char *
ccsMockCompositionIntegratedSettingGetGNOMEName (CCSGNOMEIntegratedSettingInfo *setting)
{
    CCSMockGNOMEIntegratedSettingCompositionPrivate *priv = GET_PRIVATE (CCSMockGNOMEIntegratedSettingCompositionPrivate, setting);

    return ccsGNOMEIntegratedSettingInfoGetGNOMEName (priv->gnomeIntegratedSettingInfo);
}

static CCSSettingValue *
ccsMockCompositionIntegratedSettingReadValue (CCSIntegratedSetting *setting, CCSSettingType type)
{
    CCSMockGNOMEIntegratedSettingCompositionPrivate *priv =
	    GET_PRIVATE (CCSMockGNOMEIntegratedSettingCompositionPrivate, setting);

    return ccsIntegratedSettingReadValue (priv->integratedSetting, type);
}

static void
ccsMockCompositionIntegratedSettingWriteValue (CCSIntegratedSetting *setting, CCSSettingValue *v, CCSSettingType type)
{
    CCSMockGNOMEIntegratedSettingCompositionPrivate *priv =
	    GET_PRIVATE (CCSMockGNOMEIntegratedSettingCompositionPrivate, setting);

    ccsIntegratedSettingWriteValue (priv->integratedSetting, v, type);
}

static const char *
ccsMockCompositionIntegratedSettingInfoPluginName (CCSIntegratedSettingInfo *setting)
{
    CCSMockGNOMEIntegratedSettingCompositionPrivate *priv = GET_PRIVATE (CCSMockGNOMEIntegratedSettingCompositionPrivate, setting);

    return ccsIntegratedSettingInfoPluginName (priv->integratedSettingInfo);
}

static const char *
ccsMockCompositionIntegratedSettingInfoSettingName (CCSIntegratedSettingInfo *setting)
{
    CCSMockGNOMEIntegratedSettingCompositionPrivate *priv = GET_PRIVATE (CCSMockGNOMEIntegratedSettingCompositionPrivate, setting);

    return ccsIntegratedSettingInfoSettingName (priv->integratedSettingInfo);
}

static CCSSettingType
ccsMockCompositionIntegratedSettingInfoGetType (CCSIntegratedSettingInfo *setting)
{
    CCSMockGNOMEIntegratedSettingCompositionPrivate *priv = GET_PRIVATE (CCSMockGNOMEIntegratedSettingCompositionPrivate, setting);

    return ccsIntegratedSettingInfoGetType (priv->integratedSettingInfo);
}

static void
ccsMockCompositionIntegratedSettingFree (CCSIntegratedSetting        *integratedSetting)
{
    CCSMockGNOMEIntegratedSettingCompositionPrivate *priv =
	    GET_PRIVATE (CCSMockGNOMEIntegratedSettingCompositionPrivate, integratedSetting);

    ccsIntegratedSettingUnref (priv->integratedSetting);
    ccsGNOMEIntegratedSettingInfoUnref (priv->gnomeIntegratedSettingInfo);

    ccsObjectFinalize (integratedSetting);
    (*integratedSetting->object.object_allocation->free_)
	    (integratedSetting->object.object_allocation->allocator, integratedSetting);
}

static void
ccsMockCompositionIntegratedSettingInfoFree (CCSIntegratedSettingInfo *info)
{
    return ccsMockCompositionIntegratedSettingFree ((CCSIntegratedSetting *) info);
}

static void
ccsMockCompositionGNOMEIntegratedSettingInfoFree (CCSGNOMEIntegratedSettingInfo *info)
{
    return ccsMockCompositionIntegratedSettingFree ((CCSIntegratedSetting *) info);
}

const CCSGNOMEIntegratedSettingInfoInterface ccsMockCompositionGNOMEIntegratedSettingInfo =
{
    ccsMockCompositionIntegratedSettingGetSpecialOptionType,
    ccsMockCompositionIntegratedSettingGetGNOMEName,
    ccsMockCompositionGNOMEIntegratedSettingInfoFree
};

const CCSIntegratedSettingInterface ccsMockCompositionIntegratedSetting =
{
    ccsMockCompositionIntegratedSettingReadValue,
    ccsMockCompositionIntegratedSettingWriteValue,
    ccsMockCompositionIntegratedSettingFree
};

const CCSIntegratedSettingInfoInterface ccsMockCompositionIntegratedSettingInfo =
{
    ccsMockCompositionIntegratedSettingInfoPluginName,
    ccsMockCompositionIntegratedSettingInfoSettingName,
    ccsMockCompositionIntegratedSettingInfoGetType,
    ccsMockCompositionIntegratedSettingInfoFree
};

CCSIntegratedSetting *
ccsMockCompositionIntegratedSettingNew (CCSIntegratedSetting          *integratedSetting,
					CCSGNOMEIntegratedSettingInfo *gnomeInfo,
					CCSIntegratedSettingInfo      *settingInfo,
					CCSObjectAllocationInterface  *allocator)
{
    CCSIntegratedSetting *composition = allocateCCSIntegratedSetting (allocator);

    if (!composition)
	return NULL;

    CCSMockGNOMEIntegratedSettingCompositionPrivate *priv = allocatePrivate (composition,
									     allocator);

    if (!priv)
	return NULL;

    const CCSInterface *integratedSettingImpl =
	    (const CCSInterface *) (&ccsMockCompositionIntegratedSetting);
    const CCSInterface *integratedSettingInfoImpl =
	    (const CCSInterface *) (&ccsMockCompositionIntegratedSettingInfo);
    const CCSInterface *gnomeSettingImpl =
	    (const CCSInterface *) (&ccsMockCompositionGNOMEIntegratedSettingInfo);

    priv->integratedSetting          = integratedSetting;
    priv->gnomeIntegratedSettingInfo = gnomeInfo;
    priv->integratedSettingInfo      = settingInfo;

    ccsIntegratedSettingRef (priv->integratedSetting);
    ccsGNOMEIntegratedSettingInfoRef (priv->gnomeIntegratedSettingInfo);

    ccsObjectSetPrivate (composition, (CCSPrivate *) (priv));
    ccsObjectAddInterface (composition,
			   integratedSettingImpl,
			   GET_INTERFACE_TYPE (CCSIntegratedSettingInterface));
    ccsObjectAddInterface (composition,
			   integratedSettingInfoImpl,
			   GET_INTERFACE_TYPE (CCSIntegratedSettingInfoInterface));
    ccsObjectAddInterface (composition,
			   gnomeSettingImpl,
			   GET_INTERFACE_TYPE (CCSGNOMEIntegratedSettingInfoInterface));

    ccsObjectRef (composition);

    return composition;
}

