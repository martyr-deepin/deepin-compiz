#ifndef ANIMATION_PARTIALWINDOW_H
#define ANIMATION_PARTIALWINDOW_H
#include "animation.h"
class PartialWindowAnim :
virtual public Animation
{
public:
    PartialWindowAnim (CompWindow *w,
		       WindowEvent curWindowEvent,
		       float duration,
		       const AnimEffect info,
		       const CompRect &icon);
    
    void addGeometry (const GLTexture::MatrixList &matrix,
		      const CompRegion            &region,
		      const CompRegion            &clip,
		      unsigned int                maxGridWidth,
		      unsigned int                maxGridHeight);
    
protected:
    bool mUseDrawRegion;
    CompRegion mDrawRegion;
    
};
#endif
