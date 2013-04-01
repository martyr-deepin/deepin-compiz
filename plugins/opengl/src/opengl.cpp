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

#include <core/pluginclasshandler.h>
#include "privates.h"

const float DEFAULT_Z_CAMERA = 0.866025404f;

const float RED_SATURATION_WEIGHT   = 0.30f;
const float GREEN_SATURATION_WEIGHT = 0.59f;
const float BLUE_SATURATION_WEIGHT  = 0.11f;

const unsigned short NOTHING_TRANS_FILTER = 0;
const unsigned short SCREEN_TRANS_FILTER  = 1;
const unsigned short WINDOW_TRANS_FILTER  = 2;

CompOption::Vector &
GLScreen::getOptions ()
{
    return priv->getOptions ();
}

bool
GLScreen::setOption (const CompString  &name,
		     CompOption::Value &value)
{
    return priv->setOption (name, value);
}

bool
PrivateGLScreen::setOption (const CompString  &name,
			    CompOption::Value &value)
{
    unsigned int index;

    bool rv = OpenglOptions::setOption (name, value);

    if (!rv || !CompOption::findOption (getOptions (), name, &index))
	return false;

    switch (index) {
	case OpenglOptions::TextureFilter:
	    cScreen->damageScreen ();

	    if (!optionGetTextureFilter ())
		textureFilter = GL_NEAREST;
	    else
		textureFilter = GL_LINEAR;
	    break;
	default:
	    break;
    }

    return rv;
}

class OpenglPluginVTable :
    public CompPlugin::VTableForScreenAndWindow<GLScreen, GLWindow>
{
    public:

	bool init ();
	void fini ();
};

COMPIZ_PLUGIN_20090315 (opengl, OpenglPluginVTable)

bool
OpenglPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI))
	return false;

    CompPrivate p;
    p.uval = COMPIZ_OPENGL_ABI;
    screen->storeValue ("opengl_ABI", p);

    return true;
}

void
OpenglPluginVTable::fini ()
{
    screen->eraseValue ("opengl_ABI");
}
