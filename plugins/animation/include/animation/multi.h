#ifndef ANIMATION_MULTI_H
#define ANIMATION_MULTI_H
#include "animation.h"
/// Special class, allows multiple copies of an animation to happen
/// at any one time. Create your "single copy" animation class first
/// and then create a new animation which derives from this template
/// class. Each function of your animation will be called since this
/// class overloads everything in Animation. You will have
/// access to which number effect is happening. (I think)

template <class SingleAnim, int num>
class MultiAnim :
public Animation
{
public:
    static inline int getCurrAnimNumber (AnimWindow *aw)
    {
	MultiPersistentData *pd = static_cast <MultiPersistentData *>
	(aw->persistentData["multi"]);
	if (!pd)
	{
	    pd = new MultiPersistentData ();
	    aw->persistentData["multi"] = pd;
	}
	    if (!pd)
		return 0;
	    return pd->num;
	}

	static inline void setCurrAnimNumber (AnimWindow *aw, int what)
	{
	    MultiPersistentData *pd = static_cast <MultiPersistentData *>
	    (aw->persistentData["multi"]);
	    if (!pd)
		pd = new MultiPersistentData ();
	    if (!pd)
		return;
	    pd->num = what;
	}

    public:
	MultiAnim (CompWindow *w,
		   WindowEvent curWindowEvent,
		   float duration,
		   const AnimEffect info,
		   const CompRect &icon) :
			Animation (w, curWindowEvent, duration, info, icon),
			currentAnim (0)
	{
	    for (unsigned int i = 0; i < num; i++)
		animList.push_back (new SingleAnim (w,
						    curWindowEvent,
						    duration,
						    info, icon));
	    mGlPaintAttribs.resize (num);
	    mGlPaintTransforms.resize (num);
	}
	virtual ~MultiAnim () {}

    public:

	/// Overload everything

	/// Needed since virtual method calls can't be done in the constructor.
	void init ()
	{
	    int count = 0;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		a->init ();
	    }
	}

	/// To be called during post-animation clean up.
	void cleanUp (bool closing,
			  bool destructing)
	{
	    int count = 0;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		a->cleanUp (closing,
			   destructing);
		delete a;
	    }

	    animList.clear ();
	}

	/// Returns true if frame should be skipped (e.g. due to
	/// higher timestep values). In that case no drawing is
	/// needed for that window in current frame.
	bool shouldSkipFrame (int msSinceLastPaintActual)
	{
	    int count = 0;
	    bool skip = false;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		skip |= a->shouldSkipFrame (msSinceLastPaintActual);
	    }
	    return skip;
	}

	/// Advances the animation time by the given time amount.
	/// Returns true if more animation time is left.
	bool advanceTime (int msSinceLastPaint)
	{
	    int count = 0;
	    bool advance = false;
	    advance |= Animation::advanceTime (msSinceLastPaint);
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		advance |= a->advanceTime (msSinceLastPaint);
	    }
	    return advance;
	}

	/// Computes new animation state based on remaining time.
	void step ()
	{
	    int count = 0;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		a->step ();
	    }
	}

	void updateAttrib (GLWindowPaintAttrib &attrib)
	{
	    int count = 0;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		GLWindowPaintAttrib attr (attrib);
		a->updateAttrib (attr);
		mGlPaintAttribs.at (count) = attr;
		count++;
	    }
	}

	void updateTransform (GLMatrix &transform)
	{
	    int count = 0;
	    
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		GLMatrix mat (transform);
		a->updateTransform (mat);
		mGlPaintTransforms.at (count) = mat;
		count++;
	    }
	}

	void prePaintWindow ()
	{
	    int count = 0;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		a->prePaintWindow ();
	    }
	}

	void postPaintWindow ()
	{
	    int count = 0;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		a->postPaintWindow ();
	    }
	}

	bool postPaintWindowUsed ()
	{
	    int count = 0;
	    bool used = false;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		used |= a->postPaintWindowUsed ();
	    }
	    return used;
	}

	/// Returns true if the animation is still in progress.
	bool prePreparePaint (int msSinceLastPaint)
	{
	    int count = 0;
	    bool inProgress = false;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		inProgress |= a->prePreparePaint (msSinceLastPaint);
	    }
	    return inProgress;
	}

	void postPreparePaint ()
	{
	    int count = 0;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		a->postPreparePaint ();
	    }
	}

	/// Updates the bounding box of damaged region. Should be implemented for
	/// any animation that doesn't update the whole screen.
	// NB!!
	void updateBB (CompOutput &out)
	{
	    int count = 0;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		a->updateBB (out);
	    }
	}

	bool updateBBUsed ()
	{
	    int count = 0;
	    bool used = false;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		used |= a->updateBBUsed ();
	    }
	    return used;
	}

	/// Should return true for effects that make use of a region
	/// instead of just a bounding box for damaged area->
	bool stepRegionUsed ()
	{
	    int count = 0;
	    bool used = false;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		used |= a->stepRegionUsed ();
	    }
	    return used;
	}

	bool shouldDamageWindowOnStart ()
	{
	    int count = 0;
	    bool should = false;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		should |= a->shouldDamageWindowOnStart ();
	    }
	    return should;
	}

	bool shouldDamageWindowOnEnd ()
	{
	    int count = 0;
	    bool should = false;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		should |= a->shouldDamageWindowOnStart ();
	    }
	    return should;
	}

	/// Should return false if the animation should be stopped on move
	bool moveUpdate (int dx, int dy)
	{
	    int count = 0;
	    bool update = false;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		update |= a->moveUpdate (dx, dy);
	    }
	    return update;
	}

	/// Should return false if the animation should be stopped on resize
	bool resizeUpdate (int dx, int dy,
			   int dwidth, int dheight)
	{
	    int count = 0;
	    bool update = false;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		update |= a->resizeUpdate (dx, dy, dwidth, dheight);
	    }
	    return update;
	}

	void adjustPointerIconSize ()
	{
	    int count = 0;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		a->adjustPointerIconSize ();
	    }
	}

	void addGeometry (const GLTexture::MatrixList &matrix,
			    const CompRegion            &region,
			    const CompRegion            &clip,
			    unsigned int                maxGridWidth,
			    unsigned int                maxGridHeight)
	{
	    setCurrAnimNumber (mAWindow, currentAnim);
	    animList.at (currentAnim)->addGeometry
		    (matrix, region, clip, maxGridWidth, maxGridHeight);
	}

	void drawGeometry ()
	{
	    setCurrAnimNumber (mAWindow, currentAnim);
	    animList.at (currentAnim)->drawGeometry ();
	}

	bool paintWindowUsed ()
	{
	    int count = 0;
	    foreach (SingleAnim *a, animList)
	    {
		setCurrAnimNumber (mAWindow, count);
		count++;
		a->paintWindowUsed ();
	    }
	    /* Always return true because we need to take over painting */
	    return true;
	}

	bool paintWindow (GLWindow *gWindow,
			  const GLWindowPaintAttrib &attrib,
			  const GLMatrix &transform,
			  const CompRegion &region,
			  unsigned int mask)
	{
	    int count = 0;
	    bool status = false;

	    for (currentAnim = 0; currentAnim < animList.size (); currentAnim++)
	    {
		GLWindowPaintAttrib wAttrib (mGlPaintAttribs.at (currentAnim));
		GLMatrix wTransform (mGlPaintTransforms.at (currentAnim));

		setCurrAnimNumber (mAWindow, count);
		count++;

		if (animList.at (currentAnim)->paintWindowUsed ())
		    status |= animList.at (currentAnim)->paintWindow
			    (gWindow, wAttrib, wTransform, region, mask);
		else
		    status |= gWindow->glPaint
			    (wAttrib, wTransform, region, mask);
	    }

	    return status;
	}

    private:

	std::vector <GLWindowPaintAttrib>   mGlPaintAttribs;
	std::vector <GLMatrix>		    mGlPaintTransforms;
	std::vector <SingleAnim *> animList;
	unsigned int	    currentAnim;
};
#endif
