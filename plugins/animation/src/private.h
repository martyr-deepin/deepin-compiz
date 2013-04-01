#include <string.h>
#include <stdlib.h>
#include <math.h>

#include <core/core.h>
#include <composite/composite.h>
#include <opengl/opengl.h>

#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

#include <animation/animation.h>

#include "animation_options.h"

typedef std::vector<CompWindow *> CompWindowVector;
typedef std::vector<ExtensionPluginInfo *> ExtensionPluginVector;
typedef std::vector<AnimEffect> AnimEffectVector;


class RestackInfo
{
public:
    RestackInfo (CompWindow *wRestacked,
		 CompWindow *wStart,
		 CompWindow *wEnd,
		 CompWindow *wOldAbove,
		 bool raised);

    CompWindow *wRestacked, *wStart, *wEnd, *wOldAbove;
    bool raised;
};

class IdValuePair
{
public:
    IdValuePair () : pluginInfo (0), optionId (-1), value () {}

    bool matchesPluginOption (ExtensionPluginInfo *pluginInfo,
			      int optionId) const;

    const ExtensionPluginInfo *pluginInfo;
    int optionId;
    CompOption::Value value;
};

typedef std::vector<IdValuePair> IdValuePairVector;

class OptionSet
{
public:
    OptionSet () {}

    IdValuePairVector pairs;
};

typedef std::vector<OptionSet> OptionSetVector;

class OptionSets
{
public:
    OptionSets () {}

    OptionSetVector sets;
};

class EffectSet
{
public:
    EffectSet () {}

    AnimEffectVector effects;
};

extern AnimEffect AnimEffectNone;
extern AnimEffect AnimEffectRandom;
extern AnimEffect AnimEffectCurvedFold;
extern AnimEffect AnimEffectDodge;
extern AnimEffect AnimEffectDream;
extern AnimEffect AnimEffectFade;
extern AnimEffect AnimEffectFocusFade;
extern AnimEffect AnimEffectGlide1;
extern AnimEffect AnimEffectGlide2;
extern AnimEffect AnimEffectHorizontalFolds;
extern AnimEffect AnimEffectMagicLamp;
extern AnimEffect AnimEffectMagicLampWavy;
extern AnimEffect AnimEffectRollUp;
extern AnimEffect AnimEffectSidekick;
extern AnimEffect AnimEffectWave;
extern AnimEffect AnimEffectZoom;

extern const unsigned short NUM_EFFECTS;

extern int customOptionOptionIds[AnimEventNum];

typedef struct _PluginEventInfo
{
    const char *pluginName;
    const char *activateEventName;
} PluginEventInfo;

typedef enum
{
    WatchedPluginSwitcher = 0,
    WatchedPluginRing,
    WatchedPluginShift,
    WatchedPluginScale,
    WatchedPluginGroup,
    WatchedPluginFadedesktop,
    WatchedScreenPluginNum
} WatchedScreenPlugin;

typedef enum
{
    WatchedPluginKDECompat,
    WatchedWindowPluginNum
} WatchedWindowPlugin;

// This must have the value of the first "effect setting" above
// in PrivateAnimScreenOptions
#define NUM_NONEFFECT_OPTIONS AnimationOptions::CurvedFoldAmpMult


class ExtensionPluginAnimation : public ExtensionPluginInfo
{
public:
    ExtensionPluginAnimation (const CompString &name,
			      unsigned int nEffects,
			      AnimEffect *effects,
			      CompOption::Vector *effectOptions,
			      unsigned int firstEffectOptionIndex);
    ~ExtensionPluginAnimation ();

    // Overriden methods from ExtensionPluginInfo
    void postPreparePaintGeneral ();
    void prePreparePaintGeneral ();
    void handleRestackNotify (AnimWindow *aw);
    // Always reset stacking related info when a window is opened, closed,
    // minimized, or unminimized.
    void preInitiateOpenAnim (AnimWindow *aw);
    void preInitiateCloseAnim (AnimWindow *aw);
    void preInitiateMinimizeAnim (AnimWindow *aw);
    void preInitiateUnminimizeAnim (AnimWindow *aw);
    void initPersistentData (AnimWindow *aw);
    void destroyPersistentData (AnimWindow *aw);
    void postUpdateEventEffects (AnimEvent e,
				 bool forRandom);
    void cleanUpAnimation (bool closing,
			   bool destructing);
    void postStartupCountdown ();

    // Other methods
    void handleSingleRestack (AnimWindow *aw);
    void prePaintWindowsBackToFront ();
    bool paintShouldSkipWindow (CompWindow *w);
    const CompWindowList & getWindowPaintList ();
    void resetStackingInfo ();
    static CompWindow *getBottommostInExtendedFocusChain (CompWindow *wStartPoint);
    static CompWindow *getBottommostInRestackChain (CompWindow *wStartPoint);
    void resetMarks ();
    bool markNewCopy (CompWindow *w);
    CompWindow * walkFirst ();
    CompWindow * walkNext (CompWindow *w);
    void incrementCurRestackAnimCount ();
    void decrementCurRestackAnimCount ();
    bool wontCreateCircularChain (CompWindow *wCur, CompWindow *wNext);

    static void cleanUpParentChildChainItem (AnimWindow *aw);
    static bool relevantForRestackAnim (CompWindow *w);

    /// Is restackInfo still good?
    static bool restackInfoStillGood (RestackInfo *restackInfo);

    void updateLastClientList ();

    /// A window was restacked this paint round.
    bool mAWinWasRestackedJustNow;

private:
    CompWindowVector mLastClientList; ///< Last known stacking order
    CompWindowVector mPrevClientList; ///< The stacking order before mLastClientList
    int mRestackAnimCount; ///< Count of how many windows are currently involved in
			   ///< animations that require walker (dodge & focus fade).
    std::vector<AnimWindow *> mRestackedWindows;

    CompWindowList mWindowList;
};

class PrivateAnimScreen :
    public ScreenInterface,
    public CompositeScreenInterface,
    public GLScreenInterface,
    public AnimationOptions
{
    friend class PrivateAnimWindow;
    friend class AnimWindow;

public:
    CompositeScreen *cScreen;
    GLScreen *gScreen;
    AnimScreen *aScreen;

private:
    struct timeval mLastRedrawTime;
    bool mLastRedrawTimeFresh;

    bool mPluginActive[WatchedScreenPluginNum];
    int mSwitcherPostWait;
    int mStartCountdown;
    ///< To mark windows as "created" if they were opened before compiz
    ///< was started and to prevent already opened windows from doing
    ///< open animation.

    Window mLastActiveWindow; ///< Last known active window

    bool mAnimInProgress;         ///< Is an animation currently being played?
    bool mStartingNewPaintRound;  ///< Is a new round of glPaints starting?
    bool mPrePaintWindowsBackToFrontEnabled;

    EffectSet mRandomEffects[AnimEventNum];

    OptionSets mEventOptionSets[AnimEventNum];

    // Effect extension plugins
    ExtensionPluginVector mExtensionPlugins;

    // Possible effects for each event
    AnimEffectVector mEventEffectsAllowed[AnimEventNum];

    // List of chosen effects for each event
    EffectSet mEventEffects[AnimEventNum];

    CompOutput *mOutput;

    Window mActiveWindow;
    CompMatch mNeverAnimateMatch;

    const CompWindowList *mLockedPaintList;
    unsigned int         mLockedPaintListCnt;
    unsigned int         mGetWindowPaintListEnableCnt;

    void updateEventEffects (AnimEvent e,
			     bool forRandom,
			     bool callPost = true);
    void updateAllEventEffects ();

    void updateOptionSets (AnimEvent e);
    void updateOptionSet (OptionSet *os, const char *optNamesValuesOrig);

    void activateEvent (bool activating);
    bool isWinVisible (CompWindow *w);
    AnimEvent getCorrespondingAnimEvent (AnimationOptions::Options optionId);
    void eventMatchesChanged (CompOption *opt, AnimationOptions::Options num);
    void eventOptionsChanged (CompOption *opt, AnimationOptions::Options num);
    void eventEffectsChanged (CompOption *opt, AnimationOptions::Options num);
    void eventRandomEffectsChanged (CompOption *opt, AnimationOptions::Options num);

    CompRect getIcon (CompWindow *w, bool alwaysUseMouse);
    void updateAnimStillInProgress ();

    bool isAnimEffectInList (AnimEffect theEffect,
			     EffectSet &effectList);
    bool isAnimEffectPossibleForEvent (AnimEffect theEffect,
				       AnimEvent event);

public:
    PrivateAnimScreen (CompScreen *s, AnimScreen *);
    ~PrivateAnimScreen ();

    // In order to prevent other plugins from modifying
    // the paint lists as we use it we need to lock the
    // list

    const CompWindowList & pushLockedPaintList ();
    void  popLockedPaintList ();

    void pushPaintList ();
    void popPaintList ();

    // Utility methods
    void initiateOpenAnim (PrivateAnimWindow *aw);
    void initiateCloseAnim (PrivateAnimWindow *aw);
    void initiateMinimizeAnim (PrivateAnimWindow *aw);
    void initiateUnminimizeAnim (PrivateAnimWindow *aw);
    void initiateShadeAnim (PrivateAnimWindow *aw);
    void initiateUnshadeAnim (PrivateAnimWindow *aw);
    bool initiateFocusAnim (PrivateAnimWindow *aw);

    /// Is a restacking animation currently possible?
    bool isRestackAnimPossible ();

    void initAnimationList ();
    bool isAnimEffectPossible (AnimEffect theEffect);
    inline CompOutput &output () { return *mOutput; }
    AnimEffect getActualEffect (AnimEffect effect,
				AnimEvent animEvent);
    bool shouldIgnoreWindowForAnim (CompWindow *w, bool checkPixmap);
    OptionSet *getOptionSetForSelectedRow (PrivateAnimWindow *aw,
					   Animation *anim);
    void addExtension (ExtensionPluginInfo *extensionPluginInfo,
		       bool shouldInitPersistentData);
    void removeExtension (ExtensionPluginInfo *extensionPluginInfo);
    AnimEffect getMatchingAnimSelection (CompWindow *w,
					 AnimEvent e,
					 int *duration);
    bool otherPluginsActive ();

    void enablePrePaintWindowsBackToFront (bool enabled);
    void prePaintWindowsBackToFront ();

    // CompositeScreenInterface methods
    void preparePaint (int);
    void donePaint ();
    const CompWindowList & getWindowPaintList ();

    // GLScreenInterface methods
    bool glPaintOutput (const GLScreenPaintAttrib &,
			const GLMatrix &,
			const CompRegion &,
			CompOutput *,
			unsigned int);

    // ScreenInterface methods
    void handleCompizEvent (const char * plugin, const char *event,
			    CompOption::Vector &options);
};

class PrivateAnimWindow :
    public WindowInterface,
    public GLWindowInterface
{
    friend class PrivateAnimScreen;
    friend class AnimWindow;

public:
    PrivateAnimWindow (CompWindow *, AnimWindow *aw);
    ~PrivateAnimWindow ();

    void createFocusAnimation (AnimEffect effect, int duration);
    inline void setShaded (bool shaded) { mNowShaded = shaded; }
    inline Animation *curAnimation () { return mCurAnimation; }
    inline PrivateAnimScreen *paScreen () { return mPAScreen; }
    inline AnimWindow *aWindow () { return mAWindow; }
    inline Box &BB () { return mBB; }
    inline int curAnimSelectionRow () { return mCurAnimSelectionRow; }

    void damageThisAndLastStepRegion ();
    void postAnimationCleanUp ();
    void copyResetStepRegion ();

    GLWindow          *gWindow;

private:
    CompWindow        *mWindow;
    AnimWindow        *mAWindow;

    PrivateAnimScreen *mPAScreen;

    unsigned int mState;
    unsigned int mNewState;

    Animation *mCurAnimation;

    bool mUnshadePending;
    bool mEventNotOpenClose;
    bool mNowShaded;
    bool mGrabbed;

    int mUnmapCnt;
    int mDestroyCnt;

    bool mIgnoreDamage;
    bool mFinishingAnim;

    int mCurAnimSelectionRow;
    int mPrevAnimSelectionRow;	///< For the case when one event interrupts another

    Box mBB;       ///< Bounding box of area to be damaged

    CompRegion mStepRegion;     ///< Region to damage this step
    CompRegion mLastStepRegion; ///< Region damaged last step

    bool mPluginActive[WatchedWindowPluginNum];

    // Utility methods
    unsigned int getState ();
    void updateSelectionRow (unsigned int i);
    void postAnimationCleanUpPrev (bool closing, bool clearMatchingRow);
    void postAnimationCleanUpCustom (bool closing,
				     bool destructing,
				     bool clearMatchingRow);
    void reverseAnimation ();
    void enablePainting (bool enabling);

    void notifyAnimation (bool activation);

    // WindowInterface methods
    void resizeNotify (int dx, int dy, int dwidth, int dheight);
    void moveNotify (int dx, int dy, bool immediate);
    void windowNotify (CompWindowNotify n);
    void grabNotify (int x, int y, unsigned int state, unsigned int mask);
    void ungrabNotify ();

    // GLWindowInterface methods
    bool glPaint (const GLWindowPaintAttrib &, const GLMatrix &,
		  const CompRegion &, unsigned int);
    void glAddGeometry (const GLTexture::MatrixList &,
			const CompRegion &, const CompRegion &,
			unsigned int = MAXSHORT, unsigned int = MAXSHORT);
    void glDrawTexture (GLTexture *texture, const GLMatrix &,
                        const GLWindowPaintAttrib &, unsigned int);
    //void glDrawGeometry ();
};

class RollUpAnim :
    public GridAnim
{
public:
    RollUpAnim (CompWindow *w,
		WindowEvent curWindowEvent,
		float duration,
		const AnimEffect info,
		const CompRect &icon);
protected:
    static const float kDurationFactor;

    void initGrid ();
    void step ();
};

class MagicLampAnim :
    public GridAnim
{
public:
    MagicLampAnim (CompWindow *w,
    		   WindowEvent curWindowEvent,
    		   float duration,
    		   const AnimEffect info,
    		   const CompRect &icon);
    virtual ~MagicLampAnim ();

protected:
    bool mTargetTop;
    GridModel::GridObject *mTopLeftCornerObject;
    GridModel::GridObject *mBottomLeftCornerObject;

    void initGrid ();
    void step ();
    void updateBB (CompOutput &output);
    inline bool stepRegionUsed () { return true; }
    void adjustPointerIconSize ();

    virtual bool hasMovingEnd ();
    virtual void filterTargetX (float &targetX, float x) { }
};

class MagicLampWavyAnim :
    public MagicLampAnim
{
public:
    MagicLampWavyAnim (CompWindow *w,
    		       WindowEvent curWindowEvent,
    		       float duration,
    		       const AnimEffect info,
    		       const CompRect &icon);
    ~MagicLampWavyAnim ();

protected:
    struct WaveParam
    {
	float halfWidth;
	float amp;
	float pos;
    };

    unsigned int mNumWaves;
    WaveParam *mWaves;

    void initGrid ();
    void updateBB (CompOutput &output);
    inline bool stepRegionUsed () { return false; }
    void adjustPointerIconSize ();

    bool hasMovingEnd ();
    void filterTargetX (float &targetX, float x);
};

class SidekickAnim :
    public ZoomAnim
{
public:
    SidekickAnim (CompWindow *w,
		  WindowEvent curWindowEvent,
		  float duration,
		  const AnimEffect info,
		  const CompRect &icon);
protected:
    float mNumRotations;

    float getSpringiness ();
    bool isZoomFromCenter ();
    inline bool hasExtraTransform () { return true; }
    void applyExtraTransform (float progress);
    inline bool shouldAvoidParallelogramLook () { return true; }
    bool requiresTransformedWindow () const { return true; }
};

class WaveAnim :
    public GridTransformAnim
{
public:
    WaveAnim (CompWindow *w,
	      WindowEvent curWindowEvent,
	      float duration,
	      const AnimEffect info,
	      const CompRect &icon);
protected:
    void adjustDuration ();
    void initGrid ();
    inline bool using3D () { return true; }
    void step ();
    bool requiresTransformedWindow () const { return true; }

    static const float kMinDuration;
};

class GlideAnim :
    public ZoomAnim
{
public:
    GlideAnim (CompWindow *w,
	       WindowEvent curWindowEvent,
	       float duration,
	       const AnimEffect info,
	       const CompRect &icon);

protected:
    void prePaintWindow ();
    inline bool postPaintWindowUsed () { return true; }
    void postPaintWindow ();
    void adjustDuration ();
    bool zoomToIcon ();
    void applyTransform ();
    float getFadeProgress ();
    bool requiresTransformedWindow () const { return true; }

    float getProgress ();
    virtual void getParams (float *finalDistFac,
			    float *finalRotAng,
			    float *thickness);

    float glideModRotAngle;	///< The angle of rotation, modulo 360.
};

class Glide2Anim :
    public GlideAnim
{
public:
    Glide2Anim (CompWindow *w,
		WindowEvent curWindowEvent,
		float duration,
		const AnimEffect info,
		const CompRect &icon);
protected:
    bool zoomToIcon ();
    void getParams (float *finalDistFac,
		    float *finalRotAng,
		    float *thickness);
};

class RestackPersistentData;

class RestackAnim :
    virtual public Animation
{
public:
    RestackAnim (CompWindow *w,
		 WindowEvent curWindowEvent,
		 float duration,
		 const AnimEffect info,
		 const CompRect &icon);
    void cleanUp (bool closing,
		  bool destructing);
    bool initiateRestackAnim (int duration);
    inline bool moveUpdate (int dx, int dy) { return false; }
    static bool onSameRestackChain (CompWindow *wSubject, CompWindow *wOther);

    /// Find union of restack chain (group)
    static CompRegion unionRestackChain (CompWindow *w);

    virtual bool paintedElsewhere () { return false; }

protected:
    // Overridable methods
    virtual void processCandidate (CompWindow *candidateWin,
				   CompWindow *subjectWin,
				   CompRegion &candidateAndSubjectIntersection,
				   int &numSelectedCandidates) {}
    virtual void postInitiateRestackAnim (int numSelectedCandidates,
					  int duration,
					  CompWindow *wStart,
					  CompWindow *wEnd,
					  bool raised) {}

    // Other methods
    bool overNewCopy (); ///< Is glPaint on the copy at the new position?

    RestackPersistentData *mRestackData;
};

class RestackPersistentData :
    public PersistentData
{
    friend class ExtensionPluginAnimation;
    friend class RestackAnim;
    friend class FocusFadeAnim;
    friend class DodgeAnim;

public:
    RestackPersistentData ();
    ~RestackPersistentData ();

protected:
    inline RestackInfo *restackInfo () { return mRestackInfo; }
    void resetRestackInfo (bool alsoResetChain = false);
    void setRestackInfo (CompWindow *wRestacked,
			 CompWindow *wStart,
			 CompWindow *wEnd,
			 CompWindow *wOldAbove,
			 bool raised);
    void getHostedOnWin (CompWindow *wGuest, CompWindow *wHost);

    RestackInfo *mRestackInfo;   ///< restack info if window was restacked this paint round
    CompWindow *mWinToBePaintedBeforeThis; ///< Window which should be painted before this
    CompWindow *mWinThisIsPaintedBefore; ///< the inverse relation of mWinToBePaintedBeforeThis
    CompWindow *mMoreToBePaintedPrev;
    CompWindow *mMoreToBePaintedNext; ///< doubly linked list for windows underneath that
    				     ///< raise together with this one
    bool mConfigureNotified;     ///< was mConfigureNotified before restack check
    CompWindow *mWinPassingThrough; ///< win. passing through this one during focus effect
    bool mWalkerOverNewCopy;  ///< whether walker is on the copy at the new pos.
    int mVisitCount; ///< how many times walker/glPaint has visited this window
    bool mIsSecondary; ///< whether this is one of the secondary (non-topmost) in its restack chain
};

class FocusFadeAnim :
    public RestackAnim,
    public FadeAnim
{
public:
    FocusFadeAnim (CompWindow *w,
		   WindowEvent curWindowEvent,
		   float duration,
		   const AnimEffect info,
		   const CompRect &icon);
    void updateAttrib (GLWindowPaintAttrib &attrib);
    void cleanUp (bool closing,
		  bool destructing);

protected:
    void processCandidate (CompWindow *candidateWin,
			   CompWindow *subjectWin,
			   CompRegion &candidateAndSubjectIntersection,
			   int &numSelectedCandidates);
    GLushort computeOpacity (GLushort opacityInt);
};

typedef enum
{
    DodgeDirectionUp = 0,
    DodgeDirectionRight,
    DodgeDirectionDown,
    DodgeDirectionLeft,
    DodgeDirectionXY, // movement possibly in both X and Y (for subjects)
    DodgeDirectionNone
} DodgeDirection;

class DodgePersistentData;

class DodgeAnim :
    public RestackAnim,
    public TransformAnim
{
public:
    DodgeAnim (CompWindow *w,
	       WindowEvent curWindowEvent,
	       float duration,
	       const AnimEffect info,
	       const CompRect &icon);
    void cleanUp (bool closing,
		  bool destructing);
    static int getDodgeAmount (CompRect &rect,
			       CompWindow *dw,
			       DodgeDirection dir);
    void step ();
    void updateTransform (GLMatrix &wTransform);
    bool shouldDamageWindowOnStart ();
    void updateBB (CompOutput &output);
    void postPreparePaint ();
    void calculateDodgeAmounts ();
    bool moveUpdate (int dx, int dy);

protected:
    void processCandidate (CompWindow *candidateWin,
			   CompWindow *subjectWin,
			   CompRegion &candidateAndSubjectIntersection,
			   int &numSelectedCandidates);
    void postInitiateRestackAnim (int numSelectedCandidates,
				  int duration,
				  CompWindow *wStart,
				  CompWindow *wEnd,
				  bool raised);
    bool paintedElsewhere ();
    void applyDodgeTransform ();
    float dodgeProgress ();
    void updateDodgerDodgeAmount ();

    DodgePersistentData *mDodgeData;

    CompWindow *mDodgeSubjectWin;///< The window being dodged
    float mDodgeMaxAmountX;	///< max # pixels it should dodge
				///<  (neg. value dodges leftward)
    float mDodgeMaxAmountY;	///< max # pixels it should dodge
				///<  (neg. value dodges upward)
    DodgeDirection mDodgeDirection;
    int mDodgeMode;
};

class DodgePersistentData :
    public PersistentData
{
    friend class ExtensionPluginAnimation;
    friend class DodgeAnim;

public:
    DodgePersistentData ();

private:
    int dodgeOrder;		///< dodge order (used temporarily)

    // TODO mov the below members into DodgeAnim
    bool isDodgeSubject;	///< true if this window is the cause of dodging
    bool skipPostPrepareScreen;
    CompWindow *dodgeChainStart;///< for the subject window
    CompWindow *dodgeChainPrev;	///< for dodging windows
    CompWindow *dodgeChainNext;	///< for dodging windows
};

class DreamAnim :
    public GridZoomAnim
{
public:
    DreamAnim (CompWindow *w,
	       WindowEvent curWindowEvent,
	       float duration,
	       const AnimEffect info,
	       const CompRect &icon);
protected:
    void init ();
    void initGrid ();
    void step ();
    void adjustDuration ();
    float getFadeProgress ();
    bool zoomToIcon ();
    bool requiresTransformedWindow () const { return true; }

    static const float kDurationFactor;
};

class FoldAnim :
    public GridZoomAnim
{
public:
    FoldAnim (CompWindow *w,
	      WindowEvent curWindowEvent,
	      float duration,
	      const AnimEffect info,
	      const CompRect &icon);
protected:
    inline bool using3D () { return true; }
    float getFadeProgress ();
    void updateWindowAttrib (GLWindowPaintAttrib &attrib);
    bool requiresTransformedWindow () const { return true; }
};

class CurvedFoldAnim :
    public FoldAnim
{
public:
    CurvedFoldAnim (CompWindow *w,
		    WindowEvent curWindowEvent,
		    float duration,
		    const AnimEffect info,
		    const CompRect &icon);
protected:
    void initGrid ();
    void step ();
    void updateBB (CompOutput &output);
    bool zoomToIcon ();
    float getObjectZ (GridAnim::GridModel *mModel,
		      float forwardProgress,
		      float sinForProg,
		      float relDistToCenter,
		      float curveMaxAmp);
    bool requiresTransformedWindow () const { return true; }
};

class HorizontalFoldsAnim :
    public FoldAnim
{
public:
    HorizontalFoldsAnim (CompWindow *w,
			 WindowEvent curWindowEvent,
			 float duration,
			 const AnimEffect info,
			 const CompRect &icon);
protected:
    void initGrid ();
    void step ();
    bool zoomToIcon ();
    float getObjectZ (GridAnim::GridModel *mModel,
		      float forwardProgress,
		      float sinForProg,
		      float relDistToFoldCenter,
		      float foldMaxAmp);
    bool requiresTransformedWindow () const { return true; }
};
