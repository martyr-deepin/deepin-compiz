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
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs.h>

#include "compizconfig_ccs_integrated_setting_factory_mock.h"

const CCSIntegratedSettingFactoryInterface mockIntegratedSettingFactoryInterface =
{
    CCSIntegratedSettingFactoryGMock::ccsIntegratedSettingFactoryCreateIntegratedSettingForCCSSettingNameAndType,
    CCSIntegratedSettingFactoryGMock::ccsIntegratedSettingFactoryFree
};

CCSIntegratedSettingFactory *
ccsMockIntegratedSettingFactoryNew (CCSObjectAllocationInterface *ai)
{
    CCSIntegratedSettingFactory *integratedSettingFactory =
	    reinterpret_cast <CCSIntegratedSettingFactory *> ((*ai->calloc_) (ai->allocator, 1, sizeof (CCSIntegratedSettingFactory)));

    if (!integratedSettingFactory)
	return NULL;

    CCSIntegratedSettingFactoryGMock *gmockFactory = new CCSIntegratedSettingFactoryGMock (integratedSettingFactory);

    ccsObjectInit (integratedSettingFactory, ai);
    ccsObjectSetPrivate (integratedSettingFactory, (CCSPrivate *) gmockFactory);
    ccsObjectAddInterface (integratedSettingFactory,
			   reinterpret_cast <const CCSInterface *> (&mockIntegratedSettingFactoryInterface),
			   GET_INTERFACE_TYPE (CCSIntegratedSettingFactoryInterface));

    ccsObjectRef (integratedSettingFactory);

    return integratedSettingFactory;
}

void
ccsMockIntegratedSettingFactoryFree (CCSIntegratedSettingFactory *integratedSettingFactory)
{
    CCSIntegratedSettingFactoryGMock *gmockFactory =
	    GET_PRIVATE (CCSIntegratedSettingFactoryGMock, integratedSettingFactory);

    delete gmockFactory;

    ccsObjectSetPrivate (integratedSettingFactory, NULL);
    ccsObjectFinalize (integratedSettingFactory);
    (*integratedSettingFactory->object.object_allocation->free_)
	    (integratedSettingFactory->object.object_allocation->allocator, integratedSettingFactory);
}
