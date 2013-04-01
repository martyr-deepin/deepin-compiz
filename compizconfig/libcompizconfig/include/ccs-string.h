/*
 * Compiz configuration system library
 *
 * Copyright (C) 2007  Dennis Kasprzyk <onestone@beryl-project.org>
 * Copyright (C) 2007  Danny Baumann <maniac@beryl-project.org>
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
 */

#ifndef _CCS_STRING_H
#define _CCS_STRING_H

/* Doesn't seem to make sense to have this header
 * dependency, but we can only declare a typed list
 * once */
#include <ccs-list.h>

COMPIZCONFIG_BEGIN_DECLS

typedef struct _CCSString	  CCSString;

struct _CCSString
{
    char 	 *value;
    unsigned int refCount;
};

void ccsFreeString (CCSString *str);

CCSLIST_HDR (String, CCSString);

COMPIZCONFIG_END_DECLS

#endif

