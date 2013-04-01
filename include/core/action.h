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

#ifndef _COMPACTION_H
#define _COMPACTION_H

#include "core/option.h"

#include <boost/function.hpp>

#include <X11/Xlib-xcb.h>


class PrivateAction;

#define CompModAlt        0
#define CompModMeta       1
#define CompModSuper      2
#define CompModHyper      3
#define CompModModeSwitch 4
#define CompModNumLock    5
#define CompModScrollLock 6
#define CompModNum        7

#define CompAltMask        (1 << 16)
#define CompMetaMask       (1 << 17)
#define CompSuperMask      (1 << 18)
#define CompHyperMask      (1 << 19)
#define CompModeSwitchMask (1 << 20)
#define CompNumLockMask    (1 << 21)
#define CompScrollLockMask (1 << 22)

#define CompNoMask         (1 << 25)

/**
 * Context of an event occuring.
 */
class CompAction {
    public:
	typedef enum {
	    StateInitKey     = 1 <<  0,
	    StateTermKey     = 1 <<  1,
	    StateInitButton  = 1 <<  2,
	    StateTermButton  = 1 <<  3,
	    StateInitBell    = 1 <<  4,
	    StateInitEdge    = 1 <<  5,
	    StateTermEdge    = 1 <<  6,
	    StateInitEdgeDnd = 1 <<  7,
	    StateTermEdgeDnd = 1 <<  8,
	    StateCommit      = 1 <<  9,
	    StateCancel      = 1 << 10,
	    StateAutoGrab    = 1 << 11,
	    StateNoEdgeDelay = 1 << 12,
	    StateTermTapped  = 1 << 13
	} StateEnum;

	/**
	 * Type of event a CompAction is bound to.
	 */
	typedef enum {
	    BindingTypeNone       = 0,
	    BindingTypeKey        = 1 << 0,
	    BindingTypeButton     = 1 << 1,
	    BindingTypeEdgeButton = 1 << 2
	} BindingTypeEnum;

	class KeyBinding {
	    public:
		KeyBinding ();
		KeyBinding (const KeyBinding&);
		KeyBinding (int keycode, unsigned int modifiers = 0);

		unsigned int modifiers () const;
		int keycode () const;

		bool fromString (const CompString &str);
		CompString toString () const;

	    private:
		unsigned int mModifiers;
		int          mKeycode;
	};

	class ButtonBinding {
	    public:
		ButtonBinding ();
		ButtonBinding (const ButtonBinding&);
		ButtonBinding (int button, unsigned int modifiers = 0);

		unsigned int modifiers () const;
		int button () const;

		bool fromString (const CompString &str);
		CompString toString () const;

	    private:
		unsigned int mModifiers;
		int          mButton;
	};

	typedef unsigned int State;
	typedef unsigned int BindingType;
	typedef boost::function <bool (CompAction *, State, CompOption::Vector &)> CallBack;

    public:
	CompAction ();
	CompAction (const CompAction &);
	~CompAction ();

	CallBack initiate ();
	CallBack terminate ();

	void setInitiate (const CallBack &initiate);
	void setTerminate (const CallBack &terminate);

	State state ();
	BindingType type ();

	KeyBinding & key ();
	void setKey (const KeyBinding &key);

	ButtonBinding & button ();
	void setButton (const ButtonBinding &button);

	unsigned int edgeMask ();
	void setEdgeMask (unsigned int edge);

	bool bell ();
	void setBell (bool bell);

	void setState (State state);

	void copyState (const CompAction &action);

	bool operator== (const CompAction& val) const;
	CompAction & operator= (const CompAction &action);

	bool keyFromString (const CompString &str);
	bool buttonFromString (const CompString &str);
	bool edgeMaskFromString (const CompString &str);

	CompString keyToString ();
	CompString buttonToString ();
	CompString edgeMaskToString ();

	static CompString edgeToString (unsigned int edge);

	bool active ();

	friend class CompScreenImpl;

    private:
	PrivateAction *priv;
};

#endif
