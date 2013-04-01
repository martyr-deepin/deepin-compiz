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
 * Authored By: Sam Spilsbury <sam.spilsbury@canonical.com>
 */

#ifndef _CCS_OBJECT_H
#define _CCS_OBJECT_H

#include <ccs-defs.h>

COMPIZCONFIG_BEGIN_DECLS

typedef struct _CCSInterface CCSInterface; /* Dummy typedef */
typedef struct _CCSPrivate CCSPrivate; /* Dummy typedef */
typedef struct _CCSObject CCSObject;

typedef void * (*reallocObjectProc) (void *, void *, size_t);
typedef void * (*mallocObjectProc) (void *, size_t);
typedef void * (*callocObjectProc) (void *, size_t, size_t);
typedef void (*freeObjectProc) (void *, void *);

/**
 * An interface which specifies how objects should be allocated
 *
 * The purpose of this interface is to provide wrappers around
 * the standard realloc, malloc, calloc and free functions.
 *
 * ccsObjectInit takes this allocation interface as a means of
 * being able to test what happens when an allocation fails.
 *
 * Any other objects which implement CCSObject should also take
 * this interface and pass it to ccsObjectInit. They should also
 * use this interface as a means to allocate their own data so
 * that tests for those objects can test allocation failures
 */
typedef struct _CCSObjectAllocationInterface
{
    reallocObjectProc realloc_;
    mallocObjectProc  malloc_;
    callocObjectProc  calloc_;
    freeObjectProc    free_;
    void              *allocator;
} CCSObjectAllocationInterface;

extern CCSObjectAllocationInterface ccsDefaultObjectAllocator;

/**
 *
 * CCSObject - a generic C based object
 *
 * CCSObject is a generic C based object system for compizconfig.
 * Any structure that wishes to behave in an object oriented way
 * should have a CCSObject as its first and only member, and with
 * the name "object". (The macros that use CCSObject require this
 * in order to use it safely).
 *
 * CCSObject only provides two facilites to clients - implementation
 * of interfaces (which are just structs of function pointers) and
 * private data storage.
 *
 * They can also be refcounted, however, this is only ever done safely
 * if the object implements a virtual destructor method in its interface
 * so that the macro CCSREF_OBJ knows what to destroy first.
 *
 * CCSObjects must be initialized before they are used. This can be
 * done by calling ccsObjectInit and passing in the allocated memory
 * block containing the object and a CCSObjectAllocationInterface * which
 * ccsObjectInit will store in the object and use for all future
 * allocations.
 *
 * Objects can implement interfaces by storing a struct of function pointers
 * pertaining to that interface associated with that interface's unique type
 * id generated at runtime. Eg
 *
 * ccsObjectAddInterface (object, (const CCSInterface *) &interface,
 *			  GET_INTERFACE_TYPE (Interface));
 *
 * Then you can retreive that interface and access its methods as follows:
 *
 * GET_INTERFACE (Interface, object)->method
 *
 * Objects also have one pointer available for private storage. It is
 * recommended that all nonvirtual data lives in such storage. Add
 * a private using:
 *
 * ccsObjectSetPrivate (object, (CCSPrivate *) priv);
 *
 * Setting a private member does NOT free the existing private member if
 * it is set. It is the object's responsibility to do this if it wishes
 * to set a new private member, or set the existing one to NULL.
 *
 * To finalize an object and free all related data, use ccsObjectFinalize
 *
 * ccsObjectFinalize (object);
 *
 * This will free all private data, interface arrays and other data
 * using the provided CCSObjectAllocationInterface on ccsObjectInit.
 */
struct _CCSObject
{
    CCSPrivate *priv; /* Private pointer for object storage */

    const CCSInterface **interfaces; /* An array of interfaces that this object implements */
    int          *interface_types; /* An array of interface types */
    unsigned int n_interfaces;
    unsigned int n_allocated_interfaces;

    CCSObjectAllocationInterface *object_allocation;

    unsigned int refcnt; /* Reference count of this object */
};

Bool
ccsObjectInit_ (CCSObject *object, CCSObjectAllocationInterface *interface);

#define ccsObjectInit(o, interface) (ccsObjectInit_) (&(o)->object, interface)

Bool
ccsObjectAddInterface_ (CCSObject *object, const CCSInterface *interface, int interface_type);

#define ccsObjectAddInterface(o, interface, type) (ccsObjectAddInterface_) (&(o)->object, interface, type);

Bool
ccsObjectRemoveInterface_ (CCSObject *object, int interface_type);

#define ccsObjectRemoveInterface(o, interface_type) (ccsObjectRemoveInterface_) (&(o)->object, interface_type);

const CCSInterface * ccsObjectGetInterface_ (CCSObject *object, int interface_type);

#define ccsObjectGetInterface(o, interface_type) (ccsObjectGetInterface_) (&(o)->object, interface_type)

#define ccsObjectRef(o) \
    do { ((o)->object).refcnt++; } while (FALSE)

#define ccsObjectUnref(o, freeFunc) \
    do \
    { \
	((o)->object).refcnt--; \
	if (!((o)->object).refcnt) \
	    freeFunc (o); \
    } while (FALSE)

CCSPrivate *
ccsObjectGetPrivate_ (CCSObject *object);

#define ccsObjectGetPrivate(o) (ccsObjectGetPrivate_) (&(o)->object)

void
ccsObjectSetPrivate_ (CCSObject *object, CCSPrivate *priv);

#define ccsObjectSetPrivate(o, priv) (ccsObjectSetPrivate_) (&(o)->object, priv)

void
ccsObjectFinalize_ (CCSObject *object);

#define ccsObjectFinalize(o) (ccsObjectFinalize_) (&(o)->object)

/**
 * Internal method to allocate a type.
 *
 * @brief ccsAllocateType
 * @return a new type
 */
unsigned int
ccsAllocateType ();

#define GET_INTERFACE_TYPE(Interface) \
    ccs##Interface##GetType ()

/**
 * Used to define a new interface type - you should do this if any CCSObject
 * is to implement this interface */
#define INTERFACE_TYPE(Interface) \
    unsigned int ccs##Interface##GetType () \
    { \
	static unsigned int   type_id = 0; \
	if (!type_id) \
	    type_id = ccsAllocateType (); \
	 \
	return type_id; \
    }

#define GET_INTERFACE(CType, o) (CType *) ccsObjectGetInterface (o, GET_INTERFACE_TYPE(CType))

#define CCSREF_OBJ(type,dtype) \
    void ccs##type##Ref (dtype *d) \
    { \
	ccsObjectRef (d); \
    } \
    \
    void ccs##type##Unref (dtype *d) \
    { \
	ccsObjectUnref (d, ccsFree##type); \
    } \

COMPIZCONFIG_END_DECLS

#endif
