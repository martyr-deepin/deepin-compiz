/*
 * Compiz configuration system library
 *
 * Copyright (C) 2007  Dennis Kasprzyk <onestone@opencompositing.org>
 * Copyright (C) 2007  Danny Baumann <maniac@opencompositing.org>
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

#ifndef CCS_PRIVATE_H
#define CSS_PRIVATE_H

#include <ccs.h>
#include <ccs-backend.h>

extern Bool basicMetadata;

typedef struct _CCSContextPrivate
{
    CCSDynamicBackend  *backend;
    CCSPluginList     plugins;         /* list of plugins settings
                                          were loaded for */
    CCSPluginCategory *categories;     /* list of plugin categories */
    void              *privatePtr;     /* private pointer that can be used
					  by the caller */
    char              *profile;
    Bool	      deIntegration;
    Bool              pluginListAutoSort;

    unsigned int      configWatchId;

    CCSSettingList    changedSettings; /* list of settings changed since last
                                          settings write */

    unsigned int screenNum; /* screen number this context is assigned to */
    const CCSInterfaceTable *object_interfaces;
} CCSContextPrivate;

typedef struct _CCSPluginPrivate
{
    char *name;                    /* plugin name */
    char *shortDesc;		   /* plugin short description */
    char *longDesc;		   /* plugin long description */
    char *hints;                   /* currently unused */
    char *category;		   /* plugin category name */

    CCSStringList loadAfter;       /* list of plugin names this plugin needs to
				      be loaded after */
    CCSStringList loadBefore;      /* list of plugin names this plugin needs to
				      be loaded before */
    CCSStringList requiresPlugin;  /* list of plugin names this plugin
				      requires */
    CCSStringList conflictPlugin;  /* list of plugin names this plugin
				      conflicts with */
    CCSStringList conflictFeature; /* list of feature names this plugin
				      conflicts with */
    CCSStringList providesFeature; /* list of feature names this plugin
				      provides */
    CCSStringList requiresFeature; /* list of feature names this plugin
				      requires */

    void       *privatePtr;        /* private pointer that can be used
				      by the caller */
    CCSContext *context;           /* context this plugin belongs to */

    CCSSettingList settings;
    CCSGroupList   groups;
    Bool 	   loaded;
    Bool           active;
    char *	   xmlFile;
    char *	   xmlPath;
#ifdef USE_PROTOBUF
    char *	   pbFilePath;
#endif

    CCSStrExtensionList stringExtensions;
} CCSPluginPrivate;

typedef struct _CCSSettingPrivate
{
    char *name;             /* setting name */
    char *shortDesc;        /* setting short description in current locale */
    char *longDesc;         /* setting long description in current locale */

    CCSSettingType type;    /* setting type */

    CCSSettingInfo info;    /* information assigned to this setting,
			       valid if the setting is an int, float, string
			       or list setting */

    char *group;	    /* group name in current locale */
    char *subGroup;	    /* sub group name in current locale */
    char *hints;	    /* hints in current locale */

    CCSSettingValue defaultValue; /* default value of this setting */
    CCSSettingValue *value;       /* actual value of this setting */
    Bool	    isDefault;    /* does the actual value match the default
				     value? */

    CCSPlugin *parent;            /* plugin this setting belongs to */
    void      *privatePtr;        /* private pointer for usage by the caller */
} CCSSettingPrivate;

typedef struct _CCSDynamicBackendPrivate
{
    void            *dlhand;
    CCSBackend	    *backend;
} CCSDynamicBackendPrivate;

typedef struct _CCSSettingsUpgrade
{
    char	   *profile;
    char           *file;
    char           *domain;
    unsigned int   num;
    
    CCSSettingList changedSettings;
    CCSSettingList addValueSettings;
    CCSSettingList clearValueSettings;
    CCSSettingList replaceFromValueSettings;
    CCSSettingList replaceToValueSettings;
} CCSSettingsUpgrade;

Bool ccsCheckForSettingsUpgrade (CCSContext *context);

void ccsLoadPlugins (CCSContext * context);
void ccsLoadPluginSettings (CCSPlugin * plugin);
void collateGroups (CCSPluginPrivate * p);

Bool ccsLoadPluginDefault (CCSContext *context, char *name);
void ccsLoadPluginsDefault (CCSContext *context);

void ccsCheckFileWatches (void);

typedef enum {
    OptionProfile,
    OptionBackend,
    OptionIntegration,
    OptionAutoSort
} ConfigOption;

Bool ccsReadConfig (ConfigOption option,
		    char         **value);
Bool ccsWriteConfig (ConfigOption option,
		     char         *value);
unsigned int ccsAddConfigWatch (CCSContext            *context,
				FileWatchCallbackProc callback);

#endif
