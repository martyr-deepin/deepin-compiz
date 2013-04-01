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

#ifndef _PLUGINCLASSES_H
#define _PLUGINCLASSES_H

#include <vector>

/**
 * Represents the index of a plugin's object classes
 */
class PluginClassIndex {
    public:
	PluginClassIndex () : index ((unsigned)~0), refCount (0),
			      initiated (false), failed (false),
			      pcFailed (false), pcIndex (0) {}

	unsigned int index;
	int          refCount;
	bool         initiated;
	bool         failed;
	bool         pcFailed;
	unsigned int pcIndex;
};

/**
 * Represents some storage of a plugin class on a core object,
 * usually some pointer (void *)
 */
class PluginClassStorage {

    public:
	typedef std::vector<bool> Indices;

    public:
	PluginClassStorage (Indices& iList);

    public:
	std::vector<void *> pluginClasses;

    protected:
	static unsigned int allocatePluginClassIndex (Indices& iList);
	static void freePluginClassIndex (Indices& iList, unsigned int idx);
};

#endif
