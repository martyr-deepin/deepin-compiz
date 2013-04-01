#ifndef _CCS_GNOME_GCONF_INTEGRATED_SETTING_FACTORY_H
#define _CCS_GNOME_GCONF_INTEGRATED_SETTING_FACTORY_H

#include <ccs-defs.h>
#include <ccs-object.h>

COMPIZCONFIG_BEGIN_DECLS

typedef struct _CCSIntegratedSettingFactory CCSIntegratedSettingFactory;
typedef struct _CCSGNOMEValueChangeData CCSGNOMEValueChangeData;
typedef struct _GConfClient GConfClient;

/**
 * @brief ccsGConfIntegratedSettingFactoryNew
 * @param client an existing GConfClient or NULL
 * @param data some data to pass to the change callback
 * @param ai a CCSObjectAllocationInterface
 * @return a new CCSIntegratedSettingFactory
 *
 * CCSGConfIntegratedSettingFactory implements CCSIntegratedSettingFactory *
 * and will create CCSGConfIntegratedSetting objects (which implement
 * CCSIntegratedSetting).
 */
CCSIntegratedSettingFactory *
ccsGConfIntegratedSettingFactoryNew (GConfClient		  *client,
				     CCSGNOMEValueChangeData	  *data,
				     CCSObjectAllocationInterface *ai);

COMPIZCONFIG_END_DECLS

#endif
