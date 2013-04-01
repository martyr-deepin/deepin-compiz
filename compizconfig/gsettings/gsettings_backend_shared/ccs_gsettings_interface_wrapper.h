#ifndef _CCS_GSETTINGS_INTERFACE_WRAPPER_H
#define _CCS_GSETTINGS_INTERFACE_WRAPPER_H

#include <ccs-defs.h>

COMPIZCONFIG_BEGIN_DECLS

#include "ccs_gsettings_interface.h"

CCSGSettingsWrapper *
ccsGSettingsWrapperNewForSchemaWithPath (const char *schema,
					 const char *path,
					 CCSObjectAllocationInterface *ai);

CCSGSettingsWrapper *
ccsGSettingsWrapperNewForSchema (const char *schema,
				 CCSObjectAllocationInterface *ai);

COMPIZCONFIG_END_DECLS

#endif
