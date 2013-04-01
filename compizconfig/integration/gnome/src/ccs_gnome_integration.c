/**
 *
 * compizconfig gnome integration backend
 *
 * gnome-integration.c
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
#include <stdlib.h>
#include <string.h>
#include <ccs.h>
#include <ccs-backend.h>
#include <ccs-object.h>
#include <gio/gio.h>
#include "ccs_gnome_integration.h"
#include "ccs_gnome_integration_constants.h"
#include "ccs_gnome_integrated_setting.h"

typedef struct _CCSGNOMEIntegrationBackendPrivate CCGNOMEIntegrationBackendPrivate;

struct _CCSGNOMEIntegrationBackendPrivate
{
    CCSBackend *backend;
    CCSContext *context;
    CCSIntegratedSettingFactory  *factory;
    CCSIntegratedSettingsStorage *storage;
    Bool       noWrites;
};

static CCSSetting *
findDisplaySettingForPlugin (CCSContext *context,
			     const char *plugin,
			     const char *setting)
{
    CCSPlugin  *p;
    CCSSetting *s;

    p = ccsFindPlugin (context, plugin);
    if (!p)
	return NULL;

    s = ccsFindSetting (p, setting);
    if (!s)
	return NULL;

    return s;
}

static void
unregisterAllIntegratedOptions (CCSIntegration *integration)
{
    CCGNOMEIntegrationBackendPrivate *priv = (CCGNOMEIntegrationBackendPrivate *) ccsObjectGetPrivate (integration);

    if (priv->storage)
	ccsIntegratedSettingsStorageUnref (priv->storage);

    if (priv->factory)
	ccsIntegratedSettingFactoryUnref (priv->factory);

    priv->storage = NULL;
    priv->factory = NULL;
}

static void
registerAllIntegratedOptions (CCSIntegration *integration)
{
    CCGNOMEIntegrationBackendPrivate *priv = (CCGNOMEIntegrationBackendPrivate *) ccsObjectGetPrivate (integration);

    unsigned int i = 0;
    const CCSGNOMEIntegratedSettingsList *array = ccsGNOMEIntegratedSettingsList ();

    for (; i < CCS_GNOME_INTEGRATED_SETTINGS_LIST_SIZE; i++)
    {
	CCSIntegratedSetting *setting = ccsIntegratedSettingFactoryCreateIntegratedSettingForCCSSettingNameAndType (priv->factory,
														    integration,
														    array[i].pluginName,
														    array[i].settingName,
														    TypeInt);

	ccsIntegratedSettingsStorageAddSetting (priv->storage, setting);
    }
}

static CCSIntegratedSetting *
ccsGNOMEIntegrationBackendGetIntegratedSetting (CCSIntegration *integration,
						const char		  *pluginName,
						const char		  *settingName)
{
    CCGNOMEIntegrationBackendPrivate *priv = (CCGNOMEIntegrationBackendPrivate *) ccsObjectGetPrivate (integration);

    if (ccsIntegratedSettingsStorageEmpty (priv->storage))
	registerAllIntegratedOptions (integration);

    CCSIntegratedSettingList integratedSettings = ccsIntegratedSettingsStorageFindMatchingSettingsByPluginAndSettingName (priv->storage,
															  pluginName,
															  settingName);

    if (integratedSettings)
	return integratedSettings->data;

    return NULL;
}

static unsigned int
getGnomeMouseButtonModifier (CCSIntegratedSetting *mouseButtonModifierSetting)
{
    unsigned int modMask = 0;
    CCSSettingType type = TypeString;
    CCSSettingValue *v = ccsIntegratedSettingReadValue (mouseButtonModifierSetting, type);

    if (v)
    {
	modMask = ccsStringToModifiers (v->value.asString);

	ccsFreeSettingValueWithType (v, type);
    }

    return modMask;
}

static unsigned int
getButtonBindingForSetting (CCSContext   *context,
			    const char   *plugin,
			    const char   *setting)
{
    CCSSetting *s;

    s = findDisplaySettingForPlugin (context, plugin, setting);
    if (!s)
	return 0;

    if (ccsSettingGetType (s) != TypeButton)
	return 0;

    return ccsSettingGetValue (s)->value.asButton.button;
}

static Bool
ccsGNOMEIntegrationBackendReadISAndSetSettingForType (CCSIntegratedSetting *integratedSetting,
						      CCSSetting           *setting,
						      CCSSettingValue      **v,
						      CCSSettingType       type)
{
    *v = ccsIntegratedSettingReadValue (integratedSetting, type);

    if (*v != NULL && (*v)->value.asString)
    {
	CCSSettingKeyValue key;

	memset (&key, 0, sizeof (CCSSettingKeyValue));
	if (ccsStringToKeyBinding ((*v)->value.asString, &key))
	{
	    ccsSetKey (setting, key, TRUE);
	    return TRUE;
	}
    }

    return FALSE;
}

static Bool
ccsGNOMEIntegrationBackendReadOptionIntoSetting (CCSIntegration *integration,
						 CCSContext	       *context,
						 CCSSetting	       *setting,
						 CCSIntegratedSetting   *integratedSetting)
{
    Bool       ret = FALSE;
    CCSSettingValue *v = NULL;
    CCSSettingType  type = TypeNum;

    CCGNOMEIntegrationBackendPrivate *priv = (CCGNOMEIntegrationBackendPrivate *) ccsObjectGetPrivate (integration);
    
    if (ccsIntegratedSettingsStorageEmpty (priv->storage))
	registerAllIntegratedOptions (integration);

    ret = ccsSettingIsReadableByBackend (setting);

    if (!ret)
	return FALSE;

    switch (ccsGNOMEIntegratedSettingInfoGetSpecialOptionType ((CCSGNOMEIntegratedSettingInfo *) integratedSetting)) {
    case OptionInt:
	{
	    type = TypeInt;
	    v = ccsIntegratedSettingReadValue (integratedSetting, type);
	    ccsSetInt (setting, v->value.asInt, TRUE);
	    ret = TRUE;
	}
	break;
    case OptionBool:
	{
	    type = TypeBool;
	    v = ccsIntegratedSettingReadValue (integratedSetting, type);
	    ccsSetBool (setting, v->value.asBool, TRUE);
	    ret = TRUE;
	}
	break;
    case OptionString:
	{
	    type = TypeString;
	    v = ccsIntegratedSettingReadValue (integratedSetting, type);
	    char *str = v->value.asString;

	    ccsSetString (setting, str, TRUE);
	    ret = TRUE;
	}
	break;
    case OptionKey:
	{
	    type = TypeKey;
	    if (ccsGNOMEIntegrationBackendReadISAndSetSettingForType (integratedSetting,
								      setting,
								      &v,
								      type))
		ret = TRUE;
	}
	break;
    case OptionSpecial:
	{
	    const char *settingName = ccsSettingGetName (setting);
	    const char *pluginName  = ccsPluginGetName (ccsSettingGetParent (setting));

	    if (strcmp (settingName, "current_viewport") == 0)
	    {
		type = TypeBool;
		v = ccsIntegratedSettingReadValue (integratedSetting, type);

		Bool showAll = v->value.asBool;
		ccsSetBool (setting, !showAll, TRUE);
		ret = TRUE;
	    }
	    else if (strcmp (settingName, "fullscreen_visual_bell") == 0)
	    {
		type = TypeString;
		v = ccsIntegratedSettingReadValue (integratedSetting, type);

		const char *value = v->value.asString;
		if (value)
		{
		    Bool fullscreen;

		    fullscreen = strcmp (value, "fullscreen") == 0;
		    ccsSetBool (setting, fullscreen, TRUE);
		    ret = TRUE;
		}
	    }
	    else if (strcmp (settingName, "click_to_focus") == 0)
	    {
		type = TypeString;
		v = ccsIntegratedSettingReadValue (integratedSetting, type);

		const char *focusMode = v->value.asString;

		if (focusMode)
		{
		    Bool clickToFocus = (strcmp (focusMode, "click") == 0);
		    ccsSetBool (setting, clickToFocus, TRUE);
		    ret = TRUE;
		}
	    }
	    else if ((strcmp (settingName, "run_command_screenshot_key") == 0 ||
		      strcmp (settingName, "run_command_window_screenshot_key") == 0 ||
		      strcmp (settingName, "run_command_terminal_key") == 0))
	    {
		type = TypeString;
		if (ccsGNOMEIntegrationBackendReadISAndSetSettingForType (integratedSetting,
									  setting,
									  &v,
									  type))
		    ret = TRUE;
	    }
	    else if (((strcmp (settingName, "initiate_button") == 0) &&
		      ((strcmp (pluginName, "move") == 0) ||
		       (strcmp (pluginName, "resize") == 0))) ||
		      ((strcmp (settingName, "window_menu_button") == 0) &&
		       (strcmp (pluginName, "core") == 0)))
	    {
		gboolean              resizeWithRightButton;
		CCSSettingButtonValue button;

		memset (&button, 0, sizeof (CCSSettingButtonValue));
		ccsGetButton (setting, &button);

		CCSIntegratedSettingList integratedSettingsMBM = ccsIntegratedSettingsStorageFindMatchingSettingsByPluginAndSettingName (priv->storage,
														ccsGNOMEIntegratedPluginNames.SPECIAL,
														ccsGNOMEIntegratedSettingNames.NULL_MOUSE_BUTTON_MODIFIER.compizName);

		button.buttonModMask = getGnomeMouseButtonModifier (integratedSettingsMBM->data);

		CCSIntegratedSettingList integratedSettings = ccsIntegratedSettingsStorageFindMatchingSettingsByPluginAndSettingName (priv->storage,
														ccsGNOMEIntegratedPluginNames.SPECIAL,
														ccsGNOMEIntegratedSettingNames.NULL_RESIZE_WITH_RIGHT_BUTTON.compizName);

		type = TypeBool;
		v = ccsIntegratedSettingReadValue (integratedSettings->data, type);

		resizeWithRightButton =
		   v->value.asBool;

		if (strcmp (settingName, "window_menu_button") == 0)
		    button.button = resizeWithRightButton ? 2 : 3;
		else if (strcmp (pluginName, "resize") == 0)
		    button.button = resizeWithRightButton ? 3 : 2;
		else
		    button.button = 1;

		ccsSetButton (setting, button, TRUE);
		ret = TRUE;
	    }

	}
     	break;
    default:
	break;
    }

    if (v)
	ccsFreeSettingValueWithType (v, type);

    return ret;
}

static Bool
setGnomeMouseButtonModifier (CCSIntegratedSetting *setting,
			     unsigned int modMask)
{
    char   *modifiers;
    CCSSettingValue *v = calloc (1, sizeof (CCSSettingValue));

    v->isListChild = FALSE;
    v->parent = NULL;
    v->refCount = 1;

    modifiers = ccsModifiersToString (modMask);
    if (!modifiers)
    {
	ccsFreeSettingValueWithType (v, TypeString);
	return FALSE;
    }

    v->value.asString = modifiers;
    ccsIntegratedSettingWriteValue (setting, v, TypeString);

    ccsFreeSettingValueWithType (v, TypeString);

    return TRUE;
}

static void
setButtonBindingForSetting (CCSContext   *context,
			    const char   *plugin,
			    const char   *setting,
			    unsigned int button,
			    unsigned int buttonModMask)
{
    CCSSetting            *s;
    CCSSettingButtonValue value;

    s = findDisplaySettingForPlugin (context, plugin, setting);
    if (!s)
	return;

    if (ccsSettingGetType (s) != TypeButton)
	return;

    value = ccsSettingGetValue (s)->value.asButton;

    if ((value.button != button) || (value.buttonModMask != buttonModMask))
    {
	value.button = button;
	value.buttonModMask = buttonModMask;

	ccsSetButton (s, value, TRUE);
    }
}

static Bool
ccsGNOMEIntegrationBackendKeyValueToStringValue (CCSSettingValue *keyValue,
						 CCSSettingValue *stringValue)
{
    char  *newValue;

    newValue = ccsKeyBindingToString (&keyValue->value.asKey);
    if (newValue)
    {
	if (strcmp (newValue, "Disabled") == 0)
	{
	    /* Metacity doesn't like "Disabled", it wants "disabled" */
	    newValue[0] = 'd';
	}

	stringValue->value.asString = newValue;

	return TRUE;
    }

    return FALSE;
}

static void
ccsGNOMEIntegrationBackendWriteOptionFromSetting (CCSIntegration *integration,
						  CCSContext		 *context,
						  CCSSetting		 *setting,
						  CCSIntegratedSetting   *integratedSetting)
{
    GError     *err = NULL;
    CCSSettingType type = TypeNum;

    CCGNOMEIntegrationBackendPrivate *priv = (CCGNOMEIntegrationBackendPrivate *) ccsObjectGetPrivate (integration);

    if (ccsIntegratedSettingsStorageEmpty (priv->storage))
	registerAllIntegratedOptions (integration);

    if (priv->noWrites)
	return;

    /* Do not allow recursing back into writeIntegratedSetting */
    ccsIntegrationDisallowIntegratedWrites (integration);

    CCSSettingType sType = ccsSettingGetType (setting);
    CCSSettingInfo *sInfo = ccsSettingGetInfo (setting);

    CCSSettingValue *vSetting = ccsSettingGetValue (setting);
    CCSSettingValue *v = ccsCopyValue (vSetting, sType, sInfo);

    if (!v)
	return;

    switch (ccsGNOMEIntegratedSettingInfoGetSpecialOptionType ((CCSGNOMEIntegratedSettingInfo *) integratedSetting))
    {
    case OptionInt:
	ccsIntegratedSettingWriteValue (integratedSetting, v, TypeInt);
	break;
    case OptionBool:
	ccsIntegratedSettingWriteValue (integratedSetting, v, TypeBool);
	break;
    case OptionString:
	ccsIntegratedSettingWriteValue (integratedSetting, v, TypeString);
	break;
    case OptionKey:
	{   
	    CCSSettingValue *newValue = calloc (1, sizeof (CCSSettingValue));

	    newValue->isListChild = FALSE;
	    newValue->parent = NULL;
	    newValue->refCount = 1;

	    if (ccsGNOMEIntegrationBackendKeyValueToStringValue (v, newValue))
	    {
		/* Really this is a lie - the writer expects a string
		 * but it needs to know if its a key or a string */
		type = TypeKey;
		ccsIntegratedSettingWriteValue (integratedSetting, newValue, type);
	    }

	    if (newValue)
		ccsFreeSettingValueWithType (newValue, TypeString);
	}
	break;
    case OptionSpecial:
	{
	    const char *settingName = ccsSettingGetName (setting);
	    const char *pluginName  = ccsPluginGetName (ccsSettingGetParent (setting));
	    CCSSettingValue *newValue = calloc (1, sizeof (CCSSettingValue));

	    newValue->isListChild = FALSE;
	    newValue->parent = NULL;
	    newValue->refCount = 1;

	    if (strcmp (settingName, "current_viewport") == 0)
	    {
		newValue->value.asBool = !v->value.asBool;
		type = TypeBool;

		ccsIntegratedSettingWriteValue (integratedSetting, newValue, type);
	    }
	    else if (strcmp (settingName, "fullscreen_visual_bell") == 0)
	    {
		const char *newValueString = v->value.asBool ? "fullscreen" : "frame_flash";
		newValue->value.asString = strdup (newValueString);
		type = TypeString;

		ccsIntegratedSettingWriteValue (integratedSetting, newValue, type);
	    }
	    else if (strcmp (settingName, "click_to_focus") == 0)
	    {
		const char *newValueString = v->value.asBool ? "click" : "sloppy";
		newValue->value.asString = strdup (newValueString);
		type = TypeString;

		ccsIntegratedSettingWriteValue (integratedSetting, newValue, type);
	    }
	    else if ((strcmp (settingName, "run_command_screenshot_key") == 0 ||
		      strcmp (settingName, "run_command_window_screenshot_key") == 0 ||
		      strcmp (settingName, "run_command_terminal_key") == 0))
	    {
		if (ccsGNOMEIntegrationBackendKeyValueToStringValue (v, newValue))
		{
		    /* These are actually stored as strings in the schemas */
		    type = TypeString;
		    ccsIntegratedSettingWriteValue (integratedSetting, newValue, type);
		}

	    }
	    else if (((strcmp (settingName, "initiate_button") == 0) &&
		      ((strcmp (pluginName, "move") == 0) ||
		       (strcmp (pluginName, "resize") == 0))) ||
		      ((strcmp (settingName, "window_menu_button") == 0) &&
		       (strcmp (pluginName, "core") == 0)))
	    {
		unsigned int modMask;
		Bool         resizeWithRightButton = FALSE;

		if ((getButtonBindingForSetting (priv->context, "resize",
						 "initiate_button") == 3) ||
		    (getButtonBindingForSetting (priv->context, "core",
						 "window_menu_button") == 2))
		{
		     resizeWithRightButton = TRUE;
		}

		CCSIntegratedSettingList integratedSettings = ccsIntegratedSettingsStorageFindMatchingSettingsByPluginAndSettingName (priv->storage,
														ccsGNOMEIntegratedPluginNames.SPECIAL,
														ccsGNOMEIntegratedSettingNames.NULL_RESIZE_WITH_RIGHT_BUTTON.compizName);

		newValue->value.asBool = resizeWithRightButton;
		type = TypeBool;

		ccsIntegratedSettingWriteValue (integratedSettings->data, newValue, type);

		CCSIntegratedSettingList integratedSettingsMBM = ccsIntegratedSettingsStorageFindMatchingSettingsByPluginAndSettingName (priv->storage,
														ccsGNOMEIntegratedPluginNames.SPECIAL,
														ccsGNOMEIntegratedSettingNames.NULL_MOUSE_BUTTON_MODIFIER.compizName);

		modMask = v->value.asButton.buttonModMask;
		if (setGnomeMouseButtonModifier (integratedSettingsMBM->data, modMask))
		{
		    setButtonBindingForSetting (priv->context, "move",
						"initiate_button", 1, modMask);
		    setButtonBindingForSetting (priv->context, "resize",
						"initiate_button", 
						resizeWithRightButton ? 3 : 2,
						modMask);
		    setButtonBindingForSetting (priv->context, "core",
						"window_menu_button",
						resizeWithRightButton ? 2 : 3,
						modMask);
		}
	    }

	    if (newValue)
		ccsFreeSettingValueWithType (newValue, type);
	}
     	break;
    }

    if (err)
    {
	ccsError ("%s", err->message);
	g_error_free (err);
    }

    if (v)
	ccsFreeSettingValueWithType (v, sType);

    /* we should immediately write changed settings */
    ccsWriteChangedSettings (priv->context);
    ccsIntegrationAllowIntegratedWrites (integration);
}

static void
ccsGNOMEIntegrationBackendUpdateIntegratedSettings (CCSIntegration *integration,
						    CCSContext	 *context,
						    CCSIntegratedSettingList integratedSettings)
{
    CCGNOMEIntegrationBackendPrivate *priv = (CCGNOMEIntegrationBackendPrivate *) ccsObjectGetPrivate (integration);
    Bool needInit = TRUE;

    CCSIntegratedSettingList iter = integratedSettings;

    while (iter)
    {
	CCSIntegratedSetting *integrated = iter->data;
	const char           *settingName = ccsIntegratedSettingInfoSettingName ((CCSIntegratedSettingInfo *)integrated);
	const char	     *pluginName = ccsIntegratedSettingInfoPluginName ((CCSIntegratedSettingInfo *) integrated);

	/* Special case for mouse button modifier etc */
	if ((strcmp (settingName,
		     "mouse_button_modifier") == 0) ||
	    (strcmp (settingName,
		    "resize_with_right_button") == 0))
	{
	    CCSSetting *s;

	    if (needInit)
	    {
		ccsBackendReadInit (priv->backend, priv->context);
		needInit = FALSE;
	    }

	    s = findDisplaySettingForPlugin (priv->context, "core",
					     "window_menu_button");
	    if (s)
		ccsBackendReadSetting (priv->backend, priv->context, s);

	    s = findDisplaySettingForPlugin (priv->context, "move",
					     "initiate_button");
	    if (s)
		ccsBackendReadSetting (priv->backend, priv->context, s);

	    s = findDisplaySettingForPlugin (priv->context, "resize",
					     "initiate_button");
	    if (s)
		ccsBackendReadSetting (priv->backend, priv->context, s);
	}
	else
	{
	    CCSPlugin     *plugin = NULL;
	    CCSSetting    *setting;

	    plugin = ccsFindPlugin (priv->context, pluginName);
	    if (plugin)
	    {
		setting = ccsFindSetting (plugin, settingName);

		if (setting)
		{
		    if (needInit)
		    {
			ccsBackendReadInit (priv->backend, priv->context);
			needInit = FALSE;
		    }

		    ccsBackendReadSetting (priv->backend, priv->context, setting);
		}
	    }
	}

	iter = iter->next;
    }
}

static void
ccsGNOMEIntegrationDisallowIntegratedWrites (CCSIntegration *integration)
{
    CCGNOMEIntegrationBackendPrivate *priv = (CCGNOMEIntegrationBackendPrivate *) ccsObjectGetPrivate (integration);
    priv->noWrites = TRUE;
}

static void
ccsGNOMEIntegrationAllowIntegratedWrites (CCSIntegration *integration)
{
    CCGNOMEIntegrationBackendPrivate *priv = (CCGNOMEIntegrationBackendPrivate *) ccsObjectGetPrivate (integration);
    priv->noWrites = FALSE;
}

static void
ccsGNOMEIntegrationBackendFree (CCSIntegration *integration)
{
    CCGNOMEIntegrationBackendPrivate *priv = (CCGNOMEIntegrationBackendPrivate *) ccsObjectGetPrivate (integration);

    unregisterAllIntegratedOptions (integration);

    priv->backend = NULL;

    ccsObjectFinalize (integration);
    free (integration);
}

const CCSIntegrationInterface ccsGNOMEIntegrationBackendInterface =
{
    ccsGNOMEIntegrationBackendGetIntegratedSetting,
    ccsGNOMEIntegrationBackendReadOptionIntoSetting,
    ccsGNOMEIntegrationBackendWriteOptionFromSetting,
    ccsGNOMEIntegrationBackendUpdateIntegratedSettings,
    ccsGNOMEIntegrationDisallowIntegratedWrites,
    ccsGNOMEIntegrationAllowIntegratedWrites,
    ccsGNOMEIntegrationBackendFree
};

static CCGNOMEIntegrationBackendPrivate *
addPrivate (CCSIntegration *backend,
	    CCSObjectAllocationInterface *ai)
{
    CCGNOMEIntegrationBackendPrivate *priv = (*ai->calloc_) (ai->allocator, 1, sizeof (CCGNOMEIntegrationBackendPrivate));

    if (!priv)
    {
	ccsObjectFinalize (backend);
	free (backend);
    }

    ccsObjectSetPrivate (backend, (CCSPrivate *) priv);

    return priv;
}

static CCSIntegration *
ccsGNOMEIntegrationBackendNewCommon (CCSBackend *backend,
				     CCSContext *context,
				     CCSIntegratedSettingFactory *factory,
				     CCSIntegratedSettingsStorage *storage,
				     CCSObjectAllocationInterface *ai)
{
    CCSIntegration *integration = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSIntegration));

    if (!integration)
	return NULL;

    ccsObjectInit (integration, ai);

    CCGNOMEIntegrationBackendPrivate *priv = addPrivate (integration, ai);
    priv->backend = backend;
    priv->context = context;
    priv->factory = factory;
    priv->storage = storage;
    priv->noWrites = FALSE;

    ccsObjectAddInterface (integration,
			   (const CCSInterface *) &ccsGNOMEIntegrationBackendInterface,
			   GET_INTERFACE_TYPE (CCSIntegrationInterface));

    ccsIntegrationRef (integration);

    return integration;
}

CCSIntegration *
ccsGNOMEIntegrationBackendNew (CCSBackend *backend,
			       CCSContext *context,
			       CCSIntegratedSettingFactory *factory,
			       CCSIntegratedSettingsStorage *storage,
			       CCSObjectAllocationInterface *ai)
{
    return ccsGNOMEIntegrationBackendNewCommon (backend, context, factory, storage, ai);
}
