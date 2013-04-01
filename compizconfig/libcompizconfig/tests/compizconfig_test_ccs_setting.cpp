#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>

#include "compizconfig_ccs_setting_mock.h"

using ::testing::_;
using ::testing::Return;

class CCSSettingTest :
    public ::testing::Test
{
};

TEST(CCSSettingTest, TestMock)
{
    CCSSetting *setting = ccsMockSettingNew ();
    CCSSettingGMock *mock = (CCSSettingGMock *) ccsObjectGetPrivate (setting);

    EXPECT_CALL (*mock, getName ());
    EXPECT_CALL (*mock, getShortDesc ());
    EXPECT_CALL (*mock, getLongDesc ());
    EXPECT_CALL (*mock, getType ());
    EXPECT_CALL (*mock, getInfo ());
    EXPECT_CALL (*mock, getGroup ());
    EXPECT_CALL (*mock, getSubGroup ());
    EXPECT_CALL (*mock, getHints ());
    EXPECT_CALL (*mock, getDefaultValue ());
    EXPECT_CALL (*mock, getValue ());
    EXPECT_CALL (*mock, getIsDefault ());
    EXPECT_CALL (*mock, getParent ());
    EXPECT_CALL (*mock, getPrivatePtr ());
    EXPECT_CALL (*mock, setPrivatePtr (_));
    EXPECT_CALL (*mock, setInt (_, _));
    EXPECT_CALL (*mock, setFloat (_, _));
    EXPECT_CALL (*mock, setBool (_, _));
    EXPECT_CALL (*mock, setString (_, _));
    EXPECT_CALL (*mock, setColor (_, _));
    EXPECT_CALL (*mock, setMatch (_, _));
    EXPECT_CALL (*mock, setKey (_, _));
    EXPECT_CALL (*mock, setButton (_, _));
    EXPECT_CALL (*mock, setEdge (_, _));
    EXPECT_CALL (*mock, setBell (_, _));
    EXPECT_CALL (*mock, setList (_, _));
    EXPECT_CALL (*mock, setValue (_, _));
    EXPECT_CALL (*mock, getInt (_));
    EXPECT_CALL (*mock, getFloat (_));
    EXPECT_CALL (*mock, getBool (_));
    EXPECT_CALL (*mock, getString (_));
    EXPECT_CALL (*mock, getColor (_));
    EXPECT_CALL (*mock, getMatch (_));
    EXPECT_CALL (*mock, getKey (_));
    EXPECT_CALL (*mock, getButton (_));
    EXPECT_CALL (*mock, getEdge (_));
    EXPECT_CALL (*mock, getBell (_));
    EXPECT_CALL (*mock, getList (_));
    EXPECT_CALL (*mock, resetToDefault (_));
    EXPECT_CALL (*mock, isIntegrated ());
    EXPECT_CALL (*mock, isReadOnly ());
    EXPECT_CALL (*mock, isReadableByBackend ());

    CCSSettingColorValue cv;
    CCSSettingKeyValue kv;
    CCSSettingButtonValue bv;
    CCSSettingValueList lv = NULL;

    ccsSettingGetName (setting);
    ccsSettingGetShortDesc (setting);
    ccsSettingGetLongDesc (setting);
    ccsSettingGetType (setting);
    ccsSettingGetInfo (setting);
    ccsSettingGetGroup (setting);
    ccsSettingGetSubGroup (setting);
    ccsSettingGetHints (setting);
    ccsSettingGetDefaultValue (setting);
    ccsSettingGetValue (setting);
    ccsSettingGetIsDefault (setting);
    ccsSettingGetParent (setting);
    ccsSettingGetPrivatePtr (setting);
    ccsSettingSetPrivatePtr (setting, NULL);
    ccsSetInt (setting, 1, FALSE);
    ccsSetFloat (setting, 1.0, FALSE);
    ccsSetBool (setting, TRUE, FALSE);
    ccsSetString (setting, "foo", FALSE);
    ccsSetColor (setting, cv, FALSE);
    ccsSetMatch (setting, "bar", FALSE);
    ccsSetKey (setting, kv, FALSE);
    ccsSetButton (setting, bv, FALSE);
    ccsSetEdge (setting, 1, FALSE);
    ccsSetBell (setting, TRUE, FALSE);
    ccsSetList (setting, lv, FALSE);
    ccsSetValue (setting, NULL, FALSE);
    ccsGetInt (setting, NULL);
    ccsGetFloat (setting, NULL);
    ccsGetBool (setting, NULL);
    ccsGetString (setting, NULL);
    ccsGetColor (setting, NULL);
    ccsGetMatch (setting, NULL);
    ccsGetKey (setting, NULL);
    ccsGetButton (setting, NULL);
    ccsGetEdge (setting, NULL);
    ccsGetBell (setting, NULL);
    ccsGetList (setting, NULL);
    ccsResetToDefault (setting, FALSE);
    ccsSettingIsIntegrated (setting);
    ccsSettingIsReadOnly (setting);
    ccsSettingIsReadableByBackend (setting);

    ccsSettingUnref (setting);
}
