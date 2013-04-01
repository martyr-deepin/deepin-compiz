#ifndef _CCS_GNOME_GCONF_INTEGRATED_SETTING_FACTORY_H
#define _CCS_GNOME_GCONF_INTEGRATED_SETTING_FACTORY_H

#include <ccs-defs.h>
#include <ccs-object.h>

COMPIZCONFIG_BEGIN_DECLS

typedef struct _CCSIntegratedSettingFactory CCSIntegratedSettingFactory;
typedef struct _CCSGNOMEValueChangeData CCSGNOMEValueChangeData;
typedef struct _CCSGSettingsWrapper CCSGSettingsWrapper;
typedef struct _GSettings	    GSettings;

typedef void (*CCSGNOMEIntegrationGSettingsChangedCallback) (GSettings *, gchar *, gpointer);

typedef struct _CCSGNOMEIntegrationGSettingsWrapperFactory CCSGNOMEIntegrationGSettingsWrapperFactory;
typedef struct _CCSGNOMEIntegrationGSettingsWrapperFactoryInterface CCSGNOMEIntegrationGSettingsWrapperFactoryInterface;

typedef CCSGSettingsWrapper * (*CCSGNOMEIntegrationGSettingsWrapperFactoryNewGSettingsWrapper) (CCSGNOMEIntegrationGSettingsWrapperFactory *,
												const gchar				   *schema,
												CCSGNOMEIntegrationGSettingsChangedCallback callback,
												CCSGNOMEValueChangeData			  *data,
												CCSObjectAllocationInterface		   *ai);

struct _CCSGNOMEIntegrationGSettingsWrapperFactoryInterface
{
    CCSGNOMEIntegrationGSettingsWrapperFactoryNewGSettingsWrapper newGSettingsWrapper;
};

/**
 * @brief The _CCSGNOMEIntegrationGSettingsWrapperFactory struct
 *
 * Will create new CCSGSettingsIntegratedSetting objects on demand
 */
struct _CCSGNOMEIntegrationGSettingsWrapperFactory
{
    CCSObject object;
};

unsigned int ccsCCSGNOMEIntegrationGSettingsWrapperFactoryInterfaceGetType ();

/**
 * @brief ccsGSettingsIntegratedSettingsTranslateNewGNOMEKeyForCCS
 * @param key the old style gnome key to translate
 * @return new-style key. Caller should free
 *
 * This translates new style keys (eg foo-bar) to old style keys
 * foo_bar and special cases a few keys
 */
char *
ccsGSettingsIntegratedSettingsTranslateNewGNOMEKeyForCCS (const char *key);

CCSGSettingsWrapper *
ccsGNOMEIntegrationGSettingsWrapperFactoryNewGSettingsWrapper (CCSGNOMEIntegrationGSettingsWrapperFactory *factory,
							       const gchar				  *schemaName,
							       CCSGNOMEIntegrationGSettingsChangedCallback callback,
							       CCSGNOMEValueChangeData			  *data,
							       CCSObjectAllocationInterface		  *ai);

CCSGNOMEIntegrationGSettingsWrapperFactory *
ccsGNOMEIntegrationGSettingsWrapperDefaultImplNew (CCSObjectAllocationInterface *ai);

void
ccsGNOMEIntegrationGSettingsWrapperDefaultImplFree (CCSGNOMEIntegrationGSettingsWrapperFactory *wrapperFactory);

CCSIntegratedSettingFactory *
ccsGSettingsIntegratedSettingFactoryNew (CCSGNOMEIntegrationGSettingsWrapperFactory	   *wrapperFactory,
					 CCSGNOMEValueChangeData	  *data,
					 CCSObjectAllocationInterface *ai);

COMPIZCONFIG_END_DECLS

#endif
