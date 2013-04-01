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

#include <ccs-object.h>
#include <ccs_text_file_interface.h>
#include "compizconfig_ccs_text_file_mock.h"

const CCSTextFileInterface ccsMockTextFileInterface =
{
    CCSTextFileGMock::ccsTextFileReadFromStart,
    CCSTextFileGMock::ccsTextFileAppendString,
    CCSTextFileGMock::ccsFreeTextFile
};

static void
finalizeAndFreeTextFile (CCSTextFile *textFile)
{
    CCSObjectAllocationInterface *ai = textFile->object.object_allocation;

    ccsObjectFinalize (textFile);
    (*ai->free_) (ai->allocator, textFile);
}

void
ccsFreeMockTextFile (CCSTextFile *textFile)
{
    CCSTextFileGMock *gmock = reinterpret_cast <CCSTextFileGMock *> (ccsObjectGetPrivate (textFile));
    delete gmock;

    ccsObjectSetPrivate (textFile, NULL);
    finalizeAndFreeTextFile (textFile);

}

static CCSTextFileGMock *
newCCSTextFileGMockInterface (CCSTextFile *textFile)
{
    CCSTextFileGMock *gmock = new CCSTextFileGMock ();

    if (!gmock)
    {
	finalizeAndFreeTextFile (textFile);
	return NULL;
    }

    return gmock;
}

static CCSTextFile *
allocateCCSTextFile (CCSObjectAllocationInterface *ai)
{
    CCSTextFile *textFile = reinterpret_cast <CCSTextFile *> ((*ai->calloc_) (ai->allocator, 1, sizeof (CCSTextFile)));

    if (!textFile)
	return NULL;

    ccsObjectInit (textFile, ai);

    return textFile;
}

CCSTextFile *
ccsMockTextFileNew (CCSObjectAllocationInterface *ai)
{
    CCSTextFile *textFile = allocateCCSTextFile (ai);

    if (!textFile)
	return NULL;

    CCSTextFileGMock *gmock = newCCSTextFileGMockInterface (textFile);

    if (!gmock)
	return NULL;

    ccsObjectSetPrivate (textFile, (CCSPrivate *) gmock);
    ccsObjectAddInterface (textFile,
			   (const CCSInterface *) &ccsMockTextFileInterface,
			   GET_INTERFACE_TYPE (CCSTextFileInterface));
    ccsObjectRef (textFile);

    return textFile;
}
