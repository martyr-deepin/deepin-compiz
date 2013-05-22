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

#ifndef _PRIVATEACTION_H
#define _PRIVATEACTION_H

namespace compiz
{
namespace actions
{
class BoundAction
{
    public:

	virtual ~BoundAction () {}

	virtual void setActive (bool) = 0;
};

void setActionActiveState (const CompAction  &action,
			   bool              active);
}
}

class PrivateAction :
    public compiz::actions::BoundAction
{
    public:
	PrivateAction ();
	PrivateAction (const PrivateAction&);

	void setActive (bool);

	CompAction::CallBack initiate;
	CompAction::CallBack terminate;

	CompAction::State state;

	CompAction::BindingType   type;

	CompAction::KeyBinding    key;
	CompAction::ButtonBinding button;

	bool bell;

	unsigned int edgeMask;

	bool active;

	CompPrivate priv;
};

#endif
