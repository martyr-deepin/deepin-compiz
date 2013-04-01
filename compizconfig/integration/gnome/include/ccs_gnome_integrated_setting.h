#ifndef _CCS_GNOME_INTEGRATED_SETTING_H
#define _CCS_GNOME_INTEGRATED_SETTING_H

#include <ccs-defs.h>
#include <ccs-object.h>

#include "ccs_gnome_integration_types.h"

COMPIZCONFIG_BEGIN_DECLS

typedef struct _CCSIntegratedSettingInfo      CCSIntegratedSettingInfo;

typedef struct _CCSGNOMEIntegratedSettingInfo CCSGNOMEIntegratedSettingInfo;
typedef struct _CCSGNOMEIntegratedSettingInfoInterface CCSGNOMEIntegratedSettingInfoInterface;

typedef SpecialOptionType (*CCSGNOMEIntegratedSettingInfoGetSpecialOptionType) (CCSGNOMEIntegratedSettingInfo *);
typedef const char * (*CCSGNOMEIntegratedSettingInfoGetGNOMEName) (CCSGNOMEIntegratedSettingInfo *);
typedef void (*CCSGNOMEIntegratedSettingInfoFree) (CCSGNOMEIntegratedSettingInfo *);

struct _CCSGNOMEIntegratedSettingInfoInterface
{
    CCSGNOMEIntegratedSettingInfoGetSpecialOptionType getSpecialOptionType;
    CCSGNOMEIntegratedSettingInfoGetGNOMEName         getGNOMEName;
    CCSGNOMEIntegratedSettingInfoFree                 free;
};

/**
 * @brief The _CCSGNOMEIntegratedSetting struct
 *
 * CCSGNOMEIntegratedSetting represents an integrated setting in
 * GNOME - it builds upon CCSSharedIntegratedSetting (which it composes
 * and implements) and also adds the concept of an GNOME side keyname
 * and option type for that keyname (as the types do not match 1-1)
 */
struct _CCSGNOMEIntegratedSettingInfo
{
    CCSObject object;
};

unsigned int ccsCCSGNOMEIntegratedSettingInfoInterfaceGetType ();

Bool
ccsGNOMEIntegrationFindSettingsMatchingPredicate (CCSIntegratedSetting *setting,
						  void		       *userData);

SpecialOptionType
ccsGNOMEIntegratedSettingInfoGetSpecialOptionType (CCSGNOMEIntegratedSettingInfo *);

const char *
ccsGNOMEIntegratedSettingInfoGetGNOMEName (CCSGNOMEIntegratedSettingInfo *);

CCSGNOMEIntegratedSettingInfo *
ccsGNOMEIntegratedSettingInfoNew (CCSIntegratedSettingInfo *base,
				  SpecialOptionType    type,
				  const char	   *gnomeName,
				  CCSObjectAllocationInterface *ai);

void
ccsFreeGNOMEIntegratedSettingInfo (CCSGNOMEIntegratedSettingInfo *);

CCSREF_HDR (GNOMEIntegratedSettingInfo, CCSGNOMEIntegratedSettingInfo);
CCSLIST_HDR (GNOMEIntegratedSettingInfo, CCSGNOMEIntegratedSettingInfo);

COMPIZCONFIG_END_DECLS

#endif
