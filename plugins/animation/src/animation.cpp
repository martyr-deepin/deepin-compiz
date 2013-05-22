/**
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
 * Airplane added by : Carlo Palma
 * E-mail            : carlopalma@salug.it
 * Based on code originally written by Mark J. Kilgard
 *
 * Beam-Up added by : Florencio Guimaraes
 * E-mail           : florencio@nexcorp.com.br
 *
 * Fold and Skewer added by : Tomasz Kolodziejski
 * E-mail                   : tkolodziejski@gmail.com
 *
 * Hexagon tessellator added by : Mike Slegeir
 * E-mail                       : mikeslegeir@mail.utexas.edu>
 *
 * Particle system added by : (C) 2006 Dennis Kasprzyk
 * E-mail                   : onestone@beryl-project.org
 *
 * Ported to GLES by : Travis Watkins
 *                     (C) 2011 Linaro Limited
 * E-mail            : travis.watkins@linaro.org
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
 **/

/*
 * TODO:
 *
 * - Custom bounding box update function for Airplane
 *
 * - Auto direction option: Close in opposite direction of opening
 * - Proper side surface normals for lighting
 * - decoration shadows
 *   - shadow quad generation
 *   - shadow texture coords (from clip tex. matrices)
 *   - draw shadows
 *   - fade in shadows
 *
 * - Voronoi tessellation
 * - Brick tessellation
 * - Triangle tessellation
 * - Hexagonal tessellation
 *
 * Effects:
 * - Circular action for tornado type fx
 * - Tornado 3D (especially for minimize)
 * - Helix 3D (hor. strips descend while they rotate and fade in)
 * - Glass breaking 3D
 *   - Gaussian distr. points (for gradually increasing polygon size
 *                           starting from center or near mouse pointer)
 *   - Drawing cracks
 *   - Gradual cracking
 *
 * - fix slowness during transparent cube with <100 opacity
 * - fix occasional wrong side color in some windows
 * - fix on top windows and panels
 *   (These two only matter for viewing during Rotate Cube.
 *    All windows should be painted with depth test on
 *    like 3d-plugin does)
 * - play better with rotate (fix cube face drawn on top of polygons
 *   after 45 deg. rotation)
 *
 */

#include <core/atoms.h>
#include <core/core.h>
#include <opengl/opengl.h>
#include <sys/time.h>
#include <assert.h>
#include "private.h"

using namespace compiz::core;

class AnimPluginVTable :
    public CompPlugin::VTableForScreenAndWindow<AnimScreen, AnimWindow>
{
public:
    bool init ();
    void fini ();
};

COMPIZ_PLUGIN_20090315 (animation, AnimPluginVTable);

static const unsigned short FAKE_ICON_SIZE = 4;

const unsigned short LAST_ANIM_DIRECTION = 5;

const unsigned short NUM_EFFECTS = 16;

const char *eventNames[AnimEventNum] =
{"Open", "Close", "Minimize", "Unminimize", "Shade", "Focus"};

int chosenEffectOptionIds[AnimEventNum] =
{
    AnimationOptions::OpenEffects,
    AnimationOptions::CloseEffects,
    AnimationOptions::MinimizeEffects,
    AnimationOptions::UnminimizeEffects,
    AnimationOptions::ShadeEffects,
    AnimationOptions::FocusEffects,
};

int randomEffectOptionIds[AnimEventNum] =
{
    AnimationOptions::OpenRandomEffects,
    AnimationOptions::CloseRandomEffects,
    AnimationOptions::MinimizeRandomEffects,
    AnimationOptions::UnminimizeRandomEffects,
    AnimationOptions::ShadeRandomEffects,
    -1
};

int customOptionOptionIds[AnimEventNum] =
{
    AnimationOptions::OpenOptions,
    AnimationOptions::CloseOptions,
    AnimationOptions::MinimizeOptions,
    AnimationOptions::UnminimizeOptions,
    AnimationOptions::ShadeOptions,
    AnimationOptions::FocusOptions
};

int matchOptionIds[AnimEventNum] =
{
    AnimationOptions::OpenMatches,
    AnimationOptions::CloseMatches,
    AnimationOptions::MinimizeMatches,
    AnimationOptions::UnminimizeMatches,
    AnimationOptions::ShadeMatches,
    AnimationOptions::FocusMatches
};

int durationOptionIds[AnimEventNum] =
{
    AnimationOptions::OpenDurations,
    AnimationOptions::CloseDurations,
    AnimationOptions::MinimizeDurations,
    AnimationOptions::UnminimizeDurations,
    AnimationOptions::ShadeDurations,
    AnimationOptions::FocusDurations
};

// Bind each effect in the list of chosen effects for every event, to the
// corresponding animation effect (i.e. effect with that name) if it is
// provided by a plugin, otherwise set it to None.
void
PrivateAnimScreen::updateEventEffects (AnimEvent e,
				       bool forRandom,
				       bool callPost)
{
    CompOption::Value::Vector *listVal;
    EffectSet *effectSet;
    if (forRandom)
    {
	listVal = &getOptions ()[(unsigned)randomEffectOptionIds[e]].value ().
	    list ();
	effectSet = &mRandomEffects[e];
    }
    else
    {
	listVal = &getOptions ()[(unsigned)chosenEffectOptionIds[e]].value ().
	    list ();
	effectSet = &mEventEffects[e];
    }
    unsigned int n = listVal->size ();

    effectSet->effects.clear ();
    effectSet->effects.reserve (n);

    AnimEffectVector &eventEffectsAllowed = mEventEffectsAllowed[e];

    for (unsigned int r = 0; r < n; r++) // for each row
    {
	const CompString &animName = (*listVal)[r].s ();

	// Find the animation effect with matching name
	AnimEffectVector::iterator it =
	    find_if (eventEffectsAllowed.begin (),
		     eventEffectsAllowed.end (),
		     boost::bind (&AnimEffectInfo::matchesEffectName,
				  _1, animName));

	effectSet->effects.push_back (it == eventEffectsAllowed.end () ?
				      AnimEffectNone : *it);
    }

    if (callPost)
    {
    	foreach (ExtensionPluginInfo *extPlugin, mExtensionPlugins)
	    extPlugin->postUpdateEventEffects (e, forRandom);
    }
}

void
PrivateAnimScreen::updateAllEventEffects ()
{
    // for each anim event
    for (int e = 0; e < AnimEventNum; e++)
	updateEventEffects ((AnimEvent)e, false);

    // for each anim event except focus
    for (int e = 0; e < AnimEventNum - 1; e++)
	updateEventEffects ((AnimEvent)e, true);
}

bool
PrivateAnimScreen::isAnimEffectInList (AnimEffect theEffect,
				       EffectSet &effectList)
{
    for (unsigned int i = 0; i < effectList.effects.size (); i++)
	if (effectList.effects[i] == theEffect)
	    return true;
    return false;
}

bool
PrivateAnimScreen::isAnimEffectPossibleForEvent (AnimEffect theEffect,
						 AnimEvent event)
{
    // Check all rows to see if the effect is chosen there
    unsigned int nRows = mEventEffects[event].effects.size ();
    for (unsigned int i = 0; i < nRows; i++)
    {
	AnimEffect chosenEffect = mEventEffects[event].effects[i];
	// if chosen directly
	if (chosenEffect == theEffect)
	    return true;
	// if chosen in random pool
	if (mRandomEffects[event].effects.size () &&
	    chosenEffect == AnimEffectRandom &&
	    isAnimEffectInList (theEffect, mRandomEffects[event]))
	    return true;
    }
    return false;
}

bool
PrivateAnimScreen::isAnimEffectPossible (AnimEffect theEffect)
{
    for (int e = 0; e < AnimEventNum; e++)
	if (isAnimEffectPossibleForEvent (theEffect, (AnimEvent)e))
	    return true;
    return false;
}

bool
PrivateAnimScreen::isRestackAnimPossible ()
{
    // Check all rows to see if the chosen effect is a restack animation
    unsigned int nRows = mEventEffects[AnimEventFocus].effects.size ();

    for (unsigned int i = 0; i < nRows; i++)
    {
	AnimEffect chosenEffect = mEventEffects[(unsigned)AnimEventFocus].
	    effects[i];
	if (chosenEffect->isRestackAnim)
	    return true;
    }
    return false;
}

bool
AnimScreen::isRestackAnimPossible ()
{
    return priv->isRestackAnimPossible ();
}

// Extension functions

void
AnimScreen::addExtension (ExtensionPluginInfo *extensionPluginInfo)
{
    priv->addExtension (extensionPluginInfo, true);
}

void
PrivateAnimScreen::addExtension (ExtensionPluginInfo *extensionPluginInfo,
				 bool shouldInitPersistentData)
{
    mExtensionPlugins.push_back (extensionPluginInfo);

    unsigned int nPluginEffects = extensionPluginInfo->nEffects;

    bool eventEffectsNeedUpdate[AnimEventNum] =
	{false, false, false, false, false};

    // Put this plugin's effects into mEventEffects and
    // mEventEffectsAllowed
    for (unsigned int j = 0; j < nPluginEffects; j++)
    {
	const AnimEffect effect = extensionPluginInfo->effects[j];

	// Update allowed effects for each event
	for (int e = 0; e < AnimEventNum; e++)
	{
	    if (effect->usedForEvents[e])
	    {
		mEventEffectsAllowed[e].push_back (effect);
		eventEffectsNeedUpdate[e] = true;
	    }
	}
    }

    for (int e = 0; e < AnimEventNum; e++)
	if (eventEffectsNeedUpdate[e])
	{
	    updateEventEffects ((AnimEvent)e, false, false);
	    if (e != AnimEventFocus)
		updateEventEffects ((AnimEvent)e, true, false);
	}

    if (shouldInitPersistentData)
    {
	const CompWindowList &pl = pushLockedPaintList ();
	// Initialize persistent window data for the extension plugin
	foreach (CompWindow *w, pl)
	{
	    AnimWindow *aw = AnimWindow::get (w);
	    extensionPluginInfo->initPersistentData (aw);
	}

	popLockedPaintList ();
    }
}

void
AnimScreen::removeExtension (ExtensionPluginInfo *extensionPluginInfo)
{
    priv->removeExtension (extensionPluginInfo);
}

void
PrivateAnimScreen::removeExtension (ExtensionPluginInfo *extensionPluginInfo)
{
    // Stop all ongoing animations
    const CompWindowList &pl = pushLockedPaintList ();

    foreach (CompWindow *w, pl)
    {
	PrivateAnimWindow *aw = AnimWindow::get (w)->priv;
	if (aw->curAnimation ())
	    aw->postAnimationCleanUp ();
    }

    popLockedPaintList ();

    // Find the matching plugin and delete it

    ExtensionPluginVector::iterator it = find (mExtensionPlugins.begin (),
					       mExtensionPlugins.end (),
					       extensionPluginInfo);

    if (it == mExtensionPlugins.end ())
	return; // couldn't find that extension plugin

    mExtensionPlugins.erase (it);

    if (extensionPluginInfo->nEffects == 0)
	return; // no animation effects -> we're done here


    // Also delete the "allowed effect" entries for that plugin

    for (int e = 0; e < AnimEventNum; e++)
    {
	AnimEffectVector &eventEffectsAllowed = mEventEffectsAllowed[e];

	// Find the first animation effect with matching name
	AnimEffectVector::iterator itBeginEffect =
	    find_if (eventEffectsAllowed.begin (),
		     eventEffectsAllowed.end (),
		     boost::bind (&AnimEffectInfo::matchesPluginName,
				  _1, extensionPluginInfo->name));

	if (itBeginEffect == eventEffectsAllowed.end ())
	    continue; // plugin didn't provide any effects for this event

	// Find the first animation effect with non-matching name,
	// starting with itBeginEffect
	AnimEffectVector::iterator itEndEffect =
	    find_if (itBeginEffect,
		     eventEffectsAllowed.end (),
		     boost::bind (&AnimEffectInfo::matchesPluginName,
				  _1, extensionPluginInfo->name) == false);

	eventEffectsAllowed.erase (itBeginEffect, itEndEffect);

	// Update event effects to complete removal
	updateEventEffects ((AnimEvent)e, false);
	if (e != AnimEventFocus)
	    updateEventEffects ((AnimEvent)e, true);
    }

    const CompWindowList &cpl = pushLockedPaintList ();

    // Destroy persistent window data for the extension plugin
    foreach (CompWindow *w, cpl)
    {
	AnimWindow *aw = AnimWindow::get (w);
	extensionPluginInfo->destroyPersistentData (aw);
    }

    popLockedPaintList ();
}

ExtensionPluginInfo::ExtensionPluginInfo (const CompString &name,
					  unsigned int nEffects,
					  AnimEffect *effects,
					  CompOption::Vector *effectOptions,
					  unsigned int firstEffectOptionIndex) :
    name (name),
    nEffects (nEffects),
    effects (effects),
    effectOptions (effectOptions),
    firstEffectOptionIndex (firstEffectOptionIndex)
{
}

// End of extension functions

Animation::Animation (CompWindow *w,
		      WindowEvent curWindowEvent,
		      float duration,
		      const AnimEffect info,
		      const CompRect &icon) :
    mWindow (w),
    mAWindow (AnimWindow::get (w)),
    mTotalTime (duration),
    mRemainingTime (duration),
    mTimeElapsedWithinTimeStep (0),
    mOverrideProgressDir (0),
    mCurPaintAttrib (GLWindow::defaultPaintAttrib),
    mStoredOpacity (CompositeWindow::get (w)->opacity ()),
    mCurWindowEvent (curWindowEvent),
    mInitialized (false), // store window opacity
    mInfo (info),
    mIcon (icon)
{
    if (curWindowEvent == WindowEventShade ||
	curWindowEvent == WindowEventUnshade)
    {
	mDecorTopHeight = w->output ().top;
	mDecorBottomHeight = w->output ().bottom;
    }

    texturesCache = new GLTexture::List (GLWindow::get (w)->textures ());
    PrivateAnimScreen *as = mAWindow->priv->paScreen ();

    mTimestep = as->optionGetTimeStep ();
}

Animation::~Animation ()
{
    delete texturesCache;
}

CompOption::Value &
Animation::optVal (unsigned int optionId)
{
    return mAWindow->pluginOptVal (getExtensionPluginInfo (), optionId, this);
}

/// Play the animation effect backwards from where it left off.
void
Animation::reverse ()
{
    mRemainingTime = mTotalTime - mRemainingTime;

    // avoid window remains
    if (mRemainingTime <= 0)
	mRemainingTime = 1;

    switch (mCurWindowEvent) // the old event
    {
    case WindowEventOpen:
	mCurWindowEvent = WindowEventClose;
	break;
    case WindowEventClose:
	mCurWindowEvent = WindowEventOpen;
	break;
    case WindowEventMinimize:
	mCurWindowEvent = WindowEventUnminimize;
	break;
    case WindowEventUnminimize:
	mCurWindowEvent = WindowEventMinimize;
	break;
    case WindowEventShade:
	mCurWindowEvent = WindowEventUnshade;
	break;
    case WindowEventUnshade:
	mCurWindowEvent = WindowEventShade;
	break;
    default:
	break;
    }

    // 1: forward, 2: backward  (3 - progressDir is opposite direction)
    int progressDir = 1;

    switch (mCurWindowEvent) // the new event
    {
    case WindowEventClose:
    case WindowEventMinimize:
    case WindowEventShade:
	progressDir = 2;
	break;
    default:
	break;
    }

    if (mOverrideProgressDir == 0)
	mOverrideProgressDir = progressDir;
    else if (mOverrideProgressDir == 3 - progressDir)
	mOverrideProgressDir = 0; // disable override
}

PartialWindowAnim::PartialWindowAnim (CompWindow *w,
				      WindowEvent curWindowEvent,
				      float duration,
				      const AnimEffect info,
				      const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    mUseDrawRegion (false),
    mDrawRegion ()
{
}

void
PrivateAnimWindow::updateSelectionRow (unsigned int r)
{
    mPrevAnimSelectionRow = mCurAnimSelectionRow;
    mCurAnimSelectionRow = (int)r;
}

// Assumes events in the metadata are in
// [Open, Close, Minimize, Unminimize, Shade, Focus] order
// and effects among those are in alphabetical order
// but with "(Event) None" first and "(Event) Random" last.
AnimEffect
PrivateAnimScreen::getMatchingAnimSelection (CompWindow *w,
					     AnimEvent e,
					     int *duration)
{
    PrivateAnimWindow *aw = AnimWindow::get (w)->priv;

    EffectSet *eventEffects = &mEventEffects[e];
    CompOption::Value &valMatch =
	getOptions ()[(unsigned)matchOptionIds[e]].value ();
    CompOption::Value &valDuration =
	getOptions ()[(unsigned)durationOptionIds[e]].value ();
    CompOption::Value &valCustomOptions =
	getOptions ()[(unsigned)customOptionOptionIds[e]].value ();

    unsigned int nRows = valMatch.list ().size ();
    if (nRows != eventEffects->effects.size () ||
	nRows != valDuration.list ().size () ||
	nRows != valCustomOptions.list ().size ())
    {
	compLogMessage ("animation", CompLogLevelError,
			"Animation settings mismatch in \"Animation "
			"Selection\" list for %s event.", eventNames[e]);
	return AnimEffectNone;
    }

    // Find the first row that matches this window for this event
    for (unsigned int i = 0; i < nRows; i++)
    {
	if (!valMatch.list ()[i].match ().evaluate (w))
	    continue;

	aw->updateSelectionRow (i);

	if (duration)
	    *duration = valDuration.list ()[i].i ();

	AnimEffect effect = eventEffects->effects[i];

	return (effect ? effect : AnimEffectNone);
    }

    return AnimEffectNone;
}

AnimEffect
PrivateAnimScreen::getActualEffect (AnimEffect effect,
				    AnimEvent animEvent)
{
    bool allRandom = optionGetAllRandom ();
    AnimEffectVector *randomEffects = &mRandomEffects[animEvent].effects;
    unsigned int nRandomEffects = randomEffects->size ();

    if ((effect == AnimEffectRandom) || allRandom)
    {
	unsigned int nFirstRandomEffect = 0;
	if (nRandomEffects == 0) // no random animation selected, assume "all"
	{
	    randomEffects = &mEventEffectsAllowed[animEvent];

	    // exclude None and Random
	    nFirstRandomEffect = 2;
	    nRandomEffects = randomEffects->size () - 2;
	}
	unsigned int index = nFirstRandomEffect +
	    (unsigned int)(nRandomEffects * (double)rand () / RAND_MAX);
	return (*randomEffects)[index];
    }
    else
	return effect;
}

/// Converts animation direction (up, down, left, right, random, auto)
/// to an actual direction (up, down, left, or right).
AnimDirection
Animation::getActualAnimDirection (AnimDirection dir,
				   bool openDir)
{
    if (dir == AnimDirectionRandom)
    {
	dir = (AnimDirection)(rand () % 4);
    }
    else if (dir == AnimDirectionAuto)
    {
	CompRect outRect (mAWindow->savedRectsValid () ?
			  mAWindow->savedOutRect () :
			  mWindow->outputRect ());

	// away from icon
	int centerX = outRect.x () + outRect.width () / 2 ;
	int centerY = outRect.y () + outRect.height () / 2 ;
	float relDiffX = ((float)centerX - mIcon.x ()) / outRect.width ();
	float relDiffY = ((float)centerY - mIcon.y ()) / outRect.height ();

	if (openDir)
	{
	    if (mCurWindowEvent == WindowEventMinimize ||
		mCurWindowEvent == WindowEventUnminimize)
		// min/unmin. should always result in +/- y direction
		dir = (mIcon.y () < (int)::screen->height () - mIcon.y ()) ?
		    AnimDirectionDown : AnimDirectionUp;
	    else if (fabs (relDiffY) > fabs (relDiffX))
		dir = relDiffY > 0 ? AnimDirectionDown : AnimDirectionUp;
	    else
		dir = relDiffX > 0 ? AnimDirectionRight : AnimDirectionLeft;
	}
	else
	{
	    if (mCurWindowEvent == WindowEventMinimize ||
		mCurWindowEvent == WindowEventUnminimize)
		// min/unmin. should always result in +/- y direction
		dir = (mIcon.y () < (int)::screen->height () - mIcon.y ()) ?
		    AnimDirectionUp : AnimDirectionDown;
	    else if (fabs (relDiffY) > fabs (relDiffX))
		dir = relDiffY > 0 ? AnimDirectionUp : AnimDirectionDown;
	    else
		dir = relDiffX > 0 ? AnimDirectionLeft : AnimDirectionRight;
	}
    }
    return dir;
}

float
Animation::progressLinear ()
{
    float forwardProgress =
	1 - mRemainingTime / (mTotalTime - mTimestep);
    forwardProgress = MIN (forwardProgress, 1);
    forwardProgress = MAX (forwardProgress, 0);

    if (mCurWindowEvent == WindowEventOpen ||
	mCurWindowEvent == WindowEventUnminimize ||
	mCurWindowEvent == WindowEventUnshade ||
	mCurWindowEvent == WindowEventFocus)
	forwardProgress = 1 - forwardProgress;

    return forwardProgress;
}

float
Animation::progressEaseInEaseOut ()
{
    float forwardProgress =
	1 - mRemainingTime / (mTotalTime - mTimestep);
    forwardProgress = MIN (forwardProgress, 1);
    forwardProgress = MAX (forwardProgress, 0);

    // Apply sigmoid and normalize
    forwardProgress =
	(sigmoid (forwardProgress) - sigmoid (0)) /
	(sigmoid (1) - sigmoid (0));

    if (mCurWindowEvent == WindowEventOpen ||
	mCurWindowEvent == WindowEventUnminimize ||
	mCurWindowEvent == WindowEventUnshade ||
	mCurWindowEvent == WindowEventFocus)
	forwardProgress = 1 - forwardProgress;

    return forwardProgress;
}

/// Gives some acceleration (when closing a window)
/// or deceleration (when opening a window).
/// Applies a sigmoid with slope s,
/// where minx and maxx are the
/// starting and ending points on the sigmoid.
float
Animation::progressDecelerateCustom (float progress, float minx, float maxx)
{
    float x = 1 - progress;
    float s = 8;

    return
	1 - ((sigmoid2 (minx + (x * (maxx - minx)), s) - sigmoid2 (minx, s)) /
	     (sigmoid2 (maxx, s) - sigmoid2 (minx, s)));
}

float
Animation::progressDecelerate (float progress)
{
    return progressDecelerateCustom (progress, 0.5, 0.75);
}

BoxPtr
AnimWindow::BB ()
{
    return &priv->mBB;
}

CompRegion &
AnimWindow::stepRegion ()
{
    return priv->mStepRegion;
}

void
PrivateAnimWindow::copyResetStepRegion ()
{
    mLastStepRegion = mStepRegion;

    // Reset bounding box for current step
    mBB.x1 = mBB.y1 = MAXSHORT;
    mBB.x2 = mBB.y2 = MINSHORT;
}

void
AnimWindow::expandBBWithBox (Box &source)
{
    Box &target = priv->BB ();

    if (source.x1 < target.x1)
	target.x1 = source.x1;
    if (source.x2 > target.x2)
	target.x2 = source.x2;
    if (source.y1 < target.y1)
	target.y1 = source.y1;
    if (source.y2 > target.y2)
	target.y2 = source.y2;
}

void
AnimWindow::expandBBWithPoint (float fx, float fy)
{
    Box &target = priv->BB ();

    short x = MAX (MIN (fx, MAXSHORT - 1), MINSHORT);
    short y = MAX (MIN (fy, MAXSHORT - 1), MINSHORT);

    if (target.x1 == MAXSHORT)
    {
	target.x1 = x;
	target.y1 = y;
	target.x2 = x + 1;
	target.y2 = y + 1;
	return;
    }
    if (x < target.x1)
	target.x1 = x;
    else if (x > target.x2)
	target.x2 = x;

    if (y < target.y1)
	target.y1 = y;
    else if (y > target.y2)
	target.y2 = y;
}

/// This will work for zoom-like 2D transforms,
/// but not for glide-like 3D transforms.
void
AnimWindow::expandBBWithPoint2DTransform (GLVector &coords,
					  GLMatrix &transformMat)
{
    GLVector coordsTransformed = transformMat * coords;
    expandBBWithPoint (coordsTransformed[GLVector::x],
		       coordsTransformed[GLVector::y]);
}

static bool
project (float objx, float objy, float objz, 
         const float modelview[16], const float projection[16],
         const GLint viewport[4],
         float *winx, float *winy, float *winz)
{
    unsigned int i;
    float in[4];
    float out[4];

    in[0] = objx;
    in[1] = objy;
    in[2] = objz;
    in[3] = 1.0;

    for (i = 0; i < 4; i++) {
	out[i] = 
	    in[0] * modelview[i] +
	    in[1] * modelview[4  + i] +
	    in[2] * modelview[8  + i] +
	    in[3] * modelview[12 + i];
    }

    for (i = 0; i < 4; i++) {
	in[i] = 
	    out[0] * projection[i] +
	    out[1] * projection[4  + i] +
	    out[2] * projection[8  + i] +
	    out[3] * projection[12 + i];
    }

    if (in[3] == 0.0)
	return false;

    in[0] /= in[3];
    in[1] /= in[3];
    in[2] /= in[3];
    /* Map x, y and z to range 0-1 */
    in[0] = in[0] * 0.5 + 0.5;
    in[1] = in[1] * 0.5 + 0.5;
    in[2] = in[2] * 0.5 + 0.5;

    /* Map x,y to viewport */
    in[0] = in[0] * viewport[2] + viewport[0];
    in[1] = in[1] * viewport[3] + viewport[1];

    *winx = in[0];
    *winy = in[1];
    *winz = in[2];
    return true;
}

/// Either points or objects should be non-0.
bool
AnimWindow::expandBBWithPoints3DTransform (CompOutput     &output,
					   GLMatrix       &transform,
					   const float    *points,
					   GridAnim::GridModel::GridObject *objects,
					   unsigned int   nPoints)
{
    GLfloat x, y, z;
    GLint viewport[4] =
	{output.region ()->extents.x1,
	 output.region ()->extents.y1,
	 output.width (),
	 output.height ()};

    const float *projection =
        GLScreen::get (::screen)->projectionMatrix ()->getMatrix ();

    if (points) // use points
    {
	for (; nPoints; nPoints--, points += 3)
	{
	    if (!project (points[0], points[1], points[2],
	                  transform.getMatrix (), projection,
	                  viewport,
	                  &x, &y, &z))
		return false;

	    expandBBWithPoint (x + 0.5, (::screen->height () - y) + 0.5);
	}
    }
    else // use grid model objects
    {
	GridAnim::GridModel::GridObject *object = objects;
	for (; nPoints; nPoints--, object++)
	{
	    if (!project (object->position ().x (),
	                  object->position ().y (),
	                  object->position ().z (),
	                  transform.getMatrix (), projection,
	                  viewport,
	                  &x, &y, &z))
		return false;

	    expandBBWithPoint (x + 0.5, (::screen->height () - y) + 0.5);
	}
    }
    return true;
}

void
AnimWindow::expandBBWithWindow ()
{
    CompRect outRect (savedRectsValid () ?
		      savedOutRect () :
		      mWindow->outputRect ());
    Box windowBox = {
	static_cast <short int> (outRect.x ()), static_cast <short int> (outRect.x () + outRect.width ()),
	static_cast <short int> (outRect.y ()), static_cast <short int> (outRect.y () + outRect.height ())
    };
    expandBBWithBox (windowBox);
}

void
AnimWindow::expandBBWithScreen ()
{
    Box screenBox = {0, static_cast <short int> (::screen->width ()),
		     0, static_cast <short int> (::screen->height ())};
    expandBBWithBox (screenBox);
}

void
Animation::prepareTransform (CompOutput &output,
			     GLMatrix &resultTransform,
			     GLMatrix &transform)
{
    GLMatrix sTransform;
    sTransform.toScreenSpace (&output, -DEFAULT_Z_CAMERA);
    resultTransform = sTransform * transform;
}

void
AnimWindow::resetStepRegionWithBB ()
{
    // Have a 1 pixel margin to prevent occasional 1 pixel line artifact
    CompRegion region (priv->mBB.x1 - 1,
    		       priv->mBB.y1 - 1,
    		       priv->mBB.x2 - priv->mBB.x1 + 2,
    		       priv->mBB.y2 - priv->mBB.y1 + 2);
    priv->mStepRegion = region;
}

/// Damage the union of window's bounding box
/// before and after animStepFunc does its job.
void
PrivateAnimWindow::damageThisAndLastStepRegion ()
{
    // Find union of the regions for this step and last step
    CompRegion totalRegionToDamage (mStepRegion + mLastStepRegion);

    mPAScreen->cScreen->damageRegion (totalRegionToDamage);
}

CompOutput &
AnimScreen::output ()
{
    return priv->output ();
}

bool
AnimScreen::getMousePointerXY (short *x, short *y)
{
    Window w1, w2;
    int xp, yp, xj, yj;
    unsigned int m;

    if (XQueryPointer
	(::screen->dpy (), ::screen->root (), &w1, &w2, &xj, &yj, &xp, &yp, &m))
    {
	*x = xp;
	*y = yp;
	return true;
    }
    return false;
}

unsigned int
PrivateAnimWindow::getState ()
{
    Atom actual;
    int result, format;
    unsigned long n, left;
    unsigned char *data;
    unsigned int retval = WithdrawnState;

    result = XGetWindowProperty (::screen->dpy (), mWindow->id (),
				 Atoms::wmState, 0L,
				 1L, false,
				 Atoms::wmState,
				 &actual, &format, &n, &left, &data);

    if (result == Success && data)
    {
	if (n)
    	    memcpy (&retval, data, sizeof (int));

	XFree ((void *)data);
    }

    return retval;
}

CompOption::Vector &
AnimScreen::getOptions ()
{
    return priv->getOptions ();
}

bool
AnimScreen::setOption (const CompString  &name,
		       CompOption::Value &value)
{
    return priv->setOption (name, value);
}

void
PrivateAnimScreen::eventMatchesChanged (CompOption                *opt,
					AnimationOptions::Options num)
{
    if (mExtensionPlugins.empty ())
	initAnimationList ();
    foreach (CompOption::Value &val, opt->value ().list ())
	val.match ().update ();
}

void
PrivateAnimScreen::eventOptionsChanged (CompOption                *opt,
					AnimationOptions::Options num)
{
    if (mExtensionPlugins.empty ())
	initAnimationList ();
    updateOptionSets (getCorrespondingAnimEvent (num));
}

void
PrivateAnimScreen::eventEffectsChanged (CompOption                *opt,
					AnimationOptions::Options num)
{
    if (mExtensionPlugins.empty ())
	initAnimationList ();
    updateEventEffects (getCorrespondingAnimEvent (num), false);
}

void
PrivateAnimScreen::eventRandomEffectsChanged (CompOption                *opt,
					      AnimationOptions::Options num)
{
    if (mExtensionPlugins.empty ())
	initAnimationList ();
    updateEventEffects (getCorrespondingAnimEvent (num), true);
}

void
PrivateAnimWindow::postAnimationCleanUpCustom (bool closing,
					       bool destructing,
					       bool clearMatchingRow)
{
    bool shouldDamageWindow = false;
    
    notifyAnimation (false);

    if (mCurAnimation)
    {
	if (mCurAnimation->shouldDamageWindowOnEnd ())
	    shouldDamageWindow = true;
    }
    enablePainting (false);

    if (shouldDamageWindow)
	mAWindow->expandBBWithWindow ();

    if (shouldDamageWindow ||
	(mCurAnimation &&
	 !mCurAnimation->stepRegionUsed () &&
	 mAWindow->BB ()->x1 != MAXSHORT)) // BB intialized
	mAWindow->resetStepRegionWithBB ();

    damageThisAndLastStepRegion ();

    if (mCurAnimation)
    {
	mCurAnimation->cleanUp (closing, destructing);
	delete mCurAnimation;
	mCurAnimation = 0;
    }

    mBB.x1 = mBB.y1 = MAXSHORT;
    mBB.x2 = mBB.y2 = MINSHORT;

    mState = mNewState;

    if (clearMatchingRow)
	mCurAnimSelectionRow = -1;

    mFinishingAnim = true;
    if (!destructing)
    {
	mIgnoreDamage = true;
	while (mUnmapCnt > 0)
	{
	    mWindow->unmap ();
	    mUnmapCnt--;
	}
	if (mUnmapCnt < 0)
	    mUnmapCnt = 0;
	mIgnoreDamage = false;
    }

    while (mDestroyCnt)
    {
	mWindow->destroy ();
	mDestroyCnt--;
    }
    mFinishingAnim = false;

    foreach (ExtensionPluginInfo *extPlugin, mPAScreen->mExtensionPlugins)
	extPlugin->cleanUpAnimation (closing, destructing);
}

void
AnimWindow::postAnimationCleanUp ()
{
    priv->postAnimationCleanUp ();
}

void
PrivateAnimWindow::postAnimationCleanUp ()
{
    if (mCurAnimation->curWindowEvent () == WindowEventClose)
	postAnimationCleanUpCustom (true, false, true);
    else
	postAnimationCleanUpCustom (false, false, true);
}

void
PrivateAnimWindow::postAnimationCleanUpPrev (bool closing,
					     bool clearMatchingRow)
{
    int curAnimSelectionRow = mCurAnimSelectionRow;
    // Use previous event's anim selection row
    mCurAnimSelectionRow = mPrevAnimSelectionRow;

    postAnimationCleanUpCustom (closing, false, clearMatchingRow);

    // Restore current event's anim selection row
    mCurAnimSelectionRow = curAnimSelectionRow;
}

void
PrivateAnimScreen::activateEvent (bool activating)
{
    if (activating)
    {
	if (mAnimInProgress)
	    return;
    }
    else
    {
	// Animations have finished for all windows
	// (Keep preparePaint enabled)

	aScreen->enableCustomPaintList (false);
    }
    cScreen->donePaintSetEnabled (this, activating);
    gScreen->glPaintOutputSetEnabled (this, activating);

    mAnimInProgress = activating;

    CompOption::Vector o (0);

    o.push_back (CompOption ("root", CompOption::TypeInt));
    o.push_back (CompOption ("active", CompOption::TypeBool));

    o[0].value ().set ((int) ::screen->root ());
    o[1].value ().set (activating);

    ::screen->handleCompizEvent ("animation", "activate", o);
}

void
PrivateAnimWindow::notifyAnimation (bool activation)
{
    CompOption::Vector o (0);
    
    if (!mCurAnimation)
	return;
    
    o.push_back (CompOption ("root", CompOption::TypeInt));
    o.push_back (CompOption ("window", CompOption::TypeInt));
    o.push_back (CompOption ("type", CompOption::TypeString));
    o.push_back (CompOption ("active", CompOption::TypeBool));
    
    o[0].value ().set ((int) ::screen->root ());
    o[1].value ().set ((int) mWindow->id ());
        
    switch (mCurAnimation->curWindowEvent ())
    {
	case WindowEventOpen:
	    o[2].value ().set ("open");
	    break;
	case WindowEventClose:
	    o[2].value ().set ("close");
	    break;
	case WindowEventMinimize:
	    o[2].value ().set ("minimize");
	    break;
	case WindowEventUnminimize:
	    o[2].value ().set ("unminimize");
	    break;
	case WindowEventShade:
	    o[2].value ().set ("shade");
	    break;
	case WindowEventUnshade:
	    o[2].value ().set ("unshade");
	    break;
	case WindowEventFocus:
	    o[2].value ().set ("focus");
	    break;
	case WindowEventNum:
	case WindowEventNone:
	default:
	    o[2].value ().set ("none");
	    break;
    }
    
    o[3].value ().set (activation);

    screen->handleCompizEvent ("animation", "window_animation", o);
}

bool
PrivateAnimScreen::otherPluginsActive ()
{
    for (int i = 0; i < WatchedScreenPluginNum; i++)
	if (mPluginActive[i])
	    return true;
    return false;
}

bool
Animation::shouldSkipFrame (int msSinceLastPaintActual)
{
    mTimeElapsedWithinTimeStep += msSinceLastPaintActual;
    if (mTimeElapsedWithinTimeStep < mTimestep) // if timestep not yet completed
	return true;

    mTimeElapsedWithinTimeStep = fmod (mTimeElapsedWithinTimeStep, mTimestep);
    return false;
}

bool
Animation::advanceTime (int msSinceLastPaint)
{
    mRemainingTime -= msSinceLastPaint;
    mRemainingTime = MAX (mRemainingTime, 0); // avoid sub-zero values

    mTimeSinceLastPaint = msSinceLastPaint;

    return (mRemainingTime > 0);
}

void
PrivateAnimScreen::preparePaint (int msSinceLastPaint)
{
    // Check and update "switcher post wait" counter
    if (mSwitcherPostWait > 0)
    {
	mSwitcherPostWait++;
	if (mSwitcherPostWait > 5) // wait over
	{
	    mSwitcherPostWait = 0;

	    // Reset stacking related info since it will
	    // cause problems because of the restacking
	    // just done by Switcher.
	    ExtensionPluginAnimation *extPlugin =
		static_cast<ExtensionPluginAnimation *> (mExtensionPlugins[0]);
	    extPlugin->resetStackingInfo ();
	}
    }

    foreach (ExtensionPluginInfo *extPlugin, mExtensionPlugins)
	extPlugin->prePreparePaintGeneral ();

    if (mAnimInProgress)
    {	
	int msSinceLastPaintActual;
	const CompWindowList &pl = pushLockedPaintList ();
	CompWindowList       windowsFinishedAnimations;

	struct timeval curTime;
	gettimeofday (&curTime, 0);

	if (mLastRedrawTimeFresh)
	{
	    msSinceLastPaintActual = timer::timeval_diff (&curTime, &mLastRedrawTime);
	    // handle clock rollback
	    if (msSinceLastPaintActual < 0)
		msSinceLastPaintActual = 0;
	}
	else
	    msSinceLastPaintActual = 20; // assume 20 ms passed

	mLastRedrawTime = curTime; // Store current time for next time
	mLastRedrawTimeFresh = true;

	/* Paint list includes destroyed windows */
	for (CompWindowList::const_reverse_iterator rit = pl.rbegin ();
	     rit != pl.rend (); ++rit)
	{
	    CompWindow *w = (*rit);
	    AnimWindow *animWin = AnimWindow::get (w);
	    PrivateAnimWindow *aw = animWin->priv;
	    Animation *curAnim = aw->curAnimation ();

	    if (curAnim)
	    {		
		if (!curAnim->initialized ())
		    curAnim->init ();

		curAnim->prePreparePaint (msSinceLastPaint);

		/* TODO optimize grid model by reusing one GridModel
		if (aw->com.mModel &&
		    (aw->com.mModel->winWidth != outRect.width () ||
		     aw->com.mModel->winHeight != outRect.height ()))
		{
		    // mModel needs update
		    // re-create mModel
		    if (!animEnsureModel (w))
		    {
			// Abort this window's animation
			postAnimationCleanUp (w);
			continue;
		    }
		}*/

		bool animShouldSkipFrame =
		    (curAnim->shouldSkipFrame (msSinceLastPaintActual) &&
		     // Skip only if we're not on the first animation frame
		     curAnim->initialized ());

		// Skip only if we're not on the last animation frame
		animShouldSkipFrame &=
		    curAnim->advanceTime (msSinceLastPaint);

		if (!animShouldSkipFrame)
		{
		    if (curAnim->updateBBUsed ())
		    {
			aw->copyResetStepRegion ();

			if (!curAnim->initialized () &&
			    curAnim->shouldDamageWindowOnStart ())
			    aw->aWindow ()->expandBBWithWindow ();
		    }

		    if (!curAnim->initialized ())
			curAnim->setInitialized ();

		    curAnim->step ();

		    if (curAnim->updateBBUsed ())
		    {
			foreach (CompOutput &output, ::screen->outputDevs ())
			    curAnim->updateBB (output);

			if (!curAnim->stepRegionUsed () &&
			    aw->BB ().x1 != MAXSHORT) // BB initialized
			{
			    // BB is used instead of step region,
			    // so reset step region here with BB.
			    animWin->resetStepRegionWithBB ();
			}
			if (!(cScreen->damageMask () &
			      COMPOSITE_SCREEN_DAMAGE_ALL_MASK))
			    aw->damageThisAndLastStepRegion ();
		    }
		}

		bool finished = (curAnim->remainingTime () <= 0);
		if (finished) // Animation is done
		    windowsFinishedAnimations.push_back (w);
	    }
	}

	foreach (CompWindow *w, pl)
	{
	    PrivateAnimWindow *aw = AnimWindow::get (w)->priv;
	    if (aw->curAnimation ())
		aw->curAnimation ()->postPreparePaint ();
	}

	popLockedPaintList ();
    }

    foreach (ExtensionPluginInfo *extPlugin, mExtensionPlugins)
	extPlugin->postPreparePaintGeneral ();

    cScreen->preparePaint (msSinceLastPaint);

    if (mStartCountdown)
    {
	mStartCountdown--;
	if (!mStartCountdown)
	{
	    foreach (ExtensionPluginInfo *extPlugin, mExtensionPlugins)
		extPlugin->postStartupCountdown ();
	}
    }
}

void
PrivateAnimScreen::donePaint ()
{
    assert (mAnimInProgress);

    const CompWindowList &pl = pushLockedPaintList ();
    CompWindowList       windowsFinishedAnimations;

    bool animStillInProgress = false;

    /* Paint list includes destroyed windows */
    for (CompWindowList::const_reverse_iterator rit = pl.rbegin ();
	 rit != pl.rend (); ++rit)
    {
	CompWindow *w = (*rit);
	AnimWindow *animWin = AnimWindow::get (w);
	PrivateAnimWindow *aw = animWin->priv;
	Animation *curAnim = aw->curAnimation ();

	if (curAnim)
	{
	    bool finished = (curAnim->remainingTime () <= 0);
	    if (finished) // Animation is done
		windowsFinishedAnimations.push_back (w);
	    else
		animStillInProgress = true;
	}
    }

    popLockedPaintList ();

    foreach (CompWindow *w, windowsFinishedAnimations)
    {
	AnimWindow *aw = AnimWindow::get (w);
	aw->priv->notifyAnimation (false);
	aw->priv->postAnimationCleanUp ();
    }

    if (!animStillInProgress)
    {
	activateEvent (false);
	mLastRedrawTimeFresh = false;

	// Reset stacking related info after all animations are done.
	ExtensionPluginAnimation *extPlugin =
		static_cast<ExtensionPluginAnimation *> (mExtensionPlugins[0]);
	extPlugin->resetStackingInfo ();
    }

    cScreen->damagePending ();

    cScreen->donePaint ();
}

void
PrivateAnimWindow::enablePainting (bool enabling)
{
    gWindow->glPaintSetEnabled (this, enabling);
    gWindow->glAddGeometrySetEnabled (this, enabling);
    //gWindow->glDrawGeometrySetEnabled (this, enabling);
    gWindow->glDrawTextureSetEnabled (this, enabling);
}

void
PrivateAnimWindow::glAddGeometry (const GLTexture::MatrixList &matrix,
				  const CompRegion            &region,
				  const CompRegion            &clip,
				  unsigned int                maxGridWidth,
				  unsigned int                maxGridHeight)
{
    // if window is being animated
    if (mCurAnimation)
    {
	if (mCurAnimation->initialized ())
	    mCurAnimation->addGeometry (matrix, region, clip,
					maxGridWidth, maxGridHeight);
    }
    else
    {
	gWindow->glAddGeometry (matrix, region, clip,
				maxGridWidth, maxGridHeight);
    }
}

bool
Animation::shouldDamageWindowOnStart ()
{
    return (mCurWindowEvent == WindowEventClose ||
	    mCurWindowEvent == WindowEventMinimize ||
	    mCurWindowEvent == WindowEventShade);
}

bool
Animation::shouldDamageWindowOnEnd ()
{
    return (mCurWindowEvent == WindowEventOpen ||
	    mCurWindowEvent == WindowEventUnminimize ||
	    mCurWindowEvent == WindowEventUnshade);
}

void
Animation::addGeometry (const GLTexture::MatrixList &matrix,
			const CompRegion            &region,
			const CompRegion            &clip,
			unsigned int                maxGridWidth,
			unsigned int                maxGridHeight)
{
    mAWindow->priv->gWindow->glAddGeometry (matrix, region, clip,
					    maxGridWidth, maxGridHeight);
}

void
PartialWindowAnim::addGeometry (const GLTexture::MatrixList &matrix,
				const CompRegion            &region,
				const CompRegion            &clip,
				unsigned int                maxGridWidth,
				unsigned int                maxGridHeight)
{
    if (mUseDrawRegion)
    {
	CompRegion awRegion (region.intersected (mDrawRegion));
	Animation::addGeometry (matrix, awRegion, clip,
				maxGridWidth, maxGridHeight);
    }
    else
    {
	Animation::addGeometry (matrix, region, clip,
				maxGridWidth, maxGridHeight);
    }
}

void
PrivateAnimWindow::glDrawTexture (GLTexture          *texture,
                                  const GLMatrix     &transform,
                                  const GLWindowPaintAttrib &attrib,
				  unsigned int       mask)
{
    if (mCurAnimation)
    {
	mCurAnimation->setCurPaintAttrib (attrib);
    }

    gWindow->glDrawTexture (texture, transform, attrib, mask);
}

#if 0 // Not ported yet
void
PrivateAnimWindow::glDrawGeometry ()
{
    if (mCurAnimation)
    {
	if (mCurAnimation->initialized ())
	    mCurAnimation->drawGeometry ();
    }
    else
    {
	gWindow->glDrawGeometry ();
    }
}
#endif

void
Animation::drawTexture (GLTexture          *texture,
                        const GLWindowPaintAttrib &attrib,
			unsigned int       mask)
{
    mCurPaintAttrib = attrib;
}

void
Animation::drawGeometry ()
{
#if 0 // Not ported yet
    mAWindow->priv->gWindow->glDrawGeometry ();
#endif
}

bool
PrivateAnimWindow::glPaint (const GLWindowPaintAttrib &attrib,
			    const GLMatrix &transform,
			    const CompRegion &region, unsigned int mask)
{
    bool status;

    // Is this the first glPaint call this round
    // without the mask PAINT_WINDOW_OCCLUSION_DETECTION_MASK?
    if (mPAScreen->mStartingNewPaintRound &&
	!(mask & PAINT_WINDOW_OCCLUSION_DETECTION_MASK))
    {
	mPAScreen->mStartingNewPaintRound = false;

	// Back-to-front painting of windows is starting now.
	if (mPAScreen->mPrePaintWindowsBackToFrontEnabled)
	    mPAScreen->prePaintWindowsBackToFront ();
    }

    assert (mCurAnimation);

    foreach (ExtensionPluginInfo *extPlugin, mPAScreen->mExtensionPlugins)
    {
	if (extPlugin->paintShouldSkipWindow (mWindow))
	    return false;
    }

    if (mCurAnimation->curWindowEvent () == WindowEventFocus &&
	mPAScreen->otherPluginsActive ())
    {
	postAnimationCleanUp ();
	return gWindow->glPaint (attrib, transform, region, mask);
    }

    GLWindowPaintAttrib wAttrib = attrib;
    GLMatrix wTransform (transform.getMatrix ());

    /* TODO check if this is still necessary
    if (mCurAnimation->addCustomGeometryFunc)
    {
	// Use slightly smaller brightness to force core
	// to handle <max saturation case with <max brightness.
	// Otherwise polygon effects show fully unsaturated colors
	// in that case.
	wAttrib.brightness = MAX (0, wAttrib.brightness - 1);
    } */

    //w->indexCount = 0; // TODO check if this is still necessary

    if (mCurAnimation->requiresTransformedWindow ())
	mask |= PAINT_WINDOW_TRANSFORMED_MASK;

    wAttrib.xScale = 1.0f;
    wAttrib.yScale = 1.0f;

    mCurAnimation->updateAttrib (wAttrib);
    mCurAnimation->updateTransform (wTransform);
    mCurAnimation->prePaintWindow ();

    if (mCurAnimation->paintWindowUsed ())
	status = mCurAnimation->paintWindow (gWindow, wAttrib, wTransform, region, mask);
    else
	status = gWindow->glPaint (wAttrib, wTransform, region, mask);

    if (mCurAnimation->postPaintWindowUsed ())
    {
#if 0 // Not ported yet
	// Transform to make post-paint coincide with the window
	glPushMatrix ();
	glLoadMatrixf (wTransform.getMatrix ());
#endif
	mCurAnimation->postPaintWindow ();

#if 0 // Not ported yet
	glPopMatrix ();
#endif
    }

    return status;
}

const CompWindowList &
PrivateAnimScreen::pushLockedPaintList ()
{
    if (!mLockedPaintListCnt)
    {
    	const CompWindowList &pl = cScreen->getWindowPaintList ();
	mLockedPaintList = &pl;

	if (!mGetWindowPaintListEnableCnt)
	{
	    mGetWindowPaintListEnableCnt++;
	    cScreen->getWindowPaintListSetEnabled (this, true);
	}
    }

    mLockedPaintListCnt++;
    return *mLockedPaintList;
}

void
PrivateAnimScreen::popLockedPaintList ()
{
    mLockedPaintListCnt--;

    if (!mLockedPaintListCnt)
    {
	mLockedPaintList = NULL;

	mGetWindowPaintListEnableCnt--;

	if (!mGetWindowPaintListEnableCnt)
	    cScreen->getWindowPaintListSetEnabled (this, false);
    }
}

/// This is enabled only during restack animations.
/// or when we need to lock it
const CompWindowList &
PrivateAnimScreen::getWindowPaintList ()
{
    if (mLockedPaintList)
	return *mLockedPaintList;

    ExtensionPluginAnimation *extPlugin =
	static_cast<ExtensionPluginAnimation *> (mExtensionPlugins[0]);
    return extPlugin->getWindowPaintList ();
}

/// This is enabled only during restack animations.
void
PrivateAnimScreen::prePaintWindowsBackToFront ()
{
    assert (mAnimInProgress);

    ExtensionPluginAnimation *extPlugin =
	static_cast<ExtensionPluginAnimation *> (mExtensionPlugins[0]);
    extPlugin->prePaintWindowsBackToFront ();
}

void
PrivateAnimScreen::enablePrePaintWindowsBackToFront (bool enabled)
{
    mPrePaintWindowsBackToFrontEnabled = enabled;
}

void
PrivateAnimScreen::pushPaintList ()
{
    if (!mGetWindowPaintListEnableCnt)
	cScreen->getWindowPaintListSetEnabled (this, true);

    mGetWindowPaintListEnableCnt++;
}

void
PrivateAnimScreen::popPaintList ()
{
    mGetWindowPaintListEnableCnt--;

    if (!mGetWindowPaintListEnableCnt)
	cScreen->getWindowPaintListSetEnabled (this, false);
}

void
AnimScreen::enableCustomPaintList (bool enabled)
{
    enabled ? priv->pushPaintList () : priv->popPaintList ();

    priv->enablePrePaintWindowsBackToFront (enabled);
}

static const PluginEventInfo watchedScreenPlugins[] =
{
    {"switcher", "activate"},
    {"ring", "activate"},
    {"shift", "activate"},
    {"scale", "activate"},
    {"group", "tabChangeActivate"},
    {"fadedesktop", "activate"}
};

static const PluginEventInfo watchedWindowPlugins[] =
{
    {"kdecompat", "slide"},
};

void
PrivateAnimScreen::handleCompizEvent (const char         *pluginName,
				      const char         *eventName,
				      CompOption::Vector &options)
{
    ::screen->handleCompizEvent (pluginName, eventName, options);

    for (int i = 0; i < WatchedScreenPluginNum; i++)
	if (strcmp (pluginName, watchedScreenPlugins[i].pluginName) == 0)
	{
	    if (strcmp (eventName, 
			watchedScreenPlugins[i].activateEventName) == 0)
	    {
		mPluginActive[i] =
		    CompOption::getBoolOptionNamed (options, "active", false);

		if (!mPluginActive[i] &&
		    (i == WatchedPluginSwitcher ||
		     i == WatchedPluginRing ||
		     i == WatchedPluginShift ||
		     i == WatchedPluginScale))
		{
		    mSwitcherPostWait = 1;
		}
	    }
	    break;
	}

    for (int i = 0; i < WatchedWindowPluginNum; i++)
	if (strcmp (pluginName,
		    watchedWindowPlugins[i].pluginName) == 0)
	{
	    if (strcmp (eventName,
			watchedWindowPlugins[i].activateEventName) == 0)
	    {
		Window xid = CompOption::getIntOptionNamed (options,
							    "window",
							     0);
		CompWindow *w = screen->findWindow (xid);

		if (w)
		{
		    AnimWindow *aw = AnimWindow::get (w);
		    PrivateAnimWindow *pw = aw->priv;
		    pw->mPluginActive[i] = CompOption::getBoolOptionNamed (
								    options,
								    "active",
								    false);
		}
	    }
	    break;
	}
}

/// Returns true for windows that don't have a pixmap or certain properties,
/// like the dimming layer of gksudo and x-session-manager.
inline bool
PrivateAnimScreen::shouldIgnoreWindowForAnim (CompWindow *w, bool checkPixmap)
{
    AnimWindow *aw = AnimWindow::get (w);

    for (int i = 0; i < WatchedWindowPluginNum; i++)
	if (aw->priv->mPluginActive[i])
	    return true;

    return ((checkPixmap && !CompositeWindow::get (w)->pixmap ()) ||
	    mNeverAnimateMatch.evaluate (w));
}

void
PrivateAnimWindow::reverseAnimation ()
{
    mCurAnimation->reverse ();

    // Inflict the pending unmaps
    while (mUnmapCnt > 0)
    {
	mWindow->unmap ();
	mUnmapCnt--;
    }
    if (mUnmapCnt < 0)
	mUnmapCnt = 0;
}

void
PrivateAnimScreen::initiateCloseAnim (PrivateAnimWindow *aw)
{
    CompWindow *w = aw->mWindow;

    foreach (ExtensionPluginInfo *extPlugin, mExtensionPlugins)
	extPlugin->preInitiateCloseAnim (aw->mAWindow);

    if (shouldIgnoreWindowForAnim (w, true))
	return;
    int duration = 200;
    AnimEffect chosenEffect =
	getMatchingAnimSelection (w, AnimEventClose, &duration);

    aw->mState = NormalState;
    aw->mNewState = WithdrawnState;

    if (chosenEffect != AnimEffectNone)
    {
	bool startingNew = true;
	WindowEvent curWindowEvent = WindowEventNone;

	if (aw->curAnimation ())
	    curWindowEvent = aw->curAnimation ()->curWindowEvent ();

	if (curWindowEvent != WindowEventNone)
	{
	    if (curWindowEvent == WindowEventOpen)
	    {
		startingNew = false;
		aw->reverseAnimation ();
	    }
	    /* TODO check if necessary
	    else if (aw->com.curWindowEvent == WindowEventClose)
	    {
		if (aw->com.animOverrideProgressDir == 2)
		{
		    aw->com.animRemainingTime = tmpSteps;
		    startingNew = false;
		}
	    }*/
	    else
	    {
		aw->postAnimationCleanUpPrev (true, false);
	    }
	}

	if (startingNew)
	{
	    AnimEffect effectToBePlayed =
		getActualEffect (chosenEffect, AnimEventClose);

	    // handle empty random effect list
	    if (effectToBePlayed && effectToBePlayed == AnimEffectNone)
	    {
		aw->mState = aw->mNewState;
		return;
	    }

	    aw->mCurAnimation =
		effectToBePlayed->create (w, WindowEventClose, duration,
					  effectToBePlayed, getIcon (w, true));
	    aw->mCurAnimation->adjustPointerIconSize ();
	    aw->enablePainting (true);
	}

	activateEvent (true);
	aw->notifyAnimation (true);

	// Increment 3 times to make sure close animation works
	// (e.g. for popup menus).
	for (int i = 0; i < 3; i++)
	{
	    aw->mUnmapCnt++;
	    w->incrementUnmapReference ();
	}
	cScreen->damagePending ();
    }
    /* TODO check if necessary
    else if (AnimEffectNone !=
	     getMatchingAnimSelection (w, AnimEventOpen, &duration))
    {
	// stop the current animation and prevent it from rewinding

	if (aw->com.animRemainingTime > 0 &&
	    aw->com.curWindowEvent != WindowEventOpen)
	{
	    aw->com.animRemainingTime = 0;
	}
	if ((aw->com.curWindowEvent != WindowEventNone) &&
	    (aw->com.curWindowEvent != WindowEventClose))
	{
	    postAnimationCleanUp (w);
	}
	// set some properties to make sure this window will use the
	// correct open effect the next time it's "opened"

	activateEvent (w->screen, true);
	aw->com.curWindowEvent = WindowEventClose;

	aw->mUnmapCnt++;
	w->incrementUnmapRefCnt ();

	damagePendingOnScreen (w->screen);
    }*/
    else
	aw->mState = aw->mNewState;

    // Make sure non-animated closing windows get a damage.
    if (!aw->curAnimation ())
    {
	aw->mAWindow->expandBBWithWindow ();
    }
}

CompRect
PrivateAnimScreen::getIcon (CompWindow *w, bool alwaysUseMouse)
{
    CompRect icon;

    if (!alwaysUseMouse)
    {
	icon = w->iconGeometry ();
    }
    if (alwaysUseMouse ||
	(icon.x () == 0 &&
	 icon.y () == 0 &&
	 icon.width () == 0 &&
	 icon.height () == 0)) // that is, couldn't get icon from window
    {
	// Minimize to mouse pointer if there is no
	// window list or if the window skips taskbar
	short x, y;
	if (!aScreen->getMousePointerXY (&x, &y))
	{
	    // Use screen center if can't get mouse coords
	    x = ::screen->width () / 2;
	    y = ::screen->height () / 2;
	}
	icon.setX (x);
	icon.setY (y);
	icon.setWidth (FAKE_ICON_SIZE);
	icon.setHeight (FAKE_ICON_SIZE);
    }

    return icon;
}

void
PrivateAnimScreen::initiateMinimizeAnim (PrivateAnimWindow *aw)
{
    CompWindow *w = aw->mWindow;

    if (aw->mWindow->destroyed ())
	return;

    // Store window geometry for use during animation.
    aw->mAWindow->mSavedInRect = w->inputRect ();
    aw->mAWindow->mSavedOutRect = w->outputRect ();
    aw->mAWindow->mSavedOutExtents = w->output ();
    aw->mAWindow->mSavedWinRect = w->geometry ();
    aw->mAWindow->mSavedRectsValid = true;

    aw->mNewState = IconicState;

    foreach (ExtensionPluginInfo *extPlugin, mExtensionPlugins)
	extPlugin->preInitiateMinimizeAnim (aw->mAWindow);

    int duration = 200;
    AnimEffect chosenEffect =
	getMatchingAnimSelection (w, AnimEventMinimize, &duration);

    if (chosenEffect != AnimEffectNone)
    {
	bool startingNew = true;
	WindowEvent curWindowEvent = WindowEventNone;

	if (aw->curAnimation ())
	    curWindowEvent = aw->curAnimation ()->curWindowEvent ();

	if (curWindowEvent != WindowEventNone)
	{
	    if (curWindowEvent != WindowEventUnminimize)
	    {
		aw->postAnimationCleanUpPrev (false, false);
	    }
	    else
	    {
		startingNew = false;
		aw->reverseAnimation ();
	    }
	}

	if (startingNew)
	{
	    AnimEffect effectToBePlayed =
		getActualEffect (chosenEffect, AnimEventMinimize);

	    // handle empty random effect list
	    if (effectToBePlayed == AnimEffectNone)
	    {
		aw->mState = aw->mNewState;
		return;
	    }

	    aw->mCurAnimation =
		effectToBePlayed->create (w, WindowEventMinimize, duration,
					  effectToBePlayed, getIcon (w, false));
	    aw->enablePainting (true);
	}

	activateEvent (true);
	aw->notifyAnimation (true);

	cScreen->damagePending ();
    }
    else
	aw->mState = aw->mNewState;
}

void
PrivateAnimScreen::initiateShadeAnim (PrivateAnimWindow *aw)
{
    CompWindow *w = aw->mWindow;

    int duration = 200;
    AnimEffect chosenEffect =
	getMatchingAnimSelection (w, AnimEventShade, &duration);

    aw->setShaded (true);

    if (chosenEffect != AnimEffectNone)
    {
	bool startingNew = true;
	WindowEvent curWindowEvent = WindowEventNone;
	if (aw->curAnimation ())
	    curWindowEvent = aw->curAnimation ()->curWindowEvent ();

	if (curWindowEvent != WindowEventNone)
	{
	    if (curWindowEvent != WindowEventUnshade)
	    {
		aw->postAnimationCleanUpPrev (false, false);
	    }
	    else
	    {
		startingNew = false;
		aw->reverseAnimation ();
	    }
	}

	if (startingNew)
	{
	    AnimEffect effectToBePlayed =
		getActualEffect (chosenEffect, AnimEventShade);

	    // handle empty random effect list
	    if (effectToBePlayed == AnimEffectNone)
		return;

	    aw->mCurAnimation =
		effectToBePlayed->create (w, WindowEventShade, duration,
					  effectToBePlayed, getIcon (w, false));
	    aw->enablePainting (true);
	}

	activateEvent (true);
	aw->notifyAnimation (true);

	aw->mUnmapCnt++;
	w->incrementUnmapReference ();

	cScreen->damagePending ();
    }
}

void
PrivateAnimScreen::initiateOpenAnim (PrivateAnimWindow *aw)
{
    CompWindow *w = aw->mWindow;

    int duration = 200;
    AnimEffect chosenEffect;

    aw->mNewState = NormalState;

    foreach (ExtensionPluginInfo *extPlugin, mExtensionPlugins)
	extPlugin->preInitiateOpenAnim (aw->mAWindow);

    WindowEvent curWindowEvent = WindowEventNone;
    if (aw->curAnimation ())
	curWindowEvent = aw->curAnimation ()->curWindowEvent ();

    if (!shouldIgnoreWindowForAnim (w, false) &&
	(AnimEffectNone !=
	 (chosenEffect =
	  getMatchingAnimSelection (w, AnimEventOpen, &duration)) ||
	 // reversing case
	 curWindowEvent == WindowEventClose))
    {
	bool startingNew = true;
	bool playEffect = true;

	if (curWindowEvent != WindowEventNone)
	{
	    if (curWindowEvent != WindowEventClose)
	    {
		aw->postAnimationCleanUpPrev (false, false);
	    }
	    else
	    {
		startingNew = false;
		aw->reverseAnimation ();
	    }
	}

	if (startingNew)
	{
	    AnimEffect effectToBePlayed =
		getActualEffect (chosenEffect, AnimEventOpen);

	    // handle empty random effect list
	    if (effectToBePlayed == AnimEffectNone)
		playEffect = false;

	    if (playEffect)
	    {
		aw->mCurAnimation =
		    effectToBePlayed->create (w, WindowEventOpen, duration,
					      effectToBePlayed,
					      getIcon (w, true));
		aw->mCurAnimation->adjustPointerIconSize ();
		aw->enablePainting (true);
	    }
	}

	if (playEffect)
	{
	    activateEvent (true);
	    aw->notifyAnimation (true);
	    cScreen->damagePending ();
	}
    }
}

void
PrivateAnimScreen::initiateUnminimizeAnim (PrivateAnimWindow *aw)
{
    CompWindow *w = aw->mWindow;

    if (aw->mWindow->destroyed ())
	return;

    aw->mAWindow->mSavedRectsValid = false;

    int duration = 200;
    AnimEffect chosenEffect =
	getMatchingAnimSelection (w, AnimEventUnminimize, &duration);

    aw->mNewState = NormalState;

    if (chosenEffect != AnimEffectNone &&
	!mPluginActive[3]) // fadedesktop
    {
	bool startingNew = true;
	bool playEffect = true;

	foreach (ExtensionPluginInfo *extPlugin, mExtensionPlugins)
	    extPlugin->preInitiateUnminimizeAnim (aw->mAWindow);

	// TODO Refactor the rest? (almost the same in other initiateX methods)
	WindowEvent curWindowEvent = WindowEventNone;
	if (aw->curAnimation ())
	    curWindowEvent = aw->curAnimation ()->curWindowEvent ();

	if (curWindowEvent != WindowEventNone)
	{
	    if (curWindowEvent != WindowEventMinimize)
	    {
		aw->postAnimationCleanUpPrev (false, false);
	    }
	    else
	    {
		startingNew = false;
		aw->reverseAnimation ();
	    }
	}

	if (startingNew)
	{
	    AnimEffect effectToBePlayed =
		getActualEffect (chosenEffect, AnimEventUnminimize);

	    // handle empty random effect list
	    if (effectToBePlayed == AnimEffectNone)
		playEffect = false;

	    if (playEffect)
	    {
		aw->mCurAnimation =
		    effectToBePlayed->create (w, WindowEventUnminimize,
					      duration, effectToBePlayed,
					      getIcon (w, false));
		aw->enablePainting (true);
	    }
	}

	if (playEffect)
	{
	    activateEvent (true);
	    aw->notifyAnimation (true);
	    cScreen->damagePending ();
	}
    }
}

void
PrivateAnimScreen::initiateUnshadeAnim (PrivateAnimWindow *aw)
{
    CompWindow *w = aw->mWindow;

    aw->mAWindow->mSavedRectsValid = false;

    aw->setShaded (false);

    aw->mNewState = NormalState;

    int duration = 200;
    AnimEffect chosenEffect =
	getMatchingAnimSelection (w, AnimEventShade, &duration);

    if (chosenEffect != AnimEffectNone)
    {
	bool startingNew = true;
	bool playEffect = true;

	WindowEvent curWindowEvent = WindowEventNone;
	if (aw->curAnimation ())
	    curWindowEvent = aw->curAnimation ()->curWindowEvent ();

	if (curWindowEvent != WindowEventNone)
	{
	    if (curWindowEvent != WindowEventShade)
	    {
		aw->postAnimationCleanUpPrev (false, false);
	    }
	    else
	    {
		startingNew = false;
		aw->reverseAnimation ();
	    }
	}

	if (startingNew)
	{
	    AnimEffect effectToBePlayed =
		getActualEffect (chosenEffect, AnimEventShade);

	    // handle empty random effect list
	    if (effectToBePlayed == AnimEffectNone)
		playEffect = false;

	    if (playEffect)
	    {
		aw->mCurAnimation =
		    effectToBePlayed->create (w, WindowEventUnshade,
					      duration, effectToBePlayed,
					      getIcon (w, false));
		aw->enablePainting (true);
	    }
	}

	if (playEffect)
	{
	    activateEvent (true);
	    aw->notifyAnimation (true);
	    cScreen->damagePending ();
	}
    }
}

bool
PrivateAnimScreen::initiateFocusAnim (PrivateAnimWindow *aw)
{
    CompWindow *w = aw->mWindow;
    int duration = 200;

    if (aw->curAnimation () || otherPluginsActive () ||
	// Check the "switcher post-wait" counter that effectively prevents
	// focus animation to be initiated when the zoom option value is low
	// in Switcher.
	mSwitcherPostWait)
	return false;

    AnimEffect chosenEffect =
	getMatchingAnimSelection (w, AnimEventFocus, &duration);

    if (chosenEffect != AnimEffectNone)
    {
	aw->createFocusAnimation (chosenEffect, duration);

	if (chosenEffect->isRestackAnim &&
	    !(dynamic_cast<RestackAnim *> (aw->mCurAnimation)->
	      initiateRestackAnim (duration)))
	{
	    aw->postAnimationCleanUp ();
	    return false;
	}

	activateEvent (true);
	aw->notifyAnimation (true);
	cScreen->damagePending ();
	return true;
    }
    return false;
}

void
PrivateAnimWindow::resizeNotify (int dx,
				 int dy,
				 int dwidth,
				 int dheight)
{
    if (mUnshadePending)
    {
	mUnshadePending = false;
	mPAScreen->initiateUnshadeAnim (this);
    }
    else if (mCurAnimation && mCurAnimation->inProgress () &&
	// Don't let transient window open anim be interrupted with a resize notify
	!(mCurAnimation->curWindowEvent () == WindowEventOpen &&
	  (mWindow->wmType () &
	   (CompWindowTypeDropdownMenuMask |
	    CompWindowTypePopupMenuMask |
       	    CompWindowTypeMenuMask |
	    CompWindowTypeTooltipMask |
	    CompWindowTypeNotificationMask |
	    CompWindowTypeComboMask |
	    CompWindowTypeDndMask))) &&
	// Ignore resize with dx=0, dy=0, dwidth=0, dheight=0
	!(dx == 0 && dy == 0 && dwidth == 0 && dheight == 0) &&
	!mCurAnimation->resizeUpdate (dx, dy, dwidth, dheight))
    {
	postAnimationCleanUp ();
	mPAScreen->updateAnimStillInProgress ();
    }

    mWindow->resizeNotify (dx, dy, dwidth, dheight);
}

void
PrivateAnimScreen::updateAnimStillInProgress ()
{
    bool animStillInProgress = false;
    const CompWindowList &pl = pushLockedPaintList ();

    foreach (CompWindow *w, pl)
    {
	PrivateAnimWindow *aw = AnimWindow::get (w)->priv;
	if (aw->curAnimation () &&
	    aw->curAnimation ()->inProgress ())
	{
	    animStillInProgress = true;
	    break;
	}
	else
	{
	    aw->notifyAnimation (false);
	}
    }

    popLockedPaintList ();

    if (!animStillInProgress)
	activateEvent (false);
}

void
PrivateAnimWindow::moveNotify (int  dx,
			       int  dy,
			       bool immediate)
{
    if (mCurAnimation && mCurAnimation->inProgress () &&
    	(mGrabbed || !mCurAnimation->moveUpdate (dx, dy)))
    {
	// Stop the animation
    	postAnimationCleanUp ();
    	mPAScreen->updateAnimStillInProgress ();
    }

    mWindow->moveNotify (dx, dy, immediate);
}

void
PrivateAnimWindow::grabNotify (int          x,
			       int          y,
			       unsigned int state,
			       unsigned int mask)
{
    mGrabbed = true;

    mWindow->grabNotify (x, y, state, mask);
}

void
PrivateAnimWindow::ungrabNotify ()
{
    mGrabbed = false;

    mWindow->ungrabNotify ();
}

bool
PrivateAnimScreen::glPaintOutput (const GLScreenPaintAttrib &attrib,
				  const GLMatrix            &matrix,
				  const CompRegion          &region,
				  CompOutput                *output,
				  unsigned int               mask)
{
    assert (mAnimInProgress);

    mStartingNewPaintRound = true;

    foreach (ExtensionPluginInfo *extPlugin, mExtensionPlugins)
	extPlugin->prePaintOutput (output);

    mask |= PAINT_SCREEN_WITH_TRANSFORMED_WINDOWS_MASK;

    mOutput = output;

    return gScreen->glPaintOutput (attrib, matrix, region, output, mask);
}

AnimEffectUsedFor AnimEffectUsedFor::all ()
{
  AnimEffectUsedFor usedFor;
  usedFor.open = usedFor.close = usedFor.minimize = 
  usedFor.shade = usedFor.unminimize = usedFor.focus = true;
  return usedFor;
}

AnimEffectUsedFor AnimEffectUsedFor::none ()
{
  AnimEffectUsedFor usedFor;  
  usedFor.open = usedFor.close = usedFor.minimize = 
  usedFor.shade = usedFor.unminimize = usedFor.focus = false;
  return usedFor;
}

AnimEffectUsedFor& AnimEffectUsedFor::exclude (AnimEvent event)
{
  switch (event) {
    case AnimEventOpen: open = false; break;
    case AnimEventClose: close = false; break;
    case AnimEventMinimize: minimize = false; break;
    case AnimEventUnminimize: unminimize = false; break;
    case AnimEventShade: shade = false; break;
    case AnimEventFocus: focus = false; break;
    default: break;
  }
  return *this;
}

AnimEffectUsedFor& AnimEffectUsedFor::include (AnimEvent event)
{
  switch (event) {
    case AnimEventOpen: open = true; break;
    case AnimEventClose: close = true; break;
    case AnimEventMinimize: minimize = true; break;
    case AnimEventUnminimize: unminimize = true; break;
    case AnimEventShade: shade = true; break;
    case AnimEventFocus: focus = true; break;
    default: break;
  }
  return *this;
}

AnimEffectInfo::AnimEffectInfo (const char *name,
                               AnimEffectUsedFor usedFor,
				CreateAnimFunc create,
				bool isRestackAnim) :
    name (name),
    create (create),
    isRestackAnim (isRestackAnim)
{
    usedForEvents[AnimEventOpen] = usedFor.open;
    usedForEvents[AnimEventClose] = usedFor.close;
    usedForEvents[AnimEventMinimize] = usedFor.minimize;
    usedForEvents[AnimEventUnminimize] = usedFor.unminimize;
    usedForEvents[AnimEventShade] = usedFor.shade;
    usedForEvents[AnimEventFocus] = usedFor.focus;
}

bool
AnimEffectInfo::matchesEffectName (const CompString &animName)
{
    return (0 == strcasecmp (animName.c_str (), name));
}

bool
AnimEffectInfo::matchesPluginName (const CompString &pluginName)
{
    return (0 == strncmp (pluginName.c_str (), name, pluginName.length ()));
}

AnimEffect animEffects[NUM_EFFECTS];

ExtensionPluginAnimation animExtensionPluginInfo (CompString ("animation"),
                                                  NUM_EFFECTS, animEffects, 0,
						  NUM_NONEFFECT_OPTIONS);
ExtensionPluginInfo *
Animation::getExtensionPluginInfo ()
{
    return &animExtensionPluginInfo;
}

AnimEffect AnimEffectNone;
AnimEffect AnimEffectRandom;
AnimEffect AnimEffectCurvedFold;
AnimEffect AnimEffectDodge;
AnimEffect AnimEffectDream;
AnimEffect AnimEffectFade;
AnimEffect AnimEffectFocusFade;
AnimEffect AnimEffectGlide1;
AnimEffect AnimEffectGlide2;
AnimEffect AnimEffectHorizontalFolds;
AnimEffect AnimEffectMagicLamp;
AnimEffect AnimEffectMagicLampWavy;
AnimEffect AnimEffectRollUp;
AnimEffect AnimEffectSidekick;
AnimEffect AnimEffectWave;
AnimEffect AnimEffectZoom;

PrivateAnimScreen::PrivateAnimScreen (CompScreen *s, AnimScreen *as) :
    cScreen (CompositeScreen::get (s)),
    gScreen (GLScreen::get (s)),
    aScreen (as),
    mLastRedrawTimeFresh (false),
    mSwitcherPostWait (0),
    mStartCountdown (20), // start the countdown
    mLastActiveWindow (0),
    mAnimInProgress (false),
    mStartingNewPaintRound (false),
    mPrePaintWindowsBackToFrontEnabled (false),
    mOutput (0),
    mLockedPaintList (NULL),
    mLockedPaintListCnt (0),
    mGetWindowPaintListEnableCnt (0)
{
    for (int i = 0; i < WatchedScreenPluginNum; i++)
	mPluginActive[i] = false;

    // Never animate screen-dimming layer of logout window and gksu.
    mNeverAnimateMatch |= "title=gksu";
    mNeverAnimateMatch |= "title=x-session-manager";
    mNeverAnimateMatch |= "title=gnome-session";
    mNeverAnimateMatch.update ();

    // Set-up option notifiers

#define MATCHES_BIND    \
    boost::bind (&PrivateAnimScreen::eventMatchesChanged, this, _1, _2)
#define OPTIONS_BIND    \
    boost::bind (&PrivateAnimScreen::eventOptionsChanged, this, _1, _2)
#define EFFECTS_BIND    \
    boost::bind (&PrivateAnimScreen::eventEffectsChanged, this, _1, _2)
#define RANDOM_EFFECTS_BIND    \
    boost::bind (&PrivateAnimScreen::eventRandomEffectsChanged, this, _1, _2)

    optionSetOpenMatchesNotify (MATCHES_BIND);
    optionSetCloseMatchesNotify (MATCHES_BIND);
    optionSetMinimizeMatchesNotify (MATCHES_BIND);
    optionSetUnminimizeMatchesNotify (MATCHES_BIND);
    optionSetFocusMatchesNotify (MATCHES_BIND);
    optionSetShadeMatchesNotify (MATCHES_BIND);

    optionSetOpenOptionsNotify (OPTIONS_BIND);
    optionSetCloseOptionsNotify (OPTIONS_BIND);
    optionSetMinimizeOptionsNotify (OPTIONS_BIND);
    optionSetUnminimizeOptionsNotify (OPTIONS_BIND);
    optionSetFocusOptionsNotify (OPTIONS_BIND);
    optionSetShadeOptionsNotify (OPTIONS_BIND);

    optionSetOpenEffectsNotify (EFFECTS_BIND);
    optionSetCloseEffectsNotify (EFFECTS_BIND);
    optionSetMinimizeEffectsNotify (EFFECTS_BIND);
    optionSetUnminimizeEffectsNotify (EFFECTS_BIND);
    optionSetFocusEffectsNotify (EFFECTS_BIND);
    optionSetShadeEffectsNotify (EFFECTS_BIND);

    optionSetOpenRandomEffectsNotify (RANDOM_EFFECTS_BIND);
    optionSetCloseRandomEffectsNotify (RANDOM_EFFECTS_BIND);
    optionSetMinimizeRandomEffectsNotify (RANDOM_EFFECTS_BIND);
    optionSetUnminimizeRandomEffectsNotify (RANDOM_EFFECTS_BIND);
    optionSetShadeRandomEffectsNotify (RANDOM_EFFECTS_BIND);

    ScreenInterface::setHandler (::screen);
    CompositeScreenInterface::setHandler (cScreen, false);
    GLScreenInterface::setHandler (gScreen, false);
}

PrivateAnimScreen::~PrivateAnimScreen ()
{
    if (mAnimInProgress)
	activateEvent (false);

    for (int i = 0; i < NUM_EFFECTS; i++)
	delete animEffects[i];
}

void
PrivateAnimScreen::initAnimationList ()
{
    int i = 0;

    animEffects[i++] = AnimEffectNone =
	new AnimEffectInfo ("animation:None",
                            AnimEffectUsedFor::all(),
                            0);

    animEffects[i++] = AnimEffectRandom =
	new AnimEffectInfo ("animation:Random",
                           AnimEffectUsedFor::all().exclude(AnimEventFocus),
                           0);

    animEffects[i++] = AnimEffectCurvedFold =
	new AnimEffectInfo ("animation:Curved Fold",
                           AnimEffectUsedFor::all().exclude(AnimEventFocus),
			    &createAnimation<CurvedFoldAnim>);
        
    animEffects[i++] = AnimEffectDodge =
	new AnimEffectInfo ("animation:Dodge", 
                           AnimEffectUsedFor::none().include(AnimEventFocus),
			    &createAnimation<DodgeAnim>,
			    true);
        
    animEffects[i++] = AnimEffectDream =
	new AnimEffectInfo ("animation:Dream", 
                           AnimEffectUsedFor::all().exclude(AnimEventFocus).exclude(AnimEventShade),
			    &createAnimation<DreamAnim>);

    animEffects[i++] = AnimEffectFade =
	new AnimEffectInfo ("animation:Fade",
                           AnimEffectUsedFor::all().exclude(AnimEventFocus).exclude(AnimEventShade),
			    &createAnimation<FadeAnim>);
        
    animEffects[i++] = AnimEffectFocusFade =
	new AnimEffectInfo ("animation:Focus Fade", 
                           AnimEffectUsedFor::none().include(AnimEventFocus),
			    &createAnimation<FocusFadeAnim>,
			    true);
        
    animEffects[i++] = AnimEffectGlide1 =
	new AnimEffectInfo ("animation:Glide 1",
                           AnimEffectUsedFor::all().exclude(AnimEventFocus).exclude(AnimEventShade),
			    &createAnimation<GlideAnim>);
        
    animEffects[i++] = AnimEffectGlide2 =
	new AnimEffectInfo ("animation:Glide 2",
                           AnimEffectUsedFor::all().exclude(AnimEventFocus).exclude(AnimEventShade),
			    &createAnimation<Glide2Anim>);
        
    animEffects[i++] = AnimEffectHorizontalFolds =
	new AnimEffectInfo ("animation:Horizontal Folds",
                           AnimEffectUsedFor::all().exclude(AnimEventFocus),
			    &createAnimation<HorizontalFoldsAnim>);
        
    animEffects[i++] = AnimEffectMagicLamp =
	new AnimEffectInfo ("animation:Magic Lamp",
                           AnimEffectUsedFor::all().exclude(AnimEventFocus).exclude(AnimEventShade),
			    &createAnimation<MagicLampAnim>);
        
    animEffects[i++] = AnimEffectMagicLampWavy =
	new AnimEffectInfo ("animation:Magic Lamp Wavy",
                           AnimEffectUsedFor::all().exclude(AnimEventFocus).exclude(AnimEventShade),
			    &createAnimation<MagicLampWavyAnim>);
        
    animEffects[i++] = AnimEffectRollUp =
	new AnimEffectInfo ("animation:Roll Up",
                           AnimEffectUsedFor::none().include(AnimEventShade),
			    &createAnimation<RollUpAnim>);
        
    animEffects[i++] = AnimEffectSidekick =
	new AnimEffectInfo ("animation:Sidekick",
                           AnimEffectUsedFor::all().exclude(AnimEventFocus).exclude(AnimEventShade),
			    &createAnimation<SidekickAnim>);
        
    animEffects[i++] = AnimEffectWave =
	new AnimEffectInfo ("animation:Wave",
                           AnimEffectUsedFor::all().exclude(AnimEventShade),
			    &createAnimation<WaveAnim>);
    
    animEffects[i++] = AnimEffectZoom =
	new AnimEffectInfo ("animation:Zoom", 
                           AnimEffectUsedFor::all().exclude(AnimEventFocus).exclude(AnimEventShade),
			    &createAnimation<ZoomAnim>);

    animExtensionPluginInfo.effectOptions = &getOptions ();

    // Extends itself with the basic set of animation effects.
    addExtension (&animExtensionPluginInfo, false);

    for (int e = 0; e < AnimEventNum; e++) // for each anim event
	updateOptionSets ((AnimEvent)e);

    updateAllEventEffects ();

    cScreen->preparePaintSetEnabled (this, true);
}

PrivateAnimWindow::PrivateAnimWindow (CompWindow *w,
				      AnimWindow *aw) :
    gWindow (GLWindow::get (w)),
    mWindow (w),
    mAWindow (aw),
    mPAScreen (AnimScreen::get (::screen)->priv),
    mCurAnimation (0),
    mUnshadePending (false),
    mEventNotOpenClose (false),
    mNowShaded (false),
    mGrabbed (false),
    mUnmapCnt (0),
    mDestroyCnt (0),
    mIgnoreDamage (false),
    mFinishingAnim (false),
    mCurAnimSelectionRow (-1)
{
    mBB.x1 = mBB.y1 = MAXSHORT;
    mBB.x2 = mBB.y2 = MINSHORT;

    for (int i = 0; i < WatchedWindowPluginNum; i++)
	mPluginActive[i] = false;

    if (w->minimized ())
    {
	mState = mNewState = IconicState;
    }
    else if (w->shaded ())
    {
	mState = mNewState = NormalState;
	mNowShaded = true;
    }
    else
    {
	mState = mNewState = getState ();
    }

    WindowInterface::setHandler (mWindow, true);
    GLWindowInterface::setHandler (gWindow, false);
}

PrivateAnimWindow::~PrivateAnimWindow ()
{
    notifyAnimation (false);
    postAnimationCleanUpCustom (false, true, true);
}

void
PrivateAnimWindow::windowNotify (CompWindowNotify n)
{
    switch (n)
    {
	case CompWindowNotifyEnterShowDesktopMode:
	case CompWindowNotifyMinimize:
	    mPAScreen->initiateMinimizeAnim (this);
	    mEventNotOpenClose = true;
	    break;
	case CompWindowNotifyLeaveShowDesktopMode:
	case CompWindowNotifyUnminimize:
	    mPAScreen->initiateUnminimizeAnim (this);
	    mEventNotOpenClose = true;
	    break;
	case CompWindowNotifyShade:
	    mPAScreen->initiateShadeAnim (this);
	    mEventNotOpenClose = true;
	    break;
	case CompWindowNotifyUnshade:
	    if (mNowShaded &&
		mCurAnimation &&
		mCurAnimation->curWindowEvent () == WindowEventShade)
		mPAScreen->initiateUnshadeAnim (this); // reverse the shade anim
	    break;
	case CompWindowNotifyClose:
	    if (!(mCurAnimation &&
		  (mCurAnimation->curWindowEvent () == WindowEventClose ||
		   mCurAnimation->curWindowEvent () == WindowEventUnminimize)))
		mPAScreen->initiateCloseAnim (this);
	    break;
	case CompWindowNotifyShow:
	case CompWindowNotifyBeforeMap:
	    // Prevent dialog disappearing when a dialog is reopened during
	    // its close animation.
	    if (mCurAnimation &&
		mCurAnimation->curWindowEvent () == WindowEventClose)
	    {
		mPAScreen->initiateOpenAnim (this);
		mEventNotOpenClose = false;
	    }
	    break;
	case CompWindowNotifyMap:
	    if (mNowShaded)
		mUnshadePending = true;
	    else if (!mUnshadePending &&
		     !mEventNotOpenClose &&
		     !mPAScreen->mStartCountdown &&
		     !(mCurAnimation &&
		       (mCurAnimation->curWindowEvent () ==
			WindowEventUnminimize ||
			mCurAnimation->curWindowEvent () == WindowEventOpen)))
		mPAScreen->initiateOpenAnim (this);
	    mEventNotOpenClose = false;
	    break;
	case CompWindowNotifyBeforeUnmap:
	    if (mCurAnimation && mCurAnimation->curWindowEvent () == WindowEventMinimize)
	    {
		mUnmapCnt++;
		mWindow->incrementUnmapReference ();
	    }
	    break;
	case CompWindowNotifyBeforeDestroy:
	    if (!mFinishingAnim)
	    {
		if (mPAScreen->shouldIgnoreWindowForAnim (mWindow, true))
		    break;

		/* Don't increment the destroy reference count unless
		 * the window is already animated */
		if (!mCurAnimation)
		    break;

		mDestroyCnt++;
		mWindow->incrementDestroyReference ();
	    }
	    break;
	case CompWindowNotifyUnreparent:
	    if (!mFinishingAnim)
	    {
		if (mPAScreen->shouldIgnoreWindowForAnim (mWindow, false))
		    break;
	    }
	    break;
	case CompWindowNotifyFocusChange:
	    if (!mPAScreen->mLastActiveWindow ||
	    	mPAScreen->mLastActiveWindow != mWindow->id ())
	    {
		mPAScreen->mLastActiveWindow = mWindow->id ();

		if (mPAScreen->mStartCountdown) // Don't animate at startup
		    break;

		int duration = 200;
		AnimEffect chosenEffect =
		    mPAScreen->getMatchingAnimSelection (mWindow,
							 AnimEventFocus,
							 &duration);

		if (chosenEffect &&
		    chosenEffect != AnimEffectNone &&
		    !chosenEffect->isRestackAnim)
		    mPAScreen->initiateFocusAnim (this);
	    }

	    break;
	case CompWindowNotifyRestack:
	{
	    // Prevent menu disappearing when a menu is reopened during
	    // its close animation. In that case a restack notify is thrown
	    // for menus.
	    if (mCurAnimation &&
		mCurAnimation->curWindowEvent () == WindowEventClose)
	    {
		mPAScreen->initiateOpenAnim (this);
		mEventNotOpenClose = false;
		break;
	    }

	    // Handle CompWindowNotifyRestack only when necessary.
	    if (!mPAScreen->isRestackAnimPossible ())
		break;

	    if (mPAScreen->mStartCountdown) // Don't animate at startup
		break;

	    foreach (ExtensionPluginInfo *extPlugin,
	    	     mPAScreen->mExtensionPlugins)
		extPlugin->handleRestackNotify (mAWindow);

	    break;
	}
	default:
	    break;
    }

    mWindow->windowNotify (n);
}

Animation *
AnimWindow::curAnimation ()
{
    return priv->curAnimation ();
}

AnimEffect
AnimScreen::getMatchingAnimSelection (CompWindow *w,
				      AnimEvent e,
				      int *duration)
{
    return priv->getMatchingAnimSelection (w, e, duration);
}

bool
AnimScreen::otherPluginsActive ()
{
    return priv->otherPluginsActive ();
}

bool
AnimScreen::isAnimEffectPossible (AnimEffect theEffect)
{
    return priv->isAnimEffectPossible (theEffect);
}

bool
AnimScreen::initiateFocusAnim (AnimWindow *aw)
{
    return priv->initiateFocusAnim (aw->priv);
}

/// If duration is 0, it should be set to a positive value later.
void
AnimWindow::createFocusAnimation (AnimEffect effect, int duration)
{
    priv->createFocusAnimation (effect, duration);
}

void
AnimWindow::deletePersistentData (const char *name)
{
    PersistentDataMap::iterator itData =
	persistentData.find (name);
    if (itData != persistentData.end ()) // if found
    {
	delete itData->second;
	persistentData.erase (itData);
    }
}

void
PrivateAnimWindow::createFocusAnimation (AnimEffect effect, int duration)
{
    mCurAnimation =
	effect->create (mWindow, WindowEventFocus,
			duration,
			effect,
			CompRect ());
    enablePainting (true);
}

template class PluginClassHandler<AnimScreen, CompScreen, ANIMATION_ABI>;

AnimScreen::AnimScreen (CompScreen *s) :
    PluginClassHandler<AnimScreen, CompScreen, ANIMATION_ABI> (s),
    priv (new PrivateAnimScreen (s, this))
{
    priv->initAnimationList ();
}

AnimScreen::~AnimScreen ()
{
    delete priv;
}

template class PluginClassHandler<AnimWindow, CompWindow, ANIMATION_ABI>;

AnimWindow::AnimWindow (CompWindow *w) :
    PluginClassHandler<AnimWindow, CompWindow, ANIMATION_ABI> (w),
    mWindow (w),
    priv (new PrivateAnimWindow (w, this)),
    mSavedRectsValid (false)
{
    foreach (ExtensionPluginInfo *extPlugin, priv->mPAScreen->mExtensionPlugins)
	extPlugin->initPersistentData (this);
}

AnimWindow::~AnimWindow ()
{
    delete priv;

    // Destroy each persistent data object
    PersistentDataMap::iterator itData = persistentData.begin ();
    for (; itData != persistentData.end (); ++itData)
	delete itData->second;

    persistentData.clear ();
}

bool
AnimPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) |
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) |
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI))
	 return false;

    CompPrivate p;
    p.uval = ANIMATION_ABI;
    ::screen->storeValue ("animation_ABI", p);

    return true;
}

void
AnimPluginVTable::fini ()
{
    ::screen->eraseValue ("animation_ABI");
}

