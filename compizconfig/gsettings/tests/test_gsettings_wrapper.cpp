#include <cstring>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <glib-object.h>

#include "test_gsettings_tests.h"
#include <ccs_gsettings_interface_wrapper.h>

using ::testing::NotNull;
using ::testing::Eq;
using ::testing::_;


class CCSGSettingsWrapperTest :
    public CCSGSettingsTestWithMemoryBackend
{
    public:

	CCSGSettingsWrapperTest () :
	    mockSchema ("org.compiz.mock"),
	    mockPath ("/org/compiz/mock/mock")
	{
	}

	virtual CCSObjectAllocationInterface * GetAllocator () = 0;

	virtual void SetUp ()
	{
	    CCSGSettingsTestWithMemoryBackend::SetUp ();
	}

	virtual void TearDown ()
	{
	    CCSGSettingsTestWithMemoryBackend::TearDown ();
	}

    protected:

	std::string mockSchema;
	std::string mockPath;
	boost::shared_ptr <CCSGSettingsWrapper> wrapper;
	GSettings   *settings;
};

class CCSGSettingsWrapperWithMemoryBackendEnvGoodAllocatorTest :
    public CCSGSettingsWrapperTest
{
    protected:

	CCSObjectAllocationInterface * GetAllocator ()
	{
	    return &ccsDefaultObjectAllocator;
	}
};

class CCSGSettingsWrapperWithMemoryBackendEnvGoodAllocatorAutoInitTest :
    public CCSGSettingsWrapperWithMemoryBackendEnvGoodAllocatorTest
{
    public:

	virtual void SetUp ()
	{
	    CCSGSettingsWrapperWithMemoryBackendEnvGoodAllocatorTest::SetUp ();

	    wrapper.reset (ccsGSettingsWrapperNewForSchemaWithPath (mockSchema.c_str (),
								    mockPath.c_str (),
								    GetAllocator ()),
			   boost::bind (ccsFreeGSettingsWrapper, _1));

	    ASSERT_THAT (wrapper.get (), NotNull ());

	    settings = ccsGSettingsWrapperGetGSettings (wrapper.get ());

	    ASSERT_THAT (settings, NotNull ());
	}
};

TEST_F (CCSGSettingsWrapperWithMemoryBackendEnvGoodAllocatorTest, TestWrapperConstruction)
{
    boost::shared_ptr <CCSGSettingsWrapper> wrapper (ccsGSettingsWrapperNewForSchemaWithPath (mockSchema.c_str (),
											      mockPath.c_str (),
											      &ccsDefaultObjectAllocator),
						     boost::bind (ccsFreeGSettingsWrapper, _1));

    EXPECT_THAT (wrapper.get (), NotNull ());
}

TEST_F (CCSGSettingsWrapperWithMemoryBackendEnvGoodAllocatorTest, TestGetGSettingsWrapper)
{
    boost::shared_ptr <CCSGSettingsWrapper> wrapper (ccsGSettingsWrapperNewForSchemaWithPath (mockSchema.c_str (),
											      mockPath.c_str (),
											      &ccsDefaultObjectAllocator),
						     boost::bind (ccsFreeGSettingsWrapper, _1));

    ASSERT_THAT (wrapper.get (), NotNull ());
    EXPECT_THAT (ccsGSettingsWrapperGetGSettings (wrapper.get ()), NotNull ());
}

TEST_F (CCSGSettingsWrapperWithMemoryBackendEnvGoodAllocatorAutoInitTest, TestSetValueOnWrapper)
{
    const unsigned int VALUE = 2;
    const std::string KEY ("integer-setting");
    boost::shared_ptr <GVariant> variant (g_variant_new ("i", VALUE, NULL),
					  boost::bind (g_variant_unref, _1));
    ccsGSettingsWrapperSetValue (wrapper.get (), KEY.c_str (), variant.get ());

    boost::shared_ptr <GVariant> value (g_settings_get_value (settings, KEY.c_str ()),
					boost::bind (g_variant_unref, _1));

    int v = g_variant_get_int32 (value.get ());
    EXPECT_EQ (VALUE, v);
}

TEST_F (CCSGSettingsWrapperWithMemoryBackendEnvGoodAllocatorAutoInitTest, TestGetValueOnWrapper)
{
    const double VALUE = 3.0;
    const std::string KEY ("float-setting");
    boost::shared_ptr <GVariant> variant (g_variant_new ("d", VALUE, NULL),
					  boost::bind (g_variant_unref, _1));
    g_settings_set_value (settings, KEY.c_str (), variant.get ());
    boost::shared_ptr <GVariant> value (ccsGSettingsWrapperGetValue (wrapper.get (),
								     KEY.c_str ()),
					boost::bind (g_variant_unref, _1));

    double v = (double) g_variant_get_double (value.get ());
    EXPECT_EQ (VALUE, v);
}

TEST_F (CCSGSettingsWrapperWithMemoryBackendEnvGoodAllocatorAutoInitTest, TestResetKeyOnWrapper)
{
    const char * DEFAULT = "";
    const char * VALUE = "foo";
    const std::string KEY ("string-setting");
    GVariant *variant = g_variant_new ("s", VALUE);
    ccsGSettingsWrapperSetValue (wrapper.get (), KEY.c_str (), variant);

    boost::shared_ptr <GVariant> value (g_settings_get_value (settings, KEY.c_str ()),
					boost::bind (g_variant_unref, _1));

    gsize      length;
    std::string v (g_variant_get_string (value.get (), &length));
    ASSERT_EQ (strlen (VALUE), length);
    ASSERT_THAT (v, Eq (VALUE));

    ccsGSettingsWrapperResetKey (wrapper.get (), KEY.c_str ());

    value.reset (g_settings_get_value (settings, KEY.c_str ()),
		 boost::bind (g_variant_unref, _1));

    v = std::string (g_variant_get_string (value.get (), &length));
    ASSERT_EQ (strlen (DEFAULT), length);
    ASSERT_THAT (v, Eq (DEFAULT));
}

TEST_F (CCSGSettingsWrapperWithMemoryBackendEnvGoodAllocatorAutoInitTest, TestListKeysOnWrapper)
{
    const char * EXPECTED_KEYS[] =
    {
	"bell-setting",
	"bool-list-setting",
	"boolean-setting",
	"button-setting",
	"color-list-setting",
	"color-setting",
	"edge-setting",
	"float-list-setting",
	"float-setting",
	"int-list-setting",
	"integer-setting",
	"key-setting",
	"match-list-setting",
	"match-setting",
	"string-list-setting",
	"string-setting"
    };

    boost::shared_ptr <gchar *> keys (ccsGSettingsWrapperListKeys (wrapper.get ()),
				      boost::bind (g_strfreev, _1));

    ASSERT_EQ (g_strv_length (keys.get ()),
	       sizeof (EXPECTED_KEYS) /
	       sizeof (EXPECTED_KEYS[0]));
}

TEST_F (CCSGSettingsWrapperWithMemoryBackendEnvGoodAllocatorAutoInitTest, TestGetSchemaName)
{
    EXPECT_THAT (ccsGSettingsWrapperGetSchemaName (wrapper.get ()), Eq (mockSchema));
}

TEST_F (CCSGSettingsWrapperWithMemoryBackendEnvGoodAllocatorAutoInitTest, TestGetPath)
{
    EXPECT_THAT (ccsGSettingsWrapperGetPath (wrapper.get ()), Eq (mockPath));
}

namespace signal_test
{
    class VerificationInterface
    {
	public:

	    virtual ~VerificationInterface () {}
	    virtual void Verify (GSettings *settings, gchar *keyname) = 0;
    };

    class VerificationMock :
	public VerificationInterface
    {
	public:

	    MOCK_METHOD2 (Verify, void (GSettings *settings, gchar *keyname));
    };


    void dummyChangedSignal (GSettings   *s,
			     gchar       *keyName,
			     gpointer    user_data)
    {
	VerificationInterface *verifier = reinterpret_cast <VerificationInterface *> (user_data);
	verifier->Verify (s, keyName);
    }
}

TEST_F (CCSGSettingsWrapperWithMemoryBackendEnvGoodAllocatorAutoInitTest, TestConnectToChangedSignal)
{
    std::string keyname ("int-setting");
    signal_test::VerificationMock mv;

    /* We're not able to verify the keyname
     * at the moment, need a person who knows
     * GSignal better than I do to figure this
     * one out */
    EXPECT_CALL (mv, Verify (settings, _));

    ccsGSettingsWrapperConnectToChangedSignal (wrapper.get (),
					       (GCallback) signal_test::dummyChangedSignal,
					       (gpointer) static_cast <signal_test::VerificationInterface *> (&mv));

    g_signal_emit_by_name (G_OBJECT (settings),
			   "changed",
			   settings,
			   keyname.c_str (),
			   NULL);
}
