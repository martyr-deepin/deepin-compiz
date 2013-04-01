/*
 * Copyright © 2008 Dennis Kasprzyk
 * Copyright © 2007 Novell, Inc.
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
 * Authors: Dennis Kasprzyk <onestone@compiz-fusion.org>
 *          David Reveman <davidr@novell.com>
 */

#ifndef _COMPOUTPUT_H
#define _COMPOUTPUT_H

#include "core/rect.h"
#include "core/string.h"

#include <list>
#include <vector>

/**
 * Represents a phisically attached screen in Compiz, where this
 * phisical screen is part of an X11 screen in a configuration such
 * as Xinerama, XRandR, TwinView or MergedFB
 */
class CompOutput : public CompRect {

    public:
	CompOutput ();

	CompString name () const;

	unsigned int id () const;

	/**
	 * Returns a "working area" of the screen, which the geometry
	 * which is not covered by strut windows (such as panels)
	 */
	const CompRect& workArea () const;

	void setWorkArea (const CompRect&);
	void setGeometry (int x, int y, int width, int height);
	void setId (CompString, unsigned int);

	typedef std::vector<CompOutput> vector;
	typedef std::vector<CompOutput *> ptrVector;
	typedef std::list<CompOutput *> ptrList;

    private:

	CompString   mName;
	unsigned int mId;

	CompRect     mWorkArea;
};

#endif
