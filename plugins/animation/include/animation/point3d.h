#ifndef ANIMATION_POINT_H
#define ANIMATION_POINT_H
#include "animation.h"

class Point
{
public:
     Point () : mX (0), mY (0) {}
     Point (float x, float y) : mX (x), mY (y) {}
     
     inline float x () const { return mX; }
     inline float y () const { return mY; }
     
     inline void setX (float x) { mX = x; }
     inline void setY (float y) { mY = y; }
     
     void set (float x, float y) { mX = x; mY = y; }
     
     inline void add (const Point &p) { mX += p.x (); mY += p.y (); }
     
     Point &operator= (const Point &p);
     bool operator== (const Point &p) const;
     bool operator!= (const Point &p) const;
     
private:
     float mX, mY;
};

typedef Point Vector;

class Point3d
{
    public:
	Point3d () : mX (0), mY (0), mZ (0) {}
	Point3d (float x, float y, float z) : mX (x), mY (y), mZ (z) {}

	inline float x () const { return mX; }
	inline float y () const { return mY; }
	inline float z () const { return mZ; }

	inline void setX (float x) { mX = x; }
	inline void setY (float y) { mY = y; }
	inline void setZ (float z) { mZ = z; }

	inline void set (float x, float y, float z) { mX = x; mY = y; mZ = z; }

	inline void add (const Point3d &p)
	{ mX += p.x (); mY += p.y (); mZ += p.z (); }
	inline void add (float x, float y, float z)
	{ mX += x; mY += y; mZ += z; }

	Point3d &operator= (const Point3d &p);
	bool operator== (const Point3d &p) const;
	bool operator!= (const Point3d &p) const;

    private:
	float mX, mY, mZ;
};

typedef Point3d Vector3d;

/* XXX: change this to CompRect */
typedef struct
{
    float x1, x2, y1, y2;
} Boxf;

inline Point &
Point::operator= (const Point &p)
{
    mX = p.x (); mY = p.y ();
    return *this;
}

inline bool
Point::operator== (const Point &p) const
{
    return (mX == p.x () && mY == p.y ());
}

inline bool
Point::operator!= (const Point &p) const
{
    return !(*this == p);
}

inline Point3d &
Point3d::operator= (const Point3d &p)
{
    mX = p.x (); mY = p.y (); mZ = p.z ();
    return *this;
}

inline bool
Point3d::operator== (const Point3d &p) const
{
    return (mX == p.x () && mY == p.y () && mZ == p.z ());
}

inline bool
Point3d::operator!= (const Point3d &p) const
{
    return !(*this == p);
}
#endif
