#ifndef ANIMATION_ZOOM_H
#define ANIMATION_ZOOM_H
#include "animation.h"

class ZoomAnim :
    public FadeAnim,
    virtual public TransformAnim
{
public:
     ZoomAnim (CompWindow *w,
	       WindowEvent curWindowEvent,
	       float duration,
	       const AnimEffect info,
	       const CompRect &icon);

public:
     void step () { TransformAnim::step (); }
     void adjustDuration ();
     float getFadeProgress ();
     bool updateBBUsed () { return true; }
     void updateBB (CompOutput &output) { TransformAnim::updateBB (output); }
     void applyTransform ();
protected:
     float getActualProgress ();
     Point getCenter ();
     virtual float getSpringiness ();
     virtual bool isZoomFromCenter ();
     virtual bool zoomToIcon () { return true; }
     virtual bool hasExtraTransform () { return false; }
     virtual void applyExtraTransform (float progress) {}
     virtual bool shouldAvoidParallelogramLook () { return false; }
     virtual bool scaleAroundIcon ();
     virtual bool neverSpringy () { return false; }
     void getZoomProgress (float *moveProgress,
			   float *scaleProgress,
			   bool neverSpringy);
     bool requiresTransformedWindow () const { return true; }

     static const float kDurationFactor;
     static const float kSpringyDurationFactor;
     static const float kNonspringyDurationFactor;

private:
     void getCenterScaleFull (Point *pCurCenter, Point *pCurScale,
			      Point *pWinCenter, Point *pIconCenter,
			      float *pMoveProgress);
     void getCenterScale (Point *pCurCenter, Point *pCurScale);
};

class GridZoomAnim :
    public GridTransformAnim,
    public ZoomAnim
{
public:
    GridZoomAnim (CompWindow *w,
		  WindowEvent curWindowEvent,
		  float duration,
		  const AnimEffect info,
		  const CompRect &icon);
    void init () { GridTransformAnim::init (); }
    void step () { ZoomAnim::step (); }
    void updateTransform (GLMatrix &wTransform)
    { GridTransformAnim::updateTransform (wTransform); }
    void updateBB (CompOutput &output) { GridTransformAnim::updateBB (output); }
    bool updateBBUsed () { return true; }
    bool neverSpringy () { return true; }
    float getSpringiness () { return 0; }
    bool scaleAroundIcon () { return false; }
    void adjustDuration ();
    bool requiresTransformedWindow () const { return true; }
};
#endif
