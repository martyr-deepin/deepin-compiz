#ifndef ANIMATION_TRANSFORM_H
#define ANIMATION_TRANSFORM_H
#include "animation.h"
class TransformAnim :
virtual public Animation
{
public:
    TransformAnim (CompWindow *w,
		   WindowEvent curWindowEvent,
		   float duration,
		   const AnimEffect info,
		   const CompRect &icon);
    void init ();
    void step ();
    void updateTransform (GLMatrix &wTransform);
    void updateBB (CompOutput &output);
    bool updateBBUsed () { return true; }
    
protected:
    GLMatrix mTransform;
    
    float mTransformStartProgress;
    float mTransformProgress;
    
    void perspectiveDistortAndResetZ (GLMatrix &transform);
    void applyPerspectiveSkew (CompOutput &output,
			       GLMatrix &transform,
			       Point &center);
    virtual void adjustDuration () {}
    virtual void applyTransform () {}
    virtual Point getCenter ();
    virtual bool requiresTransformedWindow () const { return true; }
    
};
#endif
