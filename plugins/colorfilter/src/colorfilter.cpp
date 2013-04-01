/*
 * Compiz/Fusion color filtering plugin
 *
 * Author : Guillaume Seguin
 * Email : guillaume@segu.in
 *
 * Copyright (c) 2007 Guillaume Seguin <guillaume@segu.in>
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
#include "colorfilter.h"
#include "parser.h"

COMPIZ_PLUGIN_20090315 (colorfilter, ColorfilterPluginVTable);

/*
 * Find fragment function by id (imported from compiz-core/src/fragment.c)
 */
ColorfilterFunction *
ColorfilterScreen::findFragmentFunction (int id)
{
    foreach (ColorfilterFunction *function, filtersFunctions)
    {
	if (function->id == (unsigned int) id)
	    return function;
    }

    return NULL;
}

/* Actions handling functions ----------------------------------------------- */

void
toggleWindowFunctions (ColorfilterWindow *cfw, bool enabled)
{
    cfw->gWindow->glDrawTextureSetEnabled (cfw, enabled);
}

/*
 * Toggle filtering for a specific window
 */
void
ColorfilterWindow::toggle ()
{
    FILTER_SCREEN (screen);

    /* Toggle window filtering flag */
    isFiltered = !isFiltered;

    /* Check exclude list */
    if (cfs->optionGetExcludeMatch ().evaluate (window))
	isFiltered = false;

    /* Ensure window is going to be repainted */
    cWindow->addDamage ();
    toggleWindowFunctions (this, isFiltered);
}

/*
 * Toggle filtering for the whole screen
 */
void
ColorfilterScreen::toggle ()
{
    /* Toggle screen filtering flag */
    isFiltered = !isFiltered;

    /* Toggle filtering for every window */
    foreach (CompWindow *w, screen->windows ())
	if (w)
	    ColorfilterWindow::get (w)->toggle ();
}

/*
 * Switch current filter
 */
void
ColorfilterScreen::switchFilter ()
{
    GLFragment::FunctionId id;
    ColorfilterFunction *function;

    /* % (count + 1) because of the cumulative filters mode */
    currentFilter = (currentFilter + 1) % (filtersFunctions.size () + 1);
    if (currentFilter == 0)
	compLogMessage ("colorfilter", CompLogLevelInfo,
			"Cumulative filters mode");
    else
    {
	id = filtersFunctions.at (currentFilter - 1)->id;
	if (id)
	{
	    function = findFragmentFunction (id);
	    compLogMessage ("colorfilter", CompLogLevelInfo,
			    "Single filter mode (using %s filter)",
			    function->name.c_str ());
	}
	else
	{
	    compLogMessage ("colorfilter", CompLogLevelInfo,
			    "Single filter mode (filter loading failure)");
	}
    }

    /* Damage currently filtered windows */
    foreach (CompWindow *w, screen->windows ())
    {
	FILTER_WINDOW (w);
	if (cfw->isFiltered)
	    cfw->cWindow->addDamage ();
    }
}

/*
 * Window filtering toggle action
 */
bool
ColorfilterScreen::toggleWindow (CompAction         *action,
				 CompAction::State  state,
				 CompOption::Vector options)
{

    CompWindow *w =
      screen->findWindow (CompOption::getIntOptionNamed (options, "window", 0));

    if (w && GL::fragmentProgram)
	ColorfilterWindow::get (w)->toggle ();

    return true;
}

/*
 * Screen filtering toggle action
 */
bool
ColorfilterScreen::toggleScreen (CompAction         *action,
				 CompAction::State  state,
				 CompOption::Vector options)
{
    screen->findWindow (CompOption::getIntOptionNamed (options, "root", 0));

    if (GL::fragmentProgram)
	toggle ();

    return true;
}

/*
 * Filter switching action
 */
bool
ColorfilterScreen::filterSwitch (CompAction         *action,
				 CompAction::State  state,
				 CompOption::Vector options)
{
    if (GL::fragmentProgram)
	switchFilter ();

    return true;
}

/* Filters handling functions ----------------------------------------------- */

/*
 * Free filters resources if any
 */
void
ColorfilterScreen::unloadFilters ()
{
    if (!filtersFunctions.empty ())
    {
	/* Destroy loaded filters one by one */
	while (!filtersFunctions.empty ())
	{
	    ColorfilterFunction *function = filtersFunctions.back ();
	    if (function->id)
		GLFragment::destroyFragmentFunction (function->id);

	    delete function;

	    filtersFunctions.pop_back ();
	}
	/* Reset current filter */
	currentFilter = 0;
    }
}

/*
 * Load filters from a list of files for current screen
 */
int
ColorfilterScreen::loadFilters (GLTexture *texture)
{
    int target, loaded, count;
    GLFragment::FunctionId function;
    CompString name, file;
    CompOption::Value::Vector filters;
    FragmentParser *parser = new FragmentParser ();
    ColorfilterFunction *func;

    /* Free previously loaded filters and malloc */
    unloadFilters ();

    filtersLoaded = true;

    /* Fetch filters filenames */
    filters = optionGetFilters ();
    count = filters.size ();

    //filtersFunctions.resize (count);

    /* The texture target that will be used for some ops */
    if (texture->target () == GL_TEXTURE_2D)
	target = COMP_FETCH_TARGET_2D;
    else
	target = COMP_FETCH_TARGET_RECT;

    /* Load each filter one by one */
    loaded = 0;
    for (int i = 0; i < count; i++)
    {
	name = CompString (basename (filters.at (i).s ().c_str ()));
	file = filters.at (i).s ();
	if (name.empty ())
	{
	    name.clear ();
	    continue;
	}

	compLogMessage ("colorfilter", CompLogLevelInfo,
			"Loading filter %s (item %s).", name.c_str (),
			file.c_str ());
	function = parser->loadFragmentProgram (file, name, target);

	func = new ColorfilterFunction ();
	if (!func)
	    continue;

	func->name = name;
	func->id   = function;

	filtersFunctions.push_back (func);
	if (func && function)
	    loaded++;
    }

    /* Warn if there was at least one loading failure */
    if (loaded < count)
	compLogMessage ("colorfilter", CompLogLevelWarn,
			"Tried to load %d filter(s), %d succeeded.",
			count, loaded);

    /* Damage currently filtered windows */
    foreach (CompWindow *w, screen->windows ())
    {
	FILTER_WINDOW (w);
	if (cfw->isFiltered)
	    cfw->cWindow->addDamage (w);
    }

    delete parser;

    return loaded;
}

/*
 * Wrapper that enables filters if the window is filtered
 */
void
ColorfilterWindow::glDrawTexture (GLTexture 		   *texture,
				  GLFragment::Attrib &attrib,
				  unsigned int		   mask)
{
    GLFragment::FunctionId function;

    FILTER_SCREEN (screen);

    bool shouldFilter = isFiltered;

    foreach (GLTexture *tex, gWindow->textures ())
    {
	if (tex->name () != texture->name ())
	    shouldFilter = false;
    }

    /* We are filtering a decoration */
    if ((cfs->optionGetFilterDecorations () &&
	isFiltered &&
	!cfs->filtersFunctions.empty ()))
	shouldFilter = true;

    /* Check if filters have to be loaded and load them if so
     * Maybe should this check be done only if a filter is going to be applied
     * for this texture? */
    if (!cfs->filtersLoaded)
	cfs->loadFilters (texture);

    /* Filter texture if :
     *   o GL_ARB_fragment_program available
     *   o Filters are loaded
     *   o Texture's window is filtered */
    /* Note : if required, filter window contents only and not decorations
     * (use that w->texture->name != texture->name for decorations) */
    if (shouldFilter) // ???
    {
	GLFragment::Attrib fa = attrib;
	if (cfs->currentFilter == 0) /* Cumulative filters mode */
	{
	    /* Enable each filter one by one */
	    foreach (ColorfilterFunction *func, cfs->filtersFunctions)
	    {
		function = func->id;
		if (function)
		    fa.addFunction (function);
	    }
	}
	/* Single filter mode */
	else if ((unsigned int) cfs->currentFilter <= cfs->filtersFunctions.size ())
	{
	    /* Enable the currently selected filter if possible (i.e. if it
	     * was successfully loaded) */
	    function = cfs->filtersFunctions.at (cfs->currentFilter - 1)->id;
	    if (function)
		fa.addFunction (function);
	}
	gWindow->glDrawTexture (texture, fa, mask);
    }
    else /* Not filtering */
    {
	gWindow->glDrawTexture (texture, attrib, mask);
    }
}

/*
 * Filter windows when they are open if they match the filtering rules
 */
void
ColorfilterScreen::windowAdd (CompWindow *w)
{
    FILTER_WINDOW (w);

    /* cfw->isFiltered is initialized to false in InitWindow, so we only
       have to toggle it to true if necessary */
    if (cfw->isFiltered && optionGetFilterMatch ().evaluate (w))
	cfw->toggle ();
}

/* Internal stuff ----------------------------------------------------------- */

/*
 * Filtering match settings update callback
 */
void
ColorfilterScreen::matchsChanged (CompOption		      *opt,
				  ColorfilterOptions::Options num)
{
    /* Re-check every window against new match settings */
    foreach (CompWindow *w, screen->windows ())
    {
	FILTER_WINDOW (w);
	if (optionGetFilterMatch ().evaluate (w) &&
	    isFiltered && !cfw->isFiltered)
	{
	    cfw->toggle ();
	}
    }
}

/*
 * Exclude match settings update callback
 */
void
ColorfilterScreen::excludeMatchsChanged (CompOption		      *opt,
					 ColorfilterOptions::Options num)
{
    /* Re-check every window against new match settings */
    foreach (CompWindow *w, screen->windows ())
    {
	bool isExcluded;

	FILTER_WINDOW (w);

	isExcluded = optionGetExcludeMatch ().evaluate (w);
	if (isExcluded && cfw->isFiltered)
	    cfw->toggle ();
	else if (!isExcluded && isFiltered && !cfw->isFiltered)
	    cfw->toggle ();
    }

}

/*
 * Filters list setting update callback
 */
void
ColorfilterScreen::filtersChanged (CompOption		       *opt,
				   ColorfilterOptions::Options num)
{
    /* Just set the filtersLoaded boolean to false, unloadFilters will be
     * called in loadFilters */
    filtersLoaded = false;
}

/*
 * Damage decorations after the "Filter Decorations" setting got changed
 */
void
ColorfilterScreen::damageDecorations (CompOption		  *opt,
				      ColorfilterOptions::Options num)
{
    cScreen->damageScreen ();
}

ColorfilterScreen::ColorfilterScreen (CompScreen *screen) :
    PluginClassHandler <ColorfilterScreen, CompScreen> (screen),
    cScreen (CompositeScreen::get (screen)),
    gScreen (GLScreen::get (screen)),
    isFiltered (false),
    currentFilter (0),
    filtersLoaded (false)
{
    optionSetToggleWindowKeyInitiate (boost::bind (
				&ColorfilterScreen::toggleWindow, this, _1, _2,
				_3));
    optionSetToggleScreenKeyInitiate (boost::bind (
				&ColorfilterScreen::toggleScreen, this, _1, _2,
				_3));
    optionSetSwitchFilterKeyInitiate (boost::bind (
				&ColorfilterScreen::filterSwitch, this, _1, _2,
				_3));


    optionSetFilterMatchNotify (boost::bind (
				&ColorfilterScreen::matchsChanged, this, _1, _2));
    optionSetExcludeMatchNotify (boost::bind (
				&ColorfilterScreen::excludeMatchsChanged, this,
				_1, _2));
    optionSetFiltersNotify (boost::bind (
				&ColorfilterScreen::filtersChanged, this, _1,
				_2));
    optionSetFilterDecorationsNotify (boost::bind (
				&ColorfilterScreen::damageDecorations, this, _1,
				_2));
};

ColorfilterScreen::~ColorfilterScreen ()
{
    writeSerializedData ();
    unloadFilters ();
}

ColorfilterWindow::ColorfilterWindow (CompWindow *window) :
    PluginClassHandler <ColorfilterWindow, CompWindow> (window),
    window (window),
    cWindow (CompositeWindow::get (window)),
    gWindow (GLWindow::get (window)),
    isFiltered (false)
{
    GLWindowInterface::setHandler (gWindow, false);
}

ColorfilterWindow::~ColorfilterWindow ()
{
    writeSerializedData ();
}

bool
ColorfilterPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
	!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
    {
	compLogMessage ("colorfilter", CompLogLevelError, "Required plugins "\
			"'opengl' and 'composite' are not loaded");
	return false;
    }

    if (!GL::fragmentProgram)
	compLogMessage ("colorfilter", CompLogLevelWarn, "No fragment" \
			"support, the plugin will continue to load but nothing"\
			"will happen");

    return true;
}
