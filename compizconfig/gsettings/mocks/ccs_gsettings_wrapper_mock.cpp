#include <ccs_gsettings_wrapper_mock.h>

const CCSGSettingsWrapperInterface mockInterface =
{
    CCSGSettingsWrapperGMock::ccsGSettingsWrapperSetValue,
    CCSGSettingsWrapperGMock::ccsGSettingsWrapperGetValue,
    CCSGSettingsWrapperGMock::ccsGSettingsWrapperResetKey,
    CCSGSettingsWrapperGMock::ccsGSettingsWrapperListKeys,
    CCSGSettingsWrapperGMock::ccsGSettingsWrapperGetGSettings,
    CCSGSettingsWrapperGMock::ccsGSettingsWrapperGetSchemaName,
    CCSGSettingsWrapperGMock::ccsGSettingsWrapperGetPath,
    CCSGSettingsWrapperGMock::ccsGSettingsWrapperConnectToChangedSignal,
    CCSGSettingsWrapperGMock::ccsFreeGSettingsWrapper
};

CCSGSettingsWrapper *
ccsMockGSettingsWrapperNew ()
{
    CCSGSettingsWrapper *wrapper = (CCSGSettingsWrapper *) calloc (1, sizeof (CCSGSettingsWrapper));

    if (!wrapper)
	return NULL;

    CCSGSettingsWrapperGMock *gmockWrapper = new CCSGSettingsWrapperGMock (wrapper);

    if (!gmockWrapper)
    {
	free (wrapper);
	return NULL;
    }

    ccsObjectInit (wrapper, &ccsDefaultObjectAllocator);
    ccsObjectAddInterface (wrapper, (const CCSInterface *) &mockInterface, GET_INTERFACE_TYPE (CCSGSettingsWrapperInterface));
    ccsObjectSetPrivate (wrapper, (CCSPrivate *) gmockWrapper);

    ccsGSettingsWrapperRef (wrapper);

    return wrapper;
}

void
ccsMockGSettingsWrapperFree (CCSGSettingsWrapper *wrapper)
{
    CCSGSettingsWrapperGMock *gmockWrapper = reinterpret_cast <CCSGSettingsWrapperGMock *> (ccsObjectGetPrivate (wrapper));

    delete gmockWrapper;

    ccsObjectSetPrivate (wrapper, NULL);
    ccsObjectFinalize (wrapper);
    free (wrapper);
}
    
