/*
 *
 * Compiz workspace name display plugin
 *
 * workspacenames.cpp
 *
 * Copyright : (C) 2008 by Danny Baumann
 * E-mail    : maniac@compiz-fusion.org
 * 
 * Ported to Compiz 0.9.x
 * Copyright : (c) 2010 Scott Moreau <oreaus@gmail.com>
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
 */

#include "workspacenames.h"

CompString
WSNamesScreen::getCurrentWSName ()
{
    int		currentVp;
    int		listSize, i;
    CompString	ret;
    CompOption::Value::Vector names;
    CompOption::Value::Vector vpNumbers;

    vpNumbers = optionGetViewports ();
    names     = optionGetNames ();

    currentVp = screen->vp ().y () * screen->vpSize ().width () +
		screen->vp ().x () + 1;
    listSize  = MIN (vpNumbers.size (), names.size ());

    for (i = 0; i < listSize; i++)
	if (vpNumbers[i].i () == currentVp)
	    return names[i].s ();

    return ret;
}

void
WSNamesScreen::renderNameText ()
{
    CompText::Attrib attrib;
    CompString	     name;

    textData.clear ();

    name = getCurrentWSName ();

    if (name.empty ())
	return;

    /* 75% of the output device as maximum width */
    attrib.maxWidth  = screen->getCurrentOutputExtents ().width () * 3 / 4;
    attrib.maxHeight = 100;

    attrib.family = "Sans";
    attrib.size = optionGetTextFontSize ();

    attrib.color[0] = optionGetFontColorRed ();
    attrib.color[1] = optionGetFontColorGreen ();
    attrib.color[2] = optionGetFontColorBlue ();
    attrib.color[3] = optionGetFontColorAlpha ();

    attrib.flags = CompText::WithBackground | CompText::Ellipsized;
    if (optionGetBoldText ())
	attrib.flags |= CompText::StyleBold;

    attrib.bgHMargin = 15;
    attrib.bgVMargin = 15;
    attrib.bgColor[0] = optionGetBackColorRed ();
    attrib.bgColor[1] = optionGetBackColorGreen ();
    attrib.bgColor[2] = optionGetBackColorBlue ();
    attrib.bgColor[3] = optionGetBackColorAlpha ();

    textData.renderText (name, attrib);
}

void
WSNamesScreen::drawText (const GLMatrix &matrix)
{
    GLfloat  alpha;
    float    x, y, border = 10.0f;
    CompRect oe = screen->getCurrentOutputExtents ();

    x = oe.centerX () - textData.getWidth () / 2;

    /* assign y (for the lower corner!) according to the setting */
    switch (optionGetTextPlacement ())
    {
	case WorkspacenamesOptions::TextPlacementCenteredOnScreen:
	    y = oe.centerY () + textData.getHeight () / 2;
	    break;
	case WorkspacenamesOptions::TextPlacementTopOfScreen:
	case WorkspacenamesOptions::TextPlacementBottomOfScreen:
	    {
		CompRect workArea = screen->currentOutputDev ().workArea ();

		if (optionGetTextPlacement () ==
		    WorkspacenamesOptions::TextPlacementTopOfScreen)
    		    y = oe.y1 () + workArea.y () +
			(2 * border) + textData.getHeight ();
		else
		    y = oe.y1 () + workArea.y () +
			workArea.height () - (2 * border);
	    }
	    break;
	default:
	    return;
	    break;
    }

    if (timer)
	alpha = timer / (optionGetFadeTime () * 1000.0f);
    else
	alpha = 1.0f;

    textData.draw (matrix, floor (x), floor (y), alpha);
}

bool
WSNamesScreen::glPaintOutput (const GLScreenPaintAttrib	&attrib,
			      const GLMatrix		&transform,
			      const CompRegion		&region,
			      CompOutput		*output,
			      unsigned int		mask)
{
    bool status;

    status = gScreen->glPaintOutput (attrib, transform, region, output, mask);

    if (textData.getWidth () && textData.getHeight ())
    {
	GLMatrix sTransform (transform);

	sTransform.toScreenSpace (output, -DEFAULT_Z_CAMERA);

	drawText (sTransform);
    }

    return status;
}

void
WSNamesScreen::preparePaint (int msSinceLastPaint)
{
    if (timer)
    {
	timer -= msSinceLastPaint;
	timer = MAX (timer, 0);

	if (!timer)
	    textData.clear ();
    }

    cScreen->preparePaint (msSinceLastPaint);
}

void
WSNamesScreen::donePaint ()
{
    /* FIXME: better only damage paint region */
    if (timer)
	cScreen->damageScreen ();

    cScreen->donePaint ();
}

bool
WSNamesScreen::hideTimeout ()
{
    timer = optionGetFadeTime () * 1000;
    if (!timer)
	textData.clear ();

    cScreen->damageScreen ();

    timeoutHandle.stop ();

    return false;
}

void
WSNamesScreen::handleEvent (XEvent *event)
{
    screen->handleEvent (event);

    if (event->type != PropertyNotify)
	return;

    if (event->xproperty.atom == Atoms::desktopViewport)
    {
	int timeout = optionGetDisplayTime () * 1000;

	timer = 0;
	if (timeoutHandle.active ())
	    timeoutHandle.stop ();

	renderNameText ();
	timeoutHandle.start (timeout, timeout + 200);

	cScreen->damageScreen ();
    }
}

WSNamesScreen::WSNamesScreen (CompScreen *screen) :
    PluginClassHandler <WSNamesScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    timer (0)
{
    ScreenInterface::setHandler (screen, true);
    CompositeScreenInterface::setHandler (cScreen, true);
    GLScreenInterface::setHandler (gScreen, true);

    timeoutHandle.start (boost::bind (&WSNamesScreen::hideTimeout, this),
			 0, 0);
}

WSNamesScreen::~WSNamesScreen ()
{
}

bool
WorkspacenamesPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	return false;

    if (!CompPlugin::checkPluginABI ("text", COMPIZ_TEXT_ABI))
	compLogMessage ("workspacenames", CompLogLevelWarn,
			"No compatible text plugin loaded");
    return true;
}
