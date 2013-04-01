/*
 * Compiz configuration system library
 *
 * ccs_text_file.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ccs-defs.h>
#include <ccs-object.h>
#include <ccs_text_file_interface.h>
#include <ccs_text_file.h>

typedef struct _CCSUnixTextFilePrivate CCSUnixTextFilePrivate;

struct _CCSUnixTextFilePrivate
{
    FILE *unixFile;
};

static void
freeAndFinalizeCCSTextFile (CCSTextFile			 *file,
			    CCSObjectAllocationInterface *ai)
{
    ccsObjectFinalize (file);
    (*ai->free_) (ai->allocator, file);
}

static char *
ccsUnixTextFileReadFromStart (CCSTextFile *textFile)
{
    CCSUnixTextFilePrivate *priv = GET_PRIVATE (CCSUnixTextFilePrivate, textFile);
    FILE		   *completedUpgrades = priv->unixFile;

    char		   *cuBuffer;
    unsigned int	   cuSize;
    size_t		   cuReadSize;

    fseek (completedUpgrades, 0, SEEK_END);
    cuSize = ftell (completedUpgrades);
    rewind (completedUpgrades);

    cuBuffer = calloc (cuSize + 1, sizeof (char));

    if (!cuBuffer)
	return NULL;

    cuReadSize = fread (cuBuffer, 1, cuSize, completedUpgrades);

    /*
	ccsWarning ("Couldn't read completed upgrades file!");
	*/
    if (cuReadSize != cuSize)
    {
	free (cuBuffer);
	return NULL;
    }

    cuBuffer[cuSize] = '\0';

    return cuBuffer;
}

static Bool
ccsUnixTextFileAppendString (CCSTextFile *textFile, const char *str)
{
    CCSUnixTextFilePrivate *priv = GET_PRIVATE (CCSUnixTextFilePrivate, textFile);
    FILE		   *completedUpgrades = priv->unixFile;

    fprintf (completedUpgrades, "%s\n", str);
    return TRUE;
}

static void
ccsUnixFreeTextFile (CCSTextFile *textFile)
{
    CCSUnixTextFilePrivate *priv = GET_PRIVATE (CCSUnixTextFilePrivate, textFile);

    fclose (priv->unixFile);
    priv->unixFile = NULL;

    freeAndFinalizeCCSTextFile (textFile,
				textFile->object.object_allocation);
}

CCSTextFileInterface ccsUnixTextFileInterface =
{
    ccsUnixTextFileReadFromStart,
    ccsUnixTextFileAppendString,
    ccsUnixFreeTextFile
};

const char * CCS_UNIX_TEXT_FILE_OPEN_MODE_READONLY = "r";
const char * CCS_UNIX_TEXT_FILE_OPEN_MODE_READWRITE = "r+";
const char * CCS_UNIX_TEXT_FILE_OPEN_MODE_READWRITECREATE = "a+";

static FILE *
openUnixFile (CCSTextFile		   *textFile,
	      CCSObjectAllocationInterface *ai,
	      const char		   *path,
	      const char		   *openMode)
{
    FILE *file = fopen (path, openMode);

    if (!file)
    {
	ccsObjectFinalize (textFile);
	(*ai->free_) (ai->allocator, textFile);
	return NULL;
    }

    return file;
}


static CCSUnixTextFilePrivate *
allocateCCSUnixTextFilePrivate (CCSTextFile			 *file,
				CCSObjectAllocationInterface	 *ai)
{
    CCSUnixTextFilePrivate *priv = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSUnixTextFilePrivate));

    if (!priv)
    {
	freeAndFinalizeCCSTextFile (file, ai);
	return NULL;
    }

    return priv;
}

static CCSTextFile *
allocateCCSTextFile (CCSObjectAllocationInterface *ai)
{
    CCSTextFile *textFile = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSTextFile));

    if (!textFile)
	return NULL;

    ccsObjectInit (textFile, ai);

    return textFile;
}

CCSTextFile *
ccsUnixTextFileNew (const char		*path,
		    CCSTextFileOpenMode openMode,
		    CCSObjectAllocationInterface *ai)
{
    const char *fopenMode = NULL;

    switch (openMode)
    {
	case ReadOnly:
	    fopenMode = CCS_UNIX_TEXT_FILE_OPEN_MODE_READONLY;
	    break;
	case ReadWrite:
	    fopenMode = CCS_UNIX_TEXT_FILE_OPEN_MODE_READWRITE;
	    break;
	case ReadWriteCreate:
	    fopenMode = CCS_UNIX_TEXT_FILE_OPEN_MODE_READWRITECREATE;
	    break;
    }

    CCSTextFile *textFile = allocateCCSTextFile (ai);

    if (!textFile)
	return NULL;

    CCSUnixTextFilePrivate *priv = allocateCCSUnixTextFilePrivate (textFile, ai);

    if (!priv)
	return NULL;

    ccsObjectSetPrivate (textFile, (CCSPrivate *) priv);

    FILE *unixFile = openUnixFile (textFile,
				   ai,
				   path,
				   fopenMode);

    if (!unixFile)
	return NULL;

    priv->unixFile = unixFile;

    ccsObjectAddInterface (textFile, (const CCSInterface *) &ccsUnixTextFileInterface,
			   GET_INTERFACE_TYPE (CCSTextFileInterface));
    ccsObjectRef (textFile);

    return textFile;
}
