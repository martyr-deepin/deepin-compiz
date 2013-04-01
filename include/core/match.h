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

#ifndef _COMPMATCH_H
#define _COMPMATCH_H

#include <core/string.h>

class PrivateMatch;
class CompWindow;
class CompDisplay;

/**
 * Object which represents a series of window properties
 * that can be matched in a window. Used for determining which
 * windows to operate on, in core or in plugins
 */
class CompMatch {
    public:
    
    /**
     * TODO
     */
	class Expression {
	    public:
		virtual ~Expression () {};
		virtual bool evaluate (CompWindow *window) = 0;
	};

    public:
	CompMatch ();
	CompMatch (const CompString);
	CompMatch (const CompMatch &);
	~CompMatch ();

	static const CompMatch emptyMatch;

	void update ();
	
	/**
	 * Returns true if the specified CompWindow has the properties
	 * specified in the match object
	 */
	bool evaluate (CompWindow *window);

	CompString toString () const;
	bool isEmpty () const;

	CompMatch & operator= (const CompMatch &);
	CompMatch & operator&= (const CompMatch &);
	CompMatch & operator|= (const CompMatch &);

	const CompMatch & operator& (const CompMatch &);
	const CompMatch & operator| (const CompMatch &);
	const CompMatch & operator! ();

	CompMatch & operator= (const CompString &);
	CompMatch & operator&= (const CompString &);
	CompMatch & operator|= (const CompString &);

	const CompMatch & operator& (const CompString &);
	const CompMatch & operator| (const CompString &);

	bool operator== (const CompMatch &) const;
	bool operator!= (const CompMatch &) const;

    private:
	PrivateMatch *priv;
};

#endif
