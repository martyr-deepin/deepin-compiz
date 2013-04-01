/*
 *
 * Compiz scale window title filter plugin
 *
 * scalefilter.c
 *
 * Copyright : (C) 2007 by Danny Baumann
 * E-mail    : maniac@opencompositing.org
 *
 * Copyright : (C) 2006 Diogo Ferreira
 * E-mail    : diogo@underdev.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 */

#include <math.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

#include <X11/Xlib.h>
#include <X11/keysymdef.h>

#include "scalefilter.h"

COMPIZ_PLUGIN_20090315 (scalefilter, ScalefilterPluginVTable);

FilterInfo::FilterInfo (ScalefilterScreen *fs, const CompOutput& output) :
    outputDevice (output),
    stringLength (0),
    textValid (false),
    fScreen (fs)
{
    memset (filterString, 0, sizeof (filterString));

    timer.setCallback (boost::bind (&FilterInfo::timeout, this));
}

void
FilterInfo::damageTextRect () const
{
    int x, y, width, height;

    if (!fScreen->cScreen)
	return;

    x = outputDevice.x () + outputDevice.width () / 2 -
	text.getWidth () / 2 - 1;
    y = outputDevice.y () + outputDevice.height () / 2 -
	text.getHeight () / 2 - 1;

    width  = text.getWidth () + 2;
    height = text.getHeight () + 2;

    CompRegion region (x, y, width, height);

    fScreen->cScreen->damageRegion (region);
}

bool
FilterInfo::hasText () const
{
    return stringLength > 0;
}

void
FilterInfo::renderText ()
{
    CompText::Attrib attrib;
    char             buffer[2 * maxFilterStringLength];

    /* damage the old draw rectangle */
    if (textValid)
	damageTextRect ();

    text.clear ();
    textValid = false;

    if (!fScreen->optionGetFilterDisplay ())
	return;

    if (stringLength == 0)
	return;

    attrib.maxWidth  = outputDevice.width ();
    attrib.maxHeight = outputDevice.height ();

    attrib.family = "Sans";
    attrib.size = fScreen->optionGetFontSize ();
    attrib.color[0] = fScreen->optionGetFontColorRed ();
    attrib.color[1] = fScreen->optionGetFontColorGreen ();
    attrib.color[2] = fScreen->optionGetFontColorBlue ();
    attrib.color[3] = fScreen->optionGetFontColorAlpha ();

    attrib.flags = CompText::WithBackground | CompText::Ellipsized;
    if (fScreen->optionGetFontBold ())
	attrib.flags |= CompText::StyleBold;

    attrib.bgHMargin = fScreen->optionGetBorderSize ();
    attrib.bgVMargin = fScreen->optionGetBorderSize ();
    attrib.bgColor[0] = fScreen->optionGetBackColorRed ();
    attrib.bgColor[1] = fScreen->optionGetBackColorGreen ();
    attrib.bgColor[2] = fScreen->optionGetBackColorBlue ();
    attrib.bgColor[3] = fScreen->optionGetBackColorAlpha ();

    wcstombs (buffer, filterString, maxFilterStringLength);

    textValid = text.renderText (buffer, attrib);

    /* damage the new draw rectangle */
    if (textValid)
	damageTextRect ();
}

void
FilterInfo::update ()
{
    CompString filterText;
    char       matchText[2 * maxFilterStringLength];

    if (fScreen->optionGetFilterCaseInsensitive ())
	filterText = "ititle=";
    else
	filterText = "title=";

    wcstombs (matchText, filterString, maxFilterStringLength);
    filterText += matchText;

    filterMatch  = fScreen->sScreen->getCustomMatch ();
    filterMatch &= filterText;
}

const CompMatch&
FilterInfo::getMatch () const
{
    if (stringLength)
	return filterMatch;

    return CompMatch::emptyMatch;
}

void
ScalefilterScreen::relayout ()
{
    if (filterInfo)
	sScreen->relayoutSlots (filterInfo->getMatch ());
    else if (matchApplied)
	sScreen->relayoutSlots (persistentMatch);
    else
	sScreen->relayoutSlots (CompMatch::emptyMatch);
}

bool
FilterInfo::timeout ()
{
    fScreen->removeFilter ();

    return false;
}

bool
ScalefilterScreen::removeFilter ()
{
    bool retval = false;

    if (filterInfo)
    {
	/* in input mode: drop current filter */
	delete filterInfo;
	filterInfo = NULL;

	retval = true;
    }
    else if (matchApplied)
    {
	/* remove filter applied previously if currently not in input mode */
	matchApplied = false;

	retval = true;
    }

    if (retval)
	doRelayout ();

    return retval;
}

void
ScalefilterScreen::handleWindowRemove (Window id)
{
    CompWindow *w;

    w = screen->findWindow (id);
    if (w)
    {
	ScaleScreen::State state;

	SCALE_SCREEN (screen);
	SCALE_WINDOW (w);

	state = ss->getState ();
	if (state != ScaleScreen::Idle && state != ScaleScreen::In)
	{
	    const ScaleScreen::WindowList& windows = ss->getWindows ();
	    if (windows.size () == 1 && windows.front () == sw)
		removeFilter ();
	}
    }
}

void
ScalefilterScreen::doRelayout ()
{
    if (filterInfo)
    {
	filterInfo->renderText ();
	filterInfo->update ();
    }

    relayout ();
}

bool
ScalefilterScreen::handleSpecialKeyPress (XKeyEvent *event,
					  bool&     drop)
{
    KeySym          ks;
    bool            retval = false;
    bool            needRelayout = false;

    ks = XKeycodeToKeysym (screen->dpy (), event->keycode, 0);

    if (ks == XK_Escape)
    {
	/* Escape key - drop current filter or remove filter applied
	   previously if currently not in input mode */
	if (removeFilter ())
	    drop = true;
	retval = true;
    }
    else if (ks == XK_Return)
    {
	if (filterInfo && filterInfo->hasText ())
	{
	    /* Return key - apply current filter persistently */
	    unsigned int count = 0;
	    persistentMatch = filterInfo->getMatch ();
	    matchApplied    = true;
	    drop            = false;
	    needRelayout    = false;

	    /* Check whether there is just one window remaining on
	     * this match, if so, no need to relayout */

	    foreach (ScaleWindow *sw, sScreen->getWindows ())
	    {
		if (persistentMatch.evaluate (sw->window))
		    count++;

		if (count > 1)
		{
		    needRelayout = true;
		    drop = true;
		    break;
		}
	    }

	    delete filterInfo;
	    filterInfo = NULL;
	}
	retval = true;
    }
    else if (ks == XK_BackSpace)
    {
	if (filterInfo)
	    needRelayout = filterInfo->handleBackspace ();
	retval = true;
    }

    if (needRelayout)
	doRelayout ();

    return retval;
}

bool
FilterInfo::handleBackspace ()
{
    if (!stringLength)
	return false;

    /* remove last character in string */
    filterString[--stringLength] = '\0';

    return true;
}

bool
FilterInfo::handleInput (const wchar_t input)
{
    int timeout = fScreen->optionGetTimeout ();

    timer.stop ();
    if (timeout > 0)
    {
	timer.setTimes (timeout, (float) timeout * 1.2);
	timer.start ();
    }

    if (stringLength < maxFilterSize)
    {
	filterString[stringLength++] = input;
	filterString[stringLength] = '\0';
	return true;
    }

    return false;
}

void
ScalefilterScreen::handleTextKeyPress (XKeyEvent *event)
{
    bool         needRelayout = false;
    int          count;
    char         buffer[10];
    wchar_t      wbuffer[10];
    KeySym       ks;
#if 0 /* needs modHandler patch */
    unsigned int mods;

    /* ignore key presses with modifiers (except Shift and
       ModeSwitch AKA AltGr) */
    mods  = event->state & ~modHandler->ignoredModMask ();
    mods &= ~modHandler->modMask (CompModModeSwitch);
    if (mods & ~ShiftMask)
	return;
#endif

    memset (buffer, 0, sizeof (buffer));
    memset (wbuffer, 0, sizeof (wbuffer));

    if (xic)
    {
	Status status;

	XSetICFocus (xic);
	count = Xutf8LookupString (xic, event, buffer, 9, &ks, &status);
	XUnsetICFocus (xic);
    }
    else
    {
	count = XLookupString (event, buffer, 9, &ks, NULL);
    }

    mbstowcs (wbuffer, buffer, 9);

    if (count > 0)
    {
	if (!filterInfo)
	    filterInfo = new FilterInfo (this, screen->currentOutputDev ());

	needRelayout = filterInfo->handleInput (wbuffer[0]);
    }

    if (needRelayout)
	doRelayout ();
}

void
ScalefilterScreen::handleEvent (XEvent *event)
{
    bool grabbed = false, dropEvent = false;

    switch (event->type) {
    case KeyPress:
	{
	    SCALE_SCREEN (screen);

	    grabbed = ss->hasGrab ();
	    if (grabbed && handleSpecialKeyPress (&event->xkey, dropEvent))
	    {
		/* don't attempt to process text input later on
		   if the input was a special key */
		grabbed = false;
	    }
	}
	break;
    case UnmapNotify:
	handleWindowRemove (event->xunmap.window);
	break;
    case DestroyNotify:
	handleWindowRemove (event->xdestroywindow.window);
	break;
    default:
	break;
    }

    if (!dropEvent)
	screen->handleEvent (event);

    switch (event->type) {
    case KeyPress:
	if (grabbed && !dropEvent)
	    handleTextKeyPress (&event->xkey);
	break;
    }
}

void
ScalefilterScreen::handleCompizEvent (const char          *pluginName,
				      const char          *eventName,
				      CompOption::Vector& options)
{
    bool activated;

    screen->handleCompizEvent (pluginName, eventName, options);

    if (strcmp (pluginName, "scale") || strcmp (eventName, "activate"))
	return;

    activated = CompOption::getBoolOptionNamed (options, "active", false);
    if (!activated && filterInfo)
    {
	delete filterInfo;
	filterInfo = NULL;
    }

    if (gScreen)
	gScreen->glPaintOutputSetEnabled (this, activated);
    screen->handleEventSetEnabled (this, activated);

    matchApplied = false;
}

void
FilterInfo::drawText (const CompOutput *output,
		      const GLMatrix&  transform) const
{
    if (!textValid)
	return;

    if (output->id () == (unsigned int) ~0 || output == &outputDevice)
    {
	GLMatrix sTransform (transform);
	float    x, y, width, height;

	width  = text.getWidth ();
	height = text.getHeight ();

	x = floor (output->x1 () + (output->width () / 2) - (width / 2));
	y = floor (output->y1 () + (output->height () / 2) + (height / 2));

	sTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);

	text.draw (sTransform, x, y, 1.0f);
    }
}

bool
ScalefilterScreen::glPaintOutput (const GLScreenPaintAttrib& attrib,
				  const GLMatrix&            transform,
				  const CompRegion&          region,
				  CompOutput                 *output,
				  unsigned int               mask)
{
    bool status;

    status = gScreen->glPaintOutput (attrib, transform, region, output, mask);

    if (status && filterInfo)
	filterInfo->drawText (output, transform);

    return status;
}

bool
ScalefilterScreen::hasFilter () const
{
    if (matchApplied)
	return true;

    if (filterInfo && filterInfo->hasText ())
	return true;

    return false;
}

bool
ScalefilterWindow::setScaledPaintAttributes (GLWindowPaintAttrib& attrib)
{
    bool ret = sWindow->setScaledPaintAttributes (attrib);

    if (ScalefilterScreen::get (screen)->hasFilter ())
    {
	SCALE_SCREEN (screen);

	if (ret && !sWindow->hasSlot () && ss->getState () != ScaleScreen::In)
	{
	    ret = false;
	    attrib.opacity = 0;
	}
    }

    return ret;
}

void
ScalefilterScreen::optionChanged (CompOption *opt,
				  Options    num)
{
    switch (num)
    {
	case FontBold:
	case FontSize:
	case FontColor:
	case BackColor:
	    if (filterInfo)
		filterInfo->renderText ();
	    break;
	default:
	    break;
    }
}

ScalefilterScreen::ScalefilterScreen (CompScreen *s) :
    PluginClassHandler <ScalefilterScreen, CompScreen> (s),
    ScalefilterOptions (),
    xic (NULL),
    filterInfo (NULL),
    matchApplied (false),
    gScreen (GLScreen::get (s)),
    cScreen (CompositeScreen::get (s)),
    sScreen (ScaleScreen::get (s))
{
    xim = XOpenIM (s->dpy (), NULL, NULL, NULL);
    //if (xim)
	//xic = XCreateIC (xim, XNClientWindow, s->root (), XNInputStyle,
	//		 XIMPreeditNothing  | XIMStatusNothing, NULL);

    if (xic)
	setlocale (LC_CTYPE, "");

    optionSetFontBoldNotify (boost::bind (&ScalefilterScreen::optionChanged,
					  this, _1, _2));
    optionSetFontSizeNotify (boost::bind (&ScalefilterScreen::optionChanged,
					  this, _1, _2));
    optionSetFontColorNotify (boost::bind (&ScalefilterScreen::optionChanged,
					   this, _1, _2));
    optionSetBackColorNotify (boost::bind (&ScalefilterScreen::optionChanged,
					   this, _1, _2));

    ScreenInterface::setHandler (screen);
    GLScreenInterface::setHandler (gScreen);
    ScaleScreenInterface::setHandler (sScreen);

    screen->handleEventSetEnabled (this, false);
    if (gScreen)
	gScreen->glPaintOutputSetEnabled (this, false);
}

ScalefilterScreen::~ScalefilterScreen ()
{
    if (filterInfo)
	delete filterInfo;
    if (xic)
	XDestroyIC (xic);
    if (xim)
	XCloseIM (xim);
}

ScalefilterWindow::ScalefilterWindow (CompWindow *w) :
    PluginClassHandler <ScalefilterWindow, CompWindow> (w),
    window (w),
    sWindow (ScaleWindow::get (w))
{
    ScaleWindowInterface::setHandler (sWindow);
}

bool
ScalefilterPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("scale", COMPIZ_SCALE_ABI))
    {
	return false;
    }

    return true;
}
