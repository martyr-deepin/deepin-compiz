/*
 * Copyright © 2010 Canonical Ltd.
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Canonical Ltd. not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Canonical Ltd. makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * CANONICAL, LTD. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL CANONICAL, LTD. BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authored by: Sam Spilsbury <sam.spilsbury@canonical.com>
 */

#include <core/valueholder.h>
#include <map>

namespace
{
  static ValueHolder *gDefault;
}

class PrivateValueHolder
{
    public:

	std::map<CompString, CompPrivate> valueMap;
};

ValueHolder::ValueHolder () :
    priv (new PrivateValueHolder)
{
}

ValueHolder::~ValueHolder ()
{
    delete priv;
}

ValueHolder *
ValueHolder::Default ()
{
    return gDefault;
}

void
ValueHolder::SetDefault (ValueHolder *v)
{
    gDefault = v;
}

void
ValueHolder::storeValue (CompString key, CompPrivate value)
{
    std::map<CompString,CompPrivate>::iterator it;

    it = priv->valueMap.find (key);

    if (it != priv->valueMap.end ())
    {
	it->second = value;
    }
    else
    {
	priv->valueMap.insert (std::pair<CompString,CompPrivate> (key, value));
    }
}

void
ValueHolder::eraseValue (CompString key)
{
    std::map<CompString,CompPrivate>::iterator it;
    it = priv->valueMap.find (key);

    if (it != priv->valueMap.end ())
    {
	priv->valueMap.erase (key);
    }
}

bool
ValueHolder::hasValue (CompString key)
{
    return (priv->valueMap.find (key) != priv->valueMap.end ());
}

CompPrivate
ValueHolder::getValue (CompString key)
{
    CompPrivate p;

    std::map<CompString,CompPrivate>::iterator it;
    it = priv->valueMap.find (key);

    if (it != priv->valueMap.end ())
    {
	return it->second;
    }
    else
    {
	p.uval = 0;
	return p;
    }
}
