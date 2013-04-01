/*
 * Compiz configuration system library
 *
 * ccs_text_file.h
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
#ifndef CCS_TEXT_FILE_H
#define CCS_TEXT_FILE_H

#include <ccs-defs.h>

COMPIZCONFIG_BEGIN_DECLS

typedef struct _CCSTextFile CCSTextFile;
typedef struct _CCSObjectAllocationInterface CCSObjectAllocationInterface;

typedef enum _CCSTextFileOpenMode
{
    ReadOnly = 1,
    ReadWrite = 2,
    ReadWriteCreate = 3
} CCSTextFileOpenMode;

CCSTextFile *
ccsUnixTextFileNew (const char		*path,
		    CCSTextFileOpenMode openMode,
		    CCSObjectAllocationInterface *ai);

COMPIZCONFIG_END_DECLS

#endif
