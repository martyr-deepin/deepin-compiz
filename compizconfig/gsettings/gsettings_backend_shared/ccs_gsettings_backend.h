#ifndef _COMPIZCONFIG_CCS_GSETTINGS_BACKEND_H
#define _COMPIZCONFIG_CCS_GSETTINGS_BACKEND_H

#include <ccs-defs.h>
#include <ccs-backend.h>
#include <glib.h>

COMPIZCONFIG_BEGIN_DECLS

typedef struct _CCSBackend CCSBackend;
typedef struct _CCSGSettingsWrapper CCSGSettingsWrapper;

Bool
ccsGSettingsBackendAttachNewToBackend (CCSBackend *backend, CCSContext *context);

void
ccsGSettingsBackendDetachFromBackend (CCSBackend *backend);

/* Default implementations, should be moved */

void
ccsGSettingsBackendUpdateCurrentProfileNameDefault (CCSBackend *backend, const char *profile);

gboolean
ccsGSettingsBackendUpdateProfileDefault (CCSBackend *backend, CCSContext *context);

void
ccsGSettingsBackendUnsetAllChangedPluginKeysInProfileDefault (CCSBackend *backend,
							      CCSContext *context,
							      GVariant *pluginsWithChangedKeys,
							      const char * profile);

gboolean ccsGSettingsBackendAddProfileDefault (CCSBackend *backend,
					       const char *profile);

void ccsGSettingsSetIntegration (CCSBackend *backend,
				 CCSIntegration *integration);

COMPIZCONFIG_END_DECLS

#endif
