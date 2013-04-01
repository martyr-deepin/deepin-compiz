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
#ifndef COMPIZCONFIG_CCS_TEST_FILE_MOCK_H
#define COMPIZCONFIG_CCS_TEST_FILE_MOCK_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <ccs-object.h>
#include <ccs-defs.h>

COMPIZCONFIG_BEGIN_DECLS

typedef struct _CCSTextFile CCSTextFile;
typedef struct _CCSObjectAllocationInterface CCSObjectAllocationInterface;

CCSTextFile * ccsMockTextFileNew (CCSObjectAllocationInterface *ai);
void          ccsFreeMockTextFile (CCSTextFile *);

COMPIZCONFIG_END_DECLS

class CCSTextFileGMockInterface
{
    public:

	virtual ~CCSTextFileGMockInterface () {}
	virtual char * readFromStart () = 0;
	virtual Bool   appendString (const char *) = 0;
	virtual void   free () = 0;
};

class CCSTextFileGMock :
    public CCSTextFileGMockInterface
{
    public:

	MOCK_METHOD0 (readFromStart, char * ());
	MOCK_METHOD1 (appendString, Bool (const char *));
	MOCK_METHOD0 (free, void ());

    public:

	static char *
	ccsTextFileReadFromStart (CCSTextFile *file)
	{
	    return reinterpret_cast <CCSTextFileGMock *> (ccsObjectGetPrivate (file))->readFromStart ();
	}

	static Bool
	ccsTextFileAppendString (CCSTextFile *file, const char *str)
	{
	    return reinterpret_cast <CCSTextFileGMock *> (ccsObjectGetPrivate (file))->appendString (str);
	}

	static void
	ccsFreeTextFile (CCSTextFile *file)
	{
	    reinterpret_cast <CCSTextFileGMock *> (ccsObjectGetPrivate (file))->free ();
	    ccsFreeMockTextFile (file);
	}

};

#endif
