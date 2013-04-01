#ifndef _CCS_COMPIZCONFIG_GNOME_GCONF_INTEGRATION
#define _CCS_COMPIZCONFIG_GNOME_GCONF_INTEGRATION

#include <ccs-defs.h>
#include "ccs_gnome_integration_types.h"

COMPIZCONFIG_BEGIN_DECLS

typedef struct _CCSGNOMEValueChangeData
{
    CCSIntegration *integration;
    CCSIntegratedSettingsStorage *storage;
    CCSIntegratedSettingFactory *factory;
    CCSContext     *context;
} CCSGNOMEValueChangeData;

/**
 * @brief ccsGNOMEIntegrationBackendNew
 * @param backend
 * @param context
 * @param factory
 * @param storage
 * @param ai
 * @return A new CCSIntegration
 *
 * The GNOME implementation of desktop environment integration - requires
 * a method to create new integrated settings, and a method to store them
 * as well.
 *
 * CCSGNOMEIntegration is a pure composition in most respects - it just
 * represents the process as to which settings should be written to
 * what keys and vice versa, it doesn't represent how those keys should
 * be written.
 */
CCSIntegration *
ccsGNOMEIntegrationBackendNew (CCSBackend *backend,
			       CCSContext *context,
			       CCSIntegratedSettingFactory *factory,
			       CCSIntegratedSettingsStorage *storage,
			       CCSObjectAllocationInterface *ai);

COMPIZCONFIG_END_DECLS

#endif
