/*
 * Compiz configuration system library
 *
 * Copyright (C) 2012 Canonical Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Authored By:
 * Sam Spilsbury <sam.spilsbury@canonical.com>
 */
#ifndef _COMPIZCONFIG_CCS_BACKEND_CONCEPT_TEST_ENVIRONMENT_INTERFACE_H
#define _COMPIZCONFIG_CCS_BACKEND_CONCEPT_TEST_ENVIRONMENT_INTERFACE_H

#include <string>
#include <boost/shared_ptr.hpp>
#include "compizconfig_ccs_settings_test_fixture.h"

typedef struct _CCSContext CCSContext;
typedef struct _CCSBackendInfo CCSBackendInfo;

class CCSContextGMock;
class CCSPluginGMock;
class CCSSettingGMock;

class CCSBackendConceptTestEnvironmentInterface :
    public CCSSettingsConceptTestEnvironmentInterface
{
    public:

	typedef boost::shared_ptr <CCSBackendConceptTestEnvironmentInterface> Ptr;

	virtual ~CCSBackendConceptTestEnvironmentInterface () {};
	virtual CCSBackend * SetUp (CCSContext *context,
				    CCSContextGMock *gmockContext) = 0;
	virtual void TearDown (CCSBackend *) = 0;

	virtual void AddProfile (const std::string &profile) = 0;
	virtual void SetGetExistingProfilesExpectation (CCSContext *,
							CCSContextGMock *) = 0;
	virtual void SetDeleteProfileExpectation (const std::string &,
						  CCSContext	    *,
						  CCSContextGMock   *) = 0;

	virtual void SetReadInitExpectation (CCSContext      *,
					     CCSContextGMock *) = 0;

	virtual void SetReadDoneExpectation (CCSContext      *,
					     CCSContextGMock *) = 0;

	virtual void SetWriteInitExpectation (CCSContext      *,
					      CCSContextGMock *) = 0;

	virtual void SetWriteDoneExpectation (CCSContext      *,
					      CCSContextGMock *) = 0;

	virtual const CCSBackendInfo * GetInfo () = 0;

	virtual void PreWrite (CCSContextGMock *,
			       CCSPluginGMock  *,
			       CCSSettingGMock *,
			       CCSSettingType) = 0;
	virtual void PostWrite (CCSContextGMock *,
				CCSPluginGMock  *,
				CCSSettingGMock *,
				CCSSettingType) = 0;

	virtual void PreRead (CCSContextGMock *,
			      CCSPluginGMock  *,
			      CCSSettingGMock *,
			      CCSSettingType) = 0;
	virtual void PostRead (CCSContextGMock *,
			       CCSPluginGMock  *,
			       CCSSettingGMock *,
			       CCSSettingType) = 0;

	virtual void PreUpdate (CCSContextGMock *,
			      CCSPluginGMock  *,
			      CCSSettingGMock *,
			      CCSSettingType) = 0;
	virtual void PostUpdate (CCSContextGMock *,
			       CCSPluginGMock  *,
			       CCSSettingGMock *,
			       CCSSettingType) = 0;

	virtual bool UpdateSettingAtKey (const std::string &plugin,
					 const std::string &setting) = 0;
};

#endif
