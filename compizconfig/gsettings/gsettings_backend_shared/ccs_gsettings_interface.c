#include <gio/gio.h>
#include "ccs_gsettings_interface.h"

INTERFACE_TYPE (CCSGSettingsWrapperInterface);
CCSREF_OBJ (GSettingsWrapper, CCSGSettingsWrapper);

void
ccsGSettingsWrapperSetValue (CCSGSettingsWrapper *wrapper,
			     const char *key,
			     GVariant *value)
{
    (*(GET_INTERFACE (CCSGSettingsWrapperInterface, wrapper))->gsettingsWrapperSetValue) (wrapper, key, value);
}

GVariant *
ccsGSettingsWrapperGetValue (CCSGSettingsWrapper *wrapper,
			     const char *key)
{
    return (*(GET_INTERFACE (CCSGSettingsWrapperInterface, wrapper))->gsettingsWrapperGetValue) (wrapper, key);
}

void
ccsGSettingsWrapperResetKey (CCSGSettingsWrapper *wrapper,
			     const char *key)
{
    return (*(GET_INTERFACE (CCSGSettingsWrapperInterface, wrapper))->gsettingsWrapperResetKey) (wrapper, key);
}

char **
ccsGSettingsWrapperListKeys (CCSGSettingsWrapper *wrapper)
{
    return (*(GET_INTERFACE (CCSGSettingsWrapperInterface, wrapper))->gsettingsWrapperListKeys) (wrapper);
}

GSettings *
ccsGSettingsWrapperGetGSettings (CCSGSettingsWrapper *wrapper)
{
    return (*(GET_INTERFACE (CCSGSettingsWrapperInterface, wrapper))->gsettingsWrapperGetGSettings) (wrapper);
}

const char *
ccsGSettingsWrapperGetSchemaName (CCSGSettingsWrapper *wrapper)
{
    return (*(GET_INTERFACE (CCSGSettingsWrapperInterface, wrapper))->gsettingsWrapperGetSchemaName) (wrapper);
}

const char *
ccsGSettingsWrapperGetPath (CCSGSettingsWrapper *wrapper)
{
    return (*(GET_INTERFACE (CCSGSettingsWrapperInterface, wrapper))->gsettingsWrapperGetPath) (wrapper);
}

void
ccsGSettingsWrapperConnectToChangedSignal (CCSGSettingsWrapper *wrapper,
					   GCallback	       callback,
					   gpointer	       data)
{
    return (*(GET_INTERFACE (CCSGSettingsWrapperInterface, wrapper))->gsettingsWrapperConnectToChangedSignal) (wrapper, callback, data);
}

void
ccsFreeGSettingsWrapper (CCSGSettingsWrapper *wrapper)
{
    return (*(GET_INTERFACE (CCSGSettingsWrapperInterface, wrapper))->gsettingsWrapperFree) (wrapper);
}
