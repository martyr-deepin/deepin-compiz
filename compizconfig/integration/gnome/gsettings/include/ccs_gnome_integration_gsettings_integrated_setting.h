#ifndef _CCS_GNOME_GSETTINGS_INTEGRATED_SETTING_H
#define _CCS_GNOME_GSETTINGS_INTEGRATED_SETTING_H

#include <ccs-defs.h>
#include <ccs-object.h>

COMPIZCONFIG_BEGIN_DECLS

typedef struct _CCSIntegratedSetting      CCSIntegratedSetting;
typedef struct _CCSGNOMEIntegratedSettingInfo CCSGNOMEIntegratedSettingInfo;
typedef struct _CCSGSettingsWrapper       CCSGSettingsWrapper;

/**
 * @brief ccsGSettingsIntegratedSettingNew
 * @param base
 * @param wrapper
 * @param ai
 * @return
 *
 * The GSettings implementation of CCSIntegratedSetting *, which will
 * write using a CCSGSettingsWrapper * object when the read and write
 * methods are called.
 */
CCSIntegratedSetting *
ccsGSettingsIntegratedSettingNew (CCSGNOMEIntegratedSettingInfo *base,
				  CCSGSettingsWrapper       *wrapper,
				  CCSObjectAllocationInterface *ai);

/**
 * @brief ccsGSettingsIntegratedSettingsTranslateOldGNOMEKeyForGSettings
 * @param key the old style gnome key to translate
 * @return new-style key. Caller should free
 *
 * This translates old style keys (eg foo_bar) to new style keys
 * foo-bar and special cases a few keys
 */
char *
ccsGSettingsIntegratedSettingsTranslateOldGNOMEKeyForGSettings (const char *key);

COMPIZCONFIG_END_DECLS

#endif
