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
#ifndef _COMPIZCONFIG_TEST_GSETTINGS_TESTS_H
#define _COMPIZCONFIG_TEST_GSETTINGS_TESTS_H

#include <gtest/gtest.h>
#include <glib.h>
#include <glib-object.h>
#include <glib_gslice_off_env.h>
#include <glib_gsettings_memory_backend_env.h>
#include <gsettings-mock-schemas-config.h>

using ::testing::TestWithParam;

class CCSGSettingsTeardownSetupInterface
{
    public:
	virtual void SetUp () = 0;
	virtual void TearDown () = 0;
};

class CCSGSettingsTestingEnv
{
    public:

	virtual void SetUpEnv ()
	{
	    gsliceEnv.SetUpEnv ();
	    g_type_init ();
	}

	virtual void TearDownEnv ()
	{
	    gsliceEnv.TearDownEnv ();
	}

    private:

	CompizGLibGSliceOffEnv gsliceEnv;
};

class CCSGSettingsMemoryBackendTestingEnv :
    public CCSGSettingsTestingEnv
{
    public:

	virtual void SetUpEnv ()
	{
	    CCSGSettingsTestingEnv::SetUpEnv ();
	    gsettingsEnv.SetUpEnv (MOCK_PATH);
	}

	virtual void TearDownEnv ()
	{
	    gsettingsEnv.TearDownEnv ();
	    CCSGSettingsTestingEnv::TearDownEnv ();
	}

    private:

	CompizGLibGSettingsMemoryBackendTestingEnv gsettingsEnv;
};

class CCSGSettingsTestCommon :
    public ::testing::Test
{
    public:

	virtual void SetUp ()
	{
	    env.SetUpEnv ();
	}

	virtual void TearDown ()
	{
	    env.TearDownEnv ();
	}

    private:

	CompizGLibGSliceOffEnv env;
};

class CCSGSettingsTest :
    public CCSGSettingsTestCommon,
    public ::testing::WithParamInterface <CCSGSettingsTeardownSetupInterface *>
{
    public:

	CCSGSettingsTest () :
	    mFuncs (GetParam ())
	{
	}

	virtual void SetUp ()
	{
	    CCSGSettingsTestCommon::SetUp ();
	    mFuncs->SetUp ();
	}

	virtual void TearDown ()
	{
	    CCSGSettingsTestCommon::TearDown ();
	    mFuncs->TearDown ();
	}

    private:

	CCSGSettingsTeardownSetupInterface *mFuncs;
};

class CCSGSettingsTestIndependent :
    public CCSGSettingsTestCommon
{
    public:

	virtual void SetUp ()
	{
	    CCSGSettingsTestCommon::SetUp ();
	    g_type_init ();
	}

	virtual void TearDown ()
	{
	    CCSGSettingsTestCommon::TearDown ();
	}
};

class CCSGSettingsTestWithMemoryBackend :
    public CCSGSettingsTestIndependent
{
    public:

	virtual void SetUp ()
	{
	    CCSGSettingsTestIndependent::SetUp ();
	    env.SetUpEnv (MOCK_PATH);
	}

	virtual void TearDown ()
	{
	    env.TearDownEnv ();
	}
    private:

	CompizGLibGSettingsMemoryBackendTestingEnv env;
};

#endif
