#include "test_gsettings_tests.h"

using ::testing::Values;

class CCSGSettingsTestWithMocks :
    public CCSGSettingsTeardownSetupInterface
{
    public:

	void SetUp () {}
	void TearDown () {}
};

class CCSGSettingsTestWithSystem :
    public CCSGSettingsTeardownSetupInterface
{
    public:

	void SetUp () {}
	void TearDown () {}
};

namespace
{
    CCSGSettingsTestWithMocks withMocks;
    CCSGSettingsTestWithSystem withSystem;
}

INSTANTIATE_TEST_CASE_P(CompizConfigGSettingsTestWithMocks, CCSGSettingsTest, Values (&withMocks, &withSystem));
