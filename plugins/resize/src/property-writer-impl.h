/*
 * Copyright © 2012 Canonical Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Novell, Inc. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Novell, Inc. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * NOVELL, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NOVELL, INC. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Daniel d'Andrada <daniel.dandrada@canonical.com>
 */
 
#ifndef RESIZE_PROPERTY_WRITER_IMPL_H
#define RESIZE_PROPERTY_WRITER_IMPL_H

#include "core/propertywriter.h"
#include "property-writer-interface.h"

namespace resize
{

class PropertyWriterImpl : public PropertyWriterInterface
{
public:
    PropertyWriterImpl (PropertyWriter *impl) : mImpl (impl) {}
    virtual ~PropertyWriterImpl() {delete mImpl;}

    virtual bool updateProperty (Window w, CompOption::Vector &v, int i)
    {
	return mImpl->updateProperty (w, v, i);
    }

    virtual void deleteProperty (Window w)
    {
	mImpl->deleteProperty (w);
    }

    virtual const CompOption::Vector & readProperty (Window w)
    {
	return mImpl->readProperty (w);
    }

    virtual void setReadTemplate (const CompOption::Vector &v)
    {
	mImpl->setReadTemplate (v);
    }

    virtual const CompOption::Vector & getReadTemplate ()
    {
	return mImpl->getReadTemplate ();
    }

    PropertyWriter* mImpl;
};

} /* namespace resize */

#endif /* RESIZE_PROPERTY_WRITER_IMPL_H */
