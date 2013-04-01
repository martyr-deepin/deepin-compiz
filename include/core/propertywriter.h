/*
 * Copyright © 2010 Sam Spilsbury
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Dennis Kasprzyk not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Dennis Kasprzyk makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DENNIS KASPRZYK DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DENNIS KASPRZYK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors: Sam Spilsbury <smspillaz@gmail.com>
 */

#ifndef _COMPPROPERTYWRITER_H
#define _COMPPROPERTYWRITER_H

#include "core/option.h"
#include "core/string.h"

#include <X11/Xatom.h>
#include <X11/X.h>

static const CompOption::Vector nilValues;

class PropertyWriter
{
    public:

	PropertyWriter ();
	PropertyWriter (CompString propName,
			CompOption::Vector &readTemplate);

	bool updateProperty (Window, CompOption::Vector &, int);
	void deleteProperty (Window);
	const CompOption::Vector & readProperty (Window);
	void setReadTemplate (const CompOption::Vector &);
	const CompOption::Vector & getReadTemplate ();

    private:

	CompOption::Vector mPropertyValues;
	Atom		   mAtom;
};

#endif
