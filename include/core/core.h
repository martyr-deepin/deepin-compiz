/*
 * Copyright © 2007 Novell, Inc.
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
 * Author: David Reveman <davidr@novell.com>
 */

#ifndef _COMPIZ_CORE_H
#define _COMPIZ_CORE_H


#include "abiversion.h"

#include <stdio.h>
#include <assert.h>

#include <string>
#include <list>
#include <cstdarg>

#include <X11/Xlib-xcb.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/sync.h>
#include <X11/Xregion.h>
#include <X11/XKBlib.h>

// X11 Bool defination breaks BOOST_FOREACH. Convert it to a typedef */
#ifdef Bool
typedef Bool XBool;
#undef Bool
typedef XBool Bool;
#endif

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH


/*
 * WORDS_BIGENDIAN should be defined before including this file for
 * IMAGE_BYTE_ORDER and BITMAP_BIT_ORDER to be set correctly.
 */
#define LSBFirst 0
#define MSBFirst 1

#ifdef WORDS_BIGENDIAN
#  define IMAGE_BYTE_ORDER MSBFirst
#  define BITMAP_BIT_ORDER MSBFirst
#else
#  define IMAGE_BYTE_ORDER LSBFirst
#  define BITMAP_BIT_ORDER LSBFirst
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY (x)

#include <core/global.h>

#include <core/pluginclasses.h>
#include <core/screen.h>
#include <core/window.h>
#include <core/plugin.h>
#include <core/option.h>
#include <core/action.h>
#include <core/icon.h>
#include <core/match.h>
#include <core/output.h>
#include <core/point.h>
#include <core/rect.h>
#include <core/session.h>
#include <core/size.h>
#include <core/region.h>
#include <core/countedlist.h>
#include <core/timeouthandler.h>
#include <core/logmessage.h>
#include <core/string.h>

#endif
