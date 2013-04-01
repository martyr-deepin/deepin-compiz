#ifndef ANIMATION_FADE_H
#define ANIMATION_FADE_H
#include "animation.h"
class FadeAnim :
virtual public Animation
{
public:
     FadeAnim (CompWindow *w,
	       WindowEvent curWindowEvent,
	       float duration,
	       const AnimEffect info,
	       const CompRect &icon);
public:
     void updateBB (CompOutput &output);
     bool updateBBUsed () { return true; }
     void updateAttrib (GLWindowPaintAttrib &wAttrib);
     virtual bool requiresTransformedWindow () const { return false; }
     virtual float getFadeProgress () { return progressLinear (); }
};
#endif
