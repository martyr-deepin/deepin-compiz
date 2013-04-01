/*
 * Animation plugin for compiz/beryl
 *
 * animation.c
 *
 * Copyright : (C) 2006 Erkin Bahceci
 * E-mail    : erkinbah@gmail.com
 *
 * Based on Wobbly and Minimize plugins by
 *           : David Reveman
 * E-mail    : davidr@novell.com>
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "private.h"

// =================  Option Related Functions  =================

AnimEvent win2AnimEventMap[WindowEventNum] =
{
    AnimEventOpen,
    AnimEventClose,
    AnimEventMinimize,
    AnimEventUnminimize,
    AnimEventShade,
    AnimEventShade,
    AnimEventFocus
};

AnimEvent
PrivateAnimScreen::getCorrespondingAnimEvent (AnimationOptions::Options optionId)
{
    switch (optionId)
    {
    case AnimationOptions::OpenOptions:
    case AnimationOptions::OpenEffects:
    case AnimationOptions::OpenRandomEffects:
	return AnimEventOpen;

    case AnimationOptions::CloseEffects:
    case AnimationOptions::CloseRandomEffects:
    case AnimationOptions::CloseOptions:
	return AnimEventClose;

    case AnimationOptions::MinimizeOptions:
    case AnimationOptions::MinimizeEffects:
    case AnimationOptions::MinimizeRandomEffects:
	return AnimEventMinimize;

    case AnimationOptions::UnminimizeOptions:
    case AnimationOptions::UnminimizeEffects:
    case AnimationOptions::UnminimizeRandomEffects:
	return AnimEventUnminimize;

    case AnimationOptions::FocusOptions:
    case AnimationOptions::FocusEffects:
	return AnimEventFocus;

    case AnimationOptions::ShadeOptions:
    case AnimationOptions::ShadeEffects:
    case AnimationOptions::ShadeRandomEffects:
	return AnimEventShade;

    default:
	return AnimEventNum;
    }
}

bool
IdValuePair::matchesPluginOption (ExtensionPluginInfo *testPluginInfo,
				  int testOptionId) const
{
    return (pluginInfo == testPluginInfo &&
	    optionId == testOptionId);
}

CompOption::Value &
AnimWindow::pluginOptVal (ExtensionPluginInfo *pluginInfo,
			  unsigned int optionId,
			  Animation *anim)
{
    PrivateAnimWindow *aw = priv;
    PrivateAnimScreen *as = aw->paScreen ();

    // Handle -1 case, used in Dodge for non-matching (stable) dodgers
    if (aw->curAnimSelectionRow () < 0)
    	return (*pluginInfo->effectOptions)[optionId].value ();

    OptionSet *os = as->getOptionSetForSelectedRow (aw, anim);

    IdValuePairVector::iterator it =
	find_if (os->pairs.begin (),
		 os->pairs.end (),
		 boost::bind (&IdValuePair::matchesPluginOption,
			      _1, pluginInfo, optionId));

    return (it == os->pairs.end () ?
	    (*pluginInfo->effectOptions)[optionId].value () :
	    (*it).value);
}

OptionSet *
PrivateAnimScreen::getOptionSetForSelectedRow (PrivateAnimWindow *aw,
					       Animation *anim)
{
    const AnimEvent  event = win2AnimEventMap[anim->curWindowEvent ()];
    OptionSets &eventOptionSets = mEventOptionSets[event];
    OptionSet  *setSelectedForRow = &eventOptionSets.sets[(unsigned int) aw->curAnimSelectionRow ()];

    return setSelectedForRow;
}

void
PrivateAnimScreen::updateOptionSet (OptionSet *os,
				    const char *optNamesValuesOrig)
{
    unsigned int len = strlen (optNamesValuesOrig);
    char *optNamesValues = (char *)calloc (len + 1, 1);

    // Find the first substring with no spaces in it
    sscanf (optNamesValuesOrig, " %s ", optNamesValues);
    if (!strlen (optNamesValues))
    {
	free (optNamesValues);
	return;
    }
    // Backup original, since strtok is destructive
    strcpy (optNamesValues, optNamesValuesOrig);

    char *name;
    char *nameTrimmed = (char *)calloc (len + 1, 1);
    char *valueStr = 0;
    const char *betweenPairs = ",";
    const char *betweenOptVal = "=";

    // Count number of pairs
    char *pairToken = (char *)optNamesValuesOrig; // TODO do with CompString
    unsigned int nPairs = 1;

    while ((pairToken = strchr (pairToken, betweenPairs[0])))
    {
	pairToken++; // skip delimiter
	nPairs++;
    }

    os->pairs.clear ();
    os->pairs.reserve (nPairs);

    // Tokenize pairs
    name = strtok (optNamesValues, betweenOptVal);

    int errorNo = -1;
    unsigned int i;
    for (i = 0; name && i < nPairs; i++)
    {
	errorNo = 0;
	if (strchr (name, betweenPairs[0])) // handle "a, b=4" case
	{
	    errorNo = 1;
	    break;
	}

	sscanf (name, " %s ", nameTrimmed);
	if (!strlen (nameTrimmed))
	{
	    errorNo = 2;
	    break;
	}
	valueStr = strtok (0, betweenPairs);
	if (!valueStr)
	{
	    errorNo = 3;
	    break;
	}

	// TODO: Fix: Convert to "pluginname:option_name" format
	// Warning: Assumes that option names in different extension plugins
	// will be different.
	bool matched = false;
	const ExtensionPluginInfo *chosenExtensionPlugin = NULL;
	CompOption *o = 0;
	int optId = -1;
	foreach (ExtensionPluginInfo *extensionPlugin, mExtensionPlugins)
	{
	    unsigned int nOptions = extensionPlugin->effectOptions->size ();
	    for (optId = (int)extensionPlugin->firstEffectOptionIndex;
		 optId < (int)nOptions; optId++)
	    {
		o = &(*extensionPlugin->effectOptions)[(unsigned)optId];

		if (strcasecmp (nameTrimmed, o->name ().c_str ()) == 0)
		{
		    matched = true;
		    chosenExtensionPlugin = extensionPlugin;
		    break;
		}
	    }
	    if (matched)
		break;
	}
	if (!matched)
	{
	    errorNo = 4;
	    break;
	}
	CompOption::Value v;

	os->pairs.push_back (IdValuePair ());
	IdValuePair *pair = &os->pairs[i];

	pair->pluginInfo = chosenExtensionPlugin;
	pair->optionId = optId;
	int valueRead = -1;
	switch (o->type ())
	{
	case CompOption::TypeBool:
	    int vb;
	    valueRead = sscanf (valueStr, " %d ", &vb);
	    if (valueRead)
		pair->value.set ((bool)vb);
	    break;
	case CompOption::TypeInt:
	{
	    int vi;
	    valueRead = sscanf (valueStr, " %d ", &vi);
	    if (valueRead > 0)
	    {
		if (o->rest ().inRange (vi))
		{
		    v.set (vi);
		    pair->value = v;
		}
		else
		    errorNo = 7;
	    }
	    break;
	}
	case CompOption::TypeFloat:
	{
	    float vf;
	    valueRead = sscanf (valueStr, " %f ", &vf);
	    if (valueRead > 0)
	    {
		if (o->rest ().inRange (vf))
		{
		    v.set (vf);
		    pair->value = v;
		}
		else
		    errorNo = 7;
	    }
	    break;
	}
	case CompOption::TypeString:
	{
	    v.set (CompString (valueStr));
	    valueRead = 1;
	    break;
	}
	case CompOption::TypeColor:
	{
	    unsigned short vc[4];
	    valueRead = sscanf (valueStr, " #%2hx%2hx%2hx%2hx ",
				&vc[0], &vc[1], &vc[2], &vc[3]);
	    if (valueRead == 4)
	    {
		CompOption::Value *pairVal = &pair->value;
		for (int j = 0; j < 4; j++)
		    vc[j] = vc[j] << 8 | vc[j];
		pairVal->set (vc);
	    }
	    else
		errorNo = 6;
	    break;
	}
	default:
	    break;
	}
	if (valueRead == 0)
	    errorNo = 6;
	if (errorNo > 0)
	    break;
	// If valueRead is -1 here, then it must be a
	// non-(int/float/string) option, which is not supported yet.
	// Such an option doesn't currently exist anyway.

	errorNo = -1;
	name = strtok (0, betweenOptVal);
    }

    if (i < nPairs)
    {
	switch (errorNo)
	{
	case -1:
	case 2:
	    compLogMessage ("animation", CompLogLevelError,
			    "Option name missing in \"%s\"",
			    optNamesValuesOrig);
	    break;
	case 1:
	case 3:
	    compLogMessage ("animation", CompLogLevelError,
			    "Option value missing in \"%s\"",
			    optNamesValuesOrig);
	    break;
	case 4:
	    //compLogMessage ("animation", CompLogLevelError,
	    //	    "Unknown option \"%s\" in \"%s\"",
	    //	    nameTrimmed, optNamesValuesOrig);
	    break;
	case 6:
	    compLogMessage ("animation", CompLogLevelError,
			    "Invalid value \"%s\" in \"%s\"",
			    valueStr, optNamesValuesOrig);
	    break;
	case 7:
	    compLogMessage ("animation", CompLogLevelError,
			    "Value \"%s\" out of range in \"%s\"",
			    valueStr, optNamesValuesOrig);
	    break;
	default:
	    break;
	}
	os->pairs.clear ();
    }
    free (optNamesValues);
    free (nameTrimmed);
}

void
PrivateAnimScreen::updateOptionSets (AnimEvent e)
{
    OptionSets *oss = &mEventOptionSets[e];
    CompOption::Value::Vector *listVal =
	&getOptions ()[(unsigned) customOptionOptionIds[e]].value ().list ();
    unsigned int n = listVal->size ();

    oss->sets.clear ();
    oss->sets.reserve (n);

    for (unsigned int i = 0; i < n; i++)
    {
	oss->sets.push_back (OptionSet ());
	updateOptionSet (&oss->sets[i], (*listVal)[i].s ().c_str ());
    }
}

