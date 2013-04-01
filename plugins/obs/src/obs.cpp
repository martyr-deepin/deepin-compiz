/*
 * Copyright © 2008 Danny Baumann
 *
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of
 * Danny Baumann not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior permission.
 * Danny Baumann makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without express or
 * implied warranty.
 *
 * DANNY BAUMANN DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL DENNIS KASPRZYK BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author: Danny Baumann <dannybaumann@web.de>
 */

#include "obs.h"

COMPIZ_PLUGIN_20090315 (obs, ObsPluginVTable);

const unsigned short MODIFIER_OPACITY    = 0;
const unsigned short MODIFIER_SATURATION = 1;
const unsigned short MODIFIER_BRIGHTNESS = 2;

void
ObsWindow::changePaintModifier (unsigned int modifier,
				int          direction)
{
    int value, step;

    if (window->overrideRedirect ())
	return;

    if (modifier == MODIFIER_OPACITY &&
	(window->type () & CompWindowTypeDesktopMask))
    {
	return;
    }

    step   = oScreen->stepOptions[modifier]->value ().i ();
    value  = customFactor[modifier] + (step * direction);

    value = MAX (MIN (value, 100), step);

    if (value != customFactor[modifier])
    {
	customFactor[modifier] = value;
	modifierChanged (modifier);
    }
}

void
ObsWindow::updatePaintModifier (unsigned int modifier)
{
    int lastFactor;

    lastFactor = customFactor[modifier];

    if (modifier == MODIFIER_OPACITY &&
	(window->type ()& CompWindowTypeDesktopMask))
    {
	customFactor[modifier] = 100;
	matchFactor[modifier]  = 100;
    }
    else
    {
	int                       i, min, lastMatchFactor;
	CompOption::Value::Vector *matches, *values;

	matches = &oScreen->matchOptions[modifier]->value ().list ();
	values  = &oScreen->valueOptions[modifier]->value ().list ();
	min     = MIN (matches->size (), values->size ());

	lastMatchFactor       = matchFactor[modifier];
	matchFactor[modifier] = 100;

	for (i = 0; i < min; i++)
	{
	    if (matches->at (i).match ().evaluate (window))
	    {
		matchFactor[modifier] = values->at (i).i ();
		break;
	    }
	}

	if (customFactor[modifier] == lastMatchFactor)
	    customFactor[modifier] = matchFactor[modifier];
    }

    if (customFactor[modifier] != lastFactor)
	modifierChanged (modifier);
}

void
ObsWindow::modifierChanged (unsigned int modifier)
{
    bool hasCustom = false;

    if (modifier == MODIFIER_OPACITY)
	gWindow->glPaintSetEnabled (this, customFactor[modifier] != 100);

    for (unsigned int i = 0; i < MODIFIER_COUNT; i++)
	if (customFactor[i] != 100)
	{
	    hasCustom = true;
	    break;
	}

    gWindow->glDrawSetEnabled (this, hasCustom);
    cWindow->addDamage ();
}

static bool
alterPaintModifier (CompAction          *action,
		    CompAction::State   state,
		    CompOption::Vector& options,
		    unsigned int        modifier,
		    int                 direction)
{
    CompWindow *w;
    Window     xid;

    xid = CompOption::getIntOptionNamed (options, "window", 0);
    w   = screen->findTopLevelWindow (xid);

    if (w)
	ObsWindow::get (w)->changePaintModifier (modifier, direction);

    return true;
}

bool
ObsWindow::glPaint (const GLWindowPaintAttrib& attrib,
		    const GLMatrix&            transform,
		    const CompRegion&          region,
		    unsigned int               mask)
{
    mask |= PAINT_WINDOW_TRANSLUCENT_MASK;

    return gWindow->glPaint (attrib, transform, region, mask);
}

/* Note: Normally plugins should wrap into glPaintWindow to modify opacity,
	 brightness and saturation. As some plugins bypass glPaintWindow when
	 they draw windows and our custom values always need to be applied,
	 we wrap into glDrawWindow here */

bool
ObsWindow::glDraw (const GLMatrix            &transform,
		   const GLWindowPaintAttrib &attrib,
		   const CompRegion          &region,
		   unsigned int        mask)
{
    GLWindowPaintAttrib wAttrib (attrib);
    int factor;

    factor = customFactor[MODIFIER_OPACITY];
    if (factor != 100)
    {
	wAttrib.opacity = factor * wAttrib.opacity / 100;
	mask |= PAINT_WINDOW_TRANSLUCENT_MASK;
    }

    factor = customFactor[MODIFIER_BRIGHTNESS];
    if (factor != 100)
	wAttrib.brightness = factor * wAttrib.brightness / 100;

    factor = customFactor[MODIFIER_SATURATION];
    if (factor != 100)
	wAttrib.saturation = factor * wAttrib.saturation / 100;

    return gWindow->glDraw (transform, wAttrib, region, mask);
}

void
ObsScreen::matchExpHandlerChanged ()
{
    screen->matchExpHandlerChanged ();

    /* match options are up to date after the call to matchExpHandlerChanged */
    foreach (CompWindow *w, screen->windows ())
    {
	for (unsigned int i = 0; i < MODIFIER_COUNT; i++)
	    ObsWindow::get (w)->updatePaintModifier (i);
    }
}

void
ObsScreen::matchPropertyChanged (CompWindow  *w)
{
    unsigned int i;

    for (i = 0; i < MODIFIER_COUNT; i++)
	ObsWindow::get (w)->updatePaintModifier (i);

    screen->matchPropertyChanged (w);
}

#define MODIFIERBIND(modifier, direction) \
    boost::bind (alterPaintModifier, _1, _2, _3, modifier, direction)

ObsScreen::ObsScreen (CompScreen *s) :
    PluginClassHandler<ObsScreen, CompScreen> (s)
{
    unsigned int mod;

    ScreenInterface::setHandler (screen);

    mod = MODIFIER_OPACITY;
    stepOptions[mod]  = &mOptions[ObsOptions::OpacityStep];
    matchOptions[mod] = &mOptions[ObsOptions::OpacityMatches];
    valueOptions[mod] = &mOptions[ObsOptions::OpacityValues];

    mod = MODIFIER_SATURATION;
    stepOptions[mod]  = &mOptions[ObsOptions::SaturationStep];
    matchOptions[mod] = &mOptions[ObsOptions::SaturationMatches];
    valueOptions[mod] = &mOptions[ObsOptions::SaturationValues];

    mod = MODIFIER_BRIGHTNESS;
    stepOptions[mod]  = &mOptions[ObsOptions::BrightnessStep];
    matchOptions[mod] = &mOptions[ObsOptions::BrightnessMatches];
    valueOptions[mod] = &mOptions[ObsOptions::BrightnessValues];

    optionSetOpacityIncreaseKeyInitiate (MODIFIERBIND (MODIFIER_OPACITY, 1));
    optionSetOpacityIncreaseButtonInitiate (MODIFIERBIND (MODIFIER_OPACITY, 1));
    optionSetOpacityDecreaseKeyInitiate (MODIFIERBIND (MODIFIER_OPACITY, -1));
    optionSetOpacityDecreaseButtonInitiate (MODIFIERBIND (MODIFIER_OPACITY, -1));

    optionSetSaturationIncreaseKeyInitiate (MODIFIERBIND (MODIFIER_SATURATION, 1));
    optionSetSaturationIncreaseButtonInitiate (MODIFIERBIND (MODIFIER_SATURATION, 1));
    optionSetSaturationDecreaseKeyInitiate (MODIFIERBIND (MODIFIER_SATURATION, -1));
    optionSetSaturationDecreaseButtonInitiate (MODIFIERBIND (MODIFIER_SATURATION, -1));

    optionSetBrightnessIncreaseKeyInitiate (MODIFIERBIND (MODIFIER_BRIGHTNESS, 1));
    optionSetBrightnessIncreaseButtonInitiate (MODIFIERBIND (MODIFIER_BRIGHTNESS, 1));
    optionSetBrightnessDecreaseKeyInitiate (MODIFIERBIND (MODIFIER_BRIGHTNESS, -1));
    optionSetBrightnessDecreaseButtonInitiate (MODIFIERBIND (MODIFIER_BRIGHTNESS, -1));
}

bool
ObsWindow::updateTimeout ()
{
    int i;
    
    for (i = 0; i < MODIFIER_COUNT; i++)
	updatePaintModifier (i);

    return false;
}

bool
ObsScreen::setOption (const CompString  &name,
		      CompOption::Value &value)
{
    CompOption   *o;
    unsigned int i;

    if (!ObsOptions::setOption (name, value))
	return false;

    o = CompOption::findOption (getOptions (), name, NULL);
    if (!o)
        return false;

    for (i = 0; i < MODIFIER_COUNT; i++)
    {
	if (o == matchOptions[i] || o == valueOptions[i])
	{
	    foreach (CompWindow *w, screen->windows ())
		ObsWindow::get (w)->updatePaintModifier (i);
	}
    }

    return true;
}

ObsWindow::ObsWindow (CompWindow *w) :
    PluginClassHandler<ObsWindow, CompWindow> (w),
    window (w),
    cWindow (CompositeWindow::get (w)),
    gWindow (GLWindow::get (w)),
    oScreen (ObsScreen::get (screen))
{
    GLWindowInterface::setHandler (gWindow, false);

    for (unsigned int i = 0; i < MODIFIER_COUNT; i++)
    {
	customFactor[i] = 100;
	matchFactor[i]  = 100;

	/* defer initializing the factors from window matches as match evalution
	 * means wrapped function calls */
	updateHandle.setTimes (0, 0);
	updateHandle.setCallback (boost::bind (&ObsWindow::updateTimeout, this));
	updateHandle.start ();
    }
}

ObsWindow::~ObsWindow ()
{
    updateHandle.stop ();
}

bool
ObsPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
    {
	 return false;
    }

    return true;
}
