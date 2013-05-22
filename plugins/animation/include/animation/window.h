#ifndef ANIMATION_WINDOW_H
#define ANIMATION_WINDOW_H
#include "animation.h"

extern template class PluginClassHandler<AnimWindow, CompWindow, ANIMATION_ABI>;

class AnimWindow :
    public PluginClassHandler<AnimWindow, CompWindow, ANIMATION_ABI>
{
     friend class PrivateAnimScreen;
     friend class PrivateAnimWindow;
     friend class AnimScreen;
     friend class Animation;
     
public:
     AnimWindow (CompWindow *);
     ~AnimWindow ();
     
     BoxPtr BB ();
     CompRegion &stepRegion ();
     void resetStepRegionWithBB ();
     
     void expandBBWithWindow ();
     void expandBBWithScreen ();
     void expandBBWithBox (Box &source);
     void expandBBWithPoint (float fx, float fy);
     void expandBBWithPoint2DTransform (GLVector &coords,
					GLMatrix &transformMat);
     bool expandBBWithPoints3DTransform (CompOutput     &output,
					 GLMatrix       &transform,
					 const float    *points,
					 GridAnim::GridModel::GridObject *objects,
					 unsigned int   nPoints);
     
     inline bool savedRectsValid () { return mSavedRectsValid; }
     inline const CompRect & saveWinRect () { return mSavedWinRect; }
     inline const CompRect & savedInRect () { return mSavedInRect; }
     inline const CompRect & savedOutRect () { return mSavedOutRect; }
     inline CompWindowExtents & savedOutExtents () { return mSavedOutExtents; }
     
     Animation *curAnimation ();
     void createFocusAnimation (AnimEffect effect, int duration = 0);
     
     void postAnimationCleanUp ();
     
     // TODO: Group persistent data for a plugin and allow a plugin to only
     // delete its own data.
     void deletePersistentData (const char *name);
     
     /// A "string -> persistent data" map for animations that require such data,
     /// like some focus animations.
     PersistentDataMap persistentData;
     
     CompWindow *mWindow;    ///< Window being animated. // TODO move to private:
private:
     PrivateAnimWindow *priv;
     
     
     bool mSavedRectsValid;
     CompRect mSavedWinRect; ///< Saved window contents geometry
     CompRect mSavedInRect;   ///< Saved window input geometry
     CompRect mSavedOutRect;  ///< Saved window output geometry
     CompWindowExtents mSavedOutExtents; ///< Saved window output extents
     
     CompOption::Value &pluginOptVal (ExtensionPluginInfo *pluginInfo,
				      unsigned int optionId,
				      Animation *anim);
     
};
#endif
