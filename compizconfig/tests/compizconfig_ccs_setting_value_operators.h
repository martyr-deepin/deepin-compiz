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
#ifndef _COMPIZCONFIG_CCS_SETTING_VALUE_OPERATORS_H
#define _COMPIZCONFIG_CCS_SETTING_VALUE_OPERATORS_H

#include <iosfwd>
#include <string>

typedef union _CCSSettingColorValue CCSSettingColorValue;
typedef struct _CCSSettingKeyValue CCSSettingKeyValue;
typedef struct _CCSSettingButtonValue CCSSettingButtonValue;
typedef struct _CCSString CCSString;

bool
operator== (const CCSSettingColorValue &lhs,
	    const CCSSettingColorValue &rhs);

std::ostream &
operator<< (std::ostream &os, const CCSSettingColorValue &v);

bool
operator== (const CCSSettingKeyValue &lhs,
	    const CCSSettingKeyValue &rhs);

std::ostream &
operator<< (std::ostream &os, const CCSSettingKeyValue &v);

bool
operator== (const CCSSettingButtonValue &lhs,
	    const CCSSettingButtonValue &rhs);

std::ostream &
operator<< (std::ostream &os, const CCSSettingButtonValue &v);

bool
operator== (const CCSString &lhs,
	    const std::string &rhs);

bool
operator== (const std::string &lhs,
	    const CCSString &rhs);

bool
operator== (const std::string &rhs,
	    CCSString	      *lhs);

std::ostream &
operator<< (std::ostream &os, CCSString &string);

#endif
