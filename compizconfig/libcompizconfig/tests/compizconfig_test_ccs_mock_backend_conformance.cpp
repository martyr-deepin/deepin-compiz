#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>

#include <compizconfig_ccs_backend_mock.h>
#include <compizconfig_backend_concept_test.h>

#include "compizconfig_ccs_backend_mock.h"

using ::testing::_;
using ::testing::Return;
using ::testing::Invoke;
using ::testing::WithArgs;
using ::testing::Combine;
using ::testing::ValuesIn;
using ::testing::Values;
using ::testing::AtLeast;

namespace
{
std::string keynameFromPluginKey (const std::string &plugin,
				  const std::string &key)
{
    return plugin + "/" + key;
}
}

template <typename T>
class ValueForKeyRetreival
{
    public:

	T GetValueForKey (const std::string &key,
			  const std::map <std::string, VariantTypes> &map)
	{
	    std::map <std::string, VariantTypes>::const_iterator it = map.find (key);

	    if (it != map.end ())
		return boost::get <T> (it->second);
	    else
		throw std::exception ();
	}
};

namespace
{
    void doNothingWithCCSSetting (CCSSetting *) {};

    const CCSBackendInfo mockBackendInfo =
    {
	"mock",
	"Mock Backend",
	"Mock Backend for libccs",
	TRUE,
	TRUE
    };
}

class MockCCSSettingsTestEnvironment :
    public CCSSettingsConceptTestEnvironmentInterface
{
    public:

	virtual void SetUp ()
	{
	}

	virtual void TearDown ()
	{
	}

	void WriteBoolAtKey (const std::string &plugin,
			     const std::string &key,
			     const VariantTypes &value)
	{
	    mValues[keynameFromPluginKey (plugin, key)] = value;
	}

	void WriteIntegerAtKey (const std::string &plugin,
				const std::string &key,
				const VariantTypes &value)

	{
	    mValues[keynameFromPluginKey (plugin, key)] = value;
	}

	void WriteFloatAtKey (const std::string &plugin,
			      const std::string &key,
			      const VariantTypes &value)
	{
	    mValues[keynameFromPluginKey (plugin, key)] = value;
	}

	void WriteMatchAtKey (const std::string &plugin,
			      const std::string &key,
			      const VariantTypes &value)
	{
	    mValues[keynameFromPluginKey (plugin, key)] = value;
	}

	void WriteStringAtKey (const std::string &plugin,
			       const std::string &key,
			       const VariantTypes &value)
	{
	    mValues[keynameFromPluginKey (plugin, key)] = value;
	}

	void WriteColorAtKey (const std::string &plugin,
			      const std::string &key,
			      const VariantTypes &value)
	{
	    mValues[keynameFromPluginKey (plugin, key)] = value;
	}

	void WriteKeyAtKey (const std::string &plugin,
			    const std::string &key,
			    const VariantTypes &value)
	{
	    mValues[keynameFromPluginKey (plugin, key)] = value;
	}

	void WriteButtonAtKey (const std::string &plugin,
			       const std::string &key,
			       const VariantTypes &value)
	{
	    mValues[keynameFromPluginKey (plugin, key)] = value;
	}

	void WriteEdgeAtKey (const std::string &plugin,
			     const std::string &key,
			     const VariantTypes &value)
	{
	    mValues[keynameFromPluginKey (plugin, key)] = value;
	}

	void WriteBellAtKey (const std::string &plugin,
			     const std::string &key,
			     const VariantTypes &value)
	{
	    mValues[keynameFromPluginKey (plugin, key)] = value;
	}

	void WriteListAtKey (const std::string &plugin,
			     const std::string &key,
			     const VariantTypes &value)
	{
	    mValues[keynameFromPluginKey (plugin, key)] = value;
	}

	virtual Bool ReadBoolAtKey (const std::string &plugin,
				    const std::string &key)
	{
            return compizconfig::test::boolToBool (
                ValueForKeyRetreival <bool> ().GetValueForKey (
                    keynameFromPluginKey (plugin, key), mValues));
	}

	virtual int ReadIntegerAtKey (const std::string &plugin,
				      const std::string &key)
	{
            return ValueForKeyRetreival <int> ().GetValueForKey (
                keynameFromPluginKey (plugin, key), mValues);
	}

	virtual float ReadFloatAtKey (const std::string &plugin,
				      const std::string &key)
	{
            return ValueForKeyRetreival <float> ().GetValueForKey (
                keynameFromPluginKey (plugin, key), mValues);
	}

	virtual const char * ReadStringAtKey (const std::string &plugin,
					      const std::string &key)
	{
            return ValueForKeyRetreival <const char *> ().GetValueForKey (
                keynameFromPluginKey (plugin, key), mValues);
	}

	virtual CCSSettingColorValue ReadColorAtKey (const std::string &plugin,
						     const std::string &key)
	{
            return ValueForKeyRetreival <CCSSettingColorValue> ().GetValueForKey (
                keynameFromPluginKey (plugin, key), mValues);
	}

	virtual CCSSettingKeyValue ReadKeyAtKey (const std::string &plugin,
						 const std::string &key)
	{
            return ValueForKeyRetreival <CCSSettingKeyValue> ().GetValueForKey (
                keynameFromPluginKey (plugin, key), mValues);
	}

	virtual CCSSettingButtonValue ReadButtonAtKey (const std::string &plugin,
						       const std::string &key)
	{
            return ValueForKeyRetreival <CCSSettingButtonValue> ().GetValueForKey (
                keynameFromPluginKey (plugin, key), mValues);
	}

	virtual unsigned int ReadEdgeAtKey (const std::string &plugin,
				       const std::string &key)
	{
            return ValueForKeyRetreival <unsigned int> ().GetValueForKey (
                keynameFromPluginKey (plugin, key), mValues);
	}

	virtual const char * ReadMatchAtKey (const std::string &plugin,
					     const std::string &key)
	{
            return ValueForKeyRetreival <const char *> ().GetValueForKey (
                keynameFromPluginKey (plugin, key), mValues);
	}

	virtual Bool ReadBellAtKey (const std::string &plugin,
				       const std::string &key)
	{
            return compizconfig::test::boolToBool (
                ValueForKeyRetreival <bool> ().GetValueForKey (
                    keynameFromPluginKey (plugin, key), mValues));
	}

	virtual CCSSettingValueList ReadListAtKey (const std::string &plugin,
						   const std::string &key,
						   CCSSetting	     *setting)
	{
	    cci::SettingValueListWrapper::Ptr lw (ValueForKeyRetreival <boost::shared_ptr <cci::SettingValueListWrapper> > ().GetValueForKey (keynameFromPluginKey (plugin, key), mValues));

	    return ccsCopyList (*lw, lw->setting ().get ());
	}

    private:

	std::map <std::string, VariantTypes> mValues;
};

class MockCCSBackendConceptTestEnvironment :
    public CCSBackendConceptTestEnvironmentInterface
{
    private:

	virtual void SetUp ()
	{
	    mMockCCSSettingTestEnvironment.SetUp ();
	}

	virtual void TearDown ()
	{
	    mMockCCSSettingTestEnvironment.SetUp ();
	}

    public:

	CCSBackend * SetUp (CCSContext *context, CCSContextGMock *gmockContext)
	{
	    mContext = context;
	    mBackend = ccsMockBackendNew ();
	    mBackendGMock = (CCSBackendGMock *) ccsObjectGetPrivate (mBackend);

	    ON_CALL (*mBackendGMock, readSetting (_, _))
		    .WillByDefault (
			WithArgs<1> (
			    Invoke (
				this,
				&MockCCSBackendConceptTestEnvironment::ReadValueIntoSetting)));

	    ON_CALL (*mBackendGMock, updateSetting (_, _, _))
		    .WillByDefault (
			WithArgs<2> (
			    Invoke (
				this,
				&MockCCSBackendConceptTestEnvironment::ReadValueIntoSetting)));

	    ON_CALL (*mBackendGMock, writeSetting (_, _))
		    .WillByDefault (
			WithArgs<1> (
			    Invoke (
				this,
				&MockCCSBackendConceptTestEnvironment::WriteValueToMap)));

	    ON_CALL (*mBackendGMock, deleteProfile (_, _))
		    .WillByDefault (
			WithArgs <1> (
			    Invoke (
				this,
				&MockCCSBackendConceptTestEnvironment::DeleteProfile)));

	    return mBackend;
	}

	const CCSBackendInfo * GetInfo ()
	{
	    EXPECT_CALL (*mBackendGMock, getInfo ()).WillOnce (Return (&mockBackendInfo));

	    return &mockBackendInfo;
	}

	void TearDown (CCSBackend *backend)
	{
	    ccsFreeMockBackend (backend);
	}

	void AddProfile (const std::string &profile)
	{
	    mProfiles.push_back (profile);
	}

	void SetGetExistingProfilesExpectation (CCSContext      *context,
						CCSContextGMock *gmockContext)
	{
	    CCSStringList stringList = NULL;

	    CCSString *defaultProfile = reinterpret_cast <CCSString *> (calloc (1, sizeof (CCSString)));
	    CCSString *currentProfile = reinterpret_cast <CCSString *> (calloc (1, sizeof (CCSString)));

	    EXPECT_CALL (*gmockContext, getProfile ());

	    defaultProfile->value = strdup ("Default");
	    currentProfile->value = strdup (ccsGetProfile (context));

	    ccsStringRef (defaultProfile);
	    ccsStringRef (currentProfile);

	    stringList = ccsStringListAppend (stringList, defaultProfile);
	    stringList = ccsStringListAppend (stringList, currentProfile);

	    for (std::vector <std::string>::iterator it = mProfiles.begin ();
		 it != mProfiles.end ();
		 ++it)
	    {
		if (*it == defaultProfile->value ||
		    *it == currentProfile->value)
		    continue;

		CCSString *string = reinterpret_cast <CCSString *> (calloc (1, sizeof (CCSString)));

		string->value = strdup ((*it).c_str ());
		ccsStringRef (string);

		stringList = ccsStringListAppend (stringList, string);
	    }

	    EXPECT_CALL (*mBackendGMock, getExistingProfiles (context)).WillOnce (Return (stringList));
	}

	void SetDeleteProfileExpectation (const std::string &profileForDeletion,
					  CCSContext	    *context,
					  CCSContextGMock   *gmockContext)
	{
	    EXPECT_CALL (*mBackendGMock, deleteProfile (context,
							Eq (profileForDeletion)));
	}

	void SetReadInitExpectation (CCSContext *context,
				     CCSContextGMock *gmockContext)
	{
	    EXPECT_CALL (*mBackendGMock, readInit (context)).WillOnce (Return (TRUE));
	}

	void SetReadDoneExpectation (CCSContext *context,
				     CCSContextGMock *gmockContext)
	{
	    EXPECT_CALL (*mBackendGMock, readDone (context));
	}

	void SetWriteInitExpectation (CCSContext *context,
				      CCSContextGMock *gmockContext)
	{
	    EXPECT_CALL (*mBackendGMock, writeInit (context)).WillOnce (Return (TRUE));
	}

	void SetWriteDoneExpectation (CCSContext *context,
				      CCSContextGMock *gmockContext)
	{
	    EXPECT_CALL (*mBackendGMock, writeDone (context));
	}

	void PreWrite (CCSContextGMock *gmockContext,
		       CCSPluginGMock  *gmockPlugin,
		       CCSSettingGMock *gmockSetting,
		       CCSSettingType  type)
	{
	    EXPECT_CALL (*mBackendGMock, writeSetting (_, _));
	    EXPECT_CALL (*gmockPlugin, getName ());
	    EXPECT_CALL (*gmockSetting, getName ());
	    EXPECT_CALL (*gmockSetting, getParent ());

	    testing::Cardinality cardinality;

	    if (type == TypeList)
		cardinality = ::testing::AtLeast (1);
	    else
		cardinality = ::testing::AtMost (1);

	    EXPECT_CALL (*gmockSetting, getType ()).Times (cardinality);
	}

	void PostWrite (CCSContextGMock *gmockContext,
			CCSPluginGMock  *gmockPlugin,
			CCSSettingGMock *gmockSetting,
			CCSSettingType  type)
	{
	}

	void PreRead (CCSContextGMock *gmockContext,
		      CCSPluginGMock  *gmockPlugin,
		      CCSSettingGMock *gmockSetting,
		      CCSSettingType  type)
	{
	    EXPECT_CALL (*mBackendGMock, readSetting (_, _));
	    EXPECT_CALL (*gmockPlugin, getName ());
	    EXPECT_CALL (*gmockSetting, getName ());
	    EXPECT_CALL (*gmockSetting, getParent ());

	    if (type == TypeList)
	    {
		EXPECT_CALL (*gmockSetting, getType ()).Times (AtLeast (1));
		EXPECT_CALL (*gmockSetting, getInfo ()).Times (AtLeast (1));
	    }
	    else
	    {
		EXPECT_CALL (*gmockSetting, getType ());
	    }
	}

	void PostRead (CCSContextGMock *gmockContext,
		       CCSPluginGMock  *gmockPlugin,
		       CCSSettingGMock *gmockSetting,
		       CCSSettingType  type)
	{
	}

	void PreUpdate (CCSContextGMock *gmockContext,
		      CCSPluginGMock  *gmockPlugin,
		      CCSSettingGMock *gmockSetting,
		      CCSSettingType  type)
	{
	    EXPECT_CALL (*mBackendGMock, updateSetting (_, _, _));
	    EXPECT_CALL (*gmockPlugin, getName ());
	    EXPECT_CALL (*gmockSetting, getName ());
	    EXPECT_CALL (*gmockSetting, getParent ());

	    if (type == TypeList)
	    {
		EXPECT_CALL (*gmockSetting, getType ()).Times (AtLeast (1));
		EXPECT_CALL (*gmockSetting, getInfo ()).Times (AtLeast (1));
	    }
	    else
	    {
		EXPECT_CALL (*gmockSetting, getType ());
	    }
	}

	void PostUpdate (CCSContextGMock *gmockContext,
		       CCSPluginGMock  *gmockPlugin,
		       CCSSettingGMock *gmockSetting,
		       CCSSettingType  type)
	{
	}

	bool UpdateSettingAtKey (const std::string &plugin,
				 const std::string &setting)
	{
	    CCSPlugin *cplugin = ccsFindPlugin (mContext, plugin.c_str ());

	    if (!cplugin)
		return false;

	    CCSSetting *csetting = ccsFindSetting (cplugin, setting.c_str ());

	    if (!csetting)
		return false;

	    ccsBackendUpdateSetting (mBackend, mContext, cplugin, csetting);
	    return true;
	}

	void WriteBoolAtKey (const std::string &plugin,
			     const std::string &key,
			     const VariantTypes &value)
	{
	    mMockCCSSettingTestEnvironment.WriteBoolAtKey (plugin, key, value);
	}

	void WriteIntegerAtKey (const std::string &plugin,
				const std::string &key,
				const VariantTypes &value)

	{
	    mMockCCSSettingTestEnvironment.WriteIntegerAtKey (plugin, key, value);
	}

	void WriteFloatAtKey (const std::string &plugin,
			      const std::string &key,
			      const VariantTypes &value)
	{
	    mMockCCSSettingTestEnvironment.WriteFloatAtKey (plugin, key, value);
	}

	void WriteMatchAtKey (const std::string &plugin,
			      const std::string &key,
			      const VariantTypes &value)
	{
	    mMockCCSSettingTestEnvironment.WriteMatchAtKey (plugin, key, value);;
	}

	void WriteStringAtKey (const std::string &plugin,
			       const std::string &key,
			       const VariantTypes &value)
	{
	    mMockCCSSettingTestEnvironment.WriteStringAtKey (plugin, key, value);
	}

	void WriteColorAtKey (const std::string &plugin,
			      const std::string &key,
			      const VariantTypes &value)
	{
	    mMockCCSSettingTestEnvironment.WriteColorAtKey (plugin, key, value);
	}

	void WriteKeyAtKey (const std::string &plugin,
			    const std::string &key,
			    const VariantTypes &value)
	{
	    mMockCCSSettingTestEnvironment.WriteKeyAtKey (plugin, key, value);
	}

	void WriteButtonAtKey (const std::string &plugin,
			       const std::string &key,
			       const VariantTypes &value)
	{
	    mMockCCSSettingTestEnvironment.WriteButtonAtKey (plugin, key, value);
	}

	void WriteEdgeAtKey (const std::string &plugin,
			     const std::string &key,
			     const VariantTypes &value)
	{
	    mMockCCSSettingTestEnvironment.WriteEdgeAtKey (plugin, key, value);
	}

	void WriteBellAtKey (const std::string &plugin,
			     const std::string &key,
			     const VariantTypes &value)
	{
	    mMockCCSSettingTestEnvironment.WriteBellAtKey (plugin, key, value);
	}

	void WriteListAtKey (const std::string &plugin,
			     const std::string &key,
			     const VariantTypes &value)
	{
	    mMockCCSSettingTestEnvironment.WriteListAtKey (plugin, key, value);
	}

	virtual Bool ReadBoolAtKey (const std::string &plugin,
				    const std::string &key)
	{
	    return mMockCCSSettingTestEnvironment.ReadBoolAtKey (plugin, key);
	}

	virtual int ReadIntegerAtKey (const std::string &plugin,
				      const std::string &key)
	{
	    return mMockCCSSettingTestEnvironment.ReadIntegerAtKey (plugin, key);
	}

	virtual float ReadFloatAtKey (const std::string &plugin,
				      const std::string &key)
	{
	    return mMockCCSSettingTestEnvironment.ReadFloatAtKey (plugin, key);;
	}

	virtual const char * ReadStringAtKey (const std::string &plugin,
					      const std::string &key)
	{
	    return mMockCCSSettingTestEnvironment.ReadStringAtKey (plugin, key);
	}

	virtual CCSSettingColorValue ReadColorAtKey (const std::string &plugin,
						     const std::string &key)
	{
	    return mMockCCSSettingTestEnvironment.ReadColorAtKey (plugin, key);
	}

	virtual CCSSettingKeyValue ReadKeyAtKey (const std::string &plugin,
						 const std::string &key)
	{
	    return mMockCCSSettingTestEnvironment.ReadKeyAtKey (plugin, key);
	}

	virtual CCSSettingButtonValue ReadButtonAtKey (const std::string &plugin,
						       const std::string &key)
	{
	    return mMockCCSSettingTestEnvironment.ReadButtonAtKey (plugin, key);
	}

	virtual unsigned int ReadEdgeAtKey (const std::string &plugin,
				       const std::string &key)
	{
	    return mMockCCSSettingTestEnvironment.ReadEdgeAtKey (plugin, key);
	}

	virtual const char * ReadMatchAtKey (const std::string &plugin,
					     const std::string &key)
	{
	    return mMockCCSSettingTestEnvironment.ReadMatchAtKey (plugin, key);
	}

	virtual Bool ReadBellAtKey (const std::string &plugin,
				       const std::string &key)
	{
	    return mMockCCSSettingTestEnvironment.ReadBellAtKey (plugin, key);
	}

	virtual CCSSettingValueList ReadListAtKey (const std::string &plugin,
						   const std::string &key,
						   CCSSetting	     *setting)
	{
	    return mMockCCSSettingTestEnvironment.ReadListAtKey (plugin, key, setting);
	}

    protected:

	bool DeleteProfile (const std::string &profileToDelete)
	{
	    std::vector <std::string>::iterator it = std::find (mProfiles.begin (),
								mProfiles.end (),
								profileToDelete);

	    if (it != mProfiles.end ())
	    {
		mProfiles.erase (it);
		return true;
	    }
	    else
	    {
		return false;
	    }
	}

    private:

	void ReadValueIntoSetting (CCSSetting *setting)
	{
	    std::string plugin (ccsPluginGetName (ccsSettingGetParent (setting)));
	    std::string key (ccsSettingGetName (setting));

	    switch (ccsSettingGetType (setting))
	    {
		case TypeBool:

		    ccsSetBool (setting, ReadBoolAtKey (plugin, key), FALSE);
		    break;

		case TypeInt:

		    ccsSetInt (setting, ReadIntegerAtKey (plugin, key), FALSE);
		    break;

		case TypeFloat:

		    ccsSetFloat (setting, ReadFloatAtKey (plugin, key), FALSE);
		    break;

		case TypeString:

		    ccsSetString (setting, ReadStringAtKey (plugin, key), FALSE);
		    break;

		case TypeMatch:

		    ccsSetMatch (setting, ReadMatchAtKey (plugin, key), FALSE);
		    break;

		case TypeColor:

		    ccsSetColor (setting, ReadColorAtKey (plugin, key), FALSE);
		    break;

		case TypeKey:

		    ccsSetKey (setting, ReadKeyAtKey (plugin, key), FALSE);
		    break;

		case TypeButton:

		    ccsSetButton (setting, ReadButtonAtKey (plugin, key), FALSE);
		    break;

		case TypeEdge:

		    ccsSetEdge (setting, ReadEdgeAtKey (plugin, key), FALSE);
		    break;

		case TypeBell:

		    ccsSetBell (setting, ReadBellAtKey (plugin, key), FALSE);
		    break;

		case TypeList:
		    ccsSetList (setting,
				cci::SettingValueListWrapper (ReadListAtKey (plugin, key, setting),
								 cci::Deep,
								 ccsSettingGetInfo (setting)->forList.listType,
								 boost::shared_ptr <CCSSettingInfo> (),
								 boost::shared_ptr <CCSSetting> (setting,
												 boost::bind (doNothingWithCCSSetting, _1))),
				FALSE);
		    break;

		default:

		    throw std::exception ();
	    }
	}

	void WriteValueToMap (CCSSetting *setting)
	{
	    std::string plugin (ccsPluginGetName (ccsSettingGetParent (setting)));
	    std::string key (ccsSettingGetName (setting));

	    Bool vBool;
	    int  vInt;
	    float vFloat;
	    char *vString;
	    CCSSettingColorValue vColor;
	    CCSSettingKeyValue vKey;
	    CCSSettingButtonValue vButton;
	    unsigned int vEdge;
	    CCSSettingValueList vList;

	    switch (ccsSettingGetType (setting))
	    {
		case TypeBool:

		    ccsGetBool (setting, &vBool);
		    WriteBoolAtKey (plugin, key, VariantTypes (vBool ? true : false));
		    break;

		case TypeInt:

		    ccsGetInt (setting, &vInt);
		    WriteIntegerAtKey (plugin, key, VariantTypes (vInt));
		    break;

		case TypeFloat:

		    ccsGetFloat (setting, &vFloat);
		    WriteFloatAtKey (plugin, key, VariantTypes (vFloat));
		    break;

		case TypeString:

		    ccsGetString (setting, &vString);
		    WriteStringAtKey (plugin, key, VariantTypes (static_cast <const char *> (vString)));
		    break;

		case TypeMatch:

		    ccsGetMatch (setting, &vString);
		    WriteStringAtKey (plugin, key, VariantTypes (static_cast <const char *> (vString)));
		    break;

		case TypeColor:

		    ccsGetColor (setting, &vColor);
		    WriteColorAtKey (plugin, key, VariantTypes (vColor));
		    break;

		case TypeKey:

		    ccsGetKey (setting, &vKey);
		    WriteKeyAtKey (plugin, key, VariantTypes (vKey));
		    break;

		case TypeButton:

		    ccsGetButton (setting, &vButton);
		    WriteButtonAtKey (plugin, key, VariantTypes (vButton));
		    break;

		case TypeEdge:

		    ccsGetEdge (setting, &vEdge);
		    WriteEdgeAtKey (plugin, key, VariantTypes (vEdge));
		    break;

		case TypeBell:

		    ccsGetBell (setting, &vBool);
		    WriteBellAtKey (plugin, key, VariantTypes (vBool ? true : false));
		    break;

		case TypeList:
		{
		    CCSSettingValueList listCopy = NULL;

		    ccsGetList (setting, &vList);
		    listCopy = ccsCopyList (vList, setting);

		    WriteListAtKey (plugin, key, VariantTypes (boost::make_shared <cci::SettingValueListWrapper> (listCopy,
														     cci::Deep,
														     ccsSettingGetInfo (setting)->forList.listType,
														     boost::shared_ptr <CCSSettingInfo> (),
														     boost::shared_ptr <CCSSetting> (setting, boost::bind (doNothingWithCCSSetting, _1)))));
		    break;
		}
		default:

		    throw std::exception ();
	    }
	}

    private:

	MockCCSSettingsTestEnvironment mMockCCSSettingTestEnvironment;
	CCSBackend *mBackend;
	CCSBackendGMock *mBackendGMock;
	CCSContext *mContext;
	std::vector <std::string> mProfiles;
};

INSTANTIATE_TEST_CASE_P (MockCCSBackendConcept, CCSBackendConformanceTestReadWrite,
			 compizconfig::test::GenerateTestingParametersForBackendInterface <MockCCSBackendConceptTestEnvironment> ());

INSTANTIATE_TEST_CASE_P (MockCCSBackendConcept, CCSBackendConformanceTestInfo,
			 compizconfig::test::GenerateTestingEnvFactoryBackendInterface <MockCCSBackendConceptTestEnvironment> ());

INSTANTIATE_TEST_CASE_P (MockCCSBackendConcept, CCSBackendConformanceTestInitFiniFuncs,
			 compizconfig::test::GenerateTestingEnvFactoryBackendInterface <MockCCSBackendConceptTestEnvironment> ());

INSTANTIATE_TEST_CASE_P (MockCCSBackendConcept, CCSBackendConformanceTestProfileHandling,
			 compizconfig::test::GenerateTestingEnvFactoryBackendInterface <MockCCSBackendConceptTestEnvironment> ());
