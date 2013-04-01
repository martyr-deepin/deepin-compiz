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
#include <ccs.h>
#include <compizconfig_ccs_list_wrapper.h>

namespace cci = compiz::config::impl;
namespace cc  = compiz::config;

namespace compiz
{
    namespace config
    {
	namespace impl
	{
	    class PrivateSettingValueListWrapper
	    {
		public:

		    PrivateSettingValueListWrapper (CCSSettingValueList                      list,
						       cci::ListStorageType                     storageType,
						       CCSSettingType                           type,
						       const boost::shared_ptr <CCSSettingInfo> &listInfo,
						       const boost::shared_ptr <CCSSetting>     &settingReference) :
			mType (type),
			mListInfo (listInfo),
			mSettingReference (settingReference),
			mListWrapper (list,
				      ccsSettingValueListFree,
				      ccsSettingValueListAppend,
				      ccsSettingValueListRemove,
				      storageType)
		    {
		    }

		    CCSSettingType                                  mType;
		    boost::shared_ptr <CCSSettingInfo>              mListInfo;
		    boost::shared_ptr <CCSSetting>                  mSettingReference;
		    SettingValueListWrapper::InternalWrapperImpl mListWrapper;
	    };
	}
    }
}

cci::SettingValueListWrapper::SettingValueListWrapper (CCSSettingValueList                      list,
							     cci::ListStorageType                     storageType,
							     CCSSettingType                           type,
							     const boost::shared_ptr <CCSSettingInfo> &listInfo,
							     const boost::shared_ptr <CCSSetting>     &settingReference) :
    priv (new cci::PrivateSettingValueListWrapper (list,
						      storageType,
						      type,
						      listInfo,
						      settingReference))
{
}

CCSSettingType
cci::SettingValueListWrapper::type ()
{
    return priv->mType;
}

cci::SettingValueListWrapper::InternalWrapper &
cci::SettingValueListWrapper::append (CCSSettingValue * const &value)
{
    return priv->mListWrapper.append (value);
}

cci::SettingValueListWrapper::InternalWrapper &
cci::SettingValueListWrapper::remove (CCSSettingValue * const &value)
{
    return priv->mListWrapper.remove (value);
}

cci::SettingValueListWrapper::operator const CCSSettingValueList & () const
{
    return priv->mListWrapper;
}

cci::SettingValueListWrapper::operator CCSSettingValueList & ()
{
    return priv->mListWrapper;
}

const boost::shared_ptr <CCSSetting> &
cci::SettingValueListWrapper::setting ()
{
    return priv->mSettingReference;
}
