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

#include "private.h"

class AnimAddonPluginVTable :
    public CompPlugin::VTableForScreenAndWindow<AnimAddonScreen, AnimAddonWindow>
{
public:
    bool init ();
    void fini ();
};

COMPIZ_PLUGIN_20090315 (animationaddon, AnimAddonPluginVTable);

// TODO Update this for each added animation effect! (total: 11)
const unsigned short NUM_EFFECTS = 11;

AnimEffect animEffects[NUM_EFFECTS];

ExtensionPluginAnimAddon animAddonExtPluginInfo (CompString ("animationaddon"),
						 NUM_EFFECTS, animEffects, NULL,
                                                 NUM_NONEFFECT_OPTIONS);

ExtensionPluginInfo *
BaseAddonAnim::getExtensionPluginInfo ()
{
    return &animAddonExtPluginInfo;
}

BaseAddonAnim::BaseAddonAnim (CompWindow *w,
			      WindowEvent curWindowEvent,
			      float duration,
			      const AnimEffect info,
			      const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    mIntenseTimeStep (AnimAddonScreen::get (::screen)->getIntenseTimeStep ()),
    mCScreen (CompositeScreen::get (::screen)),
    mGScreen (GLScreen::get (::screen)),
    mDoDepthTest (false)
{
}

AnimEffect AnimEffectAirplane;
AnimEffect AnimEffectBeamUp;
AnimEffect AnimEffectBurn;
AnimEffect AnimEffectDissolve;
AnimEffect AnimEffectDomino;
AnimEffect AnimEffectExplode;
AnimEffect AnimEffectFold;
AnimEffect AnimEffectGlide3;
AnimEffect AnimEffectLeafSpread;
AnimEffect AnimEffectRazr;
AnimEffect AnimEffectSkewer;

int AnimAddonScreen::getIntenseTimeStep ()
{
    return priv->optionGetTimeStepIntense ();
}

void
PrivateAnimAddonScreen::initAnimationList ()
{
    int i = 0;
    AnimEffectUsedFor usedFor = AnimEffectUsedFor::all()
                                .exclude(AnimEventFocus)
                                .exclude(AnimEventShade);

    animEffects[i++] = AnimEffectAirplane =
	new AnimEffectInfo ("animationaddon:Airplane", usedFor,
			    &createAnimation<AirplaneAnim>);
    animEffects[i++] = AnimEffectBeamUp =
	new AnimEffectInfo ("animationaddon:Beam Up", usedFor,
			    &createAnimation<BeamUpAnim>);
    animEffects[i++] = AnimEffectBurn =
	new AnimEffectInfo ("animationaddon:Burn", usedFor,
			    &createAnimation<BurnAnim>);
    animEffects[i++] = AnimEffectDissolve =
	new AnimEffectInfo ("animationaddon:Dissolve", usedFor,
			    &createAnimation<DissolveAnim>);
    animEffects[i++] = AnimEffectDomino =
	new AnimEffectInfo ("animationaddon:Domino", usedFor,
			    &createAnimation<DominoAnim>);
    animEffects[i++] = AnimEffectExplode =
	new AnimEffectInfo ("animationaddon:Explode", usedFor,
			    &createAnimation<ExplodeAnim>);
    animEffects[i++] = AnimEffectFold =
	new AnimEffectInfo ("animationaddon:Fold", usedFor,
			    &createAnimation<FoldAnim>);
    animEffects[i++] = AnimEffectGlide3 =
	new AnimEffectInfo ("animationaddon:Glide 3", usedFor,
			    &createAnimation<Glide3Anim>);
    animEffects[i++] = AnimEffectLeafSpread =
	new AnimEffectInfo ("animationaddon:Leaf Spread", usedFor,
			    &createAnimation<LeafSpreadAnim>);
    animEffects[i++] = AnimEffectRazr =
	new AnimEffectInfo ("animationaddon:Razr", usedFor,
			    &createAnimation<RazrAnim>);
    animEffects[i++] = AnimEffectSkewer =
	new AnimEffectInfo ("animationaddon:Skewer", usedFor,
			    &createAnimation<SkewerAnim>);

    animAddonExtPluginInfo.effectOptions = &getOptions ();

    AnimScreen *as = AnimScreen::get (::screen);

    // Extends animation plugin with this set of animation effects.
    as->addExtension (&animAddonExtPluginInfo);
}

PrivateAnimAddonScreen::PrivateAnimAddonScreen (CompScreen *s) :
    //cScreen (CompositeScreen::get (s)),
    //gScreen (GLScreen::get (s)),
    //aScreen (as),
    mOutput (s->fullscreenOutput ())
{
    initAnimationList ();
}

PrivateAnimAddonScreen::~PrivateAnimAddonScreen ()
{
    AnimScreen *as = AnimScreen::get (::screen);

    as->removeExtension (&animAddonExtPluginInfo);

    for (int i = 0; i < NUM_EFFECTS; i++)
    {
	delete animEffects[i];
	animEffects[i] = NULL;
    }
}

AnimAddonScreen::AnimAddonScreen (CompScreen *s) :
    PluginClassHandler<AnimAddonScreen, CompScreen, ANIMATIONADDON_ABI> (s),
    priv (new PrivateAnimAddonScreen (s))
{
}

AnimAddonScreen::~AnimAddonScreen ()
{
    delete priv;
}

CompOption::Vector &
AnimAddonScreen::getOptions ()
{
    return priv->getOptions ();
}

bool
AnimAddonScreen::setOption (const CompString  &name,
                            CompOption::Value &value)
{
    return priv->setOption (name, value);
}

AnimAddonWindow::AnimAddonWindow (CompWindow *w) :
    PluginClassHandler<AnimAddonWindow, CompWindow> (w),
    mWindow (w),
    aWindow (AnimWindow::get (w))
{
}

AnimAddonWindow::~AnimAddonWindow ()
{
    Animation *curAnim = aWindow->curAnimation ();

    if (!curAnim)
	return;

    // We need to interrupt and clean up the animation currently being played
    // by animationaddon for this window (if any)
    if (curAnim->remainingTime () > 0 &&
	curAnim->getExtensionPluginInfo ()->name ==
	    CompString ("animationaddon"))
    {
	aWindow->postAnimationCleanUp ();
    }
}

bool
AnimAddonPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) |
        !CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) |
        !CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI) |
        !CompPlugin::checkPluginABI ("animation", ANIMATION_ABI))
	 return false;

    CompPrivate p;
    p.uval = ANIMATIONADDON_ABI;
    ::screen->storeValue ("animationaddon_ABI", p);

    return true;
}

void
AnimAddonPluginVTable::fini ()
{
    ::screen->eraseValue ("animationaddon_ABI");
}
