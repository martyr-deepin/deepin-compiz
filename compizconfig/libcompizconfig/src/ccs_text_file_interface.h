/*
 * Compiz configuration system library
 *
 * ccs_text_file_interface.h
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
#ifndef CCS_TEXT_FILE_INTERFACE_H
#define CCS_TEXT_FILE_INTERFACE_H

#include <ccs-defs.h>
#include <ccs-object.h>

COMPIZCONFIG_BEGIN_DECLS

typedef struct _CCSTextFile CCSTextFile;
typedef struct _CCSTextFileInterface CCSTextFileInterface;

typedef char * (*CCSReadTextFileFromStart) (CCSTextFile *file);
typedef Bool   (*CCSAppendStringToTextFile) (CCSTextFile *file, const char *str);
typedef void   (*CCSFreeTextFile) (CCSTextFile *file);

struct _CCSTextFileInterface
{
    CCSReadTextFileFromStart  readFromStart;
    CCSAppendStringToTextFile appendString;
    CCSFreeTextFile	      free;
};

struct _CCSTextFile
{
    CCSObject object;
};

CCSREF_HDR (TextFile, CCSTextFile);

char *
ccsTextFileReadFromStart (CCSTextFile *);

Bool
ccsTextFileAppendString (CCSTextFile *, const char *);

void
ccsFreeTextFile (CCSTextFile *);

unsigned int ccsCCSTextFileInterfaceGetType ();

COMPIZCONFIG_END_DECLS

#endif
