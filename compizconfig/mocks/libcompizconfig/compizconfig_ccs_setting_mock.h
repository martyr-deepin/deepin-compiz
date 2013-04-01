#ifndef _COMPIZCONFIG_CCS_SETTING_MOCK
#define _COMPIZCONFIG_CCS_SETTING_MOCK

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>

using ::testing::_;
using ::testing::Return;

CCSSetting * ccsMockSettingNew ();
CCSSetting * ccsNiceMockSettingNew ();
void ccsFreeMockSetting (CCSSetting *);

class CCSSettingGMockInterface
{
    public:

	virtual ~CCSSettingGMockInterface () {};

	virtual const char * getName () = 0;
	virtual const char * getShortDesc () = 0;
	virtual const char * getLongDesc () = 0;
	virtual CCSSettingType getType () = 0;
	virtual CCSSettingInfo * getInfo () = 0;
	virtual const char * getGroup () = 0;
	virtual const char * getSubGroup () = 0;
	virtual const char * getHints () = 0;
	virtual CCSSettingValue * getDefaultValue () = 0;
	virtual CCSSettingValue * getValue () = 0;
	virtual Bool getIsDefault () = 0;
	virtual CCSPlugin * getParent () = 0;
	virtual void * getPrivatePtr () = 0;
	virtual void setPrivatePtr (void *) = 0;
	virtual Bool setInt (int, Bool) = 0;
	virtual Bool setFloat (float, Bool) = 0;
	virtual Bool setBool (Bool, Bool) = 0;
	virtual Bool setString (const char *, Bool) = 0;
	virtual Bool setColor (CCSSettingColorValue, Bool) = 0;
	virtual Bool setMatch (const char *, Bool) = 0;
	virtual Bool setKey (CCSSettingKeyValue, Bool) = 0;
	virtual Bool setButton (CCSSettingButtonValue, Bool) = 0;
	virtual Bool setEdge (unsigned int, Bool) = 0;
	virtual Bool setBell (Bool, Bool) = 0;
	virtual Bool setList (CCSSettingValueList, Bool) = 0;
	virtual Bool setValue (CCSSettingValue *, Bool) = 0;
	virtual Bool getInt (int *) = 0;
	virtual Bool getFloat (float *) = 0;
	virtual Bool getBool (Bool *) = 0;
	virtual Bool getString (char **) = 0;
	virtual Bool getColor (CCSSettingColorValue *) = 0;
	virtual Bool getMatch (char **) = 0;
	virtual Bool getKey (CCSSettingKeyValue *) = 0;
	virtual Bool getButton (CCSSettingButtonValue *) = 0;
	virtual Bool getEdge (unsigned int *) = 0;
	virtual Bool getBell (Bool *) = 0;
	virtual Bool getList (CCSSettingValueList *) = 0;
	virtual void resetToDefault (Bool) = 0;
	virtual Bool isIntegrated () = 0;
	virtual Bool isReadOnly () = 0;
	virtual Bool isReadableByBackend () = 0;
};

class CCSSettingGMock :
    public CCSSettingGMockInterface
{
    public:

	/* Mock implementations */

	CCSSettingGMock (CCSSetting *s) :
	    mSetting (s)
	{
	    /* Teach GMock how to handle it */
	    ON_CALL (*this, getType ()).WillByDefault (Return (TypeNum));
	}

	CCSSetting * setting () { return mSetting; }

	MOCK_METHOD0 (getName, const char * ());
	MOCK_METHOD0 (getShortDesc, const char * ());
	MOCK_METHOD0 (getLongDesc, const char * ());
	MOCK_METHOD0 (getType, CCSSettingType ());
	MOCK_METHOD0 (getInfo, CCSSettingInfo * ());
	MOCK_METHOD0 (getGroup, const char * ());
	MOCK_METHOD0 (getSubGroup, const char * ());
	MOCK_METHOD0 (getHints, const char * ());
	MOCK_METHOD0 (getDefaultValue, CCSSettingValue * ());
	MOCK_METHOD0 (getValue, CCSSettingValue * ());
	MOCK_METHOD0 (getIsDefault, Bool ());
	MOCK_METHOD0 (getParent, CCSPlugin * ());
	MOCK_METHOD0 (getPrivatePtr, void * ());
	MOCK_METHOD1 (setPrivatePtr, void (void *));
	MOCK_METHOD2 (setInt, Bool (int, Bool));
	MOCK_METHOD2 (setFloat, Bool (float, Bool));
	MOCK_METHOD2 (setBool, Bool (Bool, Bool));
	MOCK_METHOD2 (setString, Bool (const char *, Bool));
	MOCK_METHOD2 (setColor, Bool (CCSSettingColorValue, Bool));
	MOCK_METHOD2 (setMatch, Bool (const char *, Bool));
	MOCK_METHOD2 (setKey, Bool (CCSSettingKeyValue, Bool));
	MOCK_METHOD2 (setButton, Bool (CCSSettingButtonValue, Bool));
	MOCK_METHOD2 (setEdge, Bool (unsigned int, Bool));
	MOCK_METHOD2 (setBell, Bool (Bool, Bool));
	MOCK_METHOD2 (setList, Bool (CCSSettingValueList, Bool));
	MOCK_METHOD2 (setValue, Bool (CCSSettingValue *, Bool));
	MOCK_METHOD1 (getInt, Bool (int *));
	MOCK_METHOD1 (getFloat, Bool (float *));
	MOCK_METHOD1 (getBool, Bool (Bool *));
	MOCK_METHOD1 (getString, Bool (char **));
	MOCK_METHOD1 (getColor, Bool (CCSSettingColorValue *));
	MOCK_METHOD1 (getMatch, Bool (char **));
	MOCK_METHOD1 (getKey, Bool (CCSSettingKeyValue *));
	MOCK_METHOD1 (getButton, Bool (CCSSettingButtonValue *));
	MOCK_METHOD1 (getEdge, Bool (unsigned int *));
	MOCK_METHOD1 (getBell, Bool (Bool *));
	MOCK_METHOD1 (getList, Bool (CCSSettingValueList *));
	MOCK_METHOD1 (resetToDefault, void (Bool));
	MOCK_METHOD0 (isIntegrated, Bool ());
	MOCK_METHOD0 (isReadOnly, Bool ());
	MOCK_METHOD0 (isReadableByBackend, Bool ());

    private:

	CCSSetting *mSetting;

    public:

	static Bool ccsGetInt (CCSSetting *setting,
			int        *data)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getInt (data);
	}

	static Bool ccsGetFloat (CCSSetting *setting,
			  float      *data)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getFloat (data);
	}

	static Bool ccsGetBool (CCSSetting *setting,
			 Bool       *data)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getBool (data);
	}

	static Bool ccsGetString (CCSSetting *setting,
			   char       **data)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getString (data);
	}

	static Bool ccsGetColor (CCSSetting           *setting,
			  CCSSettingColorValue *data)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getColor (data);
	}

	static Bool ccsGetMatch (CCSSetting *setting,
			  char       **data)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getMatch (data);
	}

	static Bool ccsGetKey (CCSSetting         *setting,
			CCSSettingKeyValue *data)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getKey (data);
	}

	static Bool ccsGetButton (CCSSetting            *setting,
			   CCSSettingButtonValue *data)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getButton (data);
	}

	static Bool ccsGetEdge (CCSSetting  *setting,
			 unsigned int *data)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getEdge (data);
	}

	static Bool ccsGetBell (CCSSetting *setting,
			 Bool       *data)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getBell (data);
	}

	static Bool ccsGetList (CCSSetting          *setting,
			 CCSSettingValueList *data)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getList (data);
	}

	static Bool ccsSetInt (CCSSetting *setting,
			int        data,
			Bool	   processChanged)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->setInt (data, processChanged);
	}

	static Bool ccsSetFloat (CCSSetting *setting,
			  float      data,
			  Bool	     processChanged)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->setFloat (data, processChanged);
	}

	static Bool ccsSetBool (CCSSetting *setting,
			 Bool       data,
			 Bool	    processChanged)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->setBool (data, processChanged);
	}

	static Bool ccsSetString (CCSSetting *setting,
			   const char *data,
			   Bool	      processChanged)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->setString (data, processChanged);
	}

	static Bool ccsSetColor (CCSSetting           *setting,
			  CCSSettingColorValue data,
			  Bool		       processChanged)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->setColor (data, processChanged);
	}

	static Bool ccsSetMatch (CCSSetting *setting,
			  const char *data,
			  Bool	     processChanged)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->setMatch (data, processChanged);
	}

	static Bool ccsSetKey (CCSSetting         *setting,
			CCSSettingKeyValue data,
			Bool		   processChanged)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->setKey (data, processChanged);
	}

	static Bool ccsSetButton (CCSSetting            *setting,
			   CCSSettingButtonValue data,
			   Bool			 processChanged)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->setButton (data, processChanged);
	}

	static Bool ccsSetEdge (CCSSetting   *setting,
			 unsigned int data,
			 Bool	      processChanged)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->setEdge (data, processChanged);
	}

	static Bool ccsSetBell (CCSSetting *setting,
			 Bool       data,
			 Bool	    processChanged)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->setBell (data, processChanged);
	}

	static Bool ccsSetList (CCSSetting          *setting,
			 CCSSettingValueList data,
			 Bool	 processChanged)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->setList (data, processChanged);
	}

	static Bool ccsSetValue (CCSSetting      *setting,
			  CCSSettingValue *data,
			  Bool		  processChanged)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->setValue (data, processChanged);
	}

	static void ccsResetToDefault (CCSSetting * setting, Bool processChanged)
	{
	    ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->resetToDefault (processChanged);
	}

	static Bool ccsSettingIsIntegrated (CCSSetting *setting)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->isIntegrated ();
	}

	static Bool ccsSettingIsReadOnly (CCSSetting *setting)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->isReadOnly ();
	}

	static const char * ccsSettingGetName (CCSSetting *setting)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getName ();
	}

	static const char * ccsSettingGetShortDesc (CCSSetting *setting)

	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getShortDesc ();
	}

	static const char * ccsSettingGetLongDesc (CCSSetting *setting)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getLongDesc ();
	}

	static CCSSettingType ccsSettingGetType (CCSSetting *setting)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getType ();
	}

	static CCSSettingInfo * ccsSettingGetInfo (CCSSetting *setting)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getInfo ();
	}

	static const char * ccsSettingGetGroup (CCSSetting *setting)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getGroup ();
	}

	static const char * ccsSettingGetSubGroup (CCSSetting *setting)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getSubGroup ();
	}

	static const char * ccsSettingGetHints (CCSSetting *setting)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getHints ();
	}

	static CCSSettingValue * ccsSettingGetDefaultValue (CCSSetting *setting)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getDefaultValue ();
	}

	static CCSSettingValue *ccsSettingGetValue (CCSSetting *setting)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getValue ();
	}

	static Bool ccsSettingGetIsDefault (CCSSetting *setting)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getIsDefault ();
	}

	static CCSPlugin * ccsSettingGetParent (CCSSetting *setting)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getParent ();
	}

	static void * ccsSettingGetPrivatePtr (CCSSetting *setting)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->getPrivatePtr ();
	}

	static void ccsSettingSetPrivatePtr (CCSSetting *setting, void *ptr)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->setPrivatePtr (ptr);
	}

	static Bool ccsSettingIsReadableByBackend (CCSSetting *setting)
	{
	    return ((CCSSettingGMock *) ccsObjectGetPrivate (setting))->isReadableByBackend ();
	}

	static void ccsSettingFree (CCSSetting *setting)
	{
	    ccsFreeMockSetting (setting);
	}
};

extern CCSSettingInterface CCSSettingGMockInterface;

#endif
