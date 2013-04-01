#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>

#include "compizconfig_ccs_backend_mock.h"

using ::testing::_;
using ::testing::ReturnNull;

class CCSBackendTest :
    public ::testing::Test
{
};

TEST(CCSBackendTest, TestMock)
{
    CCSBackend *backend = ccsMockBackendNew ();
    CCSBackendGMock *mock = (CCSBackendGMock *) ccsObjectGetPrivate (backend);

    EXPECT_CALL (*mock, getInfo ());
    EXPECT_CALL (*mock, executeEvents (_));
    EXPECT_CALL (*mock, init (_));
    EXPECT_CALL (*mock, fini ());
    EXPECT_CALL (*mock, readInit (_));
    EXPECT_CALL (*mock, readSetting (_,_));
    EXPECT_CALL (*mock, readDone (_));
    EXPECT_CALL (*mock, writeInit (_));
    EXPECT_CALL (*mock, writeSetting (_, _));
    EXPECT_CALL (*mock, writeDone (_));
    EXPECT_CALL (*mock, getSettingIsIntegrated (_));
    EXPECT_CALL (*mock, getSettingIsReadOnly (_));
    EXPECT_CALL (*mock, getExistingProfiles (_)).WillRepeatedly (ReturnNull ());
    EXPECT_CALL (*mock, deleteProfile (_, _));

    ccsBackendGetInfo (backend);
    ccsBackendExecuteEvents (backend, 0);
    ccsBackendInit (backend, NULL);
    ccsBackendFini (backend);
    ccsBackendReadInit (backend, NULL);
    ccsBackendReadSetting (backend, NULL, NULL);
    ccsBackendReadDone (backend, NULL);
    ccsBackendWriteInit (backend, NULL);
    ccsBackendWriteSetting (backend, NULL, NULL);
    ccsBackendWriteDone (backend, NULL);
    ccsBackendGetSettingIsIntegrated (backend, NULL);
    ccsBackendGetSettingIsReadOnly (backend, NULL);
    ccsBackendGetExistingProfiles (backend, NULL);
    ccsBackendDeleteProfile (backend, NULL, NULL);

    ccsFreeMockBackend (backend);
}
