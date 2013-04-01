#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>

#include "compizconfig_ccs_setting_mock.h"

using ::testing::NiceMock;

CCSSettingInterface CCSSettingGMockInterface =
{
    CCSSettingGMock::ccsSettingGetName,
    CCSSettingGMock::ccsSettingGetShortDesc,
    CCSSettingGMock::ccsSettingGetLongDesc,
    CCSSettingGMock::ccsSettingGetType,
    CCSSettingGMock::ccsSettingGetInfo,
    CCSSettingGMock::ccsSettingGetGroup,
    CCSSettingGMock::ccsSettingGetSubGroup,
    CCSSettingGMock::ccsSettingGetHints,
    CCSSettingGMock::ccsSettingGetDefaultValue,
    CCSSettingGMock::ccsSettingGetValue,
    CCSSettingGMock::ccsSettingGetIsDefault,
    CCSSettingGMock::ccsSettingGetParent,
    CCSSettingGMock::ccsSettingGetPrivatePtr,
    CCSSettingGMock::ccsSettingSetPrivatePtr,
    CCSSettingGMock::ccsSetInt,
    CCSSettingGMock::ccsSetFloat,
    CCSSettingGMock::ccsSetBool,
    CCSSettingGMock::ccsSetString,
    CCSSettingGMock::ccsSetColor,
    CCSSettingGMock::ccsSetMatch,
    CCSSettingGMock::ccsSetKey,
    CCSSettingGMock::ccsSetButton,
    CCSSettingGMock::ccsSetEdge,
    CCSSettingGMock::ccsSetBell,
    CCSSettingGMock::ccsSetList,
    CCSSettingGMock::ccsSetValue,
    CCSSettingGMock::ccsGetInt,
    CCSSettingGMock::ccsGetFloat,
    CCSSettingGMock::ccsGetBool,
    CCSSettingGMock::ccsGetString,
    CCSSettingGMock::ccsGetColor,
    CCSSettingGMock::ccsGetMatch,
    CCSSettingGMock::ccsGetKey,
    CCSSettingGMock::ccsGetButton,
    CCSSettingGMock::ccsGetEdge,
    CCSSettingGMock::ccsGetBell,
    CCSSettingGMock::ccsGetList,
    CCSSettingGMock::ccsResetToDefault,
    CCSSettingGMock::ccsSettingIsIntegrated,
    CCSSettingGMock::ccsSettingIsReadOnly,
    CCSSettingGMock::ccsSettingIsReadableByBackend,
    CCSSettingGMock::ccsSettingFree
};

static CCSSetting *
allocateSettingObjectWithMockInterface ()
{
    CCSSetting *setting = (CCSSetting *) calloc (1, sizeof (CCSSetting));

    if (!setting)
	return NULL;

    ccsObjectInit (setting, &ccsDefaultObjectAllocator);

    ccsObjectAddInterface (setting, (CCSInterface *) &CCSSettingGMockInterface, GET_INTERFACE_TYPE (CCSSettingInterface));

    ccsSettingRef (setting);

    return setting;
}

CCSSetting *
ccsMockSettingNew ()
{
    CCSSetting *setting = allocateSettingObjectWithMockInterface ();

    if (!setting)
	return NULL;

    CCSSettingGMock *mock = new CCSSettingGMock (setting);
    ccsObjectSetPrivate (setting, (CCSPrivate *) mock);

    return setting;
}

CCSSetting *
ccsNiceMockSettingNew ()
{
    CCSSetting *setting = allocateSettingObjectWithMockInterface ();

    if (!setting)
	return NULL;

    NiceMock <CCSSettingGMock> *mock = new NiceMock <CCSSettingGMock> (setting);
    ccsObjectSetPrivate (setting, (CCSPrivate *) mock);

    return setting;
}

void
ccsFreeMockSetting (CCSSetting *setting)
{
    /* Need to delete the mock correctly */

    CCSSettingGMock *mock = (CCSSettingGMock *) ccsObjectGetPrivate (setting);

    delete mock;

    ccsObjectSetPrivate (setting, NULL);
    ccsObjectFinalize (setting);

    free (setting);
}
