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

#ifdef HAVE_CONFIG_H
#  include "../config.h"
#endif

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <libintl.h>
#include <dlfcn.h>
#include <dirent.h>
#include <math.h>
#include <errno.h>

#include <ccs.h>

#include "ccs-private.h"
#include "iniparser.h"
#include "ccs_settings_upgrade_internal.h"
#include "ccs_text_file_interface.h"
#include "ccs_text_file.h"

static void * wrapRealloc (void *o, void *a , size_t b)
{
    return realloc (a, b);
}

static void * wrapMalloc (void *o, size_t a)
{
    return malloc (a);
}

static void * wrapCalloc (void *o, size_t a, size_t b)
{
    return calloc (a, b);
}

static void wrapFree (void *o, void *a)
{
    free (a);
}

CCSObjectAllocationInterface ccsDefaultObjectAllocator =
{
    wrapRealloc,
    wrapMalloc,
    wrapCalloc,
    wrapFree,
    NULL
};

/* CCSObject stuff */
Bool
ccsObjectInit_(CCSObject *object, CCSObjectAllocationInterface *object_allocation)
{
    object->priv = NULL;
    object->n_interfaces = 0;
    object->n_allocated_interfaces = 0;
    object->interfaces = NULL;
    object->interface_types = NULL;
    object->object_allocation = object_allocation;
    object->refcnt = 0;

    return TRUE;
}

Bool
ccsObjectAddInterface_(CCSObject *object, const CCSInterface *interface, int interface_type)
{
    object->n_interfaces++;

    if (object->n_allocated_interfaces < object->n_interfaces)
    {
	unsigned int old_allocated_interfaces = object->n_allocated_interfaces;
	object->n_allocated_interfaces = object->n_interfaces;
	CCSInterface **ifaces = (*object->object_allocation->realloc_) (object->object_allocation->allocator, object->interfaces, object->n_allocated_interfaces * sizeof (CCSInterface *));
	int          *iface_types = (*object->object_allocation->realloc_) (object->object_allocation->allocator, object->interface_types, object->n_allocated_interfaces * sizeof (int));

	if (!ifaces || !iface_types)
	{
	    if (ifaces)
		(*object->object_allocation->free_) (object->object_allocation->allocator, ifaces);

	    if (iface_types)
		(*object->object_allocation->free_) (object->object_allocation->allocator, iface_types);

	    object->n_interfaces--;
	    object->n_allocated_interfaces = old_allocated_interfaces;
	    return FALSE;
	}
	else
	{
	    object->interfaces = (const CCSInterface **) ifaces;
	    object->interface_types = iface_types;
	}
    }

    object->interfaces[object->n_interfaces - 1] = interface;
    object->interface_types[object->n_interfaces - 1] = interface_type;

    return TRUE;
}

Bool
ccsObjectRemoveInterface_(CCSObject *object, int interface_type)
{
    unsigned int i = 0;

    if (!object->n_interfaces)
	return FALSE;

    const CCSInterface **o = object->interfaces;
    int        *type = object->interface_types;

    for (; i < object->n_interfaces; i++, o++, type++)
    {
	if (object->interface_types[i] == interface_type)
	    break;
    }

    if (i >= object->n_interfaces)
	return FALSE;

    /* Now clear this section and move everything back */
    object->interfaces[i] = NULL;

    i++;

    const CCSInterface **oLast = o;
    int *typeLast = type;

    o++;
    type++;

    memmove ((void *) oLast, (void *)o, (object->n_interfaces - i) * sizeof (CCSInterface *));
    memmove ((void *) typeLast, (void *) type, (object->n_interfaces - i) * sizeof (int));

    object->n_interfaces--;

    if (!object->n_interfaces)
    {
	free (object->interfaces);
	free (object->interface_types);
	object->interfaces = NULL;
	object->interface_types = NULL;
	object->n_allocated_interfaces = 0;
    }

    return TRUE;
}

const CCSInterface *
ccsObjectGetInterface_(CCSObject *object, int interface_type)
{
    unsigned int i = 0;

    for (; i < object->n_interfaces; i++)
    {
	if (object->interface_types[i] == interface_type)
	    return object->interfaces[i];
    }

    return NULL;
}

CCSPrivate *
ccsObjectGetPrivate_(CCSObject *object)
{
    return object->priv;
}

void
ccsObjectSetPrivate_(CCSObject *object, CCSPrivate *priv)
{
    object->priv = priv;
}

void
ccsObjectFinalize_(CCSObject *object)
{
    if (object->priv)
    {
	(*object->object_allocation->free_) (object->object_allocation->allocator, object->priv);
	object->priv = NULL;
    }

    if (object->interfaces)
    {
	(*object->object_allocation->free_) (object->object_allocation->allocator, object->interfaces);
	object->interfaces = NULL;
    }

    if (object->interface_types)
    {
	(*object->object_allocation->free_) (object->object_allocation->allocator, object->interface_types);
	object->interface_types = NULL;
    }

    object->n_interfaces = 0;
}

unsigned int
ccsAllocateType ()
{
    static unsigned int start = 0;

    start++;

    return start;
}

INTERFACE_TYPE (CCSContextInterface)
INTERFACE_TYPE (CCSPluginInterface)
INTERFACE_TYPE (CCSSettingInterface)
INTERFACE_TYPE (CCSBackendInterface);
INTERFACE_TYPE (CCSDynamicBackendInterface);
INTERFACE_TYPE (CCSIntegrationInterface);
INTERFACE_TYPE (CCSIntegratedSettingInfoInterface);
INTERFACE_TYPE (CCSIntegratedSettingInterface);
INTERFACE_TYPE (CCSIntegratedSettingsStorageInterface);
INTERFACE_TYPE (CCSIntegratedSettingFactoryInterface);

Bool basicMetadata = FALSE;

void
ccsSetBasicMetadata (Bool value)
{
    basicMetadata = value;
}
static void
initGeneralOptions (CCSContext * context)
{
    char *val;

    if (ccsReadConfig (OptionBackend, &val))
    {
	ccsSetBackend (context, val);
	free (val);
    }
    else
	ccsSetBackend (context, "ini");

    if (ccsReadConfig (OptionProfile, &val))
    {
	ccsSetProfile (context, val);
	free (val);
    }
    else
	ccsSetProfile (context, "");

    if (ccsReadConfig (OptionIntegration, &val))
    {
	ccsSetIntegrationEnabled (context, !strcasecmp (val, "true"));
	free (val);
    }
    else
	ccsSetIntegrationEnabled (context, TRUE);

    if (ccsReadConfig (OptionAutoSort, &val))
    {
	ccsSetPluginListAutoSort (context, !strcasecmp (val, "true"));
	free (val);
    }
    else
	ccsSetPluginListAutoSort (context, TRUE);
}

static void
configChangeNotify (unsigned int watchId, void *closure)
{
    CCSContext *context = (CCSContext *) closure;

    initGeneralOptions (context);
    ccsReadSettings (context);
}

CCSContext *
ccsEmptyContextNew (unsigned int screenNum, const CCSInterfaceTable *object_interfaces)
{
    CCSContext *context;

    context = calloc (1, sizeof (CCSContext));

    if (!context)
	return NULL;

    ccsObjectInit (context, &ccsDefaultObjectAllocator);

    CCSContextPrivate *ccsPrivate = calloc (1, sizeof (CCSContextPrivate));
    if (!ccsPrivate)
    {
	free (context);
	return NULL;
    }

    ccsObjectSetPrivate (context, (CCSPrivate *) ccsPrivate);

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    cPrivate->object_interfaces = object_interfaces;
    cPrivate->screenNum = screenNum;

    ccsObjectAddInterface (context, (CCSInterface *) object_interfaces->contextInterface, GET_INTERFACE_TYPE (CCSContextInterface));

    initGeneralOptions (context);
    cPrivate->configWatchId = ccsAddConfigWatch (context, configChangeNotify);

    if (cPrivate->backend)
	ccsInfo ("Backend     : %s", ccsDynamicBackendGetBackendName (cPrivate->backend));

    ccsInfo ("Integration : %s", cPrivate->deIntegration ? "true" : "false");
    ccsInfo ("Profile     : %s",
	(cPrivate->profile && strlen (cPrivate->profile)) ?
	cPrivate->profile : "default");

    return context;
}

static void *
ccsContextGetPrivatePtrDefault (CCSContext *context)
{
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    return cPrivate->privatePtr;
}

static void
ccsContextSetPrivatePtrDefault (CCSContext *context, void *ptr)
{
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    cPrivate->privatePtr = ptr;
}

static CCSPluginList
ccsContextGetPluginsDefault (CCSContext *context)
{
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    return cPrivate->plugins;
}

static CCSPluginCategory *
ccsContextGetCategoriesDefault (CCSContext *context)
{
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    return cPrivate->categories;
}

static CCSSettingList
ccsContextGetChangedSettingsDefault (CCSContext *context)
{
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    return cPrivate->changedSettings;
}

static unsigned int
ccsContextGetScreenNumDefault (CCSContext *context)
{
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    return cPrivate->screenNum;
}

static Bool
ccsContextAddChangedSettingDefault (CCSContext *context, CCSSetting *setting)
{
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    cPrivate->changedSettings = ccsSettingListAppend (cPrivate->changedSettings, setting);

    return TRUE;
}

static Bool
ccsContextClearChangedSettingsDefault (CCSContext *context)
{
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    cPrivate->changedSettings = ccsSettingListFree (cPrivate->changedSettings, FALSE);

    return TRUE;
}

static CCSSettingList
ccsContextStealChangedSettingsDefault (CCSContext *context)
{
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    CCSSettingList l = cPrivate->changedSettings;

    cPrivate->changedSettings = NULL;
    return l;
}

CCSPluginList
ccsContextGetPlugins (CCSContext *context)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextGetPlugins) (context);
}

CCSPluginCategory *
ccsContextGetCategories (CCSContext *context)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextGetCategories) (context);
}

CCSSettingList
ccsContextGetChangedSettings (CCSContext *context)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextGetChangedSettings) (context);
}

unsigned int
ccsContextGetScreenNum (CCSContext *context)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextGetScreenNum) (context);
}

Bool
ccsContextAddChangedSetting (CCSContext *context, CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextAddChangedSetting) (context, setting);
}

Bool
ccsContextClearChangedSettings (CCSContext *context)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextClearChangedSettings) (context);
}

CCSSettingList
ccsContextStealChangedSettings (CCSContext *context)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextStealChangedSettings) (context);
}

void *
ccsContextGetPrivatePtr (CCSContext *context)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextGetPrivatePtr) (context);
}

void
ccsContextSetPrivatePtr (CCSContext *context, void *ptr)
{
    (*(GET_INTERFACE (CCSContextInterface, context))->contextSetPrivatePtr) (context, ptr);
}

void *
ccsContextGetPluginsBindable (CCSContext *context)
{
    return (void *) ccsContextGetPlugins (context);
}

void *
ccsContextStealChangedSettingsBindable (CCSContext *context)
{
    return (void *) ccsContextStealChangedSettings (context);
}

void *
ccsContextGetChangedSettingsBindable (CCSContext *context)
{
    return (void *) ccsContextGetChangedSettings (context);
}


static void
ccsSetActivePluginList (CCSContext * context, CCSStringList list)
{
    CCSPluginList l;
    CCSPlugin     *plugin;

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    for (l = cPrivate->plugins; l; l = l->next)
    {
	CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, l->data);
	pPrivate->active = FALSE;
    }

    for (; list; list = list->next)
    {
	plugin = ccsFindPlugin (context, ((CCSString *)list->data)->value);

	if (plugin)
	{
	    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);
	    pPrivate->active = TRUE;
	}
    }

    /* core plugin is always active */
    plugin = ccsFindPlugin (context, "core");
    if (plugin)
    {
	CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);
	pPrivate->active = TRUE;
    }
}

CCSContext *
ccsContextNew (unsigned int screenNum, const CCSInterfaceTable *iface)
{
    CCSPlugin  *p;
    CCSContext *context = ccsEmptyContextNew (screenNum, iface);
    if (!context)
	return NULL;

    ccsLoadPlugins (context);

    /* Do settings upgrades */
    ccsCheckForSettingsUpgrade (context);

    p = ccsFindPlugin (context, "core");
    if (p)
    {
	CCSSetting    *s;

	ccsLoadPluginSettings (p);

	/* initialize plugin->active values */
	s = ccsFindSetting (p, "active_plugins");
	if (s)
	{
	    CCSStringList       list;
	    CCSSettingValueList vl;

	    ccsGetList (s, &vl);
	    list = ccsGetStringListFromValueList (vl);
	    ccsSetActivePluginList (context, list);
	    ccsStringListFree (list, TRUE);
	}
    }

    return context;
}

CCSPlugin *
ccsFindPluginDefault (CCSContext * context, const char *name)
{
    if (!name)
	name = "";

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    CCSPluginList l = cPrivate->plugins;
    while (l)
    {
	if (!strcmp (ccsPluginGetName (l->data), name))
	    return l->data;

	l = l->next;
    }

    return NULL;
}

CCSPlugin *
ccsFindPlugin (CCSContext *context, const char *name)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextFindPlugin) (context, name);
}

CCSSetting *
ccsFindSettingDefault (CCSPlugin * plugin, const char *name)
{
    if (!plugin)
	return NULL;

    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    if (!name)
	name = "";

    if (!pPrivate->loaded)
	ccsLoadPluginSettings (plugin);

    CCSSettingList l = pPrivate->settings;

    while (l)
    {
	if (!strcmp (ccsSettingGetName (l->data), name))
	    return l->data;

	l = l->next;
    }

    return NULL;
}

CCSSetting *
ccsFindSetting (CCSPlugin *plugin, const char *name)
{
    if (!plugin)
	return NULL;

    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginFindSetting) (plugin, name);
}

Bool
ccsPluginIsActiveDefault (CCSContext * context, const char *name)
{
    CCSPlugin *plugin;

    plugin = ccsFindPlugin (context, name);
    if (!plugin)
	return FALSE;

    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    return pPrivate->active;
}

Bool
ccsPluginIsActive (CCSContext *context, const char *name)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextPluginIsActive) (context, name);
}


static void
subGroupAdd (CCSSetting * setting, CCSGroup * group)
{
    CCSSubGroupList l = group->subGroups;
    CCSSubGroup     *subGroup;

    while (l)
    {
	if (!strcmp (l->data->name, ccsSettingGetSubGroup (setting)))
	{
	    l->data->settings = ccsSettingListAppend (l->data->settings,
						      setting);
	    return;
	}

	l = l->next;
    }

    subGroup = calloc (1, sizeof (CCSSubGroup));
    subGroup->refCount = 1;
    if (subGroup)
    {
	group->subGroups = ccsSubGroupListAppend (group->subGroups, subGroup);
	subGroup->name = strdup (ccsSettingGetSubGroup (setting));
	subGroup->settings = ccsSettingListAppend (subGroup->settings, setting);
    }
}

static void
groupAdd (CCSSetting * setting, CCSPluginPrivate * p)
{
    CCSGroupList l = p->groups;
    CCSGroup     *group;

    while (l)
    {
	if (!strcmp (l->data->name, ccsSettingGetGroup (setting)))
	{
	    subGroupAdd (setting, l->data);
	    return;
	}

	l = l->next;
    }

    group = calloc (1, sizeof (CCSGroup));
    if (group)
    {
	group->refCount = 1;
    	p->groups = ccsGroupListAppend (p->groups, group);
	group->name = strdup (ccsSettingGetGroup (setting));
	subGroupAdd (setting, group);
    }
}

void
collateGroups (CCSPluginPrivate * p)
{
    CCSSettingList l = p->settings;

    while (l)
    {
	groupAdd (l->data, p);
	l = l->next;
    }
}

void
ccsFreeContext (CCSContext *c)
{
    if (!c)
	return;

    (*(GET_INTERFACE (CCSContextInterface, c))->contextDestructor) (c);
}

static void
ccsFreeContextDefault (CCSContext * c)
{
    if (!c)
	return;

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, c);

    if (cPrivate->profile)
	free (cPrivate->profile);

    if (cPrivate->configWatchId)
	ccsRemoveFileWatch (cPrivate->configWatchId);

    if (cPrivate->changedSettings)
	cPrivate->changedSettings = ccsSettingListFree (cPrivate->changedSettings, FALSE);

    ccsPluginListFree (cPrivate->plugins, TRUE);

    ccsObjectFinalize (c);
    free (c);
}

void
ccsFreePlugin (CCSPlugin *p)
{
    if (!p)
	return;

    (*(GET_INTERFACE (CCSPluginInterface, p))->pluginDestructor) (p);
}

static void
ccsFreePluginDefault (CCSPlugin * p)
{
    if (!p)
	return;

    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, p);

    free (pPrivate->name);
    free (pPrivate->shortDesc);
    free (pPrivate->longDesc);
    free (pPrivate->hints);
    free (pPrivate->category);

    ccsStringListFree (pPrivate->loadAfter, TRUE);
    ccsStringListFree (pPrivate->loadBefore, TRUE);
    ccsStringListFree (pPrivate->requiresPlugin, TRUE);
    ccsStringListFree (pPrivate->conflictPlugin, TRUE);
    ccsStringListFree (pPrivate->conflictFeature, TRUE);
    ccsStringListFree (pPrivate->providesFeature, TRUE);
    ccsStringListFree (pPrivate->requiresFeature, TRUE);

    ccsSettingListFree (pPrivate->settings, TRUE);
    ccsGroupListFree (pPrivate->groups, TRUE);
    ccsStrExtensionListFree (pPrivate->stringExtensions, TRUE);

    if (pPrivate->xmlFile)
	free (pPrivate->xmlFile);

    if (pPrivate->xmlPath)
	free (pPrivate->xmlPath);

#ifdef USE_PROTOBUF
    if (pPrivate->pbFilePath)
	free (pPrivate->pbFilePath);
#endif

    ccsObjectFinalize (p);
    free (p);
}

static void
ccsFreeSettingDefault (CCSSetting *s)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, s);

    free (sPrivate->name);
    free (sPrivate->shortDesc);
    free (sPrivate->longDesc);
    free (sPrivate->group);
    free (sPrivate->subGroup);
    free (sPrivate->hints);

    switch (sPrivate->type)
    {
    case TypeInt:
	ccsIntDescListFree (sPrivate->info.forInt.desc, TRUE);
	break;
    case TypeString:
	ccsStrRestrictionListFree (sPrivate->info.forString.restriction, TRUE);
	break;
    case TypeList:
	if (sPrivate->info.forList.listType == TypeInt)
	    ccsIntDescListFree (sPrivate->info.forList.listInfo->
				forInt.desc, TRUE);
	free (sPrivate->info.forList.listInfo);
	//ccsSettingValueListFree (sPrivate->value->value.asList, TRUE);
	break;
    default:
	break;
    }

    if (&sPrivate->defaultValue != sPrivate->value)
    {
	ccsFreeSettingValue (sPrivate->value);
    }

    ccsFreeSettingValue (&sPrivate->defaultValue);

    ccsObjectFinalize (s);
    free (s);
}

void
ccsFreeSetting (CCSSetting * s)
{
    if (!s)
	return;

    (*(GET_INTERFACE (CCSSettingInterface, s))->settingDestructor) (s);
}

void
ccsFreeGroup (CCSGroup * g)
{
    if (!g)
	return;

    free (g->name);
    ccsSubGroupListFree (g->subGroups, TRUE);
    free (g);
}

void
ccsFreeSubGroup (CCSSubGroup * s)
{
    if (!s)
	return;

    free (s->name);
    ccsSettingListFree (s->settings, FALSE);
    free (s);
}

static void
ccsFreeSettingValueCommon (CCSSettingValue *v,
			   CCSSettingType  type)
{
    switch (type)
    {
    case TypeString:
	free (v->value.asString);
	break;
    case TypeMatch:
	free (v->value.asMatch);
	break;
    case TypeList:
	if (!v->isListChild)
	    ccsSettingValueListFree (v->value.asList, TRUE);
	break;
    default:
	break;
    }
}

void
ccsFreeSettingValue (CCSSettingValue * v)
{
    if (!v)
	return;

    if (!v->parent)
    {
	ccsError ("cannot free value without parent - use ccsFreeSettingValueWithType and specify type instead");
	return;
    }

    CCSSettingType type = ccsSettingGetType (v->parent);

    if (v->isListChild)
	type = ccsSettingGetInfo (v->parent)->forList.listType;

    ccsFreeSettingValueCommon (v, type);

    /* List children cannot be a default value */
    if (v->isListChild ||
	v != ccsSettingGetDefaultValue (v->parent))
	free (v);
}

void
ccsFreeSettingValueWithType (CCSSettingValue *v,
			     CCSSettingType  type)
{
    ccsFreeSettingValueCommon (v, type);

    free (v);
}

void
ccsFreePluginConflict (CCSPluginConflict * c)
{
    if (!c)
	return;

    free (c->value);

    ccsPluginListFree (c->plugins, FALSE);

    free (c);
}

CCSBackendInfo *
ccsCopyBackendInfoFromBackend (CCSBackend *backend,
			       const CCSBackendInterface *interface)
{
    const CCSBackendInfo *backendInfo = (*interface->backendGetInfo) (backend);

    if (!backendInfo)
	return NULL;

    CCSBackendInfo *info = calloc (1, sizeof (CCSBackendInfo));

    if (!info)
	return NULL;

    memcpy (info, backendInfo, sizeof (CCSBackendInfo));

    /* This is an abuse here -
     * in order to minimize code duplication ccsGetBackendInfo returns
     * const static data, but when we're dealing with the copies we're
     * dealing with heap allocated memory, since you can't access the
     * const data in the case that the libraries are not open.
     * Thus the cast. */

    info->name = (const char *) strdup (backendInfo->name);
    info->shortDesc = (const char *) strdup (backendInfo->shortDesc);
    info->longDesc = (const char *) strdup (backendInfo->longDesc);

    return info;
}

void
ccsFreeBackendInfo (CCSBackendInfo *b)
{
    if (b->name)
	free ((char *) b->name);

    if (b->shortDesc)
	free ((char *) b->shortDesc);

    if (b->longDesc)
	free ((char *) b->longDesc);

    free (b);
}

void
ccsFreeIntDesc (CCSIntDesc * i)
{
    if (!i)
	return;

    if (i->name)
	free (i->name);

    free (i);
}

void
ccsFreeStrRestriction (CCSStrRestriction * r)
{
    if (!r)
	return;

    if (r->name)
	free (r->name);

    if (r->value)
	free (r->value);

    free (r);
}

void
ccsFreeStrExtension (CCSStrExtension *e)
{
    if (!e)
	return;

    if (e->basePlugin)
	free (e->basePlugin);

    ccsStringListFree (e->baseSettings, TRUE);
    ccsStrRestrictionListFree (e->restriction, TRUE);

    free (e);
}

void
ccsFreeString (CCSString *str)
{
    if (str->value)
	free (str->value);
    
    free (str);
}

CCSREF (String, CCSString)
CCSREF (Group, CCSGroup)
CCSREF (SubGroup, CCSSubGroup)
CCSREF (SettingValue, CCSSettingValue)
CCSREF (PluginConflict, CCSPluginConflict)
CCSREF (BackendInfo, CCSBackendInfo)
CCSREF (IntDesc, CCSIntDesc)
CCSREF (StrRestriction, CCSStrRestriction)
CCSREF (StrExtension, CCSStrExtension)

CCSREF_OBJ (Plugin, CCSPlugin)
CCSREF_OBJ (Setting, CCSSetting)
CCSREF_OBJ (Backend, CCSBackend)
CCSREF_OBJ (DynamicBackend, CCSDynamicBackend)
CCSREF_OBJ (Integration, CCSIntegration)
CCSREF_OBJ (IntegratedSetting, CCSIntegratedSetting);
CCSREF_OBJ (IntegratedSettingInfo, CCSIntegratedSettingInfo);
CCSREF_OBJ (IntegratedSettingFactory, CCSIntegratedSettingFactory);
CCSREF_OBJ (IntegratedSettingsStorage, CCSIntegratedSettingsStorage);

static void *
openBackend (const char *backend)
{
    char *home = getenv ("HOME");
    char *override_backend = getenv ("LIBCOMPIZCONFIG_BACKEND_PATH");
    void *dlhand = NULL;
    char *dlname = NULL;
    char *err = NULL;

    if (override_backend && strlen (override_backend))
    {
	if (asprintf (&dlname, "%s/lib%s.so",
		      override_backend, backend) == -1)
	    dlname = NULL;

	dlerror ();
	dlhand = dlopen (dlname, RTLD_NOW | RTLD_NODELETE | RTLD_LOCAL);
	err = dlerror ();
    }

    if (!dlhand && home && strlen (home))
    {
	if (dlname)
	    free (dlname);

	if (asprintf (&dlname, "%s/.compizconfig/backends/lib%s.so",
		      home, backend) == -1)
	    dlname = NULL;

	dlerror ();
	dlhand = dlopen (dlname, RTLD_NOW | RTLD_NODELETE | RTLD_LOCAL);
	err = dlerror ();
    }

    if (!dlhand)
    {
	if (dlname)
	    free (dlname);

	if (asprintf (&dlname, "%s/compizconfig/backends/lib%s.so",
		      LIBDIR, backend) == -1)
	    dlname = NULL;
	dlhand = dlopen (dlname, RTLD_NOW | RTLD_NODELETE | RTLD_LOCAL);
	err = dlerror ();
    }

    free (dlname);

    if (err)
    {
	ccsError ("dlopen: %s", err);
    }

    return dlhand;
}

void
ccsFreeBackend (CCSBackend *backend)
{
    ccsObjectFinalize (backend);
    free (backend);
}

void
ccsFreeDynamicBackend (CCSDynamicBackend *backend)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    ccsBackendFini (dbPrivate->backend);
    ccsBackendUnref (dbPrivate->backend);

    if (dbPrivate->dlhand)
	dlclose (dbPrivate->dlhand);

    ccsObjectFinalize (backend);
    free (backend);
}

CCSBackend *
ccsBackendNewWithDynamicInterface (CCSContext *context, const CCSBackendInterface *interface)
{
    CCSBackend *backend = calloc (1, sizeof (CCSBackend));

    ccsObjectInit (backend, &ccsDefaultObjectAllocator);
    ccsBackendRef (backend);

    ccsObjectAddInterface (backend, (CCSInterface *) interface, GET_INTERFACE_TYPE (CCSBackendInterface));

    return backend;
}

CCSDynamicBackend *
ccsDynamicBackendWrapLoadedBackend (const CCSInterfaceTable *interfaces, CCSBackend *backend, void *dlhand)
{
    CCSDynamicBackend *dynamicBackend = calloc (1, sizeof (CCSDynamicBackend));
    CCSDynamicBackendPrivate *dbPrivate = NULL;

    if (!dynamicBackend)
	return NULL;

    ccsObjectInit (dynamicBackend, &ccsDefaultObjectAllocator);
    ccsDynamicBackendRef (dynamicBackend);

    dbPrivate = calloc (1, sizeof (CCSDynamicBackendPrivate));

    if (!dbPrivate)
    {
	ccsDynamicBackendUnref (dynamicBackend);
	return NULL;
    }

    dbPrivate->dlhand = dlhand;
    dbPrivate->backend = backend;

    ccsObjectSetPrivate (dynamicBackend, (CCSPrivate *) dbPrivate);
    ccsObjectAddInterface (dynamicBackend, (CCSInterface *) interfaces->dynamicBackendWrapperInterface, GET_INTERFACE_TYPE (CCSBackendInterface));
    ccsObjectAddInterface (dynamicBackend, (CCSInterface *) interfaces->dynamicBackendInterface, GET_INTERFACE_TYPE (CCSDynamicBackendInterface));

    return dynamicBackend;
}

CCSBackend *
ccsOpenBackend (const CCSInterfaceTable *interfaces, CCSContext *context, const char *name)
{
    CCSBackendInterface *vt;
    void *dlhand = openBackend (name);

    if (!dlhand)
	return NULL;

    BackendGetInfoProc getInfo = dlsym (dlhand, "getBackendInfo");
    if (!getInfo)
    {
	dlclose (dlhand);
	return NULL;
    }

    vt = getInfo ();
    if (!vt)
    {
	dlclose (dlhand);
	return NULL;
    }

    CCSBackend *backend = ccsBackendNewWithDynamicInterface (context, vt);

    if (!backend)
    {
	dlclose (dlhand);
	return NULL;
    }

    CCSDynamicBackend *backendWrapper = ccsDynamicBackendWrapLoadedBackend (interfaces, backend, dlhand);

    if (!backendWrapper)
    {
	dlclose (dlhand);
	ccsBackendUnref (backend);
	return NULL;
    }

    return (CCSBackend *) backendWrapper;
}

Bool
ccsSetBackendDefault (CCSContext * context, char *name)
{
    Bool fallbackMode = FALSE;
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    if (cPrivate->backend)
    {
	/* no action needed if the backend is the same */

	if (strcmp (ccsDynamicBackendGetBackendName (cPrivate->backend), name) == 0)
	    return TRUE;

	ccsDynamicBackendUnref (cPrivate->backend);
	cPrivate->backend = NULL;
    }

    CCSBackend *backend = ccsOpenBackend (cPrivate->object_interfaces, context, name);

    if (!backend)
    {
	ccsWarning ("unable to open backend %s, falling back to ini", name);

	backend = ccsOpenBackend (cPrivate->object_interfaces, context, "ini");
	if (!backend)
	{
	    ccsError ("failed to open any backends, aborting");
	    abort ();
	}

	fallbackMode = TRUE;
    }

    cPrivate->backend = (CCSDynamicBackend *) backend;

    CCSBackendInitFunc backendInit = (GET_INTERFACE (CCSBackendInterface, cPrivate->backend))->backendInit;

    if (backendInit)
	(*backendInit) ((CCSBackend *) cPrivate->backend, context);

    ccsDisableFileWatch (cPrivate->configWatchId);
    if (!fallbackMode)
	ccsWriteConfig (OptionBackend, name);
    ccsEnableFileWatch (cPrivate->configWatchId);

    return TRUE;
}

Bool
ccsSetBackend (CCSContext *context, char *name)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextSetBackend) (context, name);
}

CCSIntegratedSetting * ccsIntegrationGetIntegratedSetting (CCSIntegration *integration,
							   const char *pluginName,
							   const char *settingName)
{
    return (*(GET_INTERFACE (CCSIntegrationInterface, integration))->getIntegratedSetting) (integration, pluginName, settingName);
}

Bool ccsIntegrationReadOptionIntoSetting (CCSIntegration *integration,
					  CCSContext		  *context,
					  CCSSetting		  *setting,
					  CCSIntegratedSetting *integratedSetting)
{
    return (*(GET_INTERFACE (CCSIntegrationInterface, integration))->readOptionIntoSetting) (integration, context, setting, integratedSetting);
}

void ccsIntegrationWriteSettingIntoOption (CCSIntegration *integration,
					   CCSContext		   *context,
					   CCSSetting		   *setting,
					   CCSIntegratedSetting *integratedSetting)
{
    (*(GET_INTERFACE (CCSIntegrationInterface, integration))->writeSettingIntoOption) (integration, context, setting, integratedSetting);
}

void ccsIntegrationUpdateIntegratedSettings (CCSIntegration *integration,
					     CCSContext	    *context,
					     CCSIntegratedSettingList integratedSettings)
{
    (*(GET_INTERFACE (CCSIntegrationInterface, integration))->updateIntegratedSettings) (integration, context, integratedSettings);
}

void ccsIntegrationDisallowIntegratedWrites (CCSIntegration *integration)
{
    (*(GET_INTERFACE (CCSIntegrationInterface, integration))->disallowIntegratedWrites) (integration);
}

void ccsIntegrationAllowIntegratedWrites (CCSIntegration *integration)
{
    (*(GET_INTERFACE (CCSIntegrationInterface, integration))->allowIntegratedWrites) (integration);
}

void ccsFreeIntegration (CCSIntegration *integration)
{
    (*(GET_INTERFACE (CCSIntegrationInterface, integration))->freeIntegrationBackend) (integration);
}

static CCSIntegratedSetting *
ccsNullIntegrationBackendGetIntegratedSetting (CCSIntegration *integration,
					       const char	  *pluginName,
					       const char	  *settingName)
{
    return NULL;
}

static Bool
ccsNullIntegrationBackendReadOptionIntoSetting (CCSIntegration *integration,
						CCSContext	      *context,
						CCSSetting	      *setting,
						CCSIntegratedSetting  *integrated)
{
    return FALSE;
}

static void
ccsNullIntegrationBackendWriteSettingIntoOption (CCSIntegration *integration,
						 CCSContext	      *context,
						 CCSSetting	      *setting,
						 CCSIntegratedSetting *integrated)
{
}

static void
ccsNullIntegrationBackendUpdateIntegratedSettings (CCSIntegration *integration,
						   CCSContext	  *context,
						   CCSIntegratedSettingList settings)
{
}

void
ccsNullIntegrationBackendFree (CCSIntegration *integration)
{
    ccsObjectFinalize (integration);
    (*integration->object.object_allocation->free_) (integration->object.object_allocation->allocator, integration);
}

const CCSIntegrationInterface ccsNullIntegrationBackendInterface =
{
    ccsNullIntegrationBackendGetIntegratedSetting,
    ccsNullIntegrationBackendReadOptionIntoSetting,
    ccsNullIntegrationBackendWriteSettingIntoOption,
    ccsNullIntegrationBackendUpdateIntegratedSettings,
    ccsNullIntegrationBackendFree
};

CCSIntegration *
ccsNullIntegrationBackendNew (CCSObjectAllocationInterface *ai)
{
    CCSIntegration *integration = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSIntegration));

    if (!integration)
	return NULL;

    ccsObjectInit (integration, ai);
    ccsObjectAddInterface (integration, (const CCSInterface *) &ccsNullIntegrationBackendInterface, GET_INTERFACE_TYPE (CCSIntegrationInterface));
    return integration;
}

const CCSBackendInfo * ccsBackendGetInfo (CCSBackend *backend)
{
    return (*(GET_INTERFACE (CCSBackendInterface, backend))->backendGetInfo) (backend);
}

static Bool
ccsDynamicBackendSupportsIntegrationDefault (CCSDynamicBackend *backend)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    return ccsBackendGetInfo (dbPrivate->backend)->integrationSupport;
}

const char * ccsDynamicBackendGetBackendName (CCSDynamicBackend *backend)
{
    return (*(GET_INTERFACE (CCSDynamicBackendInterface, backend))->getBackendName) (backend);
}

Bool ccsDynamicBackendSupportsRead (CCSDynamicBackend *backend)
{
    return (*(GET_INTERFACE (CCSDynamicBackendInterface, backend))->supportsRead) (backend);
}

Bool ccsDynamicBackendSupportsWrite (CCSDynamicBackend *backend)
{
    return (*(GET_INTERFACE (CCSDynamicBackendInterface, backend))->supportsWrite) (backend);
}

Bool ccsDynamicBackendSupportsProfiles (CCSDynamicBackend *backend)
{
    return (*(GET_INTERFACE (CCSDynamicBackendInterface, backend))->supportsProfiles) (backend);
}

Bool ccsDynamicBackendSupportsIntegration (CCSDynamicBackend *backend)
{
    return (*(GET_INTERFACE (CCSDynamicBackendInterface, backend))->supportsIntegration) (backend);
}

CCSBackend * ccsDynamicBackendGetRawBackend (CCSDynamicBackend *backend)
{
    return (*(GET_INTERFACE (CCSDynamicBackendInterface, backend))->getRawBackend) (backend);
}

Bool ccsBackendHasExecuteEvents (CCSBackend *backend)
{
    return (GET_INTERFACE (CCSBackendInterface, backend))->executeEvents != NULL;
}

void ccsBackendExecuteEvents (CCSBackend *backend, unsigned int flags)
{
    (*(GET_INTERFACE (CCSBackendInterface, backend))->executeEvents) (backend, flags);
}

Bool ccsBackendInit (CCSBackend *backend, CCSContext *context)
{
    return (*(GET_INTERFACE (CCSBackendInterface, backend))->backendInit) (backend, context);
}

Bool ccsBackendFini (CCSBackend *backend)
{
    return (*(GET_INTERFACE (CCSBackendInterface, backend))->backendFini) (backend);
}

static Bool ccsBackendHasReadInit (CCSBackend *backend)
{
    return (GET_INTERFACE (CCSBackendInterface, backend))->readInit != NULL;
}

Bool ccsBackendReadInit (CCSBackend *backend, CCSContext *context)
{
    return (*(GET_INTERFACE (CCSBackendInterface, backend))->readInit) (backend, context);
}

static Bool ccsBackendHasReadSetting (CCSBackend *backend)
{
    return (GET_INTERFACE (CCSBackendInterface, backend))->readSetting != NULL;
}

void ccsBackendReadSetting (CCSBackend *backend, CCSContext *context, CCSSetting *setting)
{
    (*(GET_INTERFACE (CCSBackendInterface, backend))->readSetting) (backend, context, setting);
}

static Bool ccsBackendHasReadDone (CCSBackend *backend)
{
    return (GET_INTERFACE (CCSBackendInterface, backend))->readDone != NULL;
}

void ccsBackendReadDone (CCSBackend *backend, CCSContext *context)
{
    return (*(GET_INTERFACE (CCSBackendInterface, backend))->readDone) (backend, context);
}

static Bool ccsBackendHasWriteInit (CCSBackend *backend)
{
    return (GET_INTERFACE (CCSBackendInterface, backend))->writeInit != NULL;
}

Bool ccsBackendWriteInit (CCSBackend *backend, CCSContext *context)
{
    return (*(GET_INTERFACE (CCSBackendInterface, backend))->writeInit) (backend, context);
}

static Bool ccsBackendHasWriteSetting (CCSBackend *backend)
{
    return (GET_INTERFACE (CCSBackendInterface, backend))->writeSetting != NULL;
}

void ccsBackendWriteSetting (CCSBackend *backend, CCSContext *context, CCSSetting *setting)
{
    (*(GET_INTERFACE (CCSBackendInterface, backend))->writeSetting) (backend, context, setting);
}

static Bool ccsBackendHasWriteDone (CCSBackend *backend)
{
    return (GET_INTERFACE (CCSBackendInterface, backend))->writeDone != NULL;
}

void ccsBackendWriteDone (CCSBackend *backend, CCSContext *context)
{
    return (*(GET_INTERFACE (CCSBackendInterface, backend))->writeDone) (backend, context);
}

static Bool ccsBackendHasUpdateSetting (CCSBackend *backend)
{
    return (GET_INTERFACE (CCSBackendInterface, backend))->updateSetting != NULL;
}

void ccsBackendUpdateSetting (CCSBackend *backend, CCSContext *context, CCSPlugin *plugin, CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSBackendInterface, backend))->updateSetting) (backend, context, plugin, setting);
}

static Bool ccsBackendHasGetSettingIsIntegrated (CCSBackend *backend)
{
    return (GET_INTERFACE (CCSBackendInterface, backend))->getSettingIsIntegrated != NULL;
}

Bool ccsBackendGetSettingIsIntegrated (CCSBackend *backend, CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSBackendInterface, backend))->getSettingIsIntegrated) (backend, setting);
}

static Bool ccsBackendHasGetSettingIsReadOnly (CCSBackend *backend)
{
    return (GET_INTERFACE (CCSBackendInterface, backend))->getSettingIsReadOnly != NULL;
}

Bool ccsBackendGetSettingIsReadOnly (CCSBackend *backend, CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSBackendInterface, backend))->getSettingIsReadOnly) (backend, setting);
}

static Bool ccsBackendHasGetExistingProfiles (CCSBackend *backend)
{
    return (GET_INTERFACE (CCSBackendInterface, backend))->getExistingProfiles != NULL;
}

CCSStringList ccsBackendGetExistingProfiles (CCSBackend *backend, CCSContext *context)
{
    return (*(GET_INTERFACE (CCSBackendInterface, backend))->getExistingProfiles) (backend, context);
}

static Bool ccsBackendHasDeleteProfile (CCSBackend *backend)
{
    return (GET_INTERFACE (CCSBackendInterface, backend))->deleteProfile != NULL;
}

Bool ccsBackendDeleteProfile (CCSBackend *backend, CCSContext *context, char *name)
{
    return (*(GET_INTERFACE (CCSBackendInterface, backend))->deleteProfile) (backend, context, name);
}

static Bool ccsBackendHasSetIntegration (CCSBackend *backend)
{
    return (GET_INTERFACE (CCSBackendInterface, backend))->setIntegration != NULL;
}

void ccsBackendSetIntegration (CCSBackend *backend, CCSIntegration *integration)
{
    return (*(GET_INTERFACE (CCSBackendInterface, backend))->setIntegration) (backend, integration);
}

static const char *
ccsDynamicBackendGetBackendNameDefault (CCSDynamicBackend *backend)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    return ccsBackendGetInfo (dbPrivate->backend)->name;
}

static Bool
ccsDynamicBackendSupportsReadDefault (CCSDynamicBackend *backend)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    return ccsBackendHasReadSetting (dbPrivate->backend);
}

static Bool
ccsDynamicBackendSupportsWriteDefault (CCSDynamicBackend *backend)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    return ccsBackendHasWriteSetting (dbPrivate->backend);
}

static Bool
ccsDynamicBackendSupportsProfilesDefault (CCSDynamicBackend *backend)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    return ccsBackendGetInfo (dbPrivate->backend)->profileSupport;

}

static CCSBackend * ccsDynamicBackendGetRawBackendDefault (CCSDynamicBackend *backend)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    return dbPrivate->backend;
}

static const CCSBackendInfo * ccsDynamicBackendGetInfoWrapper (CCSBackend *backend)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    return ccsBackendGetInfo (dbPrivate->backend);
}

static Bool ccsDynamicBackendInitWrapper (CCSBackend *backend, CCSContext *context)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    return ccsBackendInit (dbPrivate->backend, context);
}

static Bool ccsDynamicBackendFiniWrapper (CCSBackend *backend)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    return ccsBackendFini (dbPrivate->backend);
}

static void ccsDynamicBackendExecuteEventsWrapper (CCSBackend *backend, unsigned int flags)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    if (ccsBackendHasExecuteEvents (dbPrivate->backend))
	ccsBackendExecuteEvents (dbPrivate->backend, flags);
}

static Bool ccsDynamicBackendReadInitWrapper (CCSBackend *backend, CCSContext *context)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    if (ccsBackendHasReadInit (dbPrivate->backend))
	return ccsBackendReadInit (dbPrivate->backend, context);

    return TRUE;
}

static void ccsDynamicBackendReadSettingWrapper (CCSBackend *backend, CCSContext *context, CCSSetting *setting)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    if (ccsBackendHasReadSetting (dbPrivate->backend))
	ccsBackendReadSetting (dbPrivate->backend, context, setting);
}

static void ccsDynamicBackendReadDoneWrapper (CCSBackend *backend, CCSContext *context)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    if (ccsBackendHasReadDone (dbPrivate->backend))
	ccsBackendReadDone (dbPrivate->backend, context);
}

static Bool ccsDynamicBackendWriteInitWrapper (CCSBackend *backend, CCSContext *context)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    if (ccsBackendHasWriteInit (dbPrivate->backend))
	return ccsBackendWriteInit (dbPrivate->backend, context);

    return TRUE;
}

static void ccsDynamicBackendWriteSettingWrapper (CCSBackend *backend, CCSContext *context, CCSSetting *setting)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    if (ccsBackendHasWriteSetting (dbPrivate->backend))
	ccsBackendWriteSetting (dbPrivate->backend, context, setting);
}

static void ccsDynamicBackendWriteDoneWrapper (CCSBackend *backend, CCSContext *context)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    if (ccsBackendHasWriteDone (dbPrivate->backend))
	ccsBackendWriteDone (dbPrivate->backend, context);
}

static void ccsDynamicBackendUpdateSettingWrapper (CCSBackend *backend, CCSContext *context, CCSPlugin *plugin, CCSSetting *setting)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    if (ccsBackendHasUpdateSetting (dbPrivate->backend))
	ccsBackendUpdateSetting (dbPrivate->backend, context, plugin, setting);
}

static Bool ccsDynamicBackendGetSettingIsIntegratedWrapper (CCSBackend *backend, CCSSetting *setting)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    if (ccsBackendHasGetSettingIsIntegrated (dbPrivate->backend) &&
	ccsDynamicBackendSupportsIntegration ((CCSDynamicBackend *) backend))
	return ccsBackendGetSettingIsIntegrated (dbPrivate->backend, setting);

    return FALSE;
}

static Bool ccsDynamicBackendGetSettingIsReadOnlyWrapper (CCSBackend *backend, CCSSetting *setting)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    if (ccsBackendHasGetSettingIsReadOnly (dbPrivate->backend))
	return ccsBackendGetSettingIsReadOnly (dbPrivate->backend, setting);

    return FALSE;
}

static CCSStringList ccsDynamicBackendGetExistingProfilesWrapper (CCSBackend *backend, CCSContext *context)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    if (ccsBackendHasGetExistingProfiles (dbPrivate->backend) &&
	ccsDynamicBackendSupportsProfiles ((CCSDynamicBackend *) backend))
	return ccsBackendGetExistingProfiles (dbPrivate->backend, context);

    static CCSStringList sl = NULL;

    return sl;
}

static Bool ccsDynamicBackendDeleteProfileWrapper (CCSBackend *backend, CCSContext *context, char *profile)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    if (ccsBackendHasDeleteProfile (dbPrivate->backend) &&
	ccsDynamicBackendSupportsProfiles ((CCSDynamicBackend *) backend))
	return ccsBackendDeleteProfile (dbPrivate->backend, context, profile);

    return FALSE;
}

static void ccsDynamicBackendSetIntegrationWrapper (CCSBackend *backend, CCSIntegration *integration)
{
    CCSDynamicBackendPrivate *dbPrivate = GET_PRIVATE (CCSDynamicBackendPrivate, backend);

    if (ccsBackendHasSetIntegration (backend) &&
	ccsDynamicBackendSupportsIntegration ((CCSDynamicBackend *) backend))
	return ccsBackendSetIntegration (dbPrivate->backend, integration);
}

Bool
ccsCompareLists (CCSSettingValueList l1, CCSSettingValueList l2,
		 CCSSettingListInfo info)
{
    while (l1 && l2)
    {
	switch (info.listType)
	{
	case TypeInt:
	    if (l1->data->value.asInt != l2->data->value.asInt)
		return FALSE;
	    break;
	case TypeBool:
	    {
		Bool bothTrue = (l1->data->value.asBool && l2->data->value.asBool);
		Bool bothFalse = (!l1->data->value.asBool && !l2->data->value.asBool);

		/* Use the boolean operators as TRUE/FALSE can be redefined */
		if (!bothTrue && !bothFalse)
		    return FALSE;
	    }
	    break;
	case TypeFloat:
	    if (l1->data->value.asFloat != l2->data->value.asFloat)
		return FALSE;
	    break;
	case TypeString:
	    if (strcmp (l1->data->value.asString, l2->data->value.asString))
		return FALSE;
	    break;
	case TypeMatch:
	    if (strcmp (l1->data->value.asMatch, l2->data->value.asMatch))
		return FALSE;
	    break;
	case TypeKey:
	    if (!ccsIsEqualKey
		(l1->data->value.asKey, l2->data->value.asKey))
		return FALSE;
	    break;
	case TypeButton:
	    if (!ccsIsEqualButton
		(l1->data->value.asButton, l2->data->value.asButton))
		return FALSE;
	    break;
	case TypeEdge:
	    if (l1->data->value.asEdge != l2->data->value.asEdge)
		return FALSE;
	    break;
	case TypeBell:
	    if (l1->data->value.asBell != l2->data->value.asBell)
		return FALSE;
	    break;
	case TypeColor:
	    if (!ccsIsEqualColor
		(l1->data->value.asColor, l2->data->value.asColor))
		return FALSE;
	    break;
	default:
	    return FALSE;
	    break;
	}

	l1 = l1->next;
	l2 = l2->next;
    }

    if ((!l1 && l2) || (l1 && !l2))
	return FALSE;

    return TRUE;
}

static void
copyInfo (CCSSettingInfo *from, CCSSettingInfo *to, CCSSettingType type)
{	
    memcpy (from, to, sizeof (CCSSettingInfo));

    switch (type)
    {
	case TypeInt:
	{
	    CCSIntDescList idl = from->forInt.desc;

	    to->forInt = from->forInt;
	    to->forInt.desc = NULL;
	    
	    while (idl)
	    {
		CCSIntDesc *id = malloc (sizeof (CCSIntDesc));

		if (!idl->data)
		{
		    free (id);
		    idl = idl->next;
		    continue;
		}

		memcpy (id, idl->data, sizeof (CCSIntDesc));
		
		id->name = strdup (idl->data->name);
		id->refCount = 1;

	        to->forInt.desc = ccsIntDescListAppend (to->forInt.desc, id);
		
		idl = idl->next;
	    }
	    
	    break;
	}
	case TypeFloat:
	    to->forFloat = from->forFloat;
	    break;
	case TypeString:
	{
	    CCSStrRestrictionList srl = from->forString.restriction;

	    to->forString = from->forString;
	    to->forString.restriction = NULL;
	    
	    while (srl)
	    {
		CCSStrRestriction *sr = malloc (sizeof (CCSStrRestriction));

		if (!srl->data)
		{
		    srl = srl->next;
		    free (sr);
		    continue;
		}

		memcpy (sr, srl->data, sizeof (CCSStrRestriction));
		
		sr->name = strdup (srl->data->name);
		sr->value = strdup (srl->data->value);
		sr->refCount = 1;

	        to->forString.restriction = ccsStrRestrictionListAppend (to->forString.restriction, sr);
		
		srl = srl->next;
	    }
	    
	    break;
	}
	case TypeList:
	{
	    if (from->forList.listInfo)
	    {
		to->forList.listInfo = calloc (1, sizeof (CCSSettingInfo));

		copyInfo (from->forList.listInfo, to->forList.listInfo, from->forList.listType);
	    }

	    break;
	}
	case TypeAction:
	    to->forAction.internal = from->forAction.internal;
	    break;
	default:
	    break;
    }
}

static void
copyValue (CCSSettingValue * from, CCSSettingValue * to)
{
    memcpy (to, from, sizeof (CCSSettingValue));
    CCSSettingType type = ccsSettingGetType (from->parent);

    if (from->isListChild)
	type = ccsSettingGetInfo (from->parent)->forList.listType;

    switch (type)
    {
    case TypeString:
	to->value.asString = strdup (from->value.asString);
	break;
    case TypeMatch:
	to->value.asMatch = strdup (from->value.asMatch);
	break;
    case TypeList:
	to->value.asList = NULL;
	CCSSettingValueList l = from->value.asList;
	while (l)
	{
	    CCSSettingValue *value = calloc (1, sizeof (CCSSettingValue));
	    if (!value)
		break;

	    copyValue (l->data, value);
	    value->refCount = 1;
	    to->value.asList = ccsSettingValueListAppend (to->value.asList,
							  value);
	    l = l->next;
	}
	break;
    default:
	break;
    }
}

/* TODO: CCSSetting is not meant to be copyable ... remove */
static void
copySetting (CCSSetting *from, CCSSetting *to)
{
    /* Allocate a new private ptr for the new setting */
    CCSSettingPrivate *ccsPrivate = calloc (1, sizeof (CCSSettingPrivate));

    ccsObjectSetPrivate (to, (CCSPrivate *) ccsPrivate);

    unsigned int i = 0;

    /* copy interfaces */
    for (; i < from->object.n_interfaces; ++i)
	ccsObjectAddInterface (to,
			       from->object.interfaces[i],
			       from->object.interface_types[i]);

    CCSSettingPrivate *fromPrivate = (CCSSettingPrivate *) ccsObjectGetPrivate (from);
    CCSSettingPrivate *toPrivate = (CCSSettingPrivate *) ccsObjectGetPrivate (to);

    /* copy from fromPrivate to toPrivate for now, and replace all
     * fields that should be replaced */
    memcpy (toPrivate, fromPrivate, sizeof (CCSSettingPrivate));

    if (fromPrivate->name)
	toPrivate->name = strdup (fromPrivate->name);
    if (fromPrivate->shortDesc)
	toPrivate->shortDesc = strdup (fromPrivate->shortDesc);
    if (fromPrivate->longDesc)
	toPrivate->longDesc = strdup (fromPrivate->longDesc);
    if (fromPrivate->group)
	toPrivate->group = strdup (fromPrivate->group);
    if (fromPrivate->subGroup)
	toPrivate->subGroup = strdup (fromPrivate->subGroup);
    if (fromPrivate->hints)
	toPrivate->hints = strdup (fromPrivate->hints);
    if (fromPrivate->value)
    {
	toPrivate->value = malloc (sizeof (CCSSettingValue));
	
	if (!fromPrivate->value)
	    return;

	copyValue (fromPrivate->value, toPrivate->value);

	toPrivate->value->refCount = 1;
	toPrivate->value->parent = to;
    }

    copyValue (&fromPrivate->defaultValue, &toPrivate->defaultValue);
    copyInfo (&fromPrivate->info, &toPrivate->info, fromPrivate->type);

    toPrivate->defaultValue.parent = to;
    toPrivate->privatePtr = NULL;
    
    ccsSettingRef (to);
}

static void
copyFromDefault (CCSSetting * setting)
{
    CCSSettingValue *value;

    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    if (sPrivate->value != &sPrivate->defaultValue)
	ccsFreeSettingValue (sPrivate->value);

    value = calloc (1, sizeof (CCSSettingValue));
    if (!value)
    {
	sPrivate->value = &sPrivate->defaultValue;
	sPrivate->isDefault = TRUE;
	return;
    }
    
    value->refCount = 1;

    copyValue (&sPrivate->defaultValue, value);
    sPrivate->value = value;
    sPrivate->isDefault = FALSE;
}

void
ccsSettingResetToDefaultDefault (CCSSetting * setting, Bool processChanged)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting)

    if (sPrivate->value != &sPrivate->defaultValue)
    {
	ccsFreeSettingValue (sPrivate->value);

	if (processChanged)
	    ccsContextAddChangedSetting (ccsPluginGetContext (sPrivate->parent), setting);
    }

    sPrivate->value = &sPrivate->defaultValue;
    sPrivate->isDefault = TRUE;
}

Bool
ccsCheckValueEq (const CCSSettingValue *rhs,
		 CCSSettingType	       rhsType,
		 CCSSettingInfo	       *rhsInfo,
		 const CCSSettingValue *lhs,
		 CCSSettingType	       lhsType,
		 CCSSettingInfo	       *lhsInfo)
{
    CCSSettingType type;

    if (rhsType != lhsType)
    {
	ccsWarning ("Attempted to check equality between mismatched types!");
	return FALSE;
    }

    if (rhs->isListChild)
	type = rhsInfo->forList.listType;
    else
	type = rhsType;
    
    switch (type)
    {
	case TypeInt:
	    return lhs->value.asInt == rhs->value.asInt;
	case TypeBool:
	    return lhs->value.asBool == rhs->value.asBool;
	case TypeFloat:
	    return lhs->value.asFloat == rhs->value.asFloat;
	case TypeMatch:
	    return strcmp (lhs->value.asMatch, rhs->value.asMatch) == 0;
	case TypeString:
	    return strcmp (lhs->value.asString, rhs->value.asString) == 0;
	case TypeColor:
	    return ccsIsEqualColor (lhs->value.asColor, rhs->value.asColor);
	case TypeKey:
	    return ccsIsEqualKey (lhs->value.asKey, rhs->value.asKey);
	case TypeButton:
	    return ccsIsEqualButton (lhs->value.asButton, rhs->value.asButton);
	case TypeEdge:
	    return lhs->value.asEdge == rhs->value.asEdge;
	case TypeBell:
	    return lhs->value.asBell == rhs->value.asBell;
	case TypeAction:
	    ccsWarning ("Actions are not comparable!");
	    return FALSE;
	case TypeList:
	{
	    return ccsCompareLists (lhs->value.asList, rhs->value.asList,
				    lhsInfo->forList);
	
	}
	default:
	    break;
    }
    
    ccsWarning ("Failed to check equality for value with type %i", lhsType);
    return FALSE;
}

/* FIXME: That's a lot of code for the sake of type switching ...
 * maybe we need to switch to C++ here and use templates ... */

Bool
ccsSettingSetIntDefault (CCSSetting * setting, int data, Bool processChanged)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting)

    if (sPrivate->type != TypeInt)
	return FALSE;

    if (sPrivate->isDefault && (sPrivate->defaultValue.value.asInt == data))
	return TRUE;

    if (!sPrivate->isDefault && (sPrivate->defaultValue.value.asInt == data))
    {
	ccsResetToDefault (setting, processChanged);
	return TRUE;
    }

    if (sPrivate->value->value.asInt == data)
	return TRUE;

    if ((data < sPrivate->info.forInt.min) ||
	 (data > sPrivate->info.forInt.max))
	return FALSE;

    if (sPrivate->isDefault)
	copyFromDefault (setting);

    sPrivate->value->value.asInt = data;

    if (processChanged)
	ccsContextAddChangedSetting (ccsPluginGetContext (sPrivate->parent), setting);

    return TRUE;
}

Bool
ccsSettingSetFloatDefault (CCSSetting * setting, float data, Bool processChanged)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    if (sPrivate->type != TypeFloat)
	return FALSE;

    if (sPrivate->isDefault && (sPrivate->defaultValue.value.asFloat == data))
	return TRUE;

    if (!sPrivate->isDefault && (sPrivate->defaultValue.value.asFloat == data))
    {
	ccsResetToDefault (setting, processChanged);
	return TRUE;
    }

    /* allow the values to differ a tiny bit because of
       possible rounding / precision issues */
    if (fabs (sPrivate->value->value.asFloat - data) < 1e-5)
	return TRUE;

    if ((data < sPrivate->info.forFloat.min) ||
	 (data > sPrivate->info.forFloat.max))
	return FALSE;

    if (sPrivate->isDefault)
	copyFromDefault (setting);

    sPrivate->value->value.asFloat = data;

    if (processChanged)
	ccsContextAddChangedSetting (ccsPluginGetContext (sPrivate->parent), setting);

    return TRUE;
}

Bool
ccsSettingSetBoolDefault (CCSSetting * setting, Bool data, Bool processChanged)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting)

    if (sPrivate->type != TypeBool)
	return FALSE;

    if (sPrivate->isDefault
	&& ((sPrivate->defaultValue.value.asBool && data)
	     || (!sPrivate->defaultValue.value.asBool && !data)))
	return TRUE;

    if (!sPrivate->isDefault
	&& ((sPrivate->defaultValue.value.asBool && data)
	     || (!sPrivate->defaultValue.value.asBool && !data)))
    {
	ccsResetToDefault (setting, processChanged);
	return TRUE;
    }

    if ((sPrivate->value->value.asBool && data)
	 || (!sPrivate->value->value.asBool && !data))
	return TRUE;

    if (sPrivate->isDefault)
	copyFromDefault (setting);

    sPrivate->value->value.asBool = data;

    if (processChanged)
	ccsContextAddChangedSetting (ccsPluginGetContext (sPrivate->parent), setting);

    return TRUE;
}

Bool
ccsSettingSetStringDefault (CCSSetting * setting, const char *data, Bool processChanged)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    if (sPrivate->type != TypeString)
	return FALSE;

    if (!data)
	return FALSE;

    Bool isDefault = strcmp (sPrivate->defaultValue.value.asString, data) == 0;

    if (sPrivate->isDefault && isDefault)
	return TRUE;

    if (!sPrivate->isDefault && isDefault)
    {
	ccsResetToDefault (setting, processChanged);
	return TRUE;
    }

    if (!strcmp (sPrivate->value->value.asString, data))
	return TRUE;

    if (sPrivate->isDefault)
	copyFromDefault (setting);

    free (sPrivate->value->value.asString);

    sPrivate->value->value.asString = strdup (data);

    if (processChanged)
	ccsContextAddChangedSetting (ccsPluginGetContext (sPrivate->parent), setting);

    return TRUE;
}

Bool
ccsSettingSetColorDefault (CCSSetting * setting, CCSSettingColorValue data, Bool processChanged)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting)

    if (sPrivate->type != TypeColor)
	return FALSE;

    CCSSettingColorValue defValue = sPrivate->defaultValue.value.asColor;

    Bool isDefault = ccsIsEqualColor (defValue, data);

    if (sPrivate->isDefault && isDefault)
	return TRUE;

    if (!sPrivate->isDefault && isDefault)
    {
	ccsResetToDefault (setting, processChanged);
	return TRUE;
    }

    if (ccsIsEqualColor (sPrivate->value->value.asColor, data))
	return TRUE;

    if (sPrivate->isDefault)
	copyFromDefault (setting);

    sPrivate->value->value.asColor = data;

    if (processChanged)
	ccsContextAddChangedSetting (ccsPluginGetContext (sPrivate->parent), setting);

    return TRUE;
}

Bool
ccsSettingSetMatchDefault (CCSSetting * setting, const char *data, Bool processChanged)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    if (sPrivate->type != TypeMatch)
	return FALSE;

    if (!data)
	return FALSE;

    Bool isDefault = strcmp (sPrivate->defaultValue.value.asMatch, data) == 0;

    if (sPrivate->isDefault && isDefault)
	return TRUE;

    if (!sPrivate->isDefault && isDefault)
    {
	ccsResetToDefault (setting, processChanged);
	return TRUE;
    }

    if (!strcmp (sPrivate->value->value.asMatch, data))
	return TRUE;

    if (sPrivate->isDefault)
	copyFromDefault (setting);

    free (sPrivate->value->value.asMatch);

    sPrivate->value->value.asMatch = strdup (data);

    if (processChanged)
	ccsContextAddChangedSetting (ccsPluginGetContext (sPrivate->parent), setting);

    return TRUE;
}

Bool
ccsSettingSetKeyDefault (CCSSetting * setting, CCSSettingKeyValue data, Bool processChanged)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    if (sPrivate->type != TypeKey)
	return FALSE;

    CCSSettingKeyValue defValue = sPrivate->defaultValue.value.asKey;

    Bool isDefault = ccsIsEqualKey (data, defValue);

    if (sPrivate->isDefault && isDefault)
	return TRUE;

    if (!sPrivate->isDefault && isDefault)
    {
	ccsResetToDefault (setting, processChanged);
	return TRUE;
    }

    if (ccsIsEqualKey (sPrivate->value->value.asKey, data))
	return TRUE;

    if (sPrivate->isDefault)
	copyFromDefault (setting);

    sPrivate->value->value.asKey.keysym = data.keysym;
    sPrivate->value->value.asKey.keyModMask = data.keyModMask;

    if (processChanged)
	ccsContextAddChangedSetting (ccsPluginGetContext (sPrivate->parent), setting);

    return TRUE;
}

Bool
ccsSettingSetButtonDefault (CCSSetting * setting, CCSSettingButtonValue data, Bool processChanged)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    if (sPrivate->type != TypeButton)
	return FALSE;

    CCSSettingButtonValue defValue = sPrivate->defaultValue.value.asButton;

    Bool isDefault = ccsIsEqualButton (data, defValue);

    if (sPrivate->isDefault && isDefault)
	return TRUE;

    if (!sPrivate->isDefault && isDefault)
    {
	ccsResetToDefault (setting, processChanged);
	return TRUE;
    }

    if (ccsIsEqualButton (sPrivate->value->value.asButton, data))
	return TRUE;

    if (sPrivate->isDefault)
	copyFromDefault (setting);

    sPrivate->value->value.asButton.button = data.button;
    sPrivate->value->value.asButton.buttonModMask = data.buttonModMask;
    sPrivate->value->value.asButton.edgeMask = data.edgeMask;

    if (processChanged)
	ccsContextAddChangedSetting (ccsPluginGetContext (sPrivate->parent), setting);

    return TRUE;
}

Bool
ccsSettingSetEdgeDefault (CCSSetting * setting, unsigned int data, Bool processChanged)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting)

    if (sPrivate->type != TypeEdge)
	return FALSE;

    Bool isDefault = (data == sPrivate->defaultValue.value.asEdge);

    if (sPrivate->isDefault && isDefault)
	return TRUE;

    if (!sPrivate->isDefault && isDefault)
    {
	ccsResetToDefault (setting, processChanged);
	return TRUE;
    }

    if (sPrivate->value->value.asEdge == data)
	return TRUE;

    if (sPrivate->isDefault)
	copyFromDefault (setting);

    sPrivate->value->value.asEdge = data;

    if (processChanged)
	ccsContextAddChangedSetting (ccsPluginGetContext (sPrivate->parent), setting);

    return TRUE;
}

Bool
ccsSettingSetBellDefault (CCSSetting * setting, Bool data, Bool processChanged)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    if (sPrivate->type != TypeBell)
	return FALSE;

    Bool isDefault = (data == sPrivate->defaultValue.value.asBool);

    if (sPrivate->isDefault && isDefault)
	return TRUE;

    if (!sPrivate->isDefault && isDefault)
    {
	ccsResetToDefault (setting, processChanged);
	return TRUE;
    }

    if (sPrivate->value->value.asBell == data)
	return TRUE;

    if (sPrivate->isDefault)
	copyFromDefault (setting);

    sPrivate->value->value.asBell = data;

    if (processChanged)
	ccsContextAddChangedSetting (ccsPluginGetContext (sPrivate->parent), setting);

    return TRUE;
}

CCSSettingValue *
ccsCopyValue (CCSSettingValue *orig,
	      CCSSettingType  type,
	      CCSSettingInfo  *info)
{
    CCSSettingValue *value = calloc (1, sizeof (CCSSettingValue));

    if (!value)
	return NULL;

    value->refCount = 1;
    value->parent = orig->parent;
    value->isListChild = orig->isListChild;

    CCSSettingType vType = value->isListChild ? info->forList.listType : type;

    switch (vType)
    {
	case TypeInt:
	    value->value.asInt = orig->value.asInt;
	    break;
	case TypeBool:
	    value->value.asBool = orig->value.asBool;
	    break;
	case TypeFloat:
	    value->value.asFloat = orig->value.asFloat;
	    break;
	case TypeString:
	    value->value.asString = strdup (orig->value.asString);
	    break;
	case TypeMatch:
	    value->value.asMatch = strdup (orig->value.asMatch);
	    break;
	case TypeKey:
	    memcpy (&value->value.asKey, &orig->value.asKey,
		    sizeof (CCSSettingKeyValue));
	    break;
	case TypeButton:
	    memcpy (&value->value.asButton, &orig->value.asButton,
		    sizeof (CCSSettingButtonValue));
	    break;
	case TypeEdge:
	    value->value.asEdge = orig->value.asEdge;
	    break;
	case TypeBell:
	    value->value.asBell = orig->value.asBell;
	    break;
	case TypeColor:
	    memcpy (&value->value.asColor, &orig->value.asColor,
		    sizeof (CCSSettingColorValue));
	    break;
	default:
	    free (value);
	    return NULL;
	    break;
    }

    return value;
}

CCSSettingValueList
ccsCopyList (CCSSettingValueList l1, CCSSetting * setting)
{
    CCSSettingInfo      *info = ccsSettingGetInfo (setting);
    CCSSettingType      type   = ccsSettingGetType (setting);
    CCSSettingValueList l2 = NULL;

    while (l1)
    {
	CCSSettingValue *value = ccsCopyValue (l1->data,
					       type,
					       info);

	/* FIXME If l2 != NULL, we leak l2 */
	if (!value)
	    return l2;

	l2 = ccsSettingValueListAppend (l2, value);
	l1 = l1->next;
    }

    return l2;
}

Bool
ccsSettingSetListDefault (CCSSetting * setting, CCSSettingValueList data, Bool processChanged)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting)

    if (sPrivate->type != TypeList)
	return FALSE;

    Bool isDefault = ccsCompareLists (sPrivate->defaultValue.value.asList, data,
				      sPrivate->info.forList);

    /* Don't need to worry about default values
     * when processChanged is off since use of that
     * API wants direct access to ths list for
     * temporary storage */
    if (!processChanged)
    {
	if (sPrivate->isDefault && isDefault)
	{
	    return TRUE;
	}

	if (!sPrivate->isDefault && isDefault)
	{
	    ccsResetToDefault (setting, processChanged);
	    return TRUE;
	}
    }

    if (ccsCompareLists (sPrivate->value->value.asList, data,
			 sPrivate->info.forList))
    {
	return TRUE;
    }

    if (sPrivate->isDefault)
	copyFromDefault (setting);

    ccsSettingValueListFree (sPrivate->value->value.asList, TRUE);

    sPrivate->value->value.asList = ccsCopyList (data, setting);

    if ((strcmp (sPrivate->name, "active_plugins") == 0) &&
	(strcmp (ccsPluginGetName (sPrivate->parent), "core") == 0) && processChanged)
    {
	CCSStringList list;

	list = ccsGetStringListFromValueList (sPrivate->value->value.asList);
	ccsSetActivePluginList (ccsPluginGetContext (sPrivate->parent), list);
	ccsStringListFree (list, TRUE);
    }

    if (processChanged)
	ccsContextAddChangedSetting (ccsPluginGetContext (sPrivate->parent), setting);

    return TRUE;
}

Bool
ccsSettingSetValueDefault (CCSSetting * setting, CCSSettingValue * data, Bool processChanged)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    switch (sPrivate->type)
    {
    case TypeInt:
	return ccsSetInt (setting, data->value.asInt, processChanged);
	break;
    case TypeFloat:
	return ccsSetFloat (setting, data->value.asFloat, processChanged);
	break;
    case TypeBool:
	return ccsSetBool (setting, data->value.asBool, processChanged);
	break;
    case TypeColor:
	return ccsSetColor (setting, data->value.asColor, processChanged);
	break;
    case TypeString:
	return ccsSetString (setting, data->value.asString, processChanged);
	break;
    case TypeMatch:
	return ccsSetMatch (setting, data->value.asMatch, processChanged);
	break;
    case TypeKey:
	return ccsSetKey (setting, data->value.asKey, processChanged);
	break;
    case TypeButton:
	return ccsSetButton (setting, data->value.asButton, processChanged);
	break;
    case TypeEdge:
	return ccsSetEdge (setting, data->value.asEdge, processChanged);
	break;
    case TypeBell:
	return ccsSetBell (setting, data->value.asBell, processChanged);
	break;
    case TypeList:
	return ccsSetList (setting, data->value.asList, processChanged);
    default:
	break;
    }

    return FALSE;
}

Bool
ccsSettingGetIntDefault (CCSSetting * setting, int *data)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    if (sPrivate->type != TypeInt)
	return FALSE;

    *data = sPrivate->value->value.asInt;
    return TRUE;
}

Bool
ccsSettingGetFloatDefault (CCSSetting * setting, float *data)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting)

    if (sPrivate->type != TypeFloat)
	return FALSE;

    *data = sPrivate->value->value.asFloat;
    return TRUE;
}

Bool
ccsSettingGetBoolDefault (CCSSetting * setting, Bool * data)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting)

    if (sPrivate->type != TypeBool)
	return FALSE;

    *data = sPrivate->value->value.asBool;
    return TRUE;
}

Bool
ccsSettingGetStringDefault (CCSSetting * setting, char **data)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting)

    if (sPrivate->type != TypeString)
	return FALSE;

    *data = sPrivate->value->value.asString;
    return TRUE;
}

Bool
ccsSettingGetColorDefault (CCSSetting * setting, CCSSettingColorValue * data)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting)

    if (sPrivate->type != TypeColor)
	return TRUE;

    *data = sPrivate->value->value.asColor;
    return TRUE;
}

Bool
ccsSettingGetMatchDefault (CCSSetting * setting, char **data)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting)

    if (sPrivate->type != TypeMatch)
	return FALSE;

    *data = sPrivate->value->value.asMatch;
    return TRUE;
}

Bool
ccsSettingGetKeyDefault (CCSSetting * setting, CCSSettingKeyValue * data)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting)

    if (sPrivate->type != TypeKey)
	return FALSE;

    *data = sPrivate->value->value.asKey;
    return TRUE;
}

Bool
ccsSettingGetButtonDefault (CCSSetting * setting, CCSSettingButtonValue * data)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting)

    if (sPrivate->type != TypeButton)
	return FALSE;

    *data = sPrivate->value->value.asButton;
    return TRUE;
}

Bool
ccsSettingGetEdgeDefault (CCSSetting * setting, unsigned int * data)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting)

    if (sPrivate->type != TypeEdge)
	return FALSE;

    *data = sPrivate->value->value.asEdge;
    return TRUE;
}

Bool
ccsSettingGetBellDefault (CCSSetting * setting, Bool * data)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting)

    if (sPrivate->type != TypeBell)
	return FALSE;

    *data = sPrivate->value->value.asBell;
    return TRUE;
}

Bool
ccsSettingGetListDefault (CCSSetting * setting, CCSSettingValueList * data)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting)

    if (sPrivate->type != TypeList)
	return FALSE;

    *data = sPrivate->value->value.asList;
    return TRUE;
}

Bool ccsGetInt (CCSSetting *setting,
		int        *data)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetInt) (setting, data);
}

Bool ccsGetFloat (CCSSetting *setting,
		  float      *data)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetFloat) (setting, data);
}

Bool ccsGetBool (CCSSetting *setting,
		 Bool       *data)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetBool) (setting, data);
}

Bool ccsGetString (CCSSetting *setting,
		   char       **data)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetString) (setting, data);
}

Bool ccsGetColor (CCSSetting           *setting,
		  CCSSettingColorValue *data)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetColor) (setting, data);
}

Bool ccsGetMatch (CCSSetting *setting,
		  char       **data)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetMatch) (setting, data);
}

Bool ccsGetKey (CCSSetting         *setting,
		CCSSettingKeyValue *data)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetKey) (setting, data);
}

Bool ccsGetButton (CCSSetting            *setting,
		   CCSSettingButtonValue *data)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetButton) (setting, data);
}

Bool ccsGetEdge (CCSSetting  *setting,
		 unsigned int *data)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetEdge) (setting, data);
}

Bool ccsGetBell (CCSSetting *setting,
		 Bool       *data)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetBell) (setting, data);
}

Bool ccsGetList (CCSSetting          *setting,
		 CCSSettingValueList *data)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetList) (setting, data);
}

Bool ccsSetInt (CCSSetting *setting,
		int        data,
		Bool	   processChanged)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingSetInt) (setting, data, processChanged);
}

Bool ccsSetFloat (CCSSetting *setting,
		  float      data,
		  Bool	     processChanged)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingSetFloat) (setting, data, processChanged);
}

Bool ccsSetBool (CCSSetting *setting,
		 Bool       data,
		 Bool	    processChanged)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingSetBool) (setting, data, processChanged);
}

Bool ccsSetString (CCSSetting *setting,
		   const char *data,
		   Bool	      processChanged)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingSetString) (setting, data, processChanged);
}

Bool ccsSetColor (CCSSetting           *setting,
		  CCSSettingColorValue data,
		  Bool		       processChanged)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingSetColor) (setting, data, processChanged);
}

Bool ccsSetMatch (CCSSetting *setting,
		  const char *data,
		  Bool	     processChanged)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingSetMatch) (setting, data, processChanged);
}

Bool ccsSetKey (CCSSetting         *setting,
		CCSSettingKeyValue data,
		Bool		   processChanged)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingSetKey) (setting, data, processChanged);
}

Bool ccsSetButton (CCSSetting            *setting,
		   CCSSettingButtonValue data,
		   Bool			 processChanged)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingSetButton) (setting, data, processChanged);
}

Bool ccsSetEdge (CCSSetting   *setting,
		 unsigned int data,
		 Bool	      processChanged)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingSetEdge) (setting, data, processChanged);
}

Bool ccsSetBell (CCSSetting *setting,
		 Bool       data,
		 Bool	    processChanged)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingSetBell) (setting, data, processChanged);
}

Bool ccsSetList (CCSSetting          *setting,
		 CCSSettingValueList data,
		 Bool	 processChanged)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingSetList) (setting, data, processChanged);
}

Bool ccsSetValue (CCSSetting      *setting,
		  CCSSettingValue *data,
		  Bool		  processChanged)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingSetValue) (setting, data, processChanged);
}

void ccsResetToDefault (CCSSetting * setting, Bool processChanged)
{
    (*(GET_INTERFACE (CCSSettingInterface, setting))->settingResetToDefault) (setting, processChanged);
}

Bool ccsSettingIsIntegrated (CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingIsIntegrated) (setting);
}

Bool ccsSettingIsReadOnly (CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingIsReadOnly) (setting);
}

Bool ccsSettingIsReadableByBackend (CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingIsReadableByBackend) (setting);
}

void
ccsContextDestroy (CCSContext * context)
{
    if (!context)
	return;

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    if (cPrivate->backend)
    {
	ccsDynamicBackendUnref (cPrivate->backend);
	cPrivate->backend = NULL;
    }

    ccsFreeContext (context);
}

CCSPluginList
ccsGetActivePluginListDefault (CCSContext * context)
{
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    CCSPluginList rv = NULL;
    CCSPluginList l = cPrivate->plugins;

    while (l)
    {
	CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, l->data);
	if (pPrivate->active && strcmp (ccsPluginGetName (l->data), "ccp"))
	{
	    rv = ccsPluginListAppend (rv, l->data);
	}

	l = l->next;
    }

    return rv;
}

CCSPluginList
ccsGetActivePluginList (CCSContext *context)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextGetActivePluginList) (context);
}

static CCSPlugin *
findPluginInList (CCSPluginList list, char *name)
{
    if (!name || !strlen (name))
	return NULL;

    while (list)
    {
	if (!strcmp (ccsPluginGetName (list->data), name))
	    return list->data;

	list = list->next;
    }

    return NULL;
}

typedef struct _PluginSortHelper
{
    CCSPlugin *plugin;
    CCSPluginList after;
} PluginSortHelper;

CCSStringList
ccsGetSortedPluginStringListDefault (CCSContext * context)
{
    CCSPluginList ap = ccsGetActivePluginList (context);
    CCSPluginList list;
    CCSPlugin *p = NULL;
    CCSString *strCore = calloc (1, sizeof (CCSString));

    strCore->value = strdup ("core");
    strCore->refCount = 1;

    CCSStringList rv = ccsStringListAppend (NULL, strCore);
    PluginSortHelper *ph = NULL;

    p = findPluginInList (ap, "core");
    if (p)
	ap = ccsPluginListRemove (ap, p, FALSE);

    int len = ccsPluginListLength (ap);
    if (len == 0)
    {
	ccsStringListFree (rv, TRUE);
	return NULL;
    }
    int i, j;
    /* TODO: conflict handling */

    PluginSortHelper *plugins = calloc (1, len * sizeof (PluginSortHelper));
    if (!plugins)
    {
	ccsStringListFree (rv, TRUE);
	return NULL;
    }

    for (i = 0, list = ap; i < len; i++, list = list->next)
    {
	plugins[i].plugin = list->data;
	plugins[i].after = NULL;
    }

    for (i = 0; i < len; i++)
    {
	CCSStringList l = ccsPluginGetLoadAfter (plugins[i].plugin);
	while (l)
	{
	    p = findPluginInList (ap, ((CCSString *)l->data)->value);

	    if (p && !ccsPluginListFind (plugins[i].after, p))
		plugins[i].after = ccsPluginListAppend (plugins[i].after, p);

	    l = l->next;
	}

	l = ccsPluginGetRequiresPlugins (plugins[i].plugin);
	while (l)
	{
	    Bool found = FALSE;
	    p = findPluginInList (ap, ((CCSString *)l->data)->value);

	    CCSStringList l2 = ccsPluginGetLoadBefore (plugins[i].plugin);
	    while (l2)
	    {
		if (strcmp (((CCSString *)l2->data)->value,
			    ((CCSString *)l->data)->value) == 0)
		    found = TRUE;
		l2 = l2->next;
	    }
	    
	    if (p && !ccsPluginListFind (plugins[i].after, p) && !found)
		plugins[i].after = ccsPluginListAppend (plugins[i].after, p);

	    l = l->next;
	}

	l = ccsPluginGetLoadBefore (plugins[i].plugin);
	while (l)
	{
	    p = findPluginInList (ap, ((CCSString *)l->data)->value);

	    if (p)
	    {
		ph = NULL;

		for (j = 0; j < len; j++)
		    if (p == plugins[j].plugin)
			ph = &plugins[j];

		if (ph && !ccsPluginListFind (ph->after, plugins[i].plugin))
		    ph->after = ccsPluginListAppend (ph->after,
						     plugins[i].plugin);
	    }

	    l = l->next;
	}
    }

    ccsPluginListFree (ap, FALSE);

    Bool error = FALSE;
    int removed = 0;
    Bool found;

    while (!error && removed < len)
    {
	found = FALSE;

	for (i = 0; i < len; i++)
	{
	    CCSString *strPluginName;		
		
	    if (!plugins[i].plugin)
		continue;
	    if (plugins[i].after)
		continue;

	    /* This is a special case to ensure that bench is the last plugin */
	    if (len - removed > 1 &&
		strcmp (ccsPluginGetName (plugins[i].plugin), "bench") == 0)
		continue;

	    found = TRUE;
	    removed++;
	    p = plugins[i].plugin;
	    plugins[i].plugin = NULL;

	    for (j = 0; j < len; j++)
		plugins[j].after =
		    ccsPluginListRemove (plugins[j].after, p, FALSE);

	    strPluginName = calloc (1, sizeof (CCSString));
	    
	    strPluginName->value = strdup (ccsPluginGetName (p));
	    strPluginName->refCount = 1;

	    rv = ccsStringListAppend (rv, strPluginName);
	}

	if (!found)
	    error = TRUE;
    }

    if (error)
    {
	ccsError ("Unable to generate sorted plugin list");

	for (i = 0; i < len; i++)
	{
	    ccsPluginListFree (plugins[i].after, FALSE);
	}

	ccsStringListFree (rv, TRUE);

	rv = NULL;
    }

    free (plugins);

    return rv;
}

CCSStringList
ccsGetSortedPluginStringList (CCSContext *context)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextGetSortedPluginStringList) (context);
}

const char *
ccsGetBackendDefault (CCSContext * context)
{
    if (!context)
	return NULL;

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    if (!cPrivate->backend)
	return NULL;

    return ccsDynamicBackendGetBackendName (cPrivate->backend);
}

const char *
ccsGetBackend (CCSContext *context)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextGetBackend) (context);
}

Bool
ccsGetIntegrationEnabledDefault (CCSContext * context)
{
    if (!context)
	return FALSE;

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    return cPrivate->deIntegration;
}

Bool
ccsGetIntegrationEnabled (CCSContext *context)
{
    if (!context)
	return FALSE;

    return (*(GET_INTERFACE (CCSContextInterface, context))->contextGetIntegrationEnabled) (context);
}

const char *
ccsGetProfileDefault (CCSContext * context)
{
    if (!context)
	return NULL;

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    return cPrivate->profile;
}

const char *
ccsGetProfile (CCSContext *context)
{
    if (!context)
	return NULL;

    return (*(GET_INTERFACE (CCSContextInterface, context))->contextGetProfile) (context);
}

Bool
ccsGetPluginListAutoSortDefault (CCSContext * context)
{
    if (!context)
	return FALSE;

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    return cPrivate->pluginListAutoSort;
}

Bool
ccsGetPluginListAutoSort (CCSContext *context)
{
    if (!context)
	return FALSE;

    return (*(GET_INTERFACE (CCSContextInterface, context))->contextGetPluginListAutoSort) (context);
}

void
ccsSetIntegrationEnabledDefault (CCSContext * context, Bool value)
{
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    /* no action required if nothing changed */
    if ((!cPrivate->deIntegration && !value) ||
	 (cPrivate->deIntegration && value))
	return;

    cPrivate->deIntegration = value;

    ccsDisableFileWatch (cPrivate->configWatchId);
    ccsWriteConfig (OptionIntegration, (value) ? "true" : "false");
    ccsEnableFileWatch (cPrivate->configWatchId);
}

void
ccsSetIntegrationEnabled (CCSContext *context, Bool value)
{
    (*(GET_INTERFACE (CCSContextInterface, context))->contextSetIntegrationEnabled) (context, value);
}

static void
ccsWriteAutoSortedPluginList (CCSContext *context)
{
    CCSStringList list;
    CCSPlugin     *p;

    list = ccsGetSortedPluginStringList (context);
    p    = ccsFindPlugin (context, "core");
    if (p)
    {
	CCSSetting *s;

	s = ccsFindSetting (p, "active_plugins");
	if (s)
	{
	    CCSSettingValueList vl;

	    vl = ccsGetValueListFromStringList (list, s);
	    ccsSetList (s, vl, TRUE);
	    ccsSettingValueListFree (vl, TRUE);
	    ccsWriteChangedSettings (context);
	}
    }
    ccsStringListFree (list, TRUE);
}

void
ccsSetPluginListAutoSortDefault (CCSContext * context, Bool value)
{
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    /* no action required if nothing changed */
    if ((!cPrivate->pluginListAutoSort && !value) ||
	 (cPrivate->pluginListAutoSort && value))
	return;

    cPrivate->pluginListAutoSort = value;

    ccsDisableFileWatch (cPrivate->configWatchId);
    ccsWriteConfig (OptionAutoSort, (value) ? "true" : "false");
    ccsEnableFileWatch (cPrivate->configWatchId);

    if (value)
	ccsWriteAutoSortedPluginList (context);
}

void
ccsSetPluginListAutoSort (CCSContext *context, Bool value)
{
    (*(GET_INTERFACE (CCSContextInterface, context))->contextSetPluginListAutoSort) (context, value);
}

void
ccsSetProfileDefault (CCSContext * context, char *name)
{
    if (!name)
	name = "";

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    /* no action required if profile stays the same */
    if (cPrivate->profile && (strcmp (cPrivate->profile, name) == 0))
	return;

    if (cPrivate->profile)
	free (cPrivate->profile);

    cPrivate->profile = strdup (name);

    ccsDisableFileWatch (cPrivate->configWatchId);
    ccsWriteConfig (OptionProfile, cPrivate->profile);
    ccsEnableFileWatch (cPrivate->configWatchId);
}

void
ccsSetProfile (CCSContext *context, char *name)
{
    (*(GET_INTERFACE (CCSContextInterface, context))->contextSetProfile) (context, name);
}

void
ccsProcessEventsDefault (CCSContext * context, unsigned int flags)
{
    if (!context)
	return;

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    ccsCheckFileWatches ();

    if (cPrivate->backend)
	ccsBackendExecuteEvents ((CCSBackend *) cPrivate->backend, flags);
}

void
ccsProcessEvents (CCSContext *context, unsigned int flags)
{
    if (!context)
	return;

    (*(GET_INTERFACE (CCSContextInterface, context))->contextProcessEvents) (context, flags);
}

void
ccsReadSettingsDefault (CCSContext * context)
{
    if (!context)
	return;
    
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);
    
    if (!cPrivate->backend)
	return;

    if (!ccsDynamicBackendSupportsRead (cPrivate->backend))
	return;

    if (!ccsBackendReadInit ((CCSBackend *) cPrivate->backend, context))
	return;

    CCSPluginList pl = cPrivate->plugins;
    while (pl)
    {
	CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, pl->data);
	CCSSettingList sl = pPrivate->settings;

	while (sl)
	{
	    ccsBackendReadSetting ((CCSBackend *) cPrivate->backend, context, sl->data);
	    sl = sl->next;
	}

	pl = pl->next;
    }

    ccsBackendReadDone ((CCSBackend *) cPrivate->backend, context);
}

void
ccsReadSettings (CCSContext *context)
{
    if (!context)
	return;

    (*(GET_INTERFACE (CCSContextInterface, context))->contextReadSettings) (context);
}

void
ccsReadPluginSettingsDefault (CCSPlugin * plugin)
{
    if (!plugin || !ccsPluginGetContext (plugin))
	return;

    CCSContext *context = ccsPluginGetContext (plugin);
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    if (!cPrivate->backend)
	return;

    if (!ccsDynamicBackendSupportsRead (cPrivate->backend))
	return;

    if (!ccsBackendReadInit ((CCSBackend *) cPrivate->backend, ccsPluginGetContext (plugin)))
	return;

    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    CCSSettingList sl = pPrivate->settings;
    while (sl)
    {
	ccsBackendReadSetting ((CCSBackend *) cPrivate->backend, ccsPluginGetContext (plugin), sl->data);
	sl = sl->next;
    }

    ccsBackendReadDone ((CCSBackend *) cPrivate->backend, ccsPluginGetContext (plugin));
}

void
ccsReadPluginSettings (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginReadPluginSettings) (plugin);
}

void
ccsWriteSettingsDefault (CCSContext * context)
{
    if (!context)
	return;

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    if (!cPrivate->backend)
	return;

    if (!ccsDynamicBackendSupportsWrite (cPrivate->backend))
	return;

    if (!ccsBackendWriteInit ((CCSBackend *) cPrivate->backend, context))
	return;

    CCSPluginList pl = cPrivate->plugins;
    while (pl)
    {
	CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, pl->data);
	CCSSettingList sl = pPrivate->settings;

	while (sl)
	{
	    ccsBackendWriteSetting ((CCSBackend *) cPrivate->backend, context, sl->data);
	    sl = sl->next;
	}

	pl = pl->next;
    }

    ccsBackendWriteDone ((CCSBackend *) cPrivate->backend, context);

    cPrivate->changedSettings =
	ccsSettingListFree (cPrivate->changedSettings, FALSE);
}

void
ccsWriteSettings (CCSContext *context)
{
    if (!context)
	return;

    (*(GET_INTERFACE (CCSContextInterface, context))->contextWriteSettings) (context);
}

void
ccsWriteChangedSettingsDefault (CCSContext * context)
{
    if (!context)
	return;

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);
    
    if (!cPrivate->backend)
	return;

    if (!ccsDynamicBackendSupportsWrite (cPrivate->backend))
	return;

    if (!ccsBackendWriteInit ((CCSBackend *) cPrivate->backend, context))
	return;

    /* We must immediately steal the changed settings list
     * if we recurse into this function */
    CCSSettingList changedSettings = ccsContextStealChangedSettings (context);

    if (ccsSettingListLength (changedSettings))
    {
	CCSSettingList l = changedSettings;

	while (l)
	{
	    ccsBackendWriteSetting ((CCSBackend *) cPrivate->backend, context, l->data);
	    l = l->next;
	}
    }

    ccsBackendWriteDone ((CCSBackend *) cPrivate->backend,context);

    ccsSettingListFree (changedSettings, FALSE);
}

void
ccsWriteChangedSettings (CCSContext *context)
{
    if (!context)
	return;

    (*(GET_INTERFACE (CCSContextInterface, context))->contextWriteChangedSettings) (context);
}

Bool
ccsIsEqualColor (CCSSettingColorValue c1, CCSSettingColorValue c2)
{
    if (c1.color.red == c2.color.red     &&
	c1.color.green == c2.color.green &&
	c1.color.blue == c2.color.blue   &&
	c1.color.alpha == c2.color.alpha)
    {
	return TRUE;
    }

    return FALSE;
}

Bool
ccsIsEqualKey (CCSSettingKeyValue c1, CCSSettingKeyValue c2)
{
    if (c1.keysym == c2.keysym && c1.keyModMask == c2.keyModMask)
	return TRUE;

    return FALSE;
}

Bool
ccsIsEqualButton (CCSSettingButtonValue c1, CCSSettingButtonValue c2)
{
    if (c1.button == c2.button               &&
	c1.buttonModMask == c2.buttonModMask &&
	c1.edgeMask == c2.edgeMask)
	return TRUE;

    return FALSE;
}

Bool
ccsPluginSetActive (CCSPlugin * plugin, Bool value)
{
    if (!plugin)
	return FALSE;

    CCSContext *context = ccsPluginGetContext (plugin);

    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    pPrivate->active = value;

    if (cPrivate->pluginListAutoSort)
	ccsWriteAutoSortedPluginList (ccsPluginGetContext (plugin));

    return TRUE;
}

CCSPluginConflictList
ccsCanEnablePluginDefault (CCSContext * context, CCSPlugin * plugin)
{
    CCSPluginConflictList list = NULL;
    CCSPluginList pl, pl2;
    CCSStringList sl;

    /* look if the plugin to be loaded requires a plugin not present */
    sl = ccsPluginGetRequiresPlugins (plugin);

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    while (sl)
    {
	if (!ccsFindPlugin (context, ((CCSString *)sl->data)->value))
	{
	    CCSPluginConflict *conflict = calloc (1,
						  sizeof (CCSPluginConflict));
	    if (conflict)
	    {
		conflict->refCount = 1;
		conflict->value = strdup (((CCSString *)sl->data)->value);
		conflict->type = ConflictPluginError;
		conflict->plugins = NULL;
		list = ccsPluginConflictListAppend (list, conflict);
	    }
	}
	else if (!ccsPluginIsActive (context, ((CCSString *)sl->data)->value))
	{
	    /* we've not seen a matching plugin */
	    CCSPluginConflict *conflict = calloc (1,
						  sizeof (CCSPluginConflict));
	    if (conflict)
	    {
		conflict->refCount = 1;
		conflict->value = strdup (((CCSString *)sl->data)->value);
		conflict->type = ConflictRequiresPlugin;
		conflict->plugins =
		    ccsPluginListAppend (conflict->plugins,
			    		 ccsFindPlugin (context, ((CCSString *)sl->data)->value));
		list = ccsPluginConflictListAppend (list, conflict);
	    }
	}

	sl = sl->next;
    }

    /* look if the new plugin wants a non-present feature */
    sl = ccsPluginGetRequiresFeatures (plugin);

    while (sl)
    {
	pl = cPrivate->plugins;
	pl2 = NULL;

	while (pl)
	{
	    CCSStringList featureList = ccsPluginGetProvidesFeatures (pl->data);

	    while (featureList)
	    {
		if (strcmp (((CCSString *) sl->data)->value, ((CCSString *)featureList->data)->value) == 0)
		{
		    pl2 = ccsPluginListAppend (pl2, pl->data);
		    break;
		}
		featureList = featureList->next;
	    }

	    pl = pl->next;
	}

	pl = pl2;

	while (pl)
	{
	    if (ccsPluginIsActive (context, ccsPluginGetName (pl->data)))
	    {
		ccsPluginListFree (pl2, FALSE);
		break;
	    }
	    pl = pl->next;
	}

	if (!pl)
	{
	    /* no plugin provides that feature */
	    CCSPluginConflict *conflict = calloc (1,
						  sizeof (CCSPluginConflict));

	    if (conflict)
	    {
		conflict->refCount = 1;
		conflict->value = strdup (((CCSString *)sl->data)->value);
		conflict->type = ConflictRequiresFeature;
		conflict->plugins = pl2;

		list = ccsPluginConflictListAppend (list, conflict);
	    }
	}

	sl = sl->next;
    }

    /* look if another plugin provides the same feature */
    sl = ccsPluginGetProvidesFeatures (plugin);
    while (sl)
    {
	pl = cPrivate->plugins;
	CCSPluginConflict *conflict = NULL;

	while (pl)
	{
	    if (ccsPluginIsActive (context, ccsPluginGetName (pl->data)))
	    {
		CCSStringList featureList = ccsPluginGetProvidesFeatures (pl->data);

		while (featureList)
		{
		    if (strcmp (((CCSString *)sl->data)->value, ((CCSString *)featureList->data)->value) == 0)
		    {
			if (!conflict)
			{
			    conflict = calloc (1, sizeof (CCSPluginConflict));
			    if (conflict)
			    {
				conflict->refCount = 1;
				conflict->value = strdup (((CCSString *)sl->data)->value);
				conflict->type = ConflictFeature;
			    }
			}
			if (conflict)
			    conflict->plugins =
				ccsPluginListAppend (conflict->plugins,
						     pl->data);
		    }
		    featureList = featureList->next;
		}
	    }
	    pl = pl->next;
	}

	if (conflict)
	    list = ccsPluginConflictListAppend (list, conflict);

	sl = sl->next;
    }

    /* look if another plugin provides a conflicting feature*/
    sl = ccsPluginGetProvidesFeatures (plugin);
    while (sl)
    {
	pl = cPrivate->plugins;
	CCSPluginConflict *conflict = NULL;

	while (pl)
	{
	    if (ccsPluginIsActive (context, ccsPluginGetName (pl->data)))
	    {
		CCSStringList featureList = ccsPluginGetProvidesFeatures (pl->data);
		while (featureList)
		{
		    if (strcmp (((CCSString *)sl->data)->value, ((CCSString *)featureList->data)->value) == 0)
		    {
			if (!conflict)
			{
			    conflict = calloc (1, sizeof (CCSPluginConflict));
			    if (conflict)
			    {
				conflict->refCount = 1;
				conflict->value = strdup (((CCSString *)sl->data)->value);
				conflict->type = ConflictFeature;
			    }
			}
			if (conflict)
			    conflict->plugins =
				ccsPluginListAppend (conflict->plugins,
						     pl->data);
		    }
		    featureList = featureList->next;
		}
	    }
	    pl = pl->next;
	}

	if (conflict)
	    list = ccsPluginConflictListAppend (list, conflict);

	sl = sl->next;
    }

    /* look if the plugin to be loaded conflict with a loaded plugin  */
    sl = ccsPluginGetConflictPlugins (plugin);

    while (sl)
    {
	if (ccsPluginIsActive (context, ((CCSString *)sl->data)->value))
	{
	    CCSPluginConflict *conflict = calloc (1,
						  sizeof (CCSPluginConflict));
	    if (conflict)
	    {
		conflict->refCount = 1;
		conflict->value = strdup (((CCSString *)sl->data)->value);
		conflict->type = ConflictPlugin;
		conflict->plugins =
		    ccsPluginListAppend (conflict->plugins,
			    		 ccsFindPlugin (context, ((CCSString *)sl->data)->value));
		list = ccsPluginConflictListAppend (list, conflict);
	    }
	}

	sl = sl->next;
    }

    return list;
}

CCSPluginConflictList
ccsCanEnablePlugin (CCSContext *context, CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextCanEnablePlugin) (context, plugin);
}

CCSPluginConflictList
ccsCanDisablePluginDefault (CCSContext * context, CCSPlugin * plugin)
{
    CCSPluginConflictList list = NULL;
    CCSPluginConflict *conflict = NULL;
    CCSPluginList pl;
    CCSStringList sl;

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    /* look if the plugin to be unloaded is required by another plugin */
    pl = cPrivate->plugins;

    for (; pl; pl = pl->next)
    {
	CCSStringList pluginList;

	if (pl->data == plugin)
	    continue;

	if (!ccsPluginIsActive (context, ccsPluginGetName (pl->data)))
	    continue;

	pluginList = ccsPluginGetRequiresPlugins (pl->data);

	while (pluginList)
	{
	    if (strcmp (ccsPluginGetName (plugin), ((CCSString *)pluginList->data)->value) == 0)
	    {
		if (!conflict)
		{
		    conflict = calloc (1, sizeof (CCSPluginConflict));
		    conflict->refCount = 1;
		    if (conflict)
		    {
			conflict->value = strdup (ccsPluginGetName (plugin));
			conflict->type = ConflictPluginNeeded;
		    }
		}

		if (conflict)
		    conflict->plugins =
			ccsPluginListAppend (conflict->plugins, pl->data);
		break;
	    }
	    pluginList = pluginList->next;
	}
    }

    if (conflict)
    {
	list = ccsPluginConflictListAppend (list, conflict);
	conflict = NULL;
    }

    /* look if a feature provided is required by another plugin */
    sl = ccsPluginGetProvidesFeatures (plugin);
    while (sl)
    {
	pl = cPrivate->plugins;
	for (; pl; pl = pl->next)
	{
	    CCSStringList pluginList;

	    if (pl->data == plugin)
		continue;

	    if (!ccsPluginIsActive (context, ccsPluginGetName (pl->data)))
		continue;

	    pluginList = ccsPluginGetRequiresFeatures (pl->data);

	    while (pluginList)
	    {
		if (strcmp (((CCSString *)sl->data)->value, ((CCSString *)pluginList->data)->value) == 0)
		{
		    if (!conflict)
		    {
			conflict = calloc (1, sizeof (CCSPluginConflict));

			if (conflict)
			{
			    conflict->refCount = 1;
			    conflict->value = strdup (((CCSString *)sl->data)->value);
			    conflict->type = ConflictFeatureNeeded;
			}
		    }
		    if (conflict)
			conflict->plugins =
			    ccsPluginListAppend (conflict->plugins, pl->data);
		}
		pluginList = pluginList->next;
	    }
	    
	}
	if (conflict)
	    list = ccsPluginConflictListAppend (list, conflict);
	conflict = NULL;
	sl = sl->next;
    }

    return list;
}

CCSPluginConflictList
ccsCanDisablePlugin (CCSContext *context, CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextCanDisablePlugin) (context, plugin);
}

CCSStringList
ccsGetExistingProfilesDefault (CCSContext * context)
{
    if (!context)
	return NULL;
    
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);
    
    if (!cPrivate->backend)
	return NULL;

    if (ccsDynamicBackendSupportsProfiles (cPrivate->backend))
	return ccsBackendGetExistingProfiles ((CCSBackend *) cPrivate->backend, context);

    return NULL;
}

CCSStringList
ccsGetExistingProfiles (CCSContext *context)
{
    if (!context)
	return NULL;

    return (*(GET_INTERFACE (CCSContextInterface, context))->contextGetExistingProfiles) (context);
}

void
ccsDeleteProfileDefault (CCSContext * context, char *name)
{
    if (!context)
	return;
    
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);
    
    if (!cPrivate->backend)
	return;

    if (!ccsDynamicBackendSupportsProfiles (cPrivate->backend))
	return;

    /* never ever delete default profile */
    if (!name || !strlen (name))
	return;

    /* if the current profile should be deleted,
       switch to default profile first */
    if (strcmp (cPrivate->profile, name) == 0)
	ccsSetProfile (context, "");

    ccsBackendDeleteProfile ((CCSBackend *) cPrivate->backend, context, name);
}

void
ccsDeleteProfile (CCSContext *context, char *name)
{
    if (!context)
	return;

    (*(GET_INTERFACE (CCSContextInterface, context))->contextDeleteProfile) (context, name);
}

static void
addBackendInfo (CCSBackendInfoList * bl, char *file)
{
    void *dlhand = NULL;
    char *err = NULL;
    Bool found = FALSE;

    dlerror ();

    dlhand = dlopen (file, RTLD_LAZY | RTLD_LOCAL);
    err = dlerror ();
    if (err || !dlhand)
	return;

    BackendGetInfoProc getInfo = dlsym (dlhand, "getBackendInfo");
    if (!getInfo)
    {
	dlclose (dlhand);
	return;
    }

    CCSBackendInterface *vt = getInfo ();
    if (!vt)
    {
	dlclose (dlhand);
	return;
    }

    CCSBackendInfo *info = ccsCopyBackendInfoFromBackend (NULL, vt);

    if (!info)
    {
	dlclose (dlhand);
	return;
    }

    CCSBackendInfoList l = *bl;
    while (l)
    {
	if (!strcmp (l->data->name, info->name))
	{
	    found = TRUE;
	    break;
	}

	l = l->next;
    }

    if (found)
    {
	dlclose (dlhand);
	return;
    }

    *bl = ccsBackendInfoListAppend (*bl, info);
    dlclose (dlhand);
}

static int
backendNameFilter (const struct dirent *name)
{
    int length = strlen (name->d_name);

    if (length < 7)
	return 0;

    if (strncmp (name->d_name, "lib", 3) ||
	strncmp (name->d_name + length - 3, ".so", 3))
	return 0;

    return 1;
}

static void
getBackendInfoFromDir (CCSBackendInfoList * bl,
		       char *path,
		       const char *currentBackend)
{

    struct dirent **nameList;
    int nFile, i;

    if (!path)
	return;

    nFile = scandir (path, &nameList, backendNameFilter, NULL);
    if (nFile <= 0)
	return;

    for (i = 0; i < nFile; i++)
    {
	if (strncmp (currentBackend, &(nameList[i]->d_name[3]), strlen (currentBackend)) == 0)
	    continue;

	char file[1024];
	sprintf (file, "%s/%s", path, nameList[i]->d_name);
	addBackendInfo (bl, file);
	free (nameList[i]);
    }

    free (nameList);

}

CCSBackendInfoList
ccsGetExistingBackends (CCSContext *context)
{
    CCSBackendInfoList rv = NULL;
    char *home = getenv ("HOME");
    char *overrideBackend = getenv ("LIBCOMPIZCONFIG_BACKEND_PATH");
    char *backenddir;
    const char *currentBackend = ccsGetBackend (context);

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    const CCSBackendInterface *currentBackendInterface =
	    GET_INTERFACE (CCSBackendInterface, cPrivate->backend);

    rv = ccsBackendInfoListAppend (rv, ccsCopyBackendInfoFromBackend ((CCSBackend *) cPrivate->backend,
								      currentBackendInterface));

    if (overrideBackend && strlen (overrideBackend))
    {
	if (asprintf (&backenddir, "%s",
		      overrideBackend) == -1)
	    backenddir = NULL;

	if (backenddir)
	{
	    getBackendInfoFromDir (&rv, backenddir, currentBackend);
	    free (backenddir);
	}
    }
    else
    {
	if (home && strlen (home))
	{
	    if (asprintf (&backenddir, "%s/.compizconfig/backends", home) == -1)
		backenddir = NULL;

	    if (backenddir)
	    {
		getBackendInfoFromDir (&rv, backenddir, currentBackend);
		free (backenddir);
	    }
	}

	if (asprintf (&backenddir, "%s/compizconfig/backends", LIBDIR) == -1)
	    backenddir = NULL;

	if (backenddir)
	{
	    getBackendInfoFromDir (&rv, backenddir, currentBackend);
	    free (backenddir);
	}
    }

    return rv;
}

Bool
ccsExportToFileDefault (CCSContext *context,
			const char *fileName,
			Bool       skipDefaults)
{
    IniDictionary *exportFile;
    CCSPluginList p;
    CCSSettingList s;
    CCSPlugin *plugin;
    CCSSetting *setting;
    char *keyName;

    exportFile = ccsIniNew ();
    if (!exportFile)
	return FALSE;

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    for (p = cPrivate->plugins; p; p = p->next)
    {
	plugin = p->data;
	CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

	if (!pPrivate->loaded)
	    ccsLoadPluginSettings (plugin);

	for (s = pPrivate->settings; s; s = s->next)
	{
	    setting = s->data;

	    if (skipDefaults && ccsSettingGetIsDefault (setting))
		continue;

	    if (asprintf (&keyName, "s%d_%s",
			  cPrivate->screenNum, ccsSettingGetName (setting)) == -1)
		return FALSE;

	    switch (ccsSettingGetType (setting))
 	    {
 	    case TypeBool:
		ccsIniSetBool (exportFile, ccsPluginGetName (plugin), keyName,
			       ccsSettingGetValue (setting)->value.asBool);
 		break;
 	    case TypeInt:
		ccsIniSetInt (exportFile, ccsPluginGetName (plugin), keyName,
			      ccsSettingGetValue (setting)->value.asInt);
 		break;
 	    case TypeFloat:
		ccsIniSetFloat (exportFile, ccsPluginGetName (plugin), keyName,
				ccsSettingGetValue (setting)->value.asFloat);
 		break;
 	    case TypeString:
		ccsIniSetString (exportFile, ccsPluginGetName (plugin), keyName,
				 ccsSettingGetValue (setting)->value.asString);
 		break;
 	    case TypeKey:
		ccsIniSetKey (exportFile, ccsPluginGetName (plugin), keyName,
			      ccsSettingGetValue (setting)->value.asKey);
 		break;
 	    case TypeButton:
		ccsIniSetButton (exportFile, ccsPluginGetName (plugin), keyName,
				 ccsSettingGetValue (setting)->value.asButton);
 		break;
 	    case TypeEdge:
		ccsIniSetEdge (exportFile, ccsPluginGetName (plugin), keyName,
			       ccsSettingGetValue (setting)->value.asEdge);
 		break;
 	    case TypeBell:
		ccsIniSetBell (exportFile, ccsPluginGetName (plugin), keyName,
			       ccsSettingGetValue (setting)->value.asBell);
 		break;
 	    case TypeColor:
		ccsIniSetColor (exportFile, ccsPluginGetName (plugin), keyName,
				ccsSettingGetValue (setting)->value.asColor);
 		break;
 	    case TypeMatch:
		ccsIniSetString (exportFile, ccsPluginGetName (plugin), keyName,
				 ccsSettingGetValue (setting)->value.asMatch);
 		break;
 	    case TypeList:
		ccsIniSetList (exportFile, ccsPluginGetName (plugin), keyName,
			       ccsSettingGetValue (setting)->value.asList,
			       ccsSettingGetInfo (setting)->forList.listType);
 		break;
 	    default:
 		break;
	    }
	    free (keyName);
	}
    }

    ccsIniSave (exportFile, fileName);
    ccsIniClose (exportFile);

    return TRUE;
}

Bool
ccsExportToFile (CCSContext *context, const char *fileName, Bool skipDefaults)
{
    if (!context)
	return FALSE;

    return (*(GET_INTERFACE (CCSContextInterface, context))->contextExportToFile) (context, fileName, skipDefaults);
}

/* + with a value will attempt to append or overwrite that value if there is no -
 * - with a value will attempt to clear any value set to that to the default
 * if there is no +
 * - with a value and + with a value will replace one with the other
 * - without a value and + with a value will ensure that + overwrites the setting
 */

static Bool
ccsProcessSettingPlus (IniDictionary	   *dict,
		       CCSContext 	   *context,
		       CCSSettingsUpgrade  *upgrade,
		       CCSSetting	   *setting)
{
    char         *keyName = NULL;
    char         *sectionName = strdup (ccsPluginGetName (ccsSettingGetParent (setting)));
    char         *iniValue = NULL;

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    if (asprintf (&keyName, "+s%d_%s", cPrivate->screenNum, ccsSettingGetName (setting)) == -1)
	return FALSE;

    if (ccsIniGetString (dict, sectionName, keyName, &iniValue))
    {
	CCSSetting *newSetting = malloc (sizeof (CCSSetting));

	if (!newSetting)
	    return FALSE;

	ccsObjectInit (newSetting, &ccsDefaultObjectAllocator);

	copySetting (setting, newSetting);

	switch (ccsSettingGetType (newSetting))
	{
	    case TypeInt:
	    {
		int value;
		ccsIniParseInt (iniValue, &value);

		ccsSetInt (newSetting, value, FALSE);
		break;
	    }
	    case TypeBool:
	    {
		Bool value;
		ccsIniParseBool (iniValue, &value);

		ccsSetBool (newSetting, value, FALSE);
		break;
	    }
	    case TypeFloat:
	    {
		float value;
		ccsIniParseFloat (iniValue, &value);

		ccsSetFloat (newSetting, value, FALSE);
		break;
	    }
	    case TypeString:
	    {
		char *value;
		ccsIniParseString (iniValue, &value);

		ccsSetString (newSetting, value, FALSE);
		free (value);
		break;
	    }
	    case TypeColor:
	    {
		CCSSettingColorValue value;
		ccsIniParseColor (iniValue, &value);

		ccsSetColor (newSetting, value, FALSE);
		break;
	    }
	    case TypeKey:
	    {
		CCSSettingKeyValue value;
		ccsIniParseKey (iniValue, &value);

		ccsSetKey (newSetting, value, FALSE);
		break;
	    }
	    case TypeButton:
	    {
		CCSSettingButtonValue value;
		ccsIniParseButton (iniValue, &value);

		ccsSetButton (newSetting, value, FALSE);
		break;
	    }
	    case TypeEdge:
	    {
		unsigned int	value;
		ccsIniParseEdge (iniValue, &value);

		ccsSetEdge (newSetting, value, FALSE);
		break;
	    }
	    case TypeBell:
	    {
		Bool value;
		ccsIniParseBool (iniValue, &value);

		ccsSetBell (newSetting, value, FALSE);
		break;
	    }
	    case TypeMatch:
	    {
		char  *value;
		ccsIniParseString (iniValue, &value);

		ccsSetMatch (newSetting, value, FALSE);
		free (value);
		break;
	    }
	    case TypeList:
	    {
		CCSSettingValueList value;
		ccsIniParseList (iniValue, &value, newSetting);

		ccsSetList (newSetting, value, FALSE);
		ccsSettingValueListFree (value, TRUE);
		break;
	    }
	    case TypeAction:
	    default:
		/* FIXME: cleanup */
		return FALSE;
	}

        CCSSettingList listIter = upgrade->clearValueSettings;

        while (listIter)
	{
	    CCSSetting *s = (CCSSetting *) listIter->data;

	    if (strcmp (ccsSettingGetName (s), ccsSettingGetName (newSetting)) == 0)
	    {
		upgrade->replaceToValueSettings = ccsSettingListAppend (upgrade->replaceToValueSettings, (void *) newSetting);
		upgrade->replaceFromValueSettings = ccsSettingListAppend (upgrade->replaceFromValueSettings, (void *) s);
		upgrade->clearValueSettings = ccsSettingListRemove (upgrade->clearValueSettings, (void *) s, FALSE);
		break;
	    }

	    listIter = listIter->next;
	}

	if (!listIter)
	    upgrade->addValueSettings = ccsSettingListAppend (upgrade->addValueSettings, (void *) newSetting);
	
	free (keyName);
	free (sectionName);
	free (iniValue);
	
	return TRUE;
    }
    
    free (keyName);
    free (sectionName);
    
    return FALSE;
}

static Bool
ccsProcessSettingMinus (IniDictionary      *dict,
			CCSContext 	   *context,
			CCSSettingsUpgrade *upgrade,
			CCSSetting	   *setting)
{
    char         *keyName = NULL;
    char         *sectionName = strdup (ccsPluginGetName (ccsSettingGetParent (setting)));
    char         *iniValue = NULL;

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    if (asprintf (&keyName, "-s%d_%s", cPrivate->screenNum, ccsSettingGetName (setting)) == -1)
	return FALSE;

    if (ccsIniGetString (dict, sectionName, keyName, &iniValue))
    {
	CCSSetting *newSetting = malloc (sizeof (CCSSetting));

	if (!newSetting)
	    return FALSE;

	ccsObjectInit (newSetting, &ccsDefaultObjectAllocator);

	copySetting (setting, newSetting);

	switch (ccsSettingGetType (newSetting))
	{
	    case TypeInt:
	    {
		int value;
		ccsIniParseInt (iniValue, &value);

		ccsSetInt (newSetting, value, FALSE);
		break;
	    }
	    case TypeBool:
	    {
		Bool value;
		ccsIniParseBool (iniValue, &value);

		ccsSetBool (newSetting, value, FALSE);
		break;
	    }
	    case TypeFloat:
	    {
		float value;
		ccsIniParseFloat (iniValue, &value);

		ccsSetFloat (newSetting, value, FALSE);
		break;
	    }
	    case TypeString:
	    {
		char *value;
		ccsIniParseString (iniValue, &value);

		ccsSetString (newSetting, value, FALSE);
		free (value);
		break;
	    }
	    case TypeColor:
	    {
		CCSSettingColorValue value;
		ccsIniParseColor (iniValue, &value);

		ccsSetColor (newSetting, value, FALSE);
		break;
	    }
	    case TypeKey:
	    {
		CCSSettingKeyValue value;
		ccsIniParseKey (iniValue, &value);

		ccsSetKey (newSetting, value, FALSE);
		break;
	    }
	    case TypeButton:
	    {
		CCSSettingButtonValue value;
		ccsIniParseButton (iniValue, &value);

		ccsSetButton (newSetting, value, FALSE);
		break;
	    }
	    case TypeEdge:
	    {
		unsigned int	value;
		ccsIniParseEdge (iniValue, &value);

		ccsSetEdge (newSetting, value, FALSE);
		break;
	    }
	    case TypeBell:
	    {
		Bool value;
		ccsIniParseBool (iniValue, &value);

		ccsSetBell (newSetting, value, FALSE);
		break;
	    }
	    case TypeMatch:
	    {
		char  *value;
		ccsIniParseString (iniValue, &value);

		ccsSetMatch (newSetting, value, FALSE);
		free (value);
		break;
	    }
	    case TypeList:
	    {
		CCSSettingValueList value;
		ccsIniParseList (iniValue, &value, newSetting);

		ccsSetList (newSetting, value, FALSE);
		ccsSettingValueListFree (value, TRUE);
		break;
	    }
	    case TypeAction:
	    default:
		/* FIXME: cleanup */
		return FALSE;
	}

        CCSSettingList listIter = upgrade->addValueSettings;

        while (listIter)
	{
	    CCSSetting *s = (CCSSetting *) listIter->data;

	    if (strcmp (ccsSettingGetName (s), ccsSettingGetName (newSetting)) == 0)
	    {
		upgrade->replaceFromValueSettings = ccsSettingListAppend (upgrade->replaceFromValueSettings, (void *) newSetting);
		upgrade->replaceToValueSettings = ccsSettingListAppend (upgrade->replaceToValueSettings, (void *) s);
		upgrade->addValueSettings = ccsSettingListRemove (upgrade->addValueSettings, (void *) s, FALSE);
		break;
	    }

	    listIter = listIter->next;
	}

	if (!listIter)
	    upgrade->clearValueSettings = ccsSettingListAppend (upgrade->clearValueSettings, (void *) newSetting);
	
	free (keyName);
	free (sectionName);
	free (iniValue);
	
	return TRUE;
    }
    
    free (keyName);
    free (sectionName);

    return FALSE;
}

void
ccsCollectSettingsToUpgrade (CCSContext         *context,
			     IniDictionary      *dict,
			     CCSSettingsUpgrade *upgrade)
{
    CCSPluginList      pl = ccsContextGetPlugins (context);

    while (pl)
    {
	CCSPlugin	   *plugin = (CCSPlugin *) pl->data;
	CCSSettingList     sl = ccsGetPluginSettings (plugin);

	while (sl)
	{
	    CCSSetting   *setting = sl->data;

	    ccsProcessSettingMinus (dict, context, upgrade, setting);
	    ccsProcessSettingPlus (dict, context, upgrade, setting);

	    sl = sl->next;
	}

	pl = pl->next;
    }
}

Bool
ccsProcessUpgrade (CCSContext *context,
		   CCSSettingsUpgrade *upgrade)
{
    IniDictionary      *dict = ccsIniOpen (upgrade->file);

    ccsSetProfile (context, upgrade->profile);

    ccsCollectSettingsToUpgrade (context, dict, upgrade);
    ccsUpgradeClearValues (upgrade->clearValueSettings);
    ccsUpgradeAddValues (upgrade->addValueSettings);
    ccsUpgradeReplaceValues (upgrade->replaceFromValueSettings,
			     upgrade->replaceToValueSettings);
    
    upgrade->clearValueSettings = ccsSettingListFree (upgrade->clearValueSettings, TRUE);
    upgrade->addValueSettings = ccsSettingListFree (upgrade->addValueSettings, TRUE);
    upgrade->replaceFromValueSettings = ccsSettingListFree (upgrade->replaceFromValueSettings, TRUE);
    upgrade->replaceToValueSettings = ccsSettingListFree (upgrade->replaceToValueSettings, TRUE);

    ccsIniClose (dict);

    return TRUE;
}

static int
upgradeNameFilter (const struct dirent *name)
{
    return ccsUpgradeNameFilter (name->d_name);
}

void
ccsFreeUpgrade (CCSSettingsUpgrade *upgrade)
{
    if (upgrade->profile)
	free (upgrade->profile);

    if (upgrade->domain)
	free (upgrade->domain);

    if (upgrade->file)
	free (upgrade->file);

    free (upgrade);
}

/*
 * Process a filename into the properties
 * for a settings upgrade
 * eg
 *
 * org.freedesktop.compiz.Default.01.upgrade
 *
 * gives us:
 * domain: org.freedesktop.compiz
 * number: 1
 * profile: Default
 *
 */
CCSSettingsUpgrade *
ccsSettingsUpgradeNew (const char *path, const char *name)
{
    CCSSettingsUpgrade *upgrade = calloc (1, sizeof (CCSSettingsUpgrade));
    char *upgradeName = strdup (name);
    unsigned int fnlen = strlen (path) + strlen (name) + 1;

    upgrade->file = calloc (fnlen + 1, sizeof (char));
    sprintf (upgrade->file, "%s/%s", path, name);

    upgradeName = strdup (name);

    if (!ccsUpgradeGetDomainNumAndProfile (upgradeName,
					   &upgrade->domain,
					   &upgrade->num,
					   &upgrade->profile))
    {
	ccsFreeUpgrade (upgrade);
	upgrade = NULL;
    }

    free (upgradeName);

    return upgrade;
}

static CCSTextFile *
ccsUnixOpenDoneSettingsUpgradeFile (const char *path)
{
    return ccsUnixTextFileNew (path,
			       ReadWriteCreate,
			       &ccsDefaultObjectAllocator);
}



static CCSTextFile *
ccsGetDoneSettingsUpgradeFile (const char *home)
{
    char		   *dupath = NULL;
    CCSTextFile		   *completedUpgrades = NULL;

    if (asprintf (&dupath, "%s/.config/compiz-1/compizconfig/done_upgrades", home) == -1)
	return NULL;

    completedUpgrades = ccsUnixOpenDoneSettingsUpgradeFile (dupath);
    free (dupath);

    return completedUpgrades;
}

static char *
ccsReadCompletedUpgradesIntoString (CCSTextFile *completedUpgrades)
{
    return ccsTextFileReadFromStart (completedUpgrades);
}

static unsigned int
ccsGetUpgradeFilesForProcessing (const char *upgradePath,
				 struct dirent ***passedNameList)
{
    struct dirent **nameList = NULL;
    unsigned int nFile = scandir (upgradePath, &nameList, upgradeNameFilter, alphasort);

    if (!nFile)
	return 0;

    *passedNameList = nameList;

    return nFile;
}

static Bool
ccsShouldSkipUpgrade (const char *upgrade,
		      const char *skipBuffer)
{
    char		   *matched = strstr (skipBuffer, upgrade);

    if (matched != NULL)
    {
	ccsDebug ("Skipping upgrade %s", upgrade);
	return TRUE;
    }

    return FALSE;
}

static void
ccsProcessUpgradeOnce (CCSContext	  *context,
		       CCSSettingsUpgrade *upgrade,
		       const char	  *upgradeName,
		       CCSTextFile	  *completedUpgrades)
{
    ccsDebug ("Processing upgrade %s\n profile: %s\n number: %i\n domain: %s",
	      upgradeName,
	      upgrade->profile,
	      upgrade->num,
	      upgrade->domain);

    ccsProcessUpgrade (context, upgrade);
    ccsWriteChangedSettings (context);
    ccsWriteAutoSortedPluginList (context);
    ccsDebug ("Completed upgrade %s", upgradeName);

    ccsTextFileAppendString (completedUpgrades, upgradeName);
    ccsFreeUpgrade (upgrade);
}

static const char * CCS_UPGRADE_PATH = DATADIR "/compizconfig/upgrades";

static void
ccsApplyUnappliedUpgrades (CCSContext    *context,
			   struct dirent **nameList,
			   unsigned int  nFile,
			   const char	 *completedUpradesContents,
			   CCSTextFile   *completedUpgrades)
{
    int			   i = 0;
    const char	  	   *path = CCS_UPGRADE_PATH;

    for (i = 0; i < nFile; i++)
    {
        CCSSettingsUpgrade *upgrade = NULL;
	const char *upgradeName = nameList[i]->d_name;

	if (ccsShouldSkipUpgrade (upgradeName,
				  completedUpradesContents))
	    continue;

	upgrade = ccsSettingsUpgradeNew (path, upgradeName);

	ccsProcessUpgradeOnce (context, upgrade, upgradeName, completedUpgrades);

	free (nameList[i]);
    }
}

Bool
ccsCheckForSettingsUpgradeDefault (CCSContext *context)
{
    struct dirent 	   **nameList = NULL;
    int 	  	   nFile;
    const char	  	   *path = CCS_UPGRADE_PATH;
    CCSTextFile		   *completedUpgrades;
    char		   *cuBuffer = NULL;
    char		   *home = getenv ("HOME");

    if (!home)
	return FALSE;

    completedUpgrades = ccsGetDoneSettingsUpgradeFile (home);

    if (!completedUpgrades)
	return FALSE;

    cuBuffer = ccsReadCompletedUpgradesIntoString (completedUpgrades);

    if (!cuBuffer)
    {
	ccsTextFileUnref (completedUpgrades);
	ccsWarning ("Error opening done_upgrades");
	return FALSE;
    }

    nFile = ccsGetUpgradeFilesForProcessing (path, &nameList);

    if (!nFile || !nameList)
    {
	free (cuBuffer);
	ccsTextFileUnref (completedUpgrades);
	return FALSE;
    }

    ccsApplyUnappliedUpgrades (context, nameList, nFile, cuBuffer, completedUpgrades);

    ccsTextFileUnref (completedUpgrades);
    free (cuBuffer);

    if (nameList != NULL)
	free (nameList);

    return TRUE;
}

Bool
ccsImportFromFileDefault (CCSContext *context,
			  const char *fileName,
			  Bool       overwriteNonDefault)
{
    IniDictionary *importFile;
    CCSPluginList p;
    CCSSettingList s;
    CCSPlugin *plugin;
    CCSSetting *setting;
    char *keyName;
    FILE *fp;

    /* check if the file exists first */
    fp = fopen (fileName, "r");
    if (!fp)
	return FALSE;
    fclose (fp);

    importFile = iniparser_new ((char *) fileName);
    if (!importFile)
	return FALSE;

    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    for (p = cPrivate->plugins; p; p = p->next)
    {
	plugin = p->data;
	CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

	if (!pPrivate->loaded)
	    ccsLoadPluginSettings (plugin);

	for (s = pPrivate->settings; s; s = s->next)
	{
	    setting = s->data;
	    if (!ccsSettingGetIsDefault (setting) && !overwriteNonDefault)
		continue;

	    if (asprintf (&keyName, "s%d_%s",
			  cPrivate->screenNum, ccsSettingGetName (setting)) == -1)
		return FALSE;

	    switch (ccsSettingGetType (setting))
	    {
	    case TypeBool:
		{
		    Bool value;

		    if (ccsIniGetBool (importFile, ccsPluginGetName (plugin),
 				       keyName, &value))
		    {
			ccsSetBool (setting, value, TRUE);
		    }
		}
		break;
	    case TypeInt:
		{
		    int value;

		    if (ccsIniGetInt (importFile, ccsPluginGetName (plugin),
				      keyName, &value))
			ccsSetInt (setting, value, TRUE);
		}
		break;
	    case TypeFloat:
		{
		    float value;

		    if (ccsIniGetFloat (importFile, ccsPluginGetName (plugin),
					keyName, &value))
			ccsSetFloat (setting, value, TRUE);
		}
		break;
	    case TypeString:
		{
		    char *value;

		    if (ccsIniGetString (importFile, ccsPluginGetName (plugin),
					 keyName, &value))
		    {
		    	ccsSetString (setting, value, TRUE);
		    	free (value);
		    }
		}
		break;
	    case TypeKey:
		{
		    CCSSettingKeyValue value;

		    if (ccsIniGetKey (importFile, ccsPluginGetName (plugin),
				      keyName, &value))
			ccsSetKey (setting, value, TRUE);
		}
		break;
	    case TypeButton:
		{
		    CCSSettingButtonValue value;

		    if (ccsIniGetButton (importFile, ccsPluginGetName (plugin),
					 keyName, &value))
			ccsSetButton (setting, value, TRUE);
		}
		break;
	    case TypeEdge:
		{
		    unsigned int value;

		    if (ccsIniGetEdge (importFile, ccsPluginGetName (plugin),
				       keyName, &value))
			ccsSetEdge (setting, value, TRUE);
		}
		break;
	    case TypeBell:
		{
		    Bool value;

		    if (ccsIniGetBell (importFile, ccsPluginGetName (plugin),
				       keyName, &value))
			ccsSetBell (setting, value, TRUE);
		}
		break;
	    case TypeColor:
		{
		    CCSSettingColorValue value;

		    if (ccsIniGetColor (importFile, ccsPluginGetName (plugin),
					keyName, &value))
			ccsSetColor (setting, value, TRUE);
		}
		break;
	    case TypeMatch:
		{
		    char *value;
		    if (ccsIniGetString (importFile, ccsPluginGetName (plugin),
					 keyName, &value))
		    {
		    	ccsSetMatch (setting, value, TRUE);
		    	free (value);
		    }
		}
		break;
	    case TypeList:
		{
		    CCSSettingValueList value;
		    if (ccsIniGetList (importFile, ccsPluginGetName (plugin),
				       keyName, &value, setting))
		    {
			ccsSetList (setting, value, TRUE);
			ccsSettingValueListFree (value, TRUE);
		    }
		}
		break;
	    default:
		break;
	    }

	    free (keyName);
	}
    }

    ccsIniClose (importFile);

    return TRUE;
}

Bool
ccsCheckForSettingsUpgrade (CCSContext *context)
{
    return (*(GET_INTERFACE (CCSContextInterface, context))->contextCheckForSettingsUpgrade) (context);
}

Bool
ccsImportFromFile (CCSContext *context, const char *fileName, Bool overwriteNonDefault)
{
    if (!context)
	return FALSE;

    return (*(GET_INTERFACE (CCSContextInterface, context))->contextImportFromFile) (context, fileName, overwriteNonDefault);
}

const char *
ccsPluginGetNameDefault (CCSPlugin *plugin)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    return pPrivate->name;
}

const char * ccsPluginGetShortDescDefault (CCSPlugin *plugin)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    return pPrivate->shortDesc;
}

const char * ccsPluginGetLongDescDefault (CCSPlugin *plugin)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    return pPrivate->longDesc;
}

const char * ccsPluginGetHintsDefault (CCSPlugin *plugin)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    return pPrivate->hints;
}

const char * ccsPluginGetCategoryDefault (CCSPlugin *plugin)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    return pPrivate->category;
}

CCSStringList ccsPluginGetLoadAfterDefault (CCSPlugin *plugin)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    return pPrivate->loadAfter;
}

CCSStringList ccsPluginGetLoadBeforeDefault (CCSPlugin *plugin)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    return pPrivate->loadBefore;
}

CCSStringList ccsPluginGetRequiresPluginsDefault (CCSPlugin *plugin)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    return pPrivate->requiresPlugin;
}

CCSStringList ccsPluginGetConflictPluginsDefault (CCSPlugin *plugin)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    return pPrivate->conflictPlugin;
}

CCSStringList ccsPluginGetProvidesFeaturesDefault (CCSPlugin *plugin)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    return pPrivate->providesFeature;
}

void * ccsPluginGetProvidesFeaturesBindable (CCSPlugin *plugin)
{
    return (void *) ccsPluginGetProvidesFeatures (plugin);
}

CCSStringList ccsPluginGetRequiresFeaturesDefault (CCSPlugin *plugin)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    return pPrivate->requiresFeature;
}

void * ccsPluginGetPrivatePtrDefault (CCSPlugin *plugin)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    return pPrivate->privatePtr;
}

void ccsPluginSetPrivatePtrDefault (CCSPlugin *plugin, void *ptr)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    pPrivate->privatePtr = ptr;
}

CCSContext * ccsPluginGetContextDefault (CCSPlugin *plugin)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    return pPrivate->context;
}

/* CCSPlugin accessor functions */
const char * ccsPluginGetName (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginGetName) (plugin);
}

const char * ccsPluginGetShortDesc (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginGetShortDesc) (plugin);
}

const char * ccsPluginGetLongDesc (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginGetLongDesc) (plugin);
}

const char * ccsPluginGetHints (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginGetHints) (plugin);
}

const char * ccsPluginGetCategory (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginGetCategory) (plugin);
}

CCSStringList ccsPluginGetLoadAfter (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginGetLoadAfter) (plugin);
}

CCSStringList ccsPluginGetLoadBefore (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginGetLoadBefore) (plugin);
}

CCSStringList ccsPluginGetRequiresPlugins (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginGetRequiresPlugins) (plugin);
}

CCSStringList ccsPluginGetConflictPlugins (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginGetConflictPlugins) (plugin);
}

CCSStringList ccsPluginGetProvidesFeatures (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginGetProvidesFeatures) (plugin);
}

CCSStringList ccsPluginGetRequiresFeatures (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginGetRequiresFeatures) (plugin);
}

void * ccsPluginGetPrivatePtr (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginGetPrivatePtr) (plugin);
}

void ccsPluginSetPrivatePtr (CCSPlugin *plugin, void *ptr)
{
    (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginSetPrivatePtr) (plugin, ptr);
}

CCSContext * ccsPluginGetContext (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginGetContext) (plugin);
}

CCSSettingList ccsGetPluginSettingsDefault (CCSPlugin *plugin)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    if (!pPrivate->loaded)
	ccsLoadPluginSettings (plugin);

    return pPrivate->settings;
}

CCSSettingList ccsGetPluginSettings (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginGetPluginSettings) (plugin);
}

CCSGroupList ccsGetPluginGroupsDefault (CCSPlugin *plugin)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    if (!pPrivate->loaded)
	ccsLoadPluginSettings (plugin);

    return pPrivate->groups;
}

CCSGroupList ccsGetPluginGroups (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginGetPluginGroups) (plugin);
}

const char * ccsSettingGetName (CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetName) (setting);
}

const char * ccsSettingGetShortDesc (CCSSetting *setting)

{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetShortDesc) (setting);
}

const char * ccsSettingGetLongDesc (CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetLongDesc) (setting);
}

CCSSettingType ccsSettingGetType (CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetType) (setting);
}

CCSSettingInfo * ccsSettingGetInfo (CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetInfo) (setting);
}

const char * ccsSettingGetGroup (CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetGroup) (setting);
}

const char * ccsSettingGetSubGroup (CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetSubGroup) (setting);
}

const char * ccsSettingGetHints (CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetHints) (setting);
}

CCSSettingValue * ccsSettingGetDefaultValue (CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetDefaultValue) (setting);
}

CCSSettingValue *ccsSettingGetValue (CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetValue) (setting);
}

Bool ccsSettingGetIsDefault (CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetIsDefault) (setting);
}

CCSPlugin * ccsSettingGetParent (CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetParent) (setting);
}

void * ccsSettingGetPrivatePtr (CCSSetting *setting)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingGetPrivatePtr) (setting);
}

void ccsSettingSetPrivatePtr (CCSSetting *setting, void *ptr)
{
    return (*(GET_INTERFACE (CCSSettingInterface, setting))->settingSetPrivatePtr) (setting, ptr);
}

Bool ccsSettingGetIsIntegratedDefault (CCSSetting *setting)
{
    if (!setting)
	return FALSE;

    CCSPlugin *plugin = ccsSettingGetParent (setting);
    CCSContext *context = ccsPluginGetContext (plugin);
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    if (!cPrivate->backend)
	return FALSE;

    if (ccsDynamicBackendSupportsIntegration (cPrivate->backend))
	return ccsBackendGetSettingIsIntegrated ((CCSBackend *) cPrivate->backend, setting);

    return FALSE;
}

Bool ccsSettingGetIsReadOnlyDefault (CCSSetting *setting)
{
    if (!setting)
	return FALSE;

    CCSPlugin *plugin = ccsSettingGetParent (setting);
    CCSContext *context = ccsPluginGetContext (plugin);
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    if (!cPrivate->backend)
	return FALSE;

    return ccsBackendGetSettingIsReadOnly ((CCSBackend *) cPrivate->backend, setting);
}

Bool ccsSettingGetIsReadableByBackendDefault (CCSSetting *setting)
{
    static const CCSSettingType readableSettingTypes[] =
    {
	TypeBool,
	TypeInt,
	TypeFloat,
	TypeString,
	TypeColor,
	TypeKey,
	TypeButton,
	TypeEdge,
	TypeBell,
	TypeMatch,
	TypeList
    };
    static const unsigned int readableSettingTypesNum = sizeof (readableSettingTypes) / sizeof (readableSettingTypes[0]);
    int i = 0;
    Bool isReadableType = FALSE;
    CCSSettingType type;

    CCSPlugin *plugin = ccsSettingGetParent (setting);
    CCSContext *context = ccsPluginGetContext (plugin);
    CCSContextPrivate *cPrivate = GET_PRIVATE (CCSContextPrivate, context);

    if (!cPrivate->backend)
	return FALSE;

    type = ccsSettingGetType (setting);

    /* It is impossible for certain settings to have a schema,
     * such as actions and read only settings, so in that case
     * just return FALSE since compizconfig doesn't expect us
     * to read them anyways */
    for (i = 0; i < readableSettingTypesNum; ++i)
    {
	if (readableSettingTypes[i] == type)
	{
	    isReadableType = TRUE;
	    break;
	}
    }

    if (isReadableType &&
	!ccsSettingIsReadOnly (setting))
    {
	return TRUE;
    }

    return FALSE;
}

/* Interface for CCSSetting */
const char *
ccsSettingGetNameDefault (CCSSetting *setting)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    return sPrivate->name;
}

const char * ccsSettingGetShortDescDefault (CCSSetting *setting)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    return sPrivate->shortDesc;
}

const char * ccsSettingGetLongDescDefault (CCSSetting *setting)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    return sPrivate->longDesc;
}

CCSSettingType ccsSettingGetTypeDefault (CCSSetting *setting)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    return sPrivate->type;
}

CCSSettingInfo * ccsSettingGetInfoDefault (CCSSetting *setting)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    return &sPrivate->info;
}

const char * ccsSettingGetGroupDefault (CCSSetting *setting)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    return sPrivate->group;
}

const char * ccsSettingGetSubGroupDefault (CCSSetting *setting)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    return sPrivate->subGroup;
}

const char * ccsSettingGetHintsDefault (CCSSetting *setting)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    return sPrivate->hints;
}

CCSSettingValue * ccsSettingGetDefaultValueDefault (CCSSetting *setting)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    return &sPrivate->defaultValue;
}

CCSSettingValue * ccsSettingGetValueDefault (CCSSetting *setting)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    return sPrivate->value;
}

Bool ccsSettingGetIsDefaultDefault (CCSSetting *setting)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    return sPrivate->isDefault;
}

CCSPlugin * ccsSettingGetParentDefault (CCSSetting *setting)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    return sPrivate->parent;
}

void * ccsSettingGetPrivatePtrDefault (CCSSetting *setting)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    return sPrivate->privatePtr;
}

void ccsSettingSetPrivatePtrDefault (CCSSetting *setting, void *ptr)
{
    CCSSettingPrivate *sPrivate = GET_PRIVATE (CCSSettingPrivate, setting);

    sPrivate->privatePtr = ptr;
}

CCSStrExtensionList ccsGetPluginStrExtensionsDefault (CCSPlugin *plugin)
{
    CCSPluginPrivate *pPrivate = GET_PRIVATE (CCSPluginPrivate, plugin);

    if (!pPrivate->loaded)
	ccsLoadPluginSettings (plugin);

    return pPrivate->stringExtensions;
}

CCSStrExtensionList ccsGetPluginStrExtensions (CCSPlugin *plugin)
{
    return (*(GET_INTERFACE (CCSPluginInterface, plugin))->pluginGetPluginStrExtensions) (plugin);
}

CCSSettingValue * ccsIntegratedSettingReadValue (CCSIntegratedSetting *setting, CCSSettingType type)
{
    return (*(GET_INTERFACE (CCSIntegratedSettingInterface, setting))->readValue) (setting, type);
}

void ccsIntegratedSettingWriteValue (CCSIntegratedSetting *setting, CCSSettingValue *value, CCSSettingType type)
{
    (*(GET_INTERFACE (CCSIntegratedSettingInterface, setting))->writeValue) (setting, value, type);
}

const char * ccsIntegratedSettingInfoPluginName (CCSIntegratedSettingInfo *info)
{
    return (*(GET_INTERFACE (CCSIntegratedSettingInfoInterface, info))->pluginName) (info);
}

const char * ccsIntegratedSettingInfoSettingName (CCSIntegratedSettingInfo *info)
{
    return (*(GET_INTERFACE (CCSIntegratedSettingInfoInterface, info))->settingName) (info);
}

CCSSettingType ccsIntegratedSettingInfoGetType (CCSIntegratedSettingInfo *info)
{
    return (*(GET_INTERFACE (CCSIntegratedSettingInfoInterface, info))->getType) (info);
}

void ccsFreeIntegratedSettingInfo (CCSIntegratedSettingInfo *info)
{
    (*(GET_INTERFACE (CCSIntegratedSettingInfoInterface, info))->free) (info);
}

void ccsFreeIntegratedSetting (CCSIntegratedSetting *setting)
{
    (*(GET_INTERFACE (CCSIntegratedSettingInterface, setting))->free) (setting);
}

/* CCSSharedIntegratedSettingInfo implementation */

typedef struct _CCSSharedIntegratedSettingInfoPrivate CCSSharedIntegratedSettingInfoPrivate;

struct _CCSSharedIntegratedSettingInfoPrivate
{
    const char *pluginName;
    const char *settingName;
    CCSSettingType type;
};

static const char *
ccsSharedIntegratedSettingInfoSettingName (CCSIntegratedSettingInfo *setting)
{
    CCSSharedIntegratedSettingInfoPrivate *priv = (CCSSharedIntegratedSettingInfoPrivate *) ccsObjectGetPrivate (setting);

    return priv->settingName;
}

static const char *
ccsSharedIntegratedSettingInfoPluginName (CCSIntegratedSettingInfo *setting)
{
    CCSSharedIntegratedSettingInfoPrivate *priv = (CCSSharedIntegratedSettingInfoPrivate *) ccsObjectGetPrivate (setting);

    return priv->pluginName;
}

static CCSSettingType
ccsSharedIntegratedSettingInfoGetType (CCSIntegratedSettingInfo *setting)
{
    CCSSharedIntegratedSettingInfoPrivate *priv = (CCSSharedIntegratedSettingInfoPrivate *) ccsObjectGetPrivate (setting);

    return priv->type;
}

static void
ccsSharedIntegratedSettingInfoFree (CCSIntegratedSettingInfo *setting)
{
    ccsObjectFinalize (setting);
    (*setting->object.object_allocation->free_) (setting->object.object_allocation->allocator, setting);
}

const CCSIntegratedSettingInfoInterface ccsSharedIntegratedSettingInfoInterface =
{
    ccsSharedIntegratedSettingInfoPluginName,
    ccsSharedIntegratedSettingInfoSettingName,
    ccsSharedIntegratedSettingInfoGetType,
    ccsSharedIntegratedSettingInfoFree
};

CCSIntegratedSettingInfo *
ccsSharedIntegratedSettingInfoNew (const char *pluginName,
				   const char *settingName,
				   CCSSettingType type,
				   CCSObjectAllocationInterface *ai)
{
    CCSIntegratedSettingInfo *info = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSIntegratedSetting));

    if (!info)
	return NULL;

    CCSSharedIntegratedSettingInfoPrivate *priv = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSSharedIntegratedSettingInfoPrivate));

    if (!priv)
    {
	(*ai->free_) (ai->allocator, info);
	return NULL;
    }

    priv->pluginName = pluginName;
    priv->settingName = settingName;
    priv->type = type;

    ccsObjectInit (info, ai);
    ccsObjectSetPrivate (info, (CCSPrivate *) priv);
    ccsObjectAddInterface (info, (const CCSInterface *) &ccsSharedIntegratedSettingInfoInterface, GET_INTERFACE_TYPE (CCSIntegratedSettingInfoInterface));
    ccsIntegratedSettingInfoRef (info);

    return info;
}

CCSIntegratedSettingList
ccsIntegratedSettingsStorageFindMatchingSettingsByPredicate (CCSIntegratedSettingsStorage *storage,
							     CCSIntegratedSettingsStorageFindPredicate pred,
							     void			  *data)
{
    return (*(GET_INTERFACE (CCSIntegratedSettingsStorageInterface, storage))->findMatchingSettingsByPredicate) (storage, pred, data);
}

CCSIntegratedSettingList
ccsIntegratedSettingsStorageFindMatchingSettingsByPluginAndSettingName (CCSIntegratedSettingsStorage *storage,
									const char *pluginName,
									const char *settingName)
{
    return (*(GET_INTERFACE (CCSIntegratedSettingsStorageInterface, storage))->findMatchingSettingsByPluginAndSettingName) (storage, pluginName, settingName);
}

void
ccsIntegratedSettingsStorageAddSetting (CCSIntegratedSettingsStorage *storage,
					CCSIntegratedSetting	     *setting)
{
    (*(GET_INTERFACE (CCSIntegratedSettingsStorageInterface, storage))->addSetting) (storage, setting);
}

Bool
ccsIntegratedSettingsStorageEmpty (CCSIntegratedSettingsStorage *storage)
{
    return (*(GET_INTERFACE (CCSIntegratedSettingsStorageInterface, storage))->empty) (storage);
}

void
ccsFreeIntegratedSettingsStorage (CCSIntegratedSettingsStorage *storage)
{
    (*(GET_INTERFACE (CCSIntegratedSettingsStorageInterface, storage))->free) (storage);
}

/* CCSIntegratedSettingsStorageDefault implementation */
typedef struct _CCSIntegratedSettingsStorageDefaultPrivate CCSIntegratedSettingsStorageDefaultPrivate;

struct _CCSIntegratedSettingsStorageDefaultPrivate
{
    CCSIntegratedSettingList settingList;
};

typedef struct _CCSIntegratedSettingsStorageFindByNamesData
{
    const char *pluginName;
    const char *settingName;
} CCSIntegratedSettingsStorageFindByNamesData;

static Bool
ccsIntegratedSettingsStorageFindByNamesPredicate (CCSIntegratedSetting *setting,
						  void		       *data)
{
    CCSIntegratedSettingsStorageFindByNamesData *findNamesData = (CCSIntegratedSettingsStorageFindByNamesData *) data;

    const char *sPluginName = ccsIntegratedSettingInfoPluginName ((CCSIntegratedSettingInfo *) setting);
    const char *sSettingName = ccsIntegratedSettingInfoSettingName ((CCSIntegratedSettingInfo *) setting);

    if (strcmp (sPluginName, findNamesData->pluginName) == 0 &&
	strcmp (sSettingName, findNamesData->settingName) == 0)
    {
	return TRUE;
    }

    return FALSE;
}

CCSIntegratedSettingList
ccsIntegratedSettingsStorageDefaultFindMatchingSettingsByPredicate (CCSIntegratedSettingsStorage *storage,
								    CCSIntegratedSettingsStorageFindPredicate pred,
								    void			 *data)
{
    CCSIntegratedSettingsStorageDefaultPrivate *priv = (CCSIntegratedSettingsStorageDefaultPrivate *) ccsObjectGetPrivate (storage);
    CCSIntegratedSettingList		       returnList = NULL;
    CCSIntegratedSettingList		       iter = priv->settingList;

    while (iter)
    {
	if ((*pred) (iter->data, data))
	    returnList = ccsIntegratedSettingListAppend (returnList, iter->data);

	iter = iter->next;
    }

    return returnList;
}

CCSIntegratedSettingList
ccsIntegratedSettingsStorageDefaultFindMatchingSettingsByPluginAndSettingName (CCSIntegratedSettingsStorage *storage,
									       const char *pluginName,
									       const char *settingName)
{
    CCSIntegratedSettingsStorageFindByNamesData data =
    {
	pluginName,
	settingName
    };

    return ccsIntegratedSettingsStorageFindMatchingSettingsByPredicate (storage, ccsIntegratedSettingsStorageFindByNamesPredicate, (void *) &data);
}

void
ccsIntegratedSettingsStorageDefaultAddSetting (CCSIntegratedSettingsStorage *storage,
					       CCSIntegratedSetting	    *setting)
{
    CCSIntegratedSettingsStorageDefaultPrivate *priv = (CCSIntegratedSettingsStorageDefaultPrivate *) ccsObjectGetPrivate (storage);
    priv->settingList = ccsIntegratedSettingListAppend (priv->settingList, setting);
}

Bool
ccsIntegratedSettingsStorageDefaultEmpty (CCSIntegratedSettingsStorage *storage)
{
    CCSIntegratedSettingsStorageDefaultPrivate *priv = (CCSIntegratedSettingsStorageDefaultPrivate *) ccsObjectGetPrivate (storage);
    return priv->settingList == NULL;
}

void
ccsIntegratedSettingsStorageDefaultImplFree (CCSIntegratedSettingsStorage *storage)
{
    CCSIntegratedSettingsStorageDefaultPrivate *priv = (CCSIntegratedSettingsStorageDefaultPrivate *) ccsObjectGetPrivate (storage);

    if (priv->settingList)
	ccsIntegratedSettingListFree (priv->settingList, TRUE);

    ccsObjectFinalize (storage);
    (*storage->object.object_allocation->free_) (storage->object.object_allocation->allocator, storage);
}

const CCSIntegratedSettingsStorageInterface ccsIntegratedSettingsStorageDefaultImplInterface =
{
    ccsIntegratedSettingsStorageDefaultFindMatchingSettingsByPredicate,
    ccsIntegratedSettingsStorageDefaultFindMatchingSettingsByPluginAndSettingName,
    ccsIntegratedSettingsStorageDefaultAddSetting,
    ccsIntegratedSettingsStorageDefaultEmpty,
    ccsIntegratedSettingsStorageDefaultImplFree
};

CCSIntegratedSettingsStorage *
ccsIntegratedSettingsStorageDefaultImplNew (CCSObjectAllocationInterface *ai)
{
    CCSIntegratedSettingsStorage *storage = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSIntegratedSettingsStorage));

    if (!storage)
	return NULL;

    CCSIntegratedSettingsStorageDefaultPrivate *priv = (*ai->calloc_) (ai->allocator, 1, sizeof (CCSIntegratedSettingsStorageDefaultPrivate));

    if (!priv)
    {
	(*ai->free_) (ai->allocator, storage);
	return NULL;
    }

    ccsObjectInit (storage, ai);
    ccsObjectSetPrivate (storage, (CCSPrivate *) priv);
    ccsObjectAddInterface (storage, (const CCSInterface *) &ccsIntegratedSettingsStorageDefaultImplInterface, GET_INTERFACE_TYPE (CCSIntegratedSettingsStorageInterface));

    ccsIntegratedSettingsStorageRef (storage);

    return storage;
}

CCSIntegratedSetting *
ccsIntegratedSettingFactoryCreateIntegratedSettingForCCSSettingNameAndType (CCSIntegratedSettingFactory *factory,
									    CCSIntegration	        *integration,
									    const char *pluginName,
									    const char *settingName,
									    CCSSettingType type)
{
    return (*(GET_INTERFACE (CCSIntegratedSettingFactoryInterface, factory))->createIntegratedSettingForCCSSettingNameAndType) (factory, integration, pluginName, settingName, type);
}

void
ccsFreeIntegratedSettingFactory (CCSIntegratedSettingFactory *factory)
{
    (*(GET_INTERFACE (CCSIntegratedSettingFactoryInterface, factory))->free) (factory);
}

static  const CCSPluginInterface ccsDefaultPluginInterface =
{
    ccsPluginGetNameDefault,
    ccsPluginGetShortDescDefault,
    ccsPluginGetLongDescDefault,
    ccsPluginGetHintsDefault,
    ccsPluginGetCategoryDefault,
    ccsPluginGetLoadAfterDefault,
    ccsPluginGetLoadBeforeDefault,
    ccsPluginGetRequiresPluginsDefault,
    ccsPluginGetConflictPluginsDefault,
    ccsPluginGetProvidesFeaturesDefault,
    ccsPluginGetRequiresFeaturesDefault,
    ccsPluginGetPrivatePtrDefault,
    ccsPluginSetPrivatePtrDefault,
    ccsPluginGetContextDefault,
    ccsFindSettingDefault,
    ccsGetPluginSettingsDefault,
    ccsGetPluginGroupsDefault,
    ccsReadPluginSettingsDefault,
    ccsGetPluginStrExtensionsDefault,
    ccsFreePluginDefault
};

static const CCSContextInterface ccsDefaultContextInterface =
{
    ccsContextGetPluginsDefault,
    ccsContextGetCategoriesDefault,
    ccsContextGetChangedSettingsDefault,
    ccsContextGetScreenNumDefault,
    ccsContextAddChangedSettingDefault,
    ccsContextClearChangedSettingsDefault,
    ccsContextStealChangedSettingsDefault,
    ccsContextGetPrivatePtrDefault,
    ccsContextSetPrivatePtrDefault,
    ccsLoadPluginDefault,
    ccsFindPluginDefault,
    ccsPluginIsActiveDefault,
    ccsGetActivePluginListDefault,
    ccsGetSortedPluginStringListDefault,
    ccsSetBackendDefault,
    ccsGetBackendDefault,
    ccsSetIntegrationEnabledDefault,
    ccsSetProfileDefault,
    ccsSetPluginListAutoSortDefault,
    ccsGetProfileDefault,
    ccsGetIntegrationEnabledDefault,
    ccsGetPluginListAutoSortDefault,
    ccsProcessEventsDefault,
    ccsReadSettingsDefault,
    ccsWriteSettingsDefault,
    ccsWriteChangedSettingsDefault,
    ccsExportToFileDefault,
    ccsImportFromFileDefault,
    ccsCanEnablePluginDefault,
    ccsCanDisablePluginDefault,
    ccsGetExistingProfilesDefault,
    ccsDeleteProfileDefault,
    ccsCheckForSettingsUpgradeDefault,
    ccsLoadPluginsDefault,
    ccsFreeContextDefault
};

static const CCSSettingInterface ccsDefaultSettingInterface =
{
    ccsSettingGetNameDefault,
    ccsSettingGetShortDescDefault,
    ccsSettingGetLongDescDefault,
    ccsSettingGetTypeDefault,
    ccsSettingGetInfoDefault,
    ccsSettingGetGroupDefault,
    ccsSettingGetSubGroupDefault,
    ccsSettingGetHintsDefault,
    ccsSettingGetDefaultValueDefault,
    ccsSettingGetValueDefault,
    ccsSettingGetIsDefaultDefault,
    ccsSettingGetParentDefault,
    ccsSettingGetPrivatePtrDefault,
    ccsSettingSetPrivatePtrDefault,
    ccsSettingSetIntDefault,
    ccsSettingSetFloatDefault,
    ccsSettingSetBoolDefault,
    ccsSettingSetStringDefault,
    ccsSettingSetColorDefault,
    ccsSettingSetMatchDefault,
    ccsSettingSetKeyDefault,
    ccsSettingSetButtonDefault,
    ccsSettingSetEdgeDefault,
    ccsSettingSetBellDefault,
    ccsSettingSetListDefault,
    ccsSettingSetValueDefault,
    ccsSettingGetIntDefault,
    ccsSettingGetFloatDefault,
    ccsSettingGetBoolDefault,
    ccsSettingGetStringDefault,
    ccsSettingGetColorDefault,
    ccsSettingGetMatchDefault,
    ccsSettingGetKeyDefault,
    ccsSettingGetButtonDefault,
    ccsSettingGetEdgeDefault,
    ccsSettingGetBellDefault,
    ccsSettingGetListDefault,
    ccsSettingResetToDefaultDefault,
    ccsSettingGetIsIntegratedDefault,
    ccsSettingGetIsReadOnlyDefault,
    ccsSettingGetIsReadableByBackendDefault,
    ccsFreeSettingDefault
};

const CCSBackendInterface ccsDynamicBackendInterfaceWrapper =
{
    ccsDynamicBackendGetInfoWrapper,
    ccsDynamicBackendExecuteEventsWrapper,
    ccsDynamicBackendInitWrapper,
    ccsDynamicBackendFiniWrapper,
    ccsDynamicBackendReadInitWrapper,
    ccsDynamicBackendReadSettingWrapper,
    ccsDynamicBackendReadDoneWrapper,
    ccsDynamicBackendWriteInitWrapper,
    ccsDynamicBackendWriteSettingWrapper,
    ccsDynamicBackendWriteDoneWrapper,
    ccsDynamicBackendUpdateSettingWrapper,
    ccsDynamicBackendGetSettingIsIntegratedWrapper,
    ccsDynamicBackendGetSettingIsReadOnlyWrapper,
    ccsDynamicBackendGetExistingProfilesWrapper,
    ccsDynamicBackendDeleteProfileWrapper,
    ccsDynamicBackendSetIntegrationWrapper
};

const CCSDynamicBackendInterface ccsDefaultDynamicBackendInterface =
{
    ccsDynamicBackendGetBackendNameDefault,
    ccsDynamicBackendSupportsReadDefault,
    ccsDynamicBackendSupportsWriteDefault,
    ccsDynamicBackendSupportsIntegrationDefault,
    ccsDynamicBackendSupportsProfilesDefault,
    ccsDynamicBackendGetRawBackendDefault
};

const CCSInterfaceTable ccsDefaultInterfaceTable =
{
    &ccsDefaultContextInterface,
    &ccsDefaultPluginInterface,
    &ccsDefaultSettingInterface,
    &ccsDynamicBackendInterfaceWrapper,
    &ccsDefaultDynamicBackendInterface
};
