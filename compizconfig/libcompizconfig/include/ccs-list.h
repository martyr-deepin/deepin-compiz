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

#ifndef _CCS_LIST_H
#define _CCS_LIST_H

#include <ccs-defs.h>

COMPIZCONFIG_BEGIN_DECLS

/**
 * list functions:
 * for each list there is a set of functions, explained using String as example
 *
 * ccsStringListAppend (list, item)
 * Adds an item at the end of the list. Returns the new list.
 *
 * ccsStringListPrepend (list, item)
 * Adds an item at the beginning of the list. Returns the new list.
 *
 * ccsStringListInsert (list, item, position)
 * Adds an item at a given position. Position is 0-based. If position is
 * larger than the amount of items in the list, the item is inserted at the
 * end of the list. Returns the new list.
 *
 * ccsStringListInsertBefore (list, sibling, item)
 * Inserts item before sibling into the list. If sibling is no list member,
 * item is inserted at the end. Returns the new list.
 *
 * ccsStringListLength (list)
 * Returns the amount of items in list.
 *
 * ccsStringListFind (list, item)
 * Finds and returns an item matching <item>. If nothing is found, returns NULL.
 *
 * ccsStringListGetItem (list, index)
 * Returns the list item at position <index>. If index is larger than the
 * amount of items in the list, returns NULL.
 *
 * ccsStringListRemove (list, item, freeObj)
 * Removes item from the list. If freeObj is TRUE, also frees the data item.
 * Returns the new list.
 *
 * ccsStringListFree (list, freeObj)
 * Frees the complete list. If freeObj is TRUE, also frees the data items.
 * Returns the new list (NULL).
 */
#define CCSLIST_HDR(type,dtype)		\
    typedef struct _CCS##type##List *	CCS##type##List;\
    struct _CCS##type##List	\
    {								\
	dtype   * data;			\
	CCS##type##List next;		\
    }; \
    CCS##type##List ccs##type##ListAppend (CCS##type##List list, dtype *data); \
    CCS##type##List ccs##type##ListPrepend (CCS##type##List list, dtype *data); \
    CCS##type##List ccs##type##ListInsert (CCS##type##List list, dtype *data, int position); \
    CCS##type##List ccs##type##ListInsertBefore (CCS##type##List list, CCS##type##List sibling, dtype *data); \
    unsigned int ccs##type##ListLength (CCS##type##List list); \
    CCS##type##List ccs##type##ListFind (CCS##type##List list, dtype *data); \
    CCS##type##List ccs##type##ListGetItem (CCS##type##List list, unsigned int index); \
    CCS##type##List ccs##type##ListRemove (CCS##type##List list, dtype *data, Bool freeObj); \
    CCS##type##List ccs##type##ListFree (CCS##type##List list, Bool freeObj);

COMPIZCONFIG_END_DECLS

#endif
