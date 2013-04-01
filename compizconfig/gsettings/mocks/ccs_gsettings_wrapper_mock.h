#ifndef _COMPIZCONFIG_CCS_GSETTINGS_WRAPPER_MOCK
#define _COMPIZCONFIG_CCS_GSETTINGS_WRAPPER_MOCK

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs_gsettings_interface.h>

CCSGSettingsWrapper * ccsMockGSettingsWrapperNew ();
void ccsMockGSettingsWrapperFree (CCSGSettingsWrapper *);

class CCSGSettingsWrapperMockInterface
{
    public:

	virtual ~CCSGSettingsWrapperMockInterface () {}
	virtual void setValue (const char *, GVariant *) = 0;
	virtual GVariant * getValue (const char *) = 0;
	virtual void resetKey (const char *) = 0;
	virtual char ** listKeys () = 0;
	virtual GSettings * getGSettings () = 0;
	virtual const char * getSchemaName () = 0;
	virtual const char * getPath () = 0;
	virtual void connectToChangedSignal (GCallback, gpointer) = 0;
};

class CCSGSettingsWrapperGMock :
    public CCSGSettingsWrapperMockInterface
{
    public:

	CCSGSettingsWrapperGMock (CCSGSettingsWrapper *wrapper) :
	    mWrapper (wrapper)
	{
	}

	MOCK_METHOD2 (setValue, void (const char *, GVariant *));
	MOCK_METHOD1 (getValue, GVariant * (const char *));
	MOCK_METHOD1 (resetKey, void (const char *));
	MOCK_METHOD0 (listKeys, char ** ());
	MOCK_METHOD0 (getGSettings, GSettings * ());
	MOCK_METHOD0 (getSchemaName, const char * ());
	MOCK_METHOD0 (getPath, const char * ());
	MOCK_METHOD2 (connectToChangedSignal, void (GCallback, gpointer));

    private:

	CCSGSettingsWrapper *mWrapper;

    public:

	static void
	ccsGSettingsWrapperSetValue (CCSGSettingsWrapper *wrapper,
				     const char *key,
				     GVariant *value)
	{
	    reinterpret_cast <CCSGSettingsWrapperMockInterface *> (ccsObjectGetPrivate (wrapper))->setValue (key, value);
	}

	static GVariant *
	ccsGSettingsWrapperGetValue (CCSGSettingsWrapper *wrapper,
				     const char *key)
	{
	    return reinterpret_cast <CCSGSettingsWrapperMockInterface *> (ccsObjectGetPrivate (wrapper))->getValue (key);
	}

	static void
	ccsGSettingsWrapperResetKey (CCSGSettingsWrapper *wrapper,
				     const char *key)
	{
	    reinterpret_cast <CCSGSettingsWrapperMockInterface *> (ccsObjectGetPrivate (wrapper))->resetKey (key);
	}

	static char **
	ccsGSettingsWrapperListKeys (CCSGSettingsWrapper *wrapper)
	{
	    return reinterpret_cast <CCSGSettingsWrapperMockInterface *> (ccsObjectGetPrivate (wrapper))->listKeys ();
	}

	static GSettings *
	ccsGSettingsWrapperGetGSettings (CCSGSettingsWrapper *wrapper)
	{
	    return reinterpret_cast <CCSGSettingsWrapperMockInterface *> (ccsObjectGetPrivate (wrapper))->getGSettings ();
	}

	static const char *
	ccsGSettingsWrapperGetSchemaName (CCSGSettingsWrapper *wrapper)
	{
	    return reinterpret_cast <CCSGSettingsWrapperMockInterface *> (ccsObjectGetPrivate (wrapper))->getSchemaName ();
	}

	static const char *
	ccsGSettingsWrapperGetPath (CCSGSettingsWrapper *wrapper)
	{
	    return reinterpret_cast <CCSGSettingsWrapperMockInterface *> (ccsObjectGetPrivate (wrapper))->getPath ();
	}

	static void
	ccsGSettingsWrapperConnectToChangedSignal (CCSGSettingsWrapper *wrapper,
						   GCallback	       callback,
						   gpointer	       data)
	{
	    reinterpret_cast <CCSGSettingsWrapperMockInterface *> (ccsObjectGetPrivate (wrapper))->connectToChangedSignal (callback, data);
	}

	static void
	ccsFreeGSettingsWrapper (CCSGSettingsWrapper *wrapper)
	{
	    ccsMockGSettingsWrapperFree (wrapper);
	}
};

#endif
