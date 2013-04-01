#ifndef _CCS_GNOME_INTEGRATION_TYPES_H
#define _CCS_GNOME_INTEGRATION_TYPES_H

#include <ccs-defs.h>

COMPIZCONFIG_BEGIN_DECLS

typedef struct _CCSBackend CCSBackend;
typedef struct _CCSContext CCSContext;
typedef struct _CCSObjectAllocationInterface CCSObjectAllocationInterface;
typedef struct _CCSIntegration CCSIntegration;
typedef struct _CCSIntegratedSetting CCSIntegratedSetting;
typedef struct _CCSIntegratedSettingFactory CCSIntegratedSettingFactory;
typedef struct _CCSIntegratedSettingsStorage CCSIntegratedSettingsStorage;
typedef struct _GConfClient GConfClient;

typedef enum {
    OptionInt,
    OptionBool,
    OptionKey,
    OptionString,
    OptionSpecial,
} SpecialOptionType;

COMPIZCONFIG_END_DECLS

#endif

