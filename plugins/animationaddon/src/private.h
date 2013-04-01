#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include <animation/animation.h>
#include <animation/multi.h>
#include <animationaddon/animationaddon.h>

#include "animationaddon_options.h"


extern AnimEffect AnimEffectBurn;
extern AnimEffect AnimEffectExplode;
extern AnimEffect AnimEffectAirplane;
extern AnimEffect AnimEffectBeamUp;
extern AnimEffect AnimEffectDissolve;
extern AnimEffect AnimEffectDomino;
extern AnimEffect AnimEffectFold;
extern AnimEffect AnimEffectGlide3;
extern AnimEffect AnimEffectLeafSpread;
extern AnimEffect AnimEffectRazr;
extern AnimEffect AnimEffectSkewer;

// TODO Update this for each added animation effect! (total: 11)
extern const unsigned short NUM_EFFECTS;

// This must have the value of the first "effect setting" above
// in AnimAddonScreenOptions
#define NUM_NONEFFECT_OPTIONS AnimationaddonOptions::AirplanePathLength

class ExtensionPluginAnimAddon : public ExtensionPluginInfo
{
public:
    ExtensionPluginAnimAddon (const CompString &name,
			      unsigned int nEffects,
			      AnimEffect *effects,
			      CompOption::Vector *effectOptions,
			      unsigned int firstEffectOptionIndex) :
	ExtensionPluginInfo (name, nEffects, effects, effectOptions,
			     firstEffectOptionIndex) {}
    ~ExtensionPluginAnimAddon () {}

    void prePaintOutput (CompOutput *output);
    const CompOutput *output () { return mOutput; }

private:
    const CompOutput *mOutput;
};

class PrivateAnimAddonScreen :
    public AnimationaddonOptions
{
    friend class AnimAddonScreen;

public:
    PrivateAnimAddonScreen (CompScreen *);
    ~PrivateAnimAddonScreen ();

protected:
    void initAnimationList ();

    CompOutput &mOutput;
};

class AnimAddonWindow :
    public PluginClassHandler<AnimAddonWindow, CompWindow>
{
public:
    AnimAddonWindow (CompWindow *);
    ~AnimAddonWindow ();

protected:
    CompWindow *mWindow;    ///< Window being animated.
    AnimWindow *aWindow;
};


// Particle-based animations
class BeamUpAnim : public ParticleAnim
{
public:
    BeamUpAnim (CompWindow *w,
		WindowEvent curWindowEvent,
		float duration,
		const AnimEffect info,
		const CompRect &icon);

protected:
    void init ();
    void step ();
    void genNewBeam (int x,
		     int y,
		     int width,
		     int height,
		     float size,
		     float time);
    void updateAttrib (GLWindowPaintAttrib &wAttrib);

    float mLife;
    unsigned short *mColor;
    float mSize;
    int mSpacing;
    float mSlowdown;
};

class BurnAnim : public ParticleAnim
{
public:
    BurnAnim (CompWindow *w,
              WindowEvent curWindowEvent,
              float duration,
              const AnimEffect info,
              const CompRect &icon);

protected:
    void step ();
    void genNewFire (int x,
		     int y,
		     int width,
		     int height,
		     float size,
		     float time);
    void genNewSmoke (int x,
		      int y,
		      int width,
		      int height,
		      float size,
		      float time);

    int mDirection;
    bool mMysticalFire;
    float mLife;
    unsigned short *mColor;
    float mSize;
    bool mHasSmoke;

    unsigned int mFirePSId;
    unsigned int mSmokePSId;
};

// Polygon-based animations

class ExplodeAnim : public PolygonAnim
{
public:
    ExplodeAnim (CompWindow *w,
		 WindowEvent curWindowEvent,
		 float duration,
		 const AnimEffect info,
		 const CompRect &icon);

    void init ();

protected:
    static const float kDurationFactor;
};

/// Extended polygon object for airplane folding and flying airplane fold phase.
class AirplanePolygonObject : public PolygonObject
{
public:

    Vector3d rotAxisA;			// Rotation axis vector A
    Vector3d rotAxisB;			// Rotation axis vector B

    Point3d rotAxisOffsetA;		// Rotation axis translate amount A
    Point3d rotAxisOffsetB; 	        // Rotation axis translate amount B

    float rotAngleA;			// Rotation angle A
    float finalRotAngA;			// Final rotation angle A

    float rotAngleB;			// Rotation angle B
    float finalRotAngB;			// Final rotation angle B

    // airplane fly phase:

    Vector3d centerPosFly;	// center position (offset) during the flying phases

    Vector3d flyRotation;	// airplane rotation during the flying phases
    Vector3d flyFinalRotation;	// airplane rotation during the flying phases

    float flyScale;             // Scale for airplane flying effect
    float flyFinalScale;        // Final Scale for airplane flying effect

    float flyTheta;		// Theta parameter for fly rotations and positions

    float moveStartTime2;		// Movement starts at this time ([0-1] range)
    float moveDuration2;		// Movement lasts this long     ([0-1] range)

    float moveStartTime3;		// Movement starts at this time ([0-1] range)
    float moveDuration3;		// Movement lasts this long     ([0-1] range)

    float moveStartTime4;		// Movement starts at this time ([0-1] range)
    float moveDuration4;		// Movement lasts this long     ([0-1] range)

    float moveStartTime5;	        // Movement starts at this time ([0-1] range)
    float moveDuration5;		// Movement lasts this long     ([0-1] range)
};

class AirplaneAnim :
    public PolygonAnim
{
    public:

	AirplaneAnim (CompWindow *w,
		    WindowEvent curWindowEvent,
		    float duration,
		    const AnimEffect info,
		    const CompRect &icon);

	~AirplaneAnim ();

	void
	stepPolygon (PolygonObject *p, float);

	void
	init ();

	void
	transformPolygon (const PolygonObject *p);

	bool
	tesselateIntoAirplane ();

	void
	updateBB (CompOutput &);

	void
	freePolygonObjects ();

	static const float kDurationFactor;
};


class DominoAnim : public PolygonAnim
{
public:
    DominoAnim (CompWindow *w,
		WindowEvent curWindowEvent,
		float duration,
		const AnimEffect info,
		const CompRect &icon);

    void init ();

protected:
    static const float kDurationFactor;
};

class RazrAnim : public DominoAnim
{
public:
    RazrAnim (CompWindow *w,
	      WindowEvent curWindowEvent,
	      float duration,
	      const AnimEffect info,
	      const CompRect &icon);
};


class FoldAnim : public PolygonAnim
{
public:
    FoldAnim (CompWindow *w,
	      WindowEvent curWindowEvent,
	      float duration,
	      const AnimEffect info,
	      const CompRect &icon);

    void init ();

    void stepPolygon (PolygonObject *p, float);

    static const float kDurationFactor;
};

class Glide3Anim : public PolygonAnim
{
public:
    Glide3Anim (CompWindow *w,
		WindowEvent curWindowEvent,
		float duration,
		const AnimEffect info,
		const CompRect &icon);

    bool deceleratingMotion () { return true; }

    static const float kDurationFactor;

    void
    init ();
};

class LeafSpreadAnim : public PolygonAnim
{
public:
    LeafSpreadAnim (CompWindow *w,
		    WindowEvent curWindowEvent,
		    float duration,
		    const AnimEffect info,
		    const CompRect &icon);

    void init ();

protected:
    static const float kDurationFactor;
};

class SkewerAnim : public PolygonAnim
{
public:
    SkewerAnim (CompWindow *w,
		WindowEvent curWindowEvent,
		float duration,
		const AnimEffect info,
		const CompRect &icon);

    void init ();
    void stepPolygon (PolygonObject *p,
		      float forwardProgress);

protected:
    static const float kDurationFactor;
};

/* TODO: Make a MultiAnim */

class DissolveSingleAnim :
    virtual public Animation,
    virtual public TransformAnim
{
public:
    DissolveSingleAnim (CompWindow *w,
			WindowEvent curWindowEvent,
			float duration,
			const AnimEffect info,
			const CompRect &icon);

    void step () { TransformAnim::step (); }

    void updateBB (CompOutput &output);
    bool updateBBUsed () { return true; }

    void updateTransform (GLMatrix &);

    void updateAttrib (GLWindowPaintAttrib &wAttrib);
    virtual float getDissolveSingleProgress () { return progressLinear (); }
};

class DissolveAnim :
    public MultiAnim <DissolveSingleAnim, 5>
{
public:
    DissolveAnim (CompWindow *w,
		  WindowEvent curWindowEvent,
		  float duration,
		  const AnimEffect info,
		  const CompRect &icon) :
	MultiAnim <DissolveSingleAnim, 5>::MultiAnim
		(w, curWindowEvent, duration, info, icon)
    {
    }
};
