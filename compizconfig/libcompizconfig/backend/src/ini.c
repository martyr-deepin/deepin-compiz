/**
 *
 * INI libccs backend
 *
 * ini.c
 *
 * Copyright (c) 2007 Danny Baumann <maniac@opencompositing.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 **/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#include <ccs.h>
#include <ccs-backend.h>

#include <X11/X.h>
#include <X11/Xlib.h>

#define DEFAULTPROF "Default"
#define SETTINGPATH "compiz-1/compizconfig"

typedef struct _IniPrivData
{
    CCSContext *context;
    char * lastProfile;
    IniDictionary * iniFile;
    unsigned int iniWatchId;
}

IniPrivData;

/* forward declaration */
static void setProfile (IniPrivData *data, char *profile);

static char*
getIniFileName (char *profile)
{
    char *configDir = NULL;
    char *fileName = NULL;

    configDir = getenv ("XDG_CONFIG_HOME");
    if (configDir && strlen (configDir))
    {
	if (asprintf (&fileName, "%s/%s/%s.ini", configDir, SETTINGPATH, profile) == -1)
	    return NULL;

	return fileName;
    }

    configDir = getenv ("HOME");
    if (configDir && strlen (configDir))
    {
	if (asprintf (&fileName, "%s/.config/%s/%s.ini", configDir, SETTINGPATH,
		      profile) == -1)
	    return NULL;

	return fileName;
    }

    return NULL;
}

static void
processFileEvent (unsigned int watchId,
		  void         *closure)
{
    IniPrivData *data = (IniPrivData *) closure;
    char        *fileName;

    /* our ini file has been modified, reload it */

    if (data->iniFile)
	ccsIniClose (data->iniFile);

    fileName = getIniFileName (data->lastProfile);

    if (!fileName)
	return;

    data->iniFile = ccsIniOpen (fileName);

    ccsReadSettings (data->context);

    free (fileName);
}

static void
setProfile (IniPrivData *data,
	    char        *profile)
{
    char        *fileName;
    struct stat fileStat;

    if (data->iniFile)
	ccsIniClose (data->iniFile);

    if (data->iniWatchId)
	ccsRemoveFileWatch (data->iniWatchId);

    data->iniFile = NULL;

    data->iniWatchId = 0;

    /* first we need to find the file name */
    fileName = getIniFileName (profile);

    if (!fileName)
	return;

    /* if the file does not exist, we have to create it */
    if (stat (fileName, &fileStat) == -1)
    {
	if (errno == ENOENT)
	{
	    FILE *file;
	    file = fopen (fileName, "w");

	    if (!file)
	    {
		free (fileName);
		return;
	    }
	    fclose (file);
	}
	else
	{
	    free (fileName);
	    return;
	}
    }

    data->iniWatchId = ccsAddFileWatch (fileName, TRUE,
					processFileEvent, data);

    /* load the data from the file */
    data->iniFile = ccsIniOpen (fileName);

    free (fileName);
}

static Bool
initBackend (CCSBackend *backend, CCSContext * context)
{
    IniPrivData *newData;

    newData = calloc (1, sizeof (IniPrivData));

    /* initialize the newly allocated part */
    newData->context = context;

    ccsObjectSetPrivate (backend, (CCSPrivate *) newData);

    return TRUE;
}

static Bool
finiBackend (CCSBackend * backend)
{
    IniPrivData *data;

    data = (IniPrivData *) ccsObjectGetPrivate (backend);

    if (!data)
	return FALSE;

    if (data->iniFile)
	ccsIniClose (data->iniFile);

    if (data->iniWatchId)
	ccsRemoveFileWatch (data->iniWatchId);

    if (data->lastProfile)
	free (data->lastProfile);

    free (data);
    ccsObjectSetPrivate (backend, NULL);

    return TRUE;
}

static Bool
readInit (CCSBackend *backend,
	  CCSContext * context)
{
    const char *currentProfileCCS;
    char       *currentProfile;
    IniPrivData *data;

    data = (IniPrivData *) ccsObjectGetPrivate (backend);

    if (!data)
	return FALSE;

    currentProfileCCS = ccsGetProfile (context);

    if (!currentProfileCCS || !strlen (currentProfileCCS))
	currentProfile = strdup (DEFAULTPROF);
    else
	currentProfile = strdup (currentProfileCCS);

    if (!data->lastProfile || (strcmp (data->lastProfile, currentProfile) != 0))
	setProfile (data, currentProfile);

    if (data->lastProfile)
	free (data->lastProfile);

    data->lastProfile = currentProfile;

    return (data->iniFile != NULL);
}

static void
readSetting (CCSBackend *backend,
	     CCSContext *context,
	     CCSSetting *setting)
{
    Bool         status = FALSE;
    char        *keyName;
    IniPrivData *data;

    data = (IniPrivData *) ccsObjectGetPrivate (backend);
    if (!data)
	return;

    if (asprintf (&keyName, "s%d_%s", ccsContextGetScreenNum (context), ccsSettingGetName (setting)) == -1)
	return;

    switch (ccsSettingGetType (setting))
    {
    case TypeString:
	{
	    char *value;
	    if (ccsIniGetString (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
 				 keyName, &value))
	    {
		ccsSetString (setting, value, TRUE);
		free (value);
		status = TRUE;
	    }
	}
	break;
    case TypeMatch:
	{
	    char *value;
	    if (ccsIniGetString (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
				 keyName, &value))
	    {
		ccsSetMatch (setting, value, TRUE);
		free (value);
		status = TRUE;
	    }
	}
	break;
    case TypeInt:
	{
	    int value;
	    if (ccsIniGetInt (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
			      keyName, &value))
	    {
		ccsSetInt (setting, value, TRUE);
		status = TRUE;
	    }
	}
	break;
    case TypeBool:
	{
	    Bool value;
	    if (ccsIniGetBool (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
			       keyName, &value))
	    {
		ccsSetBool (setting, (value != 0), TRUE);
		status = TRUE;
	    }
	}
	break;
    case TypeFloat:
	{
	    float value;
	    if (ccsIniGetFloat (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
				keyName, &value))
	    {
		ccsSetFloat (setting, value, TRUE);
		status = TRUE;
	    }
	}
	break;
    case TypeColor:
	{
	    CCSSettingColorValue color;

	    if (ccsIniGetColor (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
				keyName, &color))
	    {
		ccsSetColor (setting, color, TRUE);
		status = TRUE;
	    }
	}
	break;
    case TypeKey:
	{
	    CCSSettingKeyValue key;
	    if (ccsIniGetKey (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
			      keyName, &key))
	    {
		ccsSetKey (setting, key, TRUE);
		status = TRUE;
	    }
	}
	break;
    case TypeButton:
	{
	    CCSSettingButtonValue button;
	    if (ccsIniGetButton (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
				 keyName, &button))
	    {
		ccsSetButton (setting, button, TRUE);
		status = TRUE;
	    }
	}
	break;
    case TypeEdge:
	{
	    unsigned int edges;
	    if (ccsIniGetEdge (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
				 keyName, &edges))
	    {
		ccsSetEdge (setting, edges, TRUE);
		status = TRUE;
	    }
	}
	break;
    case TypeBell:
	{
	    Bool bell;
	    if (ccsIniGetBell (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
			       keyName, &bell))
	    {
		ccsSetBell (setting, bell, TRUE);
		status = TRUE;
	    }
	}
	break;
    case TypeList:
	{
	    CCSSettingValueList value;
	    if (ccsIniGetList (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
			       keyName, &value, setting))
	    {
		ccsSetList (setting, value, TRUE);
		ccsSettingValueListFree (value, TRUE);
		status = TRUE;
	    }
	}
	break;
    default:
	break;
    }

    if (!status)
    {
	/* reset setting to default if it could not be read */
	ccsResetToDefault (setting, TRUE);
    }

    if (keyName)
	free (keyName);
}

static void
readDone (CCSBackend *backend, CCSContext * context)
{
}

static Bool
writeInit (CCSBackend *backend, CCSContext * context)
{
    const char *currentProfileCCS;
    char *currentProfile;
    IniPrivData *data;

    data = (IniPrivData *) ccsObjectGetPrivate (backend);

    if (!data)
	return FALSE;

    currentProfileCCS = ccsGetProfile (context);

    if (!currentProfileCCS || !strlen (currentProfileCCS))
	currentProfile = strdup (DEFAULTPROF);
    else
	currentProfile = strdup (currentProfileCCS);

    if (!data->lastProfile || (strcmp (data->lastProfile, currentProfile) != 0))
	setProfile (data, currentProfile);

    if (data->lastProfile)
	free (data->lastProfile);

    ccsDisableFileWatch (data->iniWatchId);

    data->lastProfile = currentProfile;

    return (data->iniFile != NULL);
}

static void
writeSetting (CCSBackend *backend,
	      CCSContext *context,
	      CCSSetting *setting)
{
    char        *keyName;
    IniPrivData *data;

    data = (IniPrivData *) ccsObjectGetPrivate (backend);
    if (!data)
	return;

    if (asprintf (&keyName, "s%d_%s", ccsContextGetScreenNum (context), ccsSettingGetName (setting)) == -1)
	return;

    if (ccsSettingGetIsDefault (setting))
    {
	ccsIniRemoveEntry (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)), keyName);
	free (keyName);
	return;
    }

    switch (ccsSettingGetType (setting))
    {
    case TypeString:
	{
	    char *value;
	    if (ccsGetString (setting, &value))
		ccsIniSetString (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
 				 keyName, value);
	}
	break;
    case TypeMatch:
	{
	    char *value;
	    if (ccsGetMatch (setting, &value))
		ccsIniSetString (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
				 keyName, value);
	}
	break;
    case TypeInt:
	{
	    int value;
	    if (ccsGetInt (setting, &value))
		ccsIniSetInt (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
			      keyName, value);
	}
	break;
    case TypeFloat:
	{
	    float value;
	    if (ccsGetFloat (setting, &value))
		ccsIniSetFloat (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
				keyName, value);
	}
	break;
    case TypeBool:
	{
	    Bool value;
	    if (ccsGetBool (setting, &value))
		ccsIniSetBool (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
			       keyName, value);
	}
	break;
    case TypeColor:
	{
	    CCSSettingColorValue value;
	    if (ccsGetColor (setting, &value))
		ccsIniSetColor (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
				keyName, value);
	}
	break;
    case TypeKey:
	{
	    CCSSettingKeyValue value;
	    if (ccsGetKey (setting, &value))
		ccsIniSetKey (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
			      keyName, value);
	}
	break;
    case TypeButton:
	{
	    CCSSettingButtonValue value;
	    if (ccsGetButton (setting, &value))
		ccsIniSetButton (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
				 keyName, value);
	}
	break;
    case TypeEdge:
	{
	    unsigned int value;
	    if (ccsGetEdge (setting, &value))
		ccsIniSetEdge (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
			       keyName, value);
	}
	break;
    case TypeBell:
	{
	    Bool value;
	    if (ccsGetBell (setting, &value))
		ccsIniSetBell (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
			       keyName, value);
	}
	break;
    case TypeList:
	{
	    CCSSettingValueList value;
	    if (ccsGetList (setting, &value))
		ccsIniSetList (data->iniFile, ccsPluginGetName (ccsSettingGetParent (setting)),
			       keyName, value, ccsSettingGetInfo (setting)->forList.listType);
	}
	break;
    default:
	break;
    }

    if (keyName)
	free (keyName);
}

static void
writeDone (CCSBackend *backend, CCSContext * context)
{
    /* export the data to ensure the changes are on disk */
    char        *fileName;
    const char        *currentProfileCCS;
    char	*currentProfile;
    IniPrivData *data;

    data = (IniPrivData *) ccsObjectGetPrivate (backend);
    if (!data)
	return;

    currentProfileCCS = ccsGetProfile (context);

    if (!currentProfileCCS || !strlen (currentProfileCCS))
	currentProfile = strdup (DEFAULTPROF);
    else
	currentProfile = strdup (currentProfileCCS);

    fileName = getIniFileName (currentProfile);

    free (currentProfile);

    ccsIniSave (data->iniFile, fileName);

    ccsEnableFileWatch (data->iniWatchId);

    free (fileName);
}

static void
updateSetting (CCSBackend *backend, CCSContext *context, CCSPlugin *plugin, CCSSetting *setting)
{
    if (readInit (backend, context))
    {
	readSetting (backend, context, setting);
	readDone (backend, context);
    }
}

static Bool
getSettingIsReadOnly (CCSBackend *backend, CCSSetting * setting)
{
    /* FIXME */
    return FALSE;
}

static int
profileNameFilter (const struct dirent *name)
{
    int length = strlen (name->d_name);

    if (strncmp (name->d_name + length - 6, ".ini++", 6))
	return 0;

    return 1;
}

static CCSStringList
scanConfigDir (char * filePath)
{
    CCSStringList  ret = NULL;
    struct dirent  **nameList;
    char           *pos;
    int            nFile, i;

    nFile = scandir (filePath, &nameList, profileNameFilter, NULL);
    if (nFile <= 0)
	return NULL;

    for (i = 0; i < nFile; i++)
    {
	pos = strrchr (nameList[i]->d_name, '.');
	if (pos)
	{
	    CCSString *pString = malloc (sizeof (CCSString));
	    *pos = 0;

	    pString->value = strdup (nameList[i]->d_name);
	    pString->refCount = 1;

	    if (strcmp (nameList[i]->d_name, DEFAULTPROF) != 0)
		ret = ccsStringListAppend (ret, pString);
	}

	free (nameList[i]);
    }

    free (nameList);
    
    return ret;
}

static CCSStringList
getExistingProfiles (CCSBackend *backend, CCSContext * context)
{
    CCSStringList  ret = NULL;
    char	   *filePath = NULL;
    char           *homeDir = NULL;
    char	   *configDir = NULL;
    
    configDir = getenv ("XDG_CONFIG_HOME");
    if (configDir && strlen (configDir))
    {
	if (asprintf (&filePath, "%s/%s", configDir, SETTINGPATH) == -1)
	    return NULL;
	
	ret = scanConfigDir(filePath);
	free(filePath);

	if (ret)
	    return ret;
    }
    
    homeDir = getenv ("HOME");
    if (!homeDir)
	return NULL;

    if (asprintf (&filePath, "%s/.config/%s", homeDir, SETTINGPATH) == -1)
	filePath = NULL;

    if (!filePath)
	return NULL;

    ret = scanConfigDir(filePath);
    free(filePath);

    return ret;
}

static Bool
deleteProfile (CCSBackend *backend, CCSContext * context, char * profile)
{
    char *fileName;

    fileName = getIniFileName (profile);

    if (!fileName)
	return FALSE;

    remove (fileName);
    free (fileName);

    return TRUE;
}

const CCSBackendInfo iniBackendInfo =
{
    "ini",
    "Flat-file Configuration Backend",
    "Flat file Configuration Backend for libccs",
    FALSE,
    TRUE,
    1
};

static const CCSBackendInfo *
getInfo (CCSBackend *backend)
{
    return &iniBackendInfo;
}

static CCSBackendInterface iniVTable = {
    getInfo,
    NULL,
    initBackend,
    finiBackend,
    readInit,
    readSetting,
    readDone,
    writeInit,
    writeSetting,
    writeDone,
    updateSetting,
    NULL,
    getSettingIsReadOnly,
    getExistingProfiles,
    deleteProfile
};

CCSBackendInterface *
getBackendInfo (void)
{
    return &iniVTable;
}

