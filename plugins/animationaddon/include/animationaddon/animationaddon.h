#ifndef _COMPIZ_ANIMATIONADDON_H
#define _COMPIZ_ANIMATIONADDON_H

#define ANIMATIONADDON_ABI 20091206

#include <core/pluginclasshandler.h>

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

using namespace::std;

class PrivateAnimAddonScreen;

class AnimAddonScreen :
    public PluginClassHandler<AnimAddonScreen, CompScreen, ANIMATIONADDON_ABI>,
    public CompOption::Class
{
public:
    AnimAddonScreen (CompScreen *);
    ~AnimAddonScreen ();

    CompOption::Vector &getOptions ();
    bool setOption (const CompString &name, CompOption::Value &value);

    int getIntenseTimeStep ();

private:
    PrivateAnimAddonScreen *priv;
};

/// Base class for all polygon- and particle-based animations
class BaseAddonAnim :
    virtual public Animation
{
public:
    BaseAddonAnim (CompWindow *w,
		   WindowEvent curWindowEvent,
		   float duration,
		   const AnimEffect info,
		   const CompRect &icon);
    ~BaseAddonAnim () {}

    bool needsDepthTest () { return mDoDepthTest; }

protected:
    /// Gets info about the extension plugin that implements this animation.
    ExtensionPluginInfo *getExtensionPluginInfo ();

    int mIntenseTimeStep;

    CompositeScreen *mCScreen;
    GLScreen *mGScreen;

    bool mDoDepthTest;  ///< Whether depth testing should be used in the effect
};

// Particle stuff

class Particle
{
public:
    Particle () : life (0.0f) {}

    float life;		///< particle life
    float fade;		///< fade speed
    float width;	///< particle width
    float height;	///< particle height
    float w_mod;	///< particle size modification during life
    float h_mod;	///< particle size modification during life
    float r;		///< red value
    float g;		///< green value
    float b;		///< blue value
    float a;		///< alpha value
    float x;		///< X position
    float y;		///< Y position
    float z;		///< Z position
    float xi;		///< X direction
    float yi;		///< Y direction
    float zi;		///< Z direction
    float xg;		///< X gravity
    float yg;		///< Y gravity
    float zg;		///< Z gravity
    float xo;		///< orginal X position
    float yo;		///< orginal Y position
    float zo;		///< orginal Z position
};

class ParticleSystem
{
    friend class ParticleAnim;

public:
    ParticleSystem (int    numParticles,
                    float  slowdown,
		    float  darkenAmount,
		    GLuint blendMode);
    ~ParticleSystem ();

    void draw (int offsetX = 0, int offsetY = 0);
    void update (float time);
    vector<Particle> &particles () { return mParticles; }
    void activate () { mActive = true; }
    bool active () { return mActive; }
    void setOrigin (int x, int y) { mX = x; mY = y; }

protected:
    CompWindow *mWindow;

    vector<Particle> mParticles;

    float  mSlowDown;
    float  mDarkenAmount;
    GLuint mBlendMode;
    GLuint mTex;
    bool   mActive;
    int    mX, mY;

    GLScreen *mGScreen;

    vector<GLfloat> mVerticesCache;
    vector<GLfloat> mCoordsCache;
    vector<GLfloat> mColorsCache;
    vector<GLfloat> mDColorsCache;
};

class ParticleAnim :
    public BaseAddonAnim,
    public PartialWindowAnim
{
public:
    ParticleAnim (CompWindow *w,
		  WindowEvent curWindowEvent,
		  float duration,
		  const AnimEffect info,
		  const CompRect &icon);
    ~ParticleAnim () {}
    void postPaintWindow ();
    bool postPaintWindowUsed () { return true; }
    void updateBB (CompOutput &output);
    bool updateBBUsed () { return true; }
    bool prePreparePaint (int msSinceLastPaint);
    void initLightDarkParticles (int numLightParticles,
                                 int numDarkParticles,
                                 float lightSlowDown,
                                 float darkSlowDown);

protected:
    boost::ptr_vector<ParticleSystem> mParticleSystems;
};

// Polygon stuff

typedef enum
{
    CorrectPerspectiveNone = 0,
    CorrectPerspectivePolygon,
    CorrectPerspectiveWindow
} CorrectPerspective;

/// Base class for per-PolygonObject effect parameter classes
class PolygonEffectParameters
{
};

/// This is intended to be a closed 3D piece of a window with convex polygon
/// faces and quad-strip sides. Since decoration texture is separate from
/// the window texture, it is more complicated than it would be with a single
/// texture: we use glClipPlane with the rectangles (clips) to clip 3D space
/// to the region falling within that clip.
/// If the polygon is on an edge/corner, it also has 2D shadow quad(s)
/// (to be faded out at the beginning of 3D animation if necessary).
class PolygonObject
{
public:
    int nVertices;		///< number of total vertices (front + back)
    int nSides;			///< number of sides
    GLfloat *vertices;		///< Positions of vertices relative to center
    GLushort *sideIndices;	///< Indices of quad strip for "sides"
    GLfloat *normals;		///< Surface normals for 2+nSides faces

    Boxf boundingBox;		///< Bound. box to test intersection with clips

    // Animation effect parameters

    Point3d centerPosStart;	///< Starting position of center
    float rotAngleStart;	///< Starting rotation angle

    Point3d centerPos;		///< Position of center
    Vector3d rotAxis;		///< Rotation axis vector
    float rotAngle;		///< Rotation angle
    Point3d rotAxisOffset;	///< Rotation axis translate amount

    Point centerRelPos;		///< Relative pos of center within the window

    Vector3d finalRelPos;	///< Velocity factor for scripted movement
    float finalRotAng;		///< Final rotation angle around rotAxis

    float moveStartTime;	///< Movement starts at this time ([0-1] range)
    float moveDuration;		///< Movement lasts this long     ([0-1] range)

    float fadeStartTime;	///< Fade out starts at this time ([0,1] range)
    float fadeDuration;		///< Duration of fade out         ([0,1] range)

    /** Pointer to a struct that can contain
	custom parameters for an individual effect */
    PolygonEffectParameters *effectParameters;

    float boundSphereRadius;    ///< Radius of bounding sphere
};

/// Info about intersection of a polygon and clip
class PolygonClipInfo
{
public:
    PolygonClipInfo (const PolygonObject *p);

    const PolygonObject *p; ///< the intersecting polygon-object

    /// Texture coord.s for each vertex of the polygon-object
    /// ordered as: Front v1.x, y, v2.x, y, ...,
    ///             followed by back vertex texture coordinates.
    vector<GLfloat> vertexTexCoords;
};

class Clip4Polygons	        ///< Rectangular clips
{				///< (to hold clips passed to AddWindowGeometry)
public:
    CompRect box;		///< Coords
    Boxf boxf;			///< Float coords (for small clipping adjustment)
    GLTexture::Matrix texMatrix;///< Corresponding texture coord. matrix

    /// True if this clip likely intersects all polygons
    /// (for the window-contents clip). Used for optimization purposes.
    bool intersectsMostPolygons;

    /// For polygons that intersect this clip.
    /// Only used when intersectsMostPolygons is false.
    list<PolygonClipInfo *> intersectingPolygonInfos;

    /// Texture coord.s for each vertex of each polygon-object
    /// ordered as: Front p1.v1.x, y, p1.v2.x, .y, p2.v1.x, .y, ...,
    ///             followed by back vertex texture coordinates.
    /// Only used when intersectsMostPolygons is true.
    vector<GLfloat> polygonVertexTexCoords;
};

class PolygonAnim :
    virtual public Animation,
    public BaseAddonAnim
{
public:
    PolygonAnim (CompWindow *w,
		 WindowEvent curWindowEvent,
		 float duration,
		 const AnimEffect info,
		 const CompRect &icon);
    ~PolygonAnim ();

    void step ();
    void prePaintWindow ();
    void postPaintWindow ();
    bool postPaintWindowUsed () { return true; }
    void addGeometry (const GLTexture::MatrixList &matrix,
                      const CompRegion            &region,
                      const CompRegion            &clipRegion,
                      unsigned int                maxGridWidth,
                      unsigned int                maxGridHeight);
    void drawGeometry ();
    virtual void updateBB (CompOutput &output);
    bool updateBBUsed () { return true; }
    bool prePreparePaint (int msSinceLastPaint);
    bool moveUpdate (int dx, int dy);

    virtual void stepPolygon (PolygonObject *p,
			      float forwardProgress);
    virtual void transformPolygon (const PolygonObject *p) {}

    /// For effects that have decel. motion
    virtual bool deceleratingMotion () { return false; }

    bool tessellateIntoRectangles (int gridSizeX,
				   int gridSizeY,
				   float thickness);
    bool tessellateIntoHexagons (int gridSizeX,
				 int gridSizeY,
				 float thickness);
    bool tessellateIntoGlass (int gridSizeX,
			      int gridSizeY,
			      float thickness);

    void prePaintOutput (CompOutput *output);
    void deceleratingAnimStepPolygon (PolygonObject *p,
                                      float forwardProgress);

protected:
    void getPerspectiveCorrectionMat (const PolygonObject *p,
				      GLfloat *mat,
				      GLMatrix *matf,
				      const CompOutput &output);
    void processIntersectingPolygons ();
    virtual void freePolygonObjects ();
    void freeClipsPolygons ();
    void prepareDrawingForAttrib (GLFragment::Attrib &attrib);

    int mNumDrawGeometryCalls;
    int mNumClipsPassed;	 /**< # of clips passed to animAddWindowGeometry so far
				      in this draw step */
    bool mClipsUpdated;          ///< whether stored clips are updated in this anim step

    vector<Clip4Polygons> mClips;///< Rect. clips collected in addGeometries
    int mFirstNondrawnClip;
    vector<int> mLastClipInGroup;/**< Index of the last clip in each group of clips
				      drawn in drawGeometry func. */

    bool mDoLighting;            ///< Whether lighting should be used in the effect
    CorrectPerspective mCorrectPerspective;

    vector<PolygonObject *> mPolygons; ///< The polygons in this set
    float mThickness;		///< Window thickness (depth along z axis)
    int mNumTotalFrontVertices;	///< Total # of polygon vertices on front faces
    float mBackAndSidesFadeDur;	///< How long side and back faces should fade in/out
    float mAllFadeDuration;	/**< Duration of fade out at the end in [0,1] range
				     when all polygons fade out at the same time.
				     If >-1, this overrides fadeDuration in PolygonObject */

    bool mIncludeShadows;        ///< Whether to include shadows in polygon

private:
    inline void drawPolygonClipIntersection (const PolygonObject *p,
					     const Clip4Polygons &c,
					     const GLfloat *vertexTexCoords,
					     int pass,
					     float forwardProgress,
					     GLdouble clipPlane[4][4],
					     const CompOutput &output,
					     float newOpacity,
					     bool decelerates,
					     GLfloat skewMat[16]);
};
#endif
