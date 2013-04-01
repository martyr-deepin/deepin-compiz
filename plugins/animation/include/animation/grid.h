#ifndef ANIMATION_GRID_H
#define ANIMATION_GRID_H
#include "animation.h"
class GridAnim :
    virtual public Animation
{
 public:
     class GridModel
     {
	 friend class GridAnim;
	 
     public:
	 GridModel (CompWindow *w,
		    WindowEvent curWindowEvent,
		    int height,
		    int gridWidth,
		    int gridHeight,
		    int decorTopHeight,
		    int decorBottomHeight);
	 ~GridModel ();
	 
	 void move (float tx, float ty);
	 
	 class GridObject
	 {
	     friend class GridAnim;
	     friend class GridZoomAnim;
	     friend class GridTransformAnim;
	     
	 public:
	     GridObject ();
	     void setGridPosition (Point &gridPosition);
	     inline Point3d &position () { return mPosition; }
	     inline Point &gridPosition () { return mGridPosition; }
	     
	     inline Point &offsetTexCoordForQuadBefore ()
	     { return mOffsetTexCoordForQuadBefore; }
	     
	     inline Point &offsetTexCoordForQuadAfter ()
	     { return mOffsetTexCoordForQuadAfter; }
	     
	 private:
	     Point3d mPosition;	 ///< Position on screen
	     Point mGridPosition; ///< Position on window in [0,1] range
	     
	     Point mOffsetTexCoordForQuadBefore;
	     Point mOffsetTexCoordForQuadAfter;
	     ///< Texture x, y coordinates will be offset by given amounts
	     ///< for quads that fall after and before this object in x and y directions.
	     ///< Currently only y offset can be used.
	};

	inline GridObject *objects () { return mObjects; }
	inline unsigned int numObjects () { return mNumObjects; }
	inline Point &scale () { return mScale; }

    private:
	GridObject *mObjects; // TODO: convert to vector
	unsigned int mNumObjects;

	Point mScale;
	Point mScaleOrigin;

	void initObjects (WindowEvent curWindowEvent,
			  int height,
			  int gridWidth, int gridHeight,
			  int decorTopHeight, int decorBottomHeight);
    };

protected:
    GridModel *mModel;

    int mGridWidth;	///< Number of cells along grid width
    int mGridHeight;	///< Number of cells along grid height

    /// true if effect needs Q texture coordinates.
    /// Q texture coordinates are used to avoid jagged-looking quads
    /// ( http://www.r3.nu/~cass/qcoord/ )
    bool mUseQTexCoord;

    virtual bool using3D () { return false; }

    virtual bool requiresTransformedWindow () const { return true; }

    virtual void initGrid ();	///< Initializes grid width/height.
    				///< Default grid size is 2x2.
				///< Override for custom grid size.

public:
    GridAnim (CompWindow *w,
	      WindowEvent curWindowEvent,
	      float duration,
	      const AnimEffect info,
	      const CompRect &icon);
    ~GridAnim ();
    void init ();
    void updateBB (CompOutput &output);
    bool updateBBUsed () { return true; }
    void addGeometry (const GLTexture::MatrixList &matrix,
		      const CompRegion            &region,
		      const CompRegion            &clip,
		      unsigned int                maxGridWidth,
		      unsigned int                maxGridHeight);
    void drawGeometry ();
};
#endif
