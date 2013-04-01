#ifndef ANIMATION_ANIMEFFECT_H
#define ANIMATION_ANIMEFFECT_H
#include "animation.h"

typedef Animation *(*CreateAnimFunc) (CompWindow *w,
				       WindowEvent curWindowEvent,
				       float duration,
				       const AnimEffect info,
				       const CompRect &icon);

class AnimEffectUsedFor
{
public:
  static AnimEffectUsedFor all();
  static AnimEffectUsedFor none();
  AnimEffectUsedFor& exclude(AnimEvent event);
  AnimEffectUsedFor& include(AnimEvent event);
 
  bool open;
  bool close;
  bool minimize;
  bool shade;
  bool unminimize;
  bool focus;
};

 /// Animation info class that holds the name, the list of supported events, and
 /// the creator function for a subclass of Animation.
 /// A pointer to this class is used as an identifier for each implemented
 /// animation.
class AnimEffectInfo
{
public:
     AnimEffectInfo (const char *name,
		     AnimEffectUsedFor usedFor,
		     CreateAnimFunc create, bool isRestackAnim = false);
     ~AnimEffectInfo () {}
     
     bool matchesEffectName (const CompString &animName);
     
     bool matchesPluginName (const CompString &pluginName);
     
     const char *name; ///< Name of the animation effect, e.g. "animationpack:Implode".
							       
    /// To be set to true for the window event animation list(s) that
    /// the new animation (value) should be added to
    /// (0: open, 1: close, 2: minimize, 3: shade, 4: unminimize, 5: focus)
    bool usedForEvents[AnimEventNum];

    /// Creates an instance of the Animation subclass and returns it as an
    /// Animation instance.
    CreateAnimFunc create;

    /// Is it a complex focus animation? (i.e. restacking-related,
    /// like FocusFade/Dodge)
    bool isRestackAnim;
};

template<class T>
Animation *createAnimation (CompWindow *w,
			    WindowEvent curWindowEvent,
			    float duration,
			    const AnimEffect info,
			    const CompRect &icon)
{
    return new T (w, curWindowEvent, duration, info, icon);
}


/** The base class for all animations.
 A n*imations should derive from the closest animation class
 to override as few methods as possible. Also, an animation
 method should call ancestors' methods instead of duplicating
 their code.
 */
class Animation
{
protected:
    CompWindow *mWindow;
    AnimWindow *mAWindow;
    
    float mTotalTime;
    float mRemainingTime;
    float mTimestep;		///< to store anim. timestep at anim. start
    float mTimeElapsedWithinTimeStep;
    
    int mTimeSinceLastPaint;    ///< in milliseconds
    
    int mOverrideProgressDir;	///< 0: default dir, 1: forward, 2: backward
    
    GLWindowPaintAttrib mCurPaintAttrib;
    GLushort mStoredOpacity;
    WindowEvent mCurWindowEvent;
    bool mInitialized; ///< whether the animation is initialized (in preparePaint)
    
    AnimEffect mInfo; ///< information about the animation class
    
    CompRect mIcon;
    
    int mDecorTopHeight;
    int mDecorBottomHeight;
    
    GLTexture::List *texturesCache;
    
    CompOption::Value &optVal (unsigned int optionId);
    
    inline bool       optValB (unsigned int optionId) { return optVal (optionId).b (); }
    inline int        optValI (unsigned int optionId) { return optVal (optionId).i (); }
    inline float      optValF (unsigned int optionId) { return optVal (optionId).f (); }
    inline CompString optValS (unsigned int optionId) { return optVal (optionId).s (); }
    inline unsigned short *optValC (unsigned int optionId) { return optVal (optionId).c (); }
    
public:
    
    Animation (CompWindow *w,
	       WindowEvent curWindowEvent,
	       float duration,
	       const AnimEffect info,
	       const CompRect &icon);
    virtual ~Animation ();
    
    inline AnimEffect info () { return mInfo; }
    
    // Overridable animation methods.
    
    /// Needed since virtual method calls can't be done in the constructor.
    virtual void init () {}
    
    /// To be called during post-animation clean up.
    virtual void cleanUp (bool closing,
			  bool destructing) {}
			  
    /// Returns true if frame should be skipped (e.g. due to
    /// higher timestep values). In that case no drawing is
    /// needed for that window in current frame.
    virtual bool shouldSkipFrame (int msSinceLastPaintActual);

    /// Advances the animation time by the given time amount.
    /// Returns true if more animation time is left.
    virtual bool advanceTime (int msSinceLastPaint);

    /// Computes new animation state based on remaining time.
    virtual void step () {}
    virtual void updateAttrib (GLWindowPaintAttrib &) {}
    virtual void updateTransform (GLMatrix &) {}
    virtual void prePaintWindow () {}
    virtual void postPaintWindow () {}
    virtual bool postPaintWindowUsed () { return false; }

    /// Returns true if the animation is still in progress.
    virtual bool prePreparePaint (int msSinceLastPaint) { return false; }
    virtual void postPreparePaint () {}

    /// Updates the bounding box of damaged region. Should be implemented for
    /// any animation that doesn't update the whole screen.
    virtual void updateBB (CompOutput &) {}
    virtual bool updateBBUsed () { return false; }

    /// Should return true for effects that make use of a region
    /// instead of just a bounding box for damaged area.
    virtual bool stepRegionUsed () { return false; }

    virtual bool shouldDamageWindowOnStart ();
    virtual bool shouldDamageWindowOnEnd ();

    /// Should return false if the animation should be stopped on move
    virtual bool moveUpdate (int dx, int dy) { return true; }

    /// Should return false if the animation should be stopped on resize
    virtual bool resizeUpdate (int dx, int dy,
				int dwidth, int dheight) { return true; }

    virtual void adjustPointerIconSize () {}
    virtual void addGeometry (const GLTexture::MatrixList &matrix,
			    const CompRegion            &region,
			    const CompRegion            &clip,
			    unsigned int                maxGridWidth,
			    unsigned int                maxGridHeight);
    virtual void drawGeometry ();

    virtual bool paintWindowUsed () { return false; }
    virtual bool paintWindow (GLWindow			*gWindow,
			    const GLWindowPaintAttrib &attrib,
			    const GLMatrix		&transform,
			    const CompRegion		&region,
			    unsigned int		mask)
    {
	return gWindow->glPaint (attrib, transform, region, mask);
    }

    virtual bool requiresTransformedWindow () const { return true; }

    /// Gets info about the (extension) plugin that implements this animation.
    /// Should be overriden by a base animation class in every extension plugin.
    virtual ExtensionPluginInfo *getExtensionPluginInfo ();

    void drawTexture (GLTexture          *texture,
                      const GLWindowPaintAttrib &attrib,
		      unsigned int       mask);

    // Utility methods

    void reverse ();
    inline bool inProgress () { return (mRemainingTime > 0); }

    inline WindowEvent curWindowEvent () { return mCurWindowEvent; }
    inline float totalTime () { return mTotalTime; }
    inline float remainingTime () { return mRemainingTime; }

    float progressLinear ();
    float progressEaseInEaseOut ();
    float progressDecelerateCustom (float progress,
				    float minx, float maxx);
    float progressDecelerate (float progress);
    AnimDirection getActualAnimDirection (AnimDirection dir,
					  bool openDir);
    void perspectiveDistortAndResetZ (GLMatrix &transform);

    static void prepareTransform (CompOutput &output,
				  GLMatrix &resultTransform,
				  GLMatrix &transform);
    void setInitialized () { mInitialized = true; }
    inline bool initialized () { return mInitialized; }
    inline void setCurPaintAttrib (const GLWindowPaintAttrib &newAttrib)
    { mCurPaintAttrib = newAttrib; }
};
#endif
