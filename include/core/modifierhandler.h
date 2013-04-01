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
 *	    Sam Spilsbury <smspillaz@gmail.com>
 */

#include <core/core.h>

/**
 * Toplevel class which provides access to display
 * level modifier information
 */
class ModifierHandler
{
    public:

	ModifierHandler ();
	~ModifierHandler ();

	typedef enum
	{
	    Alt = 1,
	    Meta,
	    Super,
	    Hyper,
	    ModeSwitch,
	    NumLock,
	    ScrollLock,
	    ModNum
	} Modifier;

	typedef enum
	{
	    AltMask        = (1 << 16),
	    MetaMask       = (1 << 17),
	    SuperMask      = (1 << 18),
	    HyperMask      = (1 << 19),
	    ModeSwitchMask = (1 << 20),
	    NumLockMask    = (1 << 21),
	    ScrollLockMask = (1 << 22),
	    NoMask         = (1 << 25),
	} ModMask;

    public:


	/**
	 * Takes an X11 Keycode and returns a bitmask
	 * with modifiers that have been pressed
	 */
	unsigned int keycodeToModifiers (int keycode);

	/**
	 * Updates X11 Modifier mappings
	 */
	void updateModifierMappings ();

	/**
	 * Takes a virtual modMask and returns a real modifier mask
	 * by removing unused bits
	 */
	unsigned int virtualToRealModMask (unsigned int modMask);

	/**
	 * Returns a bit modifier mask for a Motifier enum
	 */
	unsigned int modMask (Modifier);

	/**
	 * Returns a const bit modifier mask for what should be ignored
	 */
	unsigned int ignoredModMask ();

	/**
	 * Returns a const XModifierKeymap for compiz
	 */
	const XModifierKeymap * modMap ();

	friend class CompScreenImpl;

    private:

	static const unsigned int virtualModMask[7];

	static const int maskTable[8];

	static const int maskTableSize = 8;

	ModMask    mModMask[ModNum];
	unsigned int    mIgnoredModMask;
	XModifierKeymap *mModMap;
};
