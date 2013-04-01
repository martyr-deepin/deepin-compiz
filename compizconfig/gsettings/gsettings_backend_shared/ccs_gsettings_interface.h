#ifndef _CCS_GSETTINGS_INTERFACE_H
#define _CCS_GSETTINGS_INTERFACE_H

#include <ccs-defs.h>

COMPIZCONFIG_BEGIN_DECLS

#include <glib.h>
#include <gio/gio.h>
#include <ccs-object.h>

typedef struct _CCSGSettingsWrapper	      CCSGSettingsWrapper;
typedef struct _CCSGSettingsWrapperInterface  CCSGSettingsWrapperInterface;

typedef void (*CCSGSettingsWrapperSetValue) (CCSGSettingsWrapper *, const char *, GVariant *);
typedef GVariant * (*CCSGSettingsWrapperGetValue) (CCSGSettingsWrapper *, const char *);
typedef void (*CCSGSettingsWrapperResetKey) (CCSGSettingsWrapper *, const char *);
typedef char ** (*CCSGSettingsWrapperListKeys) (CCSGSettingsWrapper *);
typedef GSettings * (*CCSGSettingsWrapperGetGSettings) (CCSGSettingsWrapper *);
typedef const char * (*CCSGSettingsWrapperGetSchemaName) (CCSGSettingsWrapper *);
typedef const char * (*CCSGSettingsWrapperGetPath) (CCSGSettingsWrapper *);
typedef void (*CCSGSettingsWrapperConnectToChangedSignal) (CCSGSettingsWrapper *, GCallback, gpointer);
typedef void (*CCSGSettingsWrapperFree) (CCSGSettingsWrapper *);

struct _CCSGSettingsWrapperInterface
{
    CCSGSettingsWrapperSetValue gsettingsWrapperSetValue;
    CCSGSettingsWrapperGetValue gsettingsWrapperGetValue;
    CCSGSettingsWrapperResetKey gsettingsWrapperResetKey;
    CCSGSettingsWrapperListKeys gsettingsWrapperListKeys;
    CCSGSettingsWrapperGetGSettings gsettingsWrapperGetGSettings;
    CCSGSettingsWrapperGetSchemaName gsettingsWrapperGetSchemaName;
    CCSGSettingsWrapperGetPath       gsettingsWrapperGetPath;
    CCSGSettingsWrapperConnectToChangedSignal gsettingsWrapperConnectToChangedSignal;
    CCSGSettingsWrapperFree gsettingsWrapperFree;
};

/**
 * @brief The _CCSGSettingsWrapper struct
 *
 * A wrapper around GSettings.
 *
 * This wrapper exists for testing purposes and presents the subset
 * of interface that we wish to use from GSettings anways. It does not
 * have any of the typed functions and it is the programmer's responsibility
 * to supply a GVariant to setValue and getValue that is valid.
 */
struct _CCSGSettingsWrapper
{
    CCSObject object;
};

void ccsGSettingsWrapperSetValue (CCSGSettingsWrapper *, const char *, GVariant *);
GVariant * ccsGSettingsWrapperGetValue (CCSGSettingsWrapper *, const char *);
void ccsGSettingsWrapperResetKey (CCSGSettingsWrapper *, const char *);
char **ccsGSettingsWrapperListKeys (CCSGSettingsWrapper *);
GSettings * ccsGSettingsWrapperGetGSettings (CCSGSettingsWrapper *);
const char * ccsGSettingsWrapperGetSchemaName (CCSGSettingsWrapper *);
const char * ccsGSettingsWrapperGetPath (CCSGSettingsWrapper *);
void ccsGSettingsWrapperConnectToChangedSignal (CCSGSettingsWrapper *, GCallback, gpointer);
void ccsFreeGSettingsWrapper (CCSGSettingsWrapper *wrapper);

unsigned int ccsCCSGSettingsWrapperInterfaceGetType ();

CCSREF_HDR (GSettingsWrapper, CCSGSettingsWrapper);

COMPIZCONFIG_END_DECLS

#endif
