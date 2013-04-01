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
#ifndef _COMPIZCONFIG_CCS_MOCKED_ALLOCATION_H
#define _COMPIZCONFIG_CCS_MOCKED_ALLOCATION_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class AllocationInterface
{
    public:

	virtual void * realloc_ (void *, size_t) = 0;
	virtual void * malloc_ (size_t) = 0;
	virtual void * calloc_ (size_t, size_t) = 0;
	virtual void free_ (void *) = 0;

    public:

	static void * wrapRealloc (void *o, void *a , size_t b)
	{
	    AllocationInterface *ao =  static_cast <AllocationInterface *> (o);
	    return ao->realloc_ (a, b);
	}

	static void * wrapMalloc (void *o, size_t a)
	{
	    AllocationInterface *ao =  static_cast <AllocationInterface *> (o);
	    return ao->malloc_ (a);
	}

	static void * wrapCalloc (void *o, size_t a, size_t b)
	{
	    AllocationInterface *ao =  static_cast <AllocationInterface *> (o);
	    return ao->calloc_ (a, b);
	}

	static void wrapFree (void *o, void *a)
	{
	    AllocationInterface *ao =  static_cast <AllocationInterface *> (o);
	    ao->free_ (a);
	}
};

class ObjectAllocationGMock :
    public AllocationInterface
{
    public:

	MOCK_METHOD2 (realloc_, void * (void *, size_t));
	MOCK_METHOD1 (malloc_, void * (size_t));
	MOCK_METHOD2 (calloc_, void * (size_t, size_t));
	MOCK_METHOD1 (free_, void (void *));

};

class FailingObjectReallocation :
    public AllocationInterface
{
    public:

	FailingObjectReallocation (unsigned int sc) :
	    successCount (sc)
	{
	}

	void * realloc_ (void *a, size_t b) { unsigned int c = successCount--; if (c) return realloc (a, b); else return NULL; }
	void * malloc_ (size_t a) { return malloc (a); }
	void * calloc_ (size_t n, size_t a) { return  calloc (n, a); }
	void free_ (void *a) { free (a); }

    private:

	unsigned int successCount;
};

class FailingObjectAllocation :
    public AllocationInterface
{
    public:

	void * realloc_ (void *a, size_t b) { return NULL; }
	void * malloc_ (size_t a) { return NULL; }
	void * calloc_ (size_t n, size_t a) { return NULL; }
	void free_ (void *a) {  }
};

CCSObjectAllocationInterface failingAllocator =
{
    AllocationInterface::wrapRealloc,
    AllocationInterface::wrapMalloc,
    AllocationInterface::wrapCalloc,
    AllocationInterface::wrapFree,
    NULL
};

#endif
