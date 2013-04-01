/**
 *
 * GSettings libccs backend
 *
 * gsettings.c
 *
 * Copyright (c) 2011 Canonical Ltd
 *
 * Based on the original compizconfig-backend-gconf
 *
 * Copyright (c) 2007 Danny Baumann <maniac@opencompositing.org>
 *
 * Parts of this code are taken from libberylsettings 
 * gconf backend, written by:
 *
 * Copyright (c) 2006 Robert Carr <racarr@opencompositing.org>
 * Copyright (c) 2007 Dennis Kasprzyk <onestone@opencompositing.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Authored By:
 *	Sam Spilsbury <sam.spilsbury@canonical.com>
 *
 **/

#define CCS_LOG_DOMAIN "gsettings"

#include "gsettings.h"
#include "gsettings_shared.h"
#include "ccs_gsettings_backend_interface.h"
#include "ccs_gsettings_backend.h"
#include "ccs_gsettings_interface.h"
#include "ccs_gsettings_interface_wrapper.h"

GVariant *
getVariantForCCSSetting (CCSBackend *backend, CCSSetting *setting)
{
    CCSGSettingsWrapper  *settings = getSettingsObjectForCCSSetting (backend, setting);
    char *cleanSettingName = getNameForCCSSetting (setting);
    gchar *pathName = makeSettingPath (ccsGSettingsBackendGetCurrentProfile (backend), setting);
    GVariant *gsettingsValue = getVariantAtKey (settings,
						cleanSettingName,
						pathName,
						ccsSettingGetType (setting));
    free (cleanSettingName);
    g_free (pathName);
    return gsettingsValue;
}

static Bool
readIntegratedOption (CCSBackend *backend,
		      CCSSetting *setting,
		      CCSIntegratedSetting *integrated)
{
    return ccsGSettingsBackendReadIntegratedOption (backend, setting, integrated);
}

Bool
readOption (CCSBackend *backend, CCSSetting * setting)
{
    Bool       ret = FALSE;
    GVariant   *gsettingsValue = NULL;

    /* It is impossible for certain settings to have a schema,
     * such as actions and read only settings, so in that case
     * just return FALSE since compizconfig doesn't expect us
     * to read them anyways */
    if (!ccsSettingIsReadableByBackend (setting))
	return FALSE;

    gsettingsValue = getVariantForCCSSetting (backend, setting);

    switch (ccsSettingGetType (setting))
    {
    case TypeString:
	{
	    const char *value;
	    value = readStringFromVariant (gsettingsValue);
	    if (value)
	    {
		ccsSetString (setting, value, TRUE);
		ret = TRUE;
	    }
	}
	break;
    case TypeMatch:
	{
	    const char * value;
	    value = readStringFromVariant (gsettingsValue);
	    if (value)
	    {
		ccsSetMatch (setting, value, TRUE);
		ret = TRUE;
	    }
	}
	break;
    case TypeInt:
	{
	    int value;
	    value = readIntFromVariant (gsettingsValue);

	    ccsSetInt (setting, value, TRUE);
	    ret = TRUE;
	}
	break;
    case TypeBool:
	{
	    Bool value;
	    value = readBoolFromVariant (gsettingsValue);

	    ccsSetBool (setting, value, TRUE);
	    ret = TRUE;
	}
	break;
    case TypeFloat:
	{
	    float value;
	    value = readFloatFromVariant (gsettingsValue);

	    ccsSetFloat (setting, value, TRUE);
	    ret = TRUE;
	}
	break;
    case TypeColor:
	{
	    Bool success = FALSE;
	    CCSSettingColorValue color = readColorFromVariant (gsettingsValue, &success);

	    if (success)
	    {
		ccsSetColor (setting, color, TRUE);
		ret = TRUE;
	    }
	}
	break;
    case TypeKey:
	{
	    Bool success = FALSE;
	    CCSSettingKeyValue key = readKeyFromVariant (gsettingsValue, &success);

	    if (success)
	    {
		ccsSetKey (setting, key, TRUE);
		ret = TRUE;
	    }
	}
	break;
    case TypeButton:
	{
	    Bool success = FALSE;
	    CCSSettingButtonValue button = readButtonFromVariant (gsettingsValue, &success);

	    if (success)
	    {
		ccsSetButton (setting, button, TRUE);
		ret = TRUE;
	    }
	}
	break;
    case TypeEdge:
	{
	    unsigned int edges = readEdgeFromVariant (gsettingsValue);

	    ccsSetEdge (setting, edges, TRUE);
	    ret = TRUE;
	}
	break;
    case TypeBell:
	{
	    Bool value;
	    value = readBoolFromVariant (gsettingsValue);

	    ccsSetBell (setting, value, TRUE);
	    ret = TRUE;
	}
	break;
    case TypeList:
	{
	    CCSSettingValueList list = readListValue (gsettingsValue, setting, &ccsDefaultObjectAllocator);

	    if (list)
	    {
		CCSSettingValueList iter = list;

		while (iter)
		{
		    ((CCSSettingValue *) iter->data)->parent = setting;
		    iter = iter->next;
		}

		ccsSetList (setting, list, TRUE);
		ccsSettingValueListFree (list, TRUE);
		ret = TRUE;
	    }
	}
	break;
    default:
	ccsWarning ("Attempt to read unsupported setting type %d!",
		    ccsSettingGetType (setting));
	break;
    }

    g_variant_unref (gsettingsValue);

    return ret;
}

static void
writeIntegratedOption (CCSBackend *backend,
		       CCSSetting *setting,
		       CCSIntegratedSetting *integrated)
{
    ccsGSettingsBackendWriteIntegratedOption (backend, setting, integrated);
}

void
writeOption (CCSBackend *backend,
	     CCSSetting *setting)
{
    CCSGSettingsWrapper  *settings = getSettingsObjectForCCSSetting (backend, setting);
    char *cleanSettingName = translateKeyForGSettings (ccsSettingGetName (setting));
    GVariant *gsettingsValue = NULL;
    Bool success = FALSE;

    switch (ccsSettingGetType (setting))
    {
    case TypeString:
	{
	    char *value;
	    if (ccsGetString (setting, &value))
	    {
		success = writeStringToVariant (value, &gsettingsValue);
	    }
	}
	break;
    case TypeMatch:
	{
	    char *value;
	    if (ccsGetMatch (setting, &value))
	    {

		success = writeStringToVariant (value, &gsettingsValue);
	    }
	}
	break;
    case TypeFloat:
	{
	    float value;
	    if (ccsGetFloat (setting, &value))
	    {
		success = writeFloatToVariant (value, &gsettingsValue);
	    }
	}
	break;
    case TypeInt:
	{
	    int value;
	    if (ccsGetInt (setting, &value))
	    {
		success = writeIntToVariant (value, &gsettingsValue);
	    }
	}
	break;
    case TypeBool:
	{
	    Bool value;
	    if (ccsGetBool (setting, &value))
	    {
		success = writeBoolToVariant (value, &gsettingsValue);
	    }
	}
	break;
    case TypeColor:
	{
	    CCSSettingColorValue value;

	    if (!ccsGetColor (setting, &value))
		break;

	    success = writeColorToVariant (value, &gsettingsValue);
	}
	break;
    case TypeKey:
	{
	    CCSSettingKeyValue key;

	    if (!ccsGetKey (setting, &key))
		break;

	    success = writeKeyToVariant (key, &gsettingsValue);
	}
	break;
    case TypeButton:
	{
	    CCSSettingButtonValue button;

	    if (!ccsGetButton (setting, &button))
		break;

	    success = writeButtonToVariant (button, &gsettingsValue);
	}
	break;
    case TypeEdge:
	{
	    unsigned int edges;

	    if (!ccsGetEdge (setting, &edges))
		break;

	    success = writeEdgeToVariant (edges, &gsettingsValue);
	}
	break;
    case TypeBell:
	{
	    Bool value;
	    if (ccsGetBell (setting, &value))
	    {
		success = writeBoolToVariant (value, &gsettingsValue);
	    }
	}
	break;
    case TypeList:
	{
	    CCSSettingValueList  list = NULL;

	    if (!ccsGetList (setting, &list))
		return;

	    success = writeListValue (list,
				      ccsSettingGetInfo (setting)->forList.listType,
				      &gsettingsValue);
	}
	break;
    default:
	ccsWarning ("Attempt to write unsupported setting type %d",
	       ccsSettingGetType (setting));
	break;
    }

    if (success && gsettingsValue)
    {
	/* g_settings_set_value will consume the reference
	 * so there is no need to unref value here */
	writeVariantToKey (settings, cleanSettingName, gsettingsValue);
    }

    free (cleanSettingName);
}

Bool
readInit (CCSBackend *backend, CCSContext * context)
{
    return ccsGSettingsBackendUpdateProfile (backend, context);
}

void
readSetting (CCSBackend *backend,
	     CCSContext *context,
	     CCSSetting *setting)
{
    Bool status;
    CCSIntegratedSetting *integrated = ccsGSettingsBackendGetIntegratedSetting (backend, setting);

    if (ccsGetIntegrationEnabled (context) &&
	integrated)
    {
	status = readIntegratedOption (backend, setting, integrated);
    }
    else
	status = readOption (backend, setting);

    if (!status)
	ccsResetToDefault (setting, TRUE);
}

Bool
writeInit (CCSBackend *backend, CCSContext * context)
{
    return ccsGSettingsBackendUpdateProfile (backend, context);
}

void
writeSetting (CCSBackend *backend,
	      CCSContext *context,
	      CCSSetting *setting)
{
    CCSIntegratedSetting *integrated = ccsGSettingsBackendGetIntegratedSetting (backend, setting);

    if (ccsGetIntegrationEnabled (context) &&
	integrated)
    {
	writeIntegratedOption (backend, setting, integrated);
    }
    else if (ccsSettingGetIsDefault (setting))
    {
	resetOptionToDefault (backend, setting);
    }
    else
	writeOption (backend, setting);

}

static void
updateSetting (CCSBackend *backend, CCSContext *context, CCSPlugin *plugin, CCSSetting *setting)
{
    CCSIntegratedSetting *integrated = ccsGSettingsBackendGetIntegratedSetting (backend, setting);

    ccsBackendReadInit (backend, context);
    if (!readOption (backend, setting))
    {
	ccsResetToDefault (setting, TRUE);
    }

    if (ccsGetIntegrationEnabled (context) &&
	integrated)
    {
	ccsBackendWriteInit (backend, context);
	ccsGSettingsBackendWriteIntegratedOption (backend, setting, integrated);
    }
}

static void
processEvents (CCSBackend *backend, unsigned int flags)
{
    if (!(flags & ProcessEventsNoGlibMainLoopMask))
    {
	while (g_main_context_pending(NULL))
	    g_main_context_iteration(NULL, FALSE);
    }
}

static Bool
initBackend (CCSBackend *backend, CCSContext * context)
{
    g_type_init ();

    return ccsGSettingsBackendAttachNewToBackend (backend, context);
}

static Bool
finiBackend (CCSBackend *backend)
{
    ccsGSettingsBackendDetachFromBackend (backend);

    return TRUE;
}

static Bool
getSettingIsIntegrated (CCSBackend *backend, CCSSetting * setting)
{
    if (!ccsGetIntegrationEnabled (ccsPluginGetContext (ccsSettingGetParent (setting))))
	return FALSE;

    if (!ccsGSettingsBackendGetIntegratedSetting (backend, setting))
	return FALSE;

    return TRUE;
}

static Bool
getSettingIsReadOnly (CCSBackend *backend, CCSSetting * setting)
{
    /* FIXME */
    return FALSE;
}

static const CCSBackendInfo *
getInfo (CCSBackend *backend)
{
    return &gsettingsBackendInfo;
}

static Bool
ccsGSettingsWrapDeleteProfile (CCSBackend *backend,
			       CCSContext *context,
			       char       *profile)
{
    return deleteProfile (backend, context, profile);
}

static CCSBackendInterface gsettingsVTable = {
    getInfo,
    processEvents,
    initBackend,
    finiBackend,
    readInit,
    readSetting,
    0,
    writeInit,
    writeSetting,
    0,
    updateSetting,
    getSettingIsIntegrated,
    getSettingIsReadOnly,
    ccsGSettingsGetExistingProfiles,
    ccsGSettingsWrapDeleteProfile,
    ccsGSettingsSetIntegration
};

CCSBackendInterface *
getBackendInfo (void)
{
    return &gsettingsVTable;
}
