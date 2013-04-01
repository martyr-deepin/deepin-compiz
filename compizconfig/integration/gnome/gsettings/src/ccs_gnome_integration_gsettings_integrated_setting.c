#include <stdlib.h>
#include <string.h>

#include <gio/gio.h>

#include <ccs.h>
#include <ccs-backend.h>
#include <ccs-object.h>

#include <ccs_gsettings_interface.h>
#include <gsettings_shared.h>

#include "ccs_gnome_integration_gsettings_integrated_setting.h"
#include "ccs_gnome_integrated_setting.h"
#include "ccs_gnome_integration_constants.h"


/* CCSGSettingsIntegratedSetting implementation */
typedef struct _CCSGSettingsIntegratedSettingPrivate CCSGSettingsIntegratedSettingPrivate;

struct _CCSGSettingsIntegratedSettingPrivate
{
    CCSGNOMEIntegratedSettingInfo *gnomeIntegratedSetting;
    CCSGSettingsWrapper	      *wrapper;
};

char *
ccsGSettingsIntegratedSettingsTranslateOldGNOMEKeyForGSettings (const char *key)
{
    char *newKey = translateKeyForGSettings (key);

    if (g_strcmp0 (newKey, "run-command-screenshot") == 0)
    {
	free (newKey);
	newKey = strdup ("screenshot");
    }
    else if (g_strcmp0 (newKey, "run-command-window-screenshot") == 0)
    {
	free (newKey);
	newKey = strdup ("window-screenshot");
    }
    else if (g_strcmp0 (newKey, "run-command-terminal") == 0)
    {
	free (newKey);
	newKey = strdup ("terminal");
    }

    return newKey;
}

SpecialOptionType
ccsGSettingsIntegratedSettingGetSpecialOptionType (CCSGNOMEIntegratedSettingInfo *setting)
{
    CCSGSettingsIntegratedSettingPrivate *priv = (CCSGSettingsIntegratedSettingPrivate *) ccsObjectGetPrivate (setting);

    return ccsGNOMEIntegratedSettingInfoGetSpecialOptionType (priv->gnomeIntegratedSetting);
}

const char *
ccsGSettingsIntegratedSettingGetGNOMEName (CCSGNOMEIntegratedSettingInfo *setting)
{
    CCSGSettingsIntegratedSettingPrivate *priv = (CCSGSettingsIntegratedSettingPrivate *) ccsObjectGetPrivate (setting);

    return ccsGNOMEIntegratedSettingInfoGetGNOMEName (priv->gnomeIntegratedSetting);
}

CCSSettingValue *
ccsGSettingsIntegratedSettingReadValue (CCSIntegratedSetting *setting, CCSSettingType type)
{
    CCSGSettingsIntegratedSettingPrivate *priv = (CCSGSettingsIntegratedSettingPrivate *) ccsObjectGetPrivate (setting);
    CCSSettingValue		     *v = calloc (1, sizeof (CCSSettingValue));
    const char			     *gnomeKeyName = ccsGNOMEIntegratedSettingInfoGetGNOMEName ((CCSGNOMEIntegratedSettingInfo *) setting);
    char			     *gsettingsTranslatedName = ccsGSettingsIntegratedSettingsTranslateOldGNOMEKeyForGSettings (gnomeKeyName);

    v->isListChild = FALSE;
    v->parent = NULL;
    v->refCount = 1;

    GVariant *variant = ccsGSettingsWrapperGetValue (priv->wrapper, gsettingsTranslatedName);

    if (!variant)
    {
	free (gsettingsTranslatedName);
	free (v);
	return NULL;
    }

    const GVariantType *variantType = G_VARIANT_TYPE (g_variant_get_type_string (variant));

    switch (type)
    {
	case TypeInt:
	    if (!g_variant_type_equal (variantType, G_VARIANT_TYPE_INT32))
	    {
		ccsError ("Expected integer value");
		free (v);
		v = NULL;
		break;
	    }

	    v->value.asInt = readIntFromVariant (variant);
	    break;
	case TypeBool:
	    if (!g_variant_type_equal (variantType, G_VARIANT_TYPE_BOOLEAN))
	    {
		ccsError ("Expected boolean value");
		free (v);
		v = NULL;
		break;
	    }

	    v->value.asBool = readBoolFromVariant (variant);
	    break;
	case TypeString:
	{
	    if (!g_variant_type_equal (variantType, G_VARIANT_TYPE_STRING))
	    {
		ccsError ("Expected string value");
		free (v);
		v = NULL;
		break;
	    }

	    const char *str = readStringFromVariant (variant);
	    v->value.asString = strdup (str ? str : "");
	    break;
	}
	case TypeKey:
	{
	    if (!g_variant_type_equal (variantType, G_VARIANT_TYPE ("as")))
	    {
		ccsError ("Expected array-of-string value");
		free (v);
		v = NULL;
		break;
	    }

	    gsize len;
	    const gchar **strv = g_variant_get_strv (variant, &len);

	    if (strv)
		v->value.asString = strdup (strv[0] ? strv[0] : "");
	    else
		v->value.asString = strdup ("");

	    g_free (strv);
	    break;
	}
	default:
	    g_assert_not_reached ();
    }

    g_variant_unref (variant);
    free (gsettingsTranslatedName);

    return v;
}

void
ccsGSettingsIntegratedSettingWriteValue (CCSIntegratedSetting *setting, CCSSettingValue *v, CCSSettingType type)
{
    CCSGSettingsIntegratedSettingPrivate *priv = (CCSGSettingsIntegratedSettingPrivate *) ccsObjectGetPrivate (setting);
    const char			     *gnomeKeyName = ccsGNOMEIntegratedSettingInfoGetGNOMEName ((CCSGNOMEIntegratedSettingInfo *) setting);
    char			     *gsettingsTranslatedName = ccsGSettingsIntegratedSettingsTranslateOldGNOMEKeyForGSettings (gnomeKeyName);

    GVariant           *variant = ccsGSettingsWrapperGetValue (priv->wrapper, gsettingsTranslatedName);
    const GVariantType *variantType = g_variant_get_type (variant);
    GVariant           *newVariant = NULL;

    if (!variant)
    {
	ccsError ("NULL encountered while reading GSettings value");
	free (gsettingsTranslatedName);
	return;
    }

    switch (type)
    {
	case TypeInt:
	    {
		if (!g_variant_type_equal (variantType, G_VARIANT_TYPE_INT32))
		    ccsError ("Expected integer value");
		else
		{
		    int currentValue = readIntFromVariant (variant);

		    if ((currentValue != v->value.asInt))
			writeIntToVariant (v->value.asInt, &newVariant);
		}
	    }
	    break;
	case TypeBool:
	    {
		if (!g_variant_type_equal (variantType, G_VARIANT_TYPE_BOOLEAN))
		    ccsError ("Expected boolean value");
		else
		{
		    gboolean currentValue = readBoolFromVariant (variant);

		    if ((currentValue != v->value.asBool))
			writeBoolToVariant (v->value.asBool, &newVariant);
		}
	    }
	    break;
	case TypeString:
	    {
		if (!g_variant_type_equal (variantType, G_VARIANT_TYPE_STRING))
		    ccsError ("Expected string value");
		else
		{
		    const char  *defaultValue = "";
		    const char  *newValue = v->value.asString ? v->value.asString : defaultValue;
		    gsize len = 0;
		    const gchar *currentValue = g_variant_get_string (variant, &len);

		    if (currentValue)
		    {
			if (strcmp (currentValue, newValue) != 0)
			    writeStringToVariant (newValue, &newVariant);
		    }
		}
	    }
	    break;
	case TypeKey:
	    {
		if (!g_variant_type_equal (variantType, G_VARIANT_TYPE ("as")))
		    ccsError ("Expected array-of-string value");
		else
		{
		    const char  *defaultValue = "";
		    GVariantBuilder strvBuilder;

		    g_variant_builder_init (&strvBuilder, G_VARIANT_TYPE ("as"));
		    g_variant_builder_add (&strvBuilder, "s", v->value.asString ? v->value.asString :  defaultValue);
		    newVariant = g_variant_builder_end (&strvBuilder);
		}
	    }
	    break;
	default:
	    g_assert_not_reached ();
	    break;
    }

    /* g_settings_set_value consumes the reference */
    if (newVariant)
	ccsGSettingsWrapperSetValue (priv->wrapper, gsettingsTranslatedName, newVariant);
    else
	ccsGSettingsWrapperResetKey (priv->wrapper, gsettingsTranslatedName);

    g_variant_unref (variant);
    free (gsettingsTranslatedName);
}

const char *
ccsGSettingsIntegratedSettingInfoPluginName (CCSIntegratedSettingInfo *setting)
{
    CCSGSettingsIntegratedSettingPrivate *priv = (CCSGSettingsIntegratedSettingPrivate *) ccsObjectGetPrivate (setting);

    return ccsIntegratedSettingInfoPluginName ((CCSIntegratedSettingInfo *) priv->gnomeIntegratedSetting);
}

const char *
ccsGSettingsIntegratedSettingInfoSettingName (CCSIntegratedSettingInfo *setting)
{
    CCSGSettingsIntegratedSettingPrivate *priv = (CCSGSettingsIntegratedSettingPrivate *) ccsObjectGetPrivate (setting);

    return ccsIntegratedSettingInfoSettingName ((CCSIntegratedSettingInfo *) priv->gnomeIntegratedSetting);
}

CCSSettingType
ccsGSettingsIntegratedSettingInfoGetType (CCSIntegratedSettingInfo *setting)
{
    CCSGSettingsIntegratedSettingPrivate *priv = (CCSGSettingsIntegratedSettingPrivate *) ccsObjectGetPrivate (setting);

    return ccsIntegratedSettingInfoGetType ((CCSIntegratedSettingInfo *) priv->gnomeIntegratedSetting);
}

void
ccsGSettingsIntegratedSettingFree (CCSIntegratedSetting *setting)
{
    CCSGSettingsIntegratedSettingPrivate *priv = (CCSGSettingsIntegratedSettingPrivate *) ccsObjectGetPrivate (setting);

    if (priv->wrapper)
	ccsGSettingsWrapperUnref (priv->wrapper);

    ccsIntegratedSettingInfoUnref ((CCSIntegratedSettingInfo *) priv->gnomeIntegratedSetting);
    ccsObjectFinalize (setting);

    (*setting->object.object_allocation->free_) (setting->object.object_allocation->allocator, setting);
}

void
ccsGSettingsIntegratedSettingInfoFree (CCSIntegratedSettingInfo *info)
{
    return ccsGSettingsIntegratedSettingFree ((CCSIntegratedSetting *) info);
}

void
ccsGSettingsGNOMEIntegratedSettingInfoFree (CCSGNOMEIntegratedSettingInfo *info)
{
    return ccsGSettingsIntegratedSettingFree ((CCSIntegratedSetting *) info);
}

const CCSGNOMEIntegratedSettingInfoInterface ccsGSettingsGNOMEIntegratedSettingInterface =
{
    ccsGSettingsIntegratedSettingGetSpecialOptionType,
    ccsGSettingsIntegratedSettingGetGNOMEName,
    ccsGSettingsGNOMEIntegratedSettingInfoFree
};

const CCSIntegratedSettingInterface ccsGSettingsIntegratedSettingInterface =
{
    ccsGSettingsIntegratedSettingReadValue,
    ccsGSettingsIntegratedSettingWriteValue,
    ccsGSettingsIntegratedSettingFree
};

const CCSIntegratedSettingInfoInterface ccsGSettingsIntegratedSettingInfoInterface =
{
    ccsGSettingsIntegratedSettingInfoPluginName,
    ccsGSettingsIntegratedSettingInfoSettingName,
    ccsGSettingsIntegratedSettingInfoGetType,
    ccsGSettingsIntegratedSettingInfoFree
};

CCSIntegratedSetting *
ccsGSettingsIntegratedSettingNew (CCSGNOMEIntegratedSettingInfo *base,
				  CCSGSettingsWrapper	*wrapper,
				  CCSObjectAllocationInterface *ai)
{
    CCSIntegratedSetting *setting = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSIntegratedSetting));

    if (!setting)
	return NULL;

    CCSGSettingsIntegratedSettingPrivate *priv = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSGSettingsIntegratedSettingPrivate));

    if (!priv)
    {
	(*ai->free_) (ai->allocator, priv);
	return NULL;
    }

    priv->gnomeIntegratedSetting = base;
    priv->wrapper = wrapper;

    ccsGSettingsWrapperRef (priv->wrapper);

    ccsObjectInit (setting, ai);
    ccsObjectSetPrivate (setting, (CCSPrivate *) priv);
    ccsObjectAddInterface (setting, (const CCSInterface *) &ccsGSettingsIntegratedSettingInterface, GET_INTERFACE_TYPE (CCSIntegratedSettingInterface));
    ccsObjectAddInterface (setting, (const CCSInterface *) &ccsGSettingsIntegratedSettingInfoInterface, GET_INTERFACE_TYPE (CCSIntegratedSettingInfoInterface));
    ccsObjectAddInterface (setting, (const CCSInterface *) &ccsGSettingsGNOMEIntegratedSettingInterface, GET_INTERFACE_TYPE (CCSGNOMEIntegratedSettingInfoInterface));
    ccsIntegratedSettingRef (setting);

    return setting;
}
