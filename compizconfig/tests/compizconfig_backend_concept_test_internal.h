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
#ifndef _COMPIZCONFIG_CCS_BACKEND_CONCEPT_TEST_INTERNAL_H
#define _COMPIZCONFIG_CCS_BACKEND_CONCEPT_TEST_INTERNAL_H

#include <list>

#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <ccs.h>

#include "gtest_shared_characterwrapper.h"
#include "compizconfig_test_value_combiners.h"
#include "compizconfig_ccs_setting_value_operators.h"
#include "compizconfig_ccs_item_in_list_matcher.h"
#include "compizconfig_ccs_list_equality.h"
#include "compizconfig_ccs_list_wrapper.h"
#include "compizconfig_ccs_variant_types.h"
#include "compizconfig_ccs_settings_test_fixture.h"

namespace cci = compiz::config::impl;
namespace cc  = compiz::config;

class CCSBackendConceptTestEnvironmentInterface;
class CCSSettingGMock;

namespace compizconfig
{
    namespace test
    {
	typedef boost::function <void ()> WriteFunc;
	typedef boost::function <CCSSettingValueList (CCSSetting *)> ConstructorFunc;

	cci::SettingValueListWrapper::Ptr
	CCSListConstructionExpectationsSetter (const ConstructorFunc &c,
					       CCSSettingType        type,
					       cci::ListStorageType  storageType);

	Bool boolToBool (bool v);
	CCSSettingGMock * getSettingGMockFromSetting (const boost::shared_ptr <CCSSetting> &setting);
	void SetIntWriteExpectation (const std::string                                                   &plugin,
				     const std::string                                                   &key,
				     const VariantTypes                                                  &value,
				     const boost::shared_ptr <CCSSetting>                                &setting,
				     const WriteFunc                                                     &write,
				     const boost::shared_ptr <CCSBackendConceptTestEnvironmentInterface> &env);
	void SetBoolWriteExpectation (const std::string                                                   &plugin,
				      const std::string                                                   &key,
				      const VariantTypes                                                  &value,
				      const boost::shared_ptr <CCSSetting>                                &setting,
				      const WriteFunc                                                     &write,
				      const boost::shared_ptr <CCSBackendConceptTestEnvironmentInterface> &env);
	void SetFloatWriteExpectation (const std::string                                                   &plugin,
				       const std::string                                                   &key,
				       const VariantTypes                                                  &value,
				       const boost::shared_ptr <CCSSetting>                                &setting,
				       const WriteFunc                                                     &write,
				       const boost::shared_ptr <CCSBackendConceptTestEnvironmentInterface> &env);
	void SetStringWriteExpectation (const std::string                                                   &plugin,
					const std::string                                                   &key,
					const VariantTypes                                                  &value,
					const boost::shared_ptr <CCSSetting>                                &setting,
					const WriteFunc                                                     &write,
					const boost::shared_ptr <CCSBackendConceptTestEnvironmentInterface> &env);
	void SetColorWriteExpectation (const std::string                                                   &plugin,
				       const std::string                                                   &key,
				       const VariantTypes                                                  &value,
				       const boost::shared_ptr <CCSSetting>                                &setting,
				       const WriteFunc                                                     &write,
				       const boost::shared_ptr <CCSBackendConceptTestEnvironmentInterface> &env);
	void SetKeyWriteExpectation (const std::string                                                   &plugin,
				     const std::string                                                   &key,
				     const VariantTypes                                                  &value,
				     const boost::shared_ptr <CCSSetting>                                &setting,
				     const WriteFunc                                                     &write,
				     const boost::shared_ptr <CCSBackendConceptTestEnvironmentInterface> &env);
	void SetButtonWriteExpectation (const std::string                                                   &plugin,
					const std::string                                                   &key,
					const VariantTypes                                                  &value,
					const boost::shared_ptr <CCSSetting>                                &setting,
					const WriteFunc                                                     &write,
					const boost::shared_ptr <CCSBackendConceptTestEnvironmentInterface> &env);
	void SetEdgeWriteExpectation (const std::string                                                   &plugin,
				      const std::string                                                   &key,
				      const VariantTypes                                                  &value,
				      const boost::shared_ptr <CCSSetting>                                &setting,
				      const WriteFunc                                                     &write,
				      const boost::shared_ptr <CCSBackendConceptTestEnvironmentInterface> &env);
	void SetBellWriteExpectation (const std::string                                                   &plugin,
				      const std::string                                                   &key,
				      const VariantTypes                                                  &value,
				      const boost::shared_ptr <CCSSetting>                                &setting,
				      const WriteFunc                                                     &write,
				      const boost::shared_ptr <CCSBackendConceptTestEnvironmentInterface> &env);
	void SetMatchWriteExpectation (const std::string                                                   &plugin,
				       const std::string                                                   &key,
				       const VariantTypes                                                  &value,
				       const boost::shared_ptr <CCSSetting>                                &setting,
				       const WriteFunc                                                     &write,
				       const boost::shared_ptr <CCSBackendConceptTestEnvironmentInterface> &env);
	void SetListWriteExpectation (const std::string                                                   &plugin,
				      const std::string                                                   &key,
				      const VariantTypes                                                  &value,
				      const boost::shared_ptr <CCSSetting>                                &setting,
				      const WriteFunc                                                     &write,
				      const boost::shared_ptr <CCSBackendConceptTestEnvironmentInterface> &env);
	void SetIntReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value);
	void SetBoolReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value);
	void SetBellReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value);
	void SetFloatReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value);
	void SetStringReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value);
	void SetMatchReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value);
	void SetColorReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value);
	void SetKeyReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value);
	void SetButtonReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value);
	void SetEdgeReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value);
	void SetListReadExpectation (CCSSettingGMock *gmock, const VariantTypes &value);
    }
}

#endif

