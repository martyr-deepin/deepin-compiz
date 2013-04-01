#include <cstring>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

#include <ccs.h>

#include "ccs_text_file_interface.h"
#include "compizconfig_ccs_text_file_mock.h"

#include <gtest_shared_characterwrapper.h>
#include <gtest_shared_autodestroy.h>

using ::testing::_;
using ::testing::Return;
using ::testing::Eq;

class CCSTextFileTest :
    public ::testing::Test
{
};

namespace
{
    const std::string CCS_TEXT_FILE_TESTING_CONTENTS;
    const std::string CCS_TEXT_FILE_APPEND_CONTENTS;
}

MATCHER(BoolTrue, "Boolean True") { if (arg) return true; else return false; }

TEST(CCSTextFileTest, TestMock)
{
    boost::shared_ptr <CCSTextFile> textFile (AutoDestroy (ccsMockTextFileNew (&ccsDefaultObjectAllocator),
							   ccsTextFileUnref));
    CCSTextFileGMock *mock = reinterpret_cast <CCSTextFileGMock *> (ccsObjectGetPrivate (textFile));

    CharacterWrapper temporaryFileContents (strdup (CCS_TEXT_FILE_TESTING_CONTENTS.c_str ()));
    char             *temporaryFileContentsC = temporaryFileContents;

    EXPECT_CALL (*mock, readFromStart ()).WillOnce (Return (temporaryFileContentsC));
    EXPECT_CALL (*mock, appendString (Eq (CCS_TEXT_FILE_APPEND_CONTENTS))).WillOnce (Return (TRUE));
    EXPECT_CALL (*mock, free ());

    EXPECT_THAT (ccsTextFileReadFromStart (textFile.get ()), Eq (CCS_TEXT_FILE_TESTING_CONTENTS));
    EXPECT_THAT (ccsTextFileAppendString (textFile.get (), CCS_TEXT_FILE_APPEND_CONTENTS.c_str ()), BoolTrue ());
}
