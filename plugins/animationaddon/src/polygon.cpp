/*
 * Animation plugin for compiz/beryl
 *
 * animation.c
 *
 * Copyright : (C) 2006 Erkin Bahceci
 * E-mail    : erkinbah@gmail.com
 *
 * Based on Wobbly and Minimize plugins by
 *           : David Reveman
 * E-mail    : davidr@novell.com>
 *
 * Particle system added by : (C) 2006 Dennis Kasprzyk
 * E-mail                   : onestone@beryl-project.org
 *
 * Beam-Up added by : Florencio Guimaraes
 * E-mail           : florencio@nexcorp.com.br
 *
 * Hexagon tessellator added by : Mike Slegeir
 * E-mail                       : mikeslegeir@mail.utexas.edu>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <GL/glu.h>
#include "private.h"

static const unsigned short MIN_WINDOW_GRID_SIZE = 10;

PolygonAnim::PolygonAnim (CompWindow *w,
			  WindowEvent curWindowEvent,
			  float duration,
			  const AnimEffect info,
			  const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, duration, info, icon),
    BaseAddonAnim::BaseAddonAnim (w, curWindowEvent, duration, info, icon)
{
    mAllFadeDuration = -1.0f;
    mIncludeShadows = false;
}

typedef struct
{
    float dist;
    float x, y; // relative from center

} SpokeVertex;

typedef struct
{
    float direction;
    float length;
    SpokeVertex *spokeVertex;

} Spoke;

typedef struct
{
    bool isTriangle;       // false if 4 sided, true if 3 sided
    float centerX, centerY;
    float pt0X, pt0Y,
	  pt1X, pt1Y,
	  pt2X, pt2Y,
	  pt3X, pt3Y;     // if is_triangle is true, these are unused

} Shard;

// Frees up polygon objects in pset
void
PolygonAnim::freePolygonObjects ()
{
    while (!mPolygons.empty ())
    {
        PolygonObject *p = mPolygons.back ();

	if (p->nVertices > 0)
	{
	    if (p->vertices)
		free (p->vertices);
	    if (p->sideIndices)
		free (p->sideIndices);
	    if (p->normals)
		free (p->normals);
	}
	if (p->effectParameters)
	    delete p->effectParameters;

	delete p;

	mPolygons.pop_back ();
    }
}

// Frees up intersecting polygon info of PolygonSet clips
void
PolygonAnim::freeClipsPolygons ()
{
    foreach (Clip4Polygons &c, mClips)
    {
	foreach (PolygonClipInfo *pci, c.intersectingPolygonInfos)
	    delete pci;
    }
}

PolygonAnim::~PolygonAnim () // was freePolygonSet
{
    freePolygonObjects ();
    freeClipsPolygons ();
}

// Tessellates window into extruded rectangular objects
bool
PolygonAnim::tessellateIntoRectangles (int gridSizeX,
                                       int gridSizeY,
                                       float thickness)
{
    // boundaries of polygon tessellation
    int winLimitsX;
    int winLimitsY;
    int winLimitsW;
    int winLimitsH;

    CompRect inRect (mAWindow->savedRectsValid () ?
		     mAWindow->savedInRect () :
		     mWindow->borderRect ());
    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());

    if (mIncludeShadows)
    {
	winLimitsX = outRect.x ();
	winLimitsY = outRect.y ();
	winLimitsW = outRect.width () - 1; // avoid artifact on right edge
	winLimitsH = outRect.height ();
    }
    else
    {
	winLimitsX = inRect.x ();
	winLimitsY = inRect.y ();
	winLimitsW = inRect.width ();
	winLimitsH = inRect.height ();
    }
    float minRectSize = MIN_WINDOW_GRID_SIZE;
    float rectW = winLimitsW / (float)gridSizeX;
    float rectH = winLimitsH / (float)gridSizeY;

    if (rectW < minRectSize)
	gridSizeX = winLimitsW / minRectSize;	// int div.
    if (rectH < minRectSize)
	gridSizeY = winLimitsH / minRectSize;	// int div.

    freePolygonObjects ();

    mPolygons.clear ();
    for (int i = 0; i < gridSizeX * gridSizeY; i++)
    {
	mPolygons.push_back (new PolygonObject);
    }

    thickness /= ::screen->width ();
    mThickness = thickness;
    mNumTotalFrontVertices = 0;

    float cellW = (float)winLimitsW / gridSizeX;
    float cellH = (float)winLimitsH / gridSizeY;
    float halfW = cellW / 2;
    float halfH = cellH / 2;

    float halfThick = mThickness / 2;
    vector<PolygonObject *>::iterator it = mPolygons.begin ();

    for (int y = 0; y < gridSizeY; y++)
    {
	float posY = winLimitsY + cellH * (y + 0.5);

	for (int x = 0; x < gridSizeX; x++, it++)
	{
	    PolygonObject *p = *it;
	    p->centerPos.set (winLimitsX + cellW * (x + 0.5), posY, -halfThick);
	    p->centerPosStart = p->centerPos;

	    p->rotAngle = p->rotAngleStart = 0;

	    p->centerRelPos.set ((x + 0.5) / gridSizeX,
				 (y + 0.5) / gridSizeY);
	    p->nSides = 4;
	    p->nVertices = 2 * 4;
	    mNumTotalFrontVertices += 4;

	    // 4 front, 4 back vertices
	    p->vertices = (GLfloat *)calloc (8 * 3, sizeof (GLfloat));
	    if (!p->vertices)
	    {
		compLogMessage ("animationaddon", CompLogLevelError,
				"Not enough memory");
		freePolygonObjects ();
		return false;
	    }

	    // Vertex normals
	    p->normals = (GLfloat *)calloc (8 * 3, sizeof (GLfloat));
	    if (!p->normals)
	    {
		compLogMessage ("animationaddon", CompLogLevelError,
				"Not enough memory");
		freePolygonObjects ();
		return false;
	    }

	    GLfloat *pv = p->vertices;

	    // Determine 4 front vertices in ccw direction
	    pv[0] = -halfW;
	    pv[1] = -halfH;
	    pv[2] = halfThick;

	    pv[3] = -halfW;
	    pv[4] = halfH;
	    pv[5] = halfThick;

	    pv[6] = halfW;
	    pv[7] = halfH;
	    pv[8] = halfThick;

	    pv[9] = halfW;
	    pv[10] = -halfH;
	    pv[11] = halfThick;

	    // Determine 4 back vertices in cw direction
	    pv[12] = halfW;
	    pv[13] = -halfH;
	    pv[14] = -halfThick;

	    pv[15] = halfW;
	    pv[16] = halfH;
	    pv[17] = -halfThick;

	    pv[18] = -halfW;
	    pv[19] = halfH;
	    pv[20] = -halfThick;

	    pv[21] = -halfW;
	    pv[22] = -halfH;
	    pv[23] = -halfThick;

	    // 16 indices for 4 sides (for quads)
	    p->sideIndices = (GLushort *)calloc (4 * 4, sizeof (GLushort));
	    if (!p->sideIndices)
	    {
		compLogMessage ("animationaddon", CompLogLevelError,
				"Not enough memory");
		freePolygonObjects ();
		return false;
	    }

	    GLushort *ind = p->sideIndices;
	    GLfloat *nor = p->normals;

	    int id = 0;

	    // Left face
	    ind[id++] = 6; // First vertex
	    ind[id++] = 1;
	    ind[id++] = 0;
	    ind[id++] = 7;
	    nor[6 * 3 + 0] = -1; // Flat shading only uses 1st vertex's normal
	    nor[6 * 3 + 1] = 0; // in a polygon, vertex 6 for this face.
	    nor[6 * 3 + 2] = 0;

	    // Bottom face
	    ind[id++] = 1;
	    ind[id++] = 6;
	    ind[id++] = 5;
	    ind[id++] = 2;
	    nor[1 * 3 + 0] = 0;
	    nor[1 * 3 + 1] = 1;
	    nor[1 * 3 + 2] = 0;

	    // Right face
	    ind[id++] = 2;
	    ind[id++] = 5;
	    ind[id++] = 4;
	    ind[id++] = 3;
	    nor[2 * 3 + 0] = 1;
	    nor[2 * 3 + 1] = 0;
	    nor[2 * 3 + 2] = 0;

	    // Top face
	    ind[id++] = 7;
	    ind[id++] = 0;
	    ind[id++] = 3;
	    ind[id++] = 4;
	    nor[7 * 3 + 0] = 0;
	    nor[7 * 3 + 1] = -1;
	    nor[7 * 3 + 2] = 0;

	    // Front face normal
	    nor[0] = 0;
	    nor[1] = 0;
	    nor[2] = 1;

	    // Back face normal
	    nor[4 * 3 + 0] = 0;
	    nor[4 * 3 + 1] = 0;
	    nor[4 * 3 + 2] = -1;

	    // Determine bounding box (to test intersection with clips)
	    p->boundingBox.x1 = -halfW;
	    p->boundingBox.y1 = -halfH;
	    p->boundingBox.x2 = halfW;
	    p->boundingBox.y2 = halfH;

	    p->boundSphereRadius =
		sqrt (halfW * halfW + halfH * halfH + halfThick * halfThick);

	    // Reset remaining members
	    p->effectParameters = NULL;
	    p->moveStartTime = 0;
	    p->moveDuration = 0;
	    p->fadeStartTime = 0;
	    p->fadeDuration = 0;
	}
    }
    return true;
}

// Tessellates window into extruded hexagon objects
bool
PolygonAnim::tessellateIntoHexagons (int gridSizeX,
				     int gridSizeY,
				     float thickness)
{
    // boundaries of polygon tessellation
    int winLimitsX;
    int winLimitsY;
    int winLimitsW;
    int winLimitsH;

    CompRect inRect (mAWindow->savedRectsValid () ?
		     mAWindow->savedInRect () :
		     mWindow->borderRect ());
    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());

    if (mIncludeShadows)
    {
	winLimitsX = outRect.x ();
	winLimitsY = outRect.y ();
	winLimitsW = outRect.width () - 1; // avoid artifact on right edge
	winLimitsH = outRect.height ();
    }
    else
    {
	winLimitsX = inRect.x ();
	winLimitsY = inRect.y ();
	winLimitsW = inRect.width ();
	winLimitsH = inRect.height ();
    }
    float minSize = 20;
    float hexW = winLimitsW / (float)gridSizeX;
    float hexH = winLimitsH / (float)gridSizeY;

    if (hexW < minSize)
	gridSizeX = winLimitsW / minSize;	// int div.
    if (hexH < minSize)
	gridSizeY = winLimitsH / minSize;	// int div.

    freePolygonObjects ();
    for (int i = 0; i < (gridSizeY + 1) * gridSizeX + ((gridSizeY + 1 ) / 2); i++)
	mPolygons.push_back (new PolygonObject);

    thickness /= ::screen->width ();
    mThickness = thickness;
    mNumTotalFrontVertices = 0;

    float cellW = (float)winLimitsW / gridSizeX;
    float cellH = (float)winLimitsH / gridSizeY;
    float halfW = cellW / 2;
    float twoThirdsH = 2*cellH / 3;
    float thirdH = cellH / 3;

    float halfThick = mThickness / 2;
    vector<PolygonObject *>::iterator it  = mPolygons.begin ();

    for (int y = 0; y < gridSizeY+1; y++)
    {
	float posY = winLimitsY + cellH * (y);
	int numPolysinRow = (y%2==0) ? gridSizeX : (gridSizeX + 1);

	// Clip polygons to the window dimensions
	float topY, topRightY, topLeftY, bottomY, bottomLeftY, bottomRightY;

	if (y == 0)
	{
	    topY = topRightY = topLeftY = 0;
	    bottomY = twoThirdsH;
	    bottomLeftY = bottomRightY = thirdH;
	}
	else if (y == gridSizeY)
	{
	    bottomY = bottomLeftY = bottomRightY = 0;
	    topY = -twoThirdsH;
	    topLeftY = topRightY = -thirdH;
	}
	else
	{
	    topY = -twoThirdsH;
	    topLeftY = topRightY = -thirdH;
	    bottomLeftY = bottomRightY = thirdH;
	    bottomY = twoThirdsH;
	}

	for (int x = 0; x < numPolysinRow; x++, it++)
	{
	    PolygonObject *p = *it;
	    // Clip odd rows when necessary
	    float topLeftX, topRightX, bottomLeftX, bottomRightX;

	    if (y%2 == 1)
	    {
		if (x == 0)
		{
		    topLeftX = bottomLeftX = 0;
		    topRightX = halfW;
		    bottomRightX = halfW;
		}
		else if (x == numPolysinRow-1)
		{
		    topRightX = bottomRightX = 0;
		    topLeftX = -halfW;
		    bottomLeftX = -halfW;
		}
		else
		{
		    topLeftX = bottomLeftX = -halfW;
		    topRightX = bottomRightX = halfW;
		}
	    }
	    else
	    {
		topLeftX = bottomLeftX = -halfW;
		topRightX = bottomRightX = halfW;
	    }

	    p->centerPos.set (winLimitsX + cellW * (x + (y%2 ? 0.0 : 0.5)),
			      posY, -halfThick);
	    p->centerPosStart = p->centerPos;

	    p->rotAngle = p->rotAngleStart = 0;

	    p->centerRelPos.set ((x + 0.5) / gridSizeX,
				 (y + 0.5) / gridSizeY);
	    p->nSides = 6;
	    p->nVertices = 2 * 6;
	    mNumTotalFrontVertices += 6;

	    // 6 front, 6 back vertices
	    p->vertices = (GLfloat *)calloc (6 * 2 * 3, sizeof (GLfloat));
	    if (!p->vertices)
	    {
		compLogMessage ("animationaddon", CompLogLevelError,
				"Not enough memory");
		freePolygonObjects ();
		return false;
	    }

	    // Vertex normals
	    p->normals = (GLfloat *)calloc (6 * 2 * 3, sizeof (GLfloat));
	    if (!p->normals)
	    {
		compLogMessage ("animationaddon", CompLogLevelError,
				"Not enough memory");
		freePolygonObjects ();
		return false;
	    }

	    GLfloat *pv = p->vertices;

	    // Determine 6 front vertices in ccw direction
	    // Starting at top
	    pv[0] = 0;
	    pv[1] = topY;
	    pv[2] = halfThick;

	    pv[3] = topLeftX;
	    pv[4] = topLeftY;
	    pv[5] = halfThick;

	    pv[6] = bottomLeftX;
	    pv[7] = bottomLeftY;
	    pv[8] = halfThick;

	    pv[9] = 0;
	    pv[10] = bottomY;
	    pv[11] = halfThick;

	    pv[12] = bottomRightX;
	    pv[13] = bottomRightY;
	    pv[14] = halfThick;

	    pv[15] = topRightX;
	    pv[16] = topRightY;
	    pv[17] = halfThick;

	    // Determine 6 back vertices in cw direction
	    pv[18] = topRightX;
	    pv[19] = topRightY;
	    pv[20] = -halfThick;

	    pv[21] = bottomRightX;
	    pv[22] = bottomRightY;
	    pv[23] = -halfThick;

	    pv[24] = 0;
	    pv[25] = bottomY;
	    pv[26] = -halfThick;

	    pv[27] = bottomLeftX;
	    pv[28] = bottomLeftY;
	    pv[29] = -halfThick;

	    pv[30] = topLeftX;
	    pv[31] = topLeftY;
	    pv[32] = -halfThick;

	    pv[33] = 0;
	    pv[34] = topY;
	    pv[35] = -halfThick;

	    // 24 indices per 6 sides (for quads)
	    p->sideIndices = (GLushort *)calloc (4 * 6, sizeof (GLushort));
	    if (!p->sideIndices)
	    {
		compLogMessage ("animationaddon", CompLogLevelError,
				"Not enough memory");
		freePolygonObjects ();
		return false;
	    }

	    GLushort *ind = p->sideIndices;
	    GLfloat *nor = p->normals;

	    int id = 0;

	    // Approximate normals

	    // upper left side face
	    ind[id++] = 11; // First vertex
	    ind[id++] = 10;
	    ind[id++] = 1;
	    ind[id++] = 0;
	    nor[11 * 3 + 0] = -1; // Flat shading only uses 1st vertex's normal
	    nor[11 * 3 + 1] = -1; // in a polygon, vertex 11 for this face.
	    nor[11 * 3 + 2] = 0;
	    if (y == 0) // top half cropped
		nor[11 * 3 + 0] = 0;

	    // left side face
	    ind[id++] = 1;
	    ind[id++] = 10;
	    ind[id++] = 9;
	    ind[id++] = 2;
	    nor[1 * 3 + 0] = -1;
	    nor[1 * 3 + 1] = 0;
	    nor[1 * 3 + 2] = 0;

	    // lower left side face
	    ind[id++] = 2;
	    ind[id++] = 9;
	    ind[id++] = 8;
	    ind[id++] = 3;
	    nor[2 * 3 + 0] = -1;
	    nor[2 * 3 + 1] = 1;
	    nor[2 * 3 + 2] = 0;
	    if (y == gridSizeY) // bottom half cropped
		nor[2 * 3 + 0] = 0;

	    // lower right side face
	    ind[id++] = 3;
	    ind[id++] = 8;
	    ind[id++] = 7;
	    ind[id++] = 4;
	    nor[3 * 3 + 0] = 1;
	    nor[3 * 3 + 1] = 1;
	    nor[3 * 3 + 2] = 0;
	    if (y == gridSizeY) // bottom half cropped
		nor[3 * 3 + 0] = 0;

	    // right side face
	    ind[id++] = 4;
	    ind[id++] = 7;
	    ind[id++] = 6;
	    ind[id++] = 5;
	    nor[4 * 3 + 0] = 1;
	    nor[4 * 3 + 1] = 0;
	    nor[4 * 3 + 2] = 0;

	    // upper right side face
	    ind[id++] = 5;
	    ind[id++] = 6;
	    ind[id++] = 11;
	    ind[id++] = 0;
	    nor[5 * 3 + 0] = 1;
	    nor[5 * 3 + 1] = -1;
	    nor[5 * 3 + 2] = 0;
	    if (y == 0) // top half cropped
		nor[5 * 3 + 0] = 0;

	    // Front face normal
	    nor[0] = 0;
	    nor[1] = 0;
	    nor[2] = 1;

	    // Back face normal
	    nor[6 * 3 + 0] = 0;
	    nor[6 * 3 + 1] = 0;
	    nor[6 * 3 + 2] = -1;

	    // Determine bounding box (to test intersection with clips)
	    p->boundingBox.x1 = topLeftX;
	    p->boundingBox.y1 = topY;
	    p->boundingBox.x2 = bottomRightX;
	    p->boundingBox.y2 = bottomY;

	    p->boundSphereRadius = sqrt ((topRightX - topLeftX) * (topRightX - topLeftX) / 4 +
					(bottomY - topY) * (bottomY - topY) / 4 +
					halfThick * halfThick);

	    // Reset remaining members
	    p->effectParameters = NULL;
	    p->moveStartTime = 0;
	    p->moveDuration = 0;
	    p->fadeStartTime = 0;
	    p->fadeDuration = 0;
	}
    }
    return true;
}

/*        90        //degree orientation
 *         |
 *    180--+--0
 *         |
 *        270
 * This function tessellates the window into radial shards, with
 * each shard split into the number of "tiers". This forms a broken
 * glass or spiderweb appearance.
 */
bool
PolygonAnim::tessellateIntoGlass (int spokeMultiplier,
				  int numTiers,
				  float thickness)
{
    int numSpokes = 4 * spokeMultiplier;
    int winLimitsX, winLimitsY, winLimitsW, winLimitsH;
    float centerX, centerY;
    float topBottomLength, leftRightLength;

    Spoke spokes[numSpokes];
    memset (spokes, 0, sizeof (Spoke) * numSpokes);

    for (int i = 0; i < numSpokes; i++)
    {
	spokes[i].spokeVertex =
	    (SpokeVertex *) calloc (numTiers, sizeof (SpokeVertex));
    }

    CompRect inRect (mAWindow->savedRectsValid () ?
		     mAWindow->savedInRect () :
		     mWindow->borderRect ());
    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());

    if (mIncludeShadows)
    {
	winLimitsX = outRect.x ();
	winLimitsY = outRect.y ();
	winLimitsW = outRect.width () - 1; // avoid artifact on right edge
	winLimitsH = outRect.height ();
    }
    else
    {
	winLimitsX = inRect.x ();
	winLimitsY = inRect.y ();
	winLimitsW = inRect.width ();
	winLimitsH = inRect.height ();
    }

    //tessellation looks horrible if its too small, its better
    //just to skip it
    if (winLimitsW < 100 || winLimitsH < 100)
	return false;

    centerX = (winLimitsW / 2.0) + winLimitsX;
    centerY = (winLimitsH / 2.0) + winLimitsY;

    /* Calculate corner angles */
    float cornerAngle[4];
    cornerAngle[0] = atanf ((centerY - winLimitsY) /
			     (winLimitsX + winLimitsW - centerX));
    cornerAngle[1] = M_PI - cornerAngle[0];
    cornerAngle[2] = M_PI + cornerAngle[0];
    cornerAngle[3] = 2 * M_PI - cornerAngle[0];

    float range;

    //calculate the vertex positions
    for (int i = 0; i < numSpokes; i++)
    {
	/* The spokes must go into the corners. The remaining spokes fit between the corner spokes */
	if ((i % spokeMultiplier) == 0)
	{
	    spokes[i].direction = cornerAngle[i / spokeMultiplier];
	}
	else
	{
	    range = cornerAngle[((i / spokeMultiplier) + 1) % 4 ] -
		cornerAngle[i / spokeMultiplier];
	    if (range < 0)
		range = 2 * M_PI - cornerAngle[i / spokeMultiplier] +
		    cornerAngle[((i / spokeMultiplier) + 1) % 4 ];

	    spokes[i].direction = cornerAngle[i / spokeMultiplier] +
		(i % spokeMultiplier) * range / spokeMultiplier;

	    if (spokes[i].direction > 2 * M_PI)
		spokes[i].direction -= 2 * M_PI;

	    // Random direction
	    spokes[i].direction += range * (float) rand () / 3 / RAND_MAX;
	}


	//calculate the length of the spoke
	//calculate top/bottom lenght
	if ((spokes[i].direction < M_PI))
	    topBottomLength = (centerY - winLimitsY);
	else
	    topBottomLength = ((winLimitsY + winLimitsH)- centerY);

	topBottomLength /= sinf (spokes[i].direction);
	if (topBottomLength < 0) topBottomLength *= -1;

	//calculate left right length
	if ((spokes[i].direction < M_PI / 2) || (spokes[i].direction > 3 * M_PI / 2))
	    leftRightLength = (winLimitsX + winLimitsW) - centerX;
	else
	    leftRightLength = centerX - winLimitsX;

	leftRightLength /= cosf (spokes[i].direction);

	if (leftRightLength < 0) leftRightLength *= -1;

	//take the smaller of the two
	if (leftRightLength < topBottomLength)
	    spokes[i].length = leftRightLength;
	else
	    spokes[i].length = topBottomLength;


	float percent = 1.0 / ((float) numTiers);
	//calculate spoke vertexes
	for (int j = 0 ; j < numTiers; j++)
	{
	    spokes[i].spokeVertex[j].dist = percent * (j + 1) * spokes[i].length;

	    spokes[i].spokeVertex[j].x =
		centerX + (spokes[i].spokeVertex[j].dist * cos (spokes[i].direction));
	    spokes[i].spokeVertex[j].y =
		centerY + (spokes[i].spokeVertex[j].dist * sin (spokes[i].direction));
	}
    }


    Shard shards[numSpokes][numTiers];

    //calculate the center and bounds of each polygon
    for (int i = 0; i < numSpokes; i++)
    {
	for (int j = 0; j < numTiers; j++)
	{
	    switch (j)
	    {
	    case 0:
		//the first tier is triangles
		shards[i][j].isTriangle = true;
		shards[i][j].pt0X = centerX;
		shards[i][j].pt0Y = centerY;

		shards[i][j].pt1X = spokes[i].spokeVertex[j].x;
		shards[i][j].pt1Y = spokes[i].spokeVertex[j].y;

		shards[i][j].pt2X = spokes[(i + 1) % numSpokes].spokeVertex[j].x;
		shards[i][j].pt2Y = spokes[(i + 1) % numSpokes].spokeVertex[j].y;

		shards[i][j].pt3X = shards[i][j].pt0X;//fourth point is not used
		shards[i][j].pt3Y = shards[i][j].pt0Y;

		//find lengths
		shards[i][j].centerX = (shards[i][j].pt2X + shards[i][j].pt1X + shards[i][j].pt0X)/3;
		shards[i][j].centerY = (shards[i][j].pt2Y + shards[i][j].pt1Y + shards[i][j].pt0Y)/3;

		break;



	    default:
		//the other tiers are 4 sided polygons
		shards[i][j].isTriangle = false;
		shards[i][j].pt0X = spokes[i].spokeVertex[j - 1].x;
		shards[i][j].pt0Y = spokes[i].spokeVertex[j - 1].y;

		shards[i][j].pt1X = spokes[i].spokeVertex[j].x;
		shards[i][j].pt1Y = spokes[i].spokeVertex[j].y;

		if (i != numSpokes - 1)
		{
		    shards[i][j].pt2X = spokes[i + 1].spokeVertex[j].x;
		    shards[i][j].pt2Y = spokes[i + 1].spokeVertex[j].y;

		    shards[i][j].pt3X = spokes[i + 1].spokeVertex[j - 1].x;
		    shards[i][j].pt3Y = spokes[i + 1].spokeVertex[j - 1].y;
		}
		else
		{
		    shards[i][j].pt2X = spokes[0].spokeVertex[j].x;
		    shards[i][j].pt2Y = spokes[0].spokeVertex[j].y;

		    shards[i][j].pt3X = spokes[0].spokeVertex[j - 1].x;
		    shards[i][j].pt3Y = spokes[0].spokeVertex[j - 1].y;
		}

		//calculate the center of the polygon
		shards[i][j].centerX =
		    (shards[i][j].pt0X + shards[i][j].pt1X + shards[i][j].pt2X + shards[i][j].pt3X)/4;
		shards[i][j].centerY =
		    (shards[i][j].pt0Y + shards[i][j].pt1Y + shards[i][j].pt2Y + shards[i][j].pt3Y)/4;

		break;
	    }
	}

    }

    //set up polygons
    freePolygonObjects ();
    for (int i = 0; i < numSpokes * numTiers; i++)
	mPolygons.push_back (new PolygonObject);

    thickness /= ::screen->width ();
    mThickness = thickness;
    mNumTotalFrontVertices = 0;

    float halfThick = mThickness / 2;
    vector<PolygonObject *>::iterator it = mPolygons.begin ();

    for (int yc = 0; yc <  numSpokes; yc++) //spokes
    {
	for (int xc = 0; xc < numTiers; xc++, it++) //tiers
	{
	    PolygonObject *p = *it;
	    p->centerPos.set (shards[yc][xc].centerX,
			      shards[yc][xc].centerY, -halfThick);
	    p->centerPosStart = p->centerPos;

	    p->centerRelPos.set ((shards[yc][xc].centerX - winLimitsX) /
				 winLimitsW,
				 (shards[yc][xc].centerY - winLimitsY) /
				 winLimitsH);

	    p->rotAngle = p->rotAngleStart = 0;

	    p->nSides = 4;
	    p->nVertices = 2 * 4;
	    mNumTotalFrontVertices += 4;

	    // 4 front, 4 back vertices
	    p->vertices = (GLfloat *)calloc (8 * 3, sizeof (GLfloat));
	    if (!p->vertices)
	    {
		compLogMessage ("animationaddon",
				CompLogLevelError, "Not enough memory");
		freePolygonObjects ();
		return false;
	    }

	    // Vertex normals
	    p->normals = (GLfloat *)calloc (8 * 3, sizeof (GLfloat));
	    if (!p->normals)
	    {
		compLogMessage ("animationaddon",
				CompLogLevelError,
				"Not enough memory");
		freePolygonObjects ();
		return false;
	    }

	    GLfloat *pv = p->vertices;

	    // Determine 4 front vertices in ccw direction
	    pv[0] = -shards[yc][xc].centerX + shards[yc][xc].pt3X;
	    pv[1] = -shards[yc][xc].centerY + shards[yc][xc].pt3Y;
	    pv[2] = halfThick;

	    pv[3] = -shards[yc][xc].centerX + shards[yc][xc].pt2X;
	    pv[4] = -shards[yc][xc].centerY + shards[yc][xc].pt2Y;
	    pv[5] = halfThick;

	    pv[6] = -shards[yc][xc].centerX + shards[yc][xc].pt1X;
	    pv[7] = -shards[yc][xc].centerY + shards[yc][xc].pt1Y;
	    pv[8] = halfThick;

	    pv[9] = -shards[yc][xc].centerX + shards[yc][xc].pt0X;
	    pv[10] = -shards[yc][xc].centerY + shards[yc][xc].pt0Y;
	    pv[11] = halfThick;

	    // Determine 4 back vertices in cw direction
	    pv[12] = -shards[yc][xc].centerX + shards[yc][xc].pt0X;
	    pv[13] = -shards[yc][xc].centerY + shards[yc][xc].pt0Y;
	    pv[14] = -halfThick;

	    pv[15] = -shards[yc][xc].centerX + shards[yc][xc].pt1X;
	    pv[16] = -shards[yc][xc].centerY + shards[yc][xc].pt1Y;
	    pv[17] = -halfThick;

	    pv[18] = -shards[yc][xc].centerX + shards[yc][xc].pt2X;
	    pv[19] = -shards[yc][xc].centerY + shards[yc][xc].pt2Y;
	    pv[20] = -halfThick;

	    pv[21] = -shards[yc][xc].centerX + shards[yc][xc].pt3X;
	    pv[22] = -shards[yc][xc].centerY + shards[yc][xc].pt3Y;
	    pv[23] = -halfThick;

	    // 16 indices for 4 sides (for quads)
	    p->sideIndices = (GLushort *)calloc (4 * 4, sizeof (GLushort));
	    if (!p->sideIndices)
	    {
		compLogMessage ("animationaddon",
				CompLogLevelError, "Not enough memory");
		freePolygonObjects ();
		return false;
	    }

	    GLushort *ind = p->sideIndices;
	    GLfloat *nor = p->normals;

	    int id = 0;

	    // Left face
	    ind[id++] = 6; // First vertex
	    ind[id++] = 1;
	    ind[id++] = 0;
	    ind[id++] = 7;
	    nor[6 * 3 + 0] = -1; // Flat shading only uses 1st vertex's normal
	    nor[6 * 3 + 1] = 0; // in a polygon, vertex 6 for this face.
	    nor[6 * 3 + 2] = 0;

	    // Bottom face
	    ind[id++] = 1;
	    ind[id++] = 6;
	    ind[id++] = 5;
	    ind[id++] = 2;
	    nor[1 * 3 + 0] = 0;
	    nor[1 * 3 + 1] = 1;
	    nor[1 * 3 + 2] = 0;

	    // Right face
	    ind[id++] = 2;
	    ind[id++] = 5;
	    ind[id++] = 4;
	    ind[id++] = 3;
	    nor[2 * 3 + 0] = 1;
	    nor[2 * 3 + 1] = 0;
	    nor[2 * 3 + 2] = 0;

	    // Top face
	    ind[id++] = 7;
	    ind[id++] = 0;
	    ind[id++] = 3;
	    ind[id++] = 4;
	    nor[7 * 3 + 0] = 0;
	    nor[7 * 3 + 1] = -1;
	    nor[7 * 3 + 2] = 0;

	    // Front face normal
	    nor[0] = 0;
	    nor[1] = 0;
	    nor[2] = 1;

	    // Back face normal
	    nor[4 * 3 + 0] = 0;
	    nor[4 * 3 + 1] = 0;
	    nor[4 * 3 + 2] = -1;

	    // Determine bounding box (to test intersection with clips)
	    p->boundingBox.x1 = -shards[yc][xc].pt3X;
	    p->boundingBox.y1 = -shards[yc][xc].pt3Y;
	    p->boundingBox.x2 = shards[yc][xc].pt1X;
	    p->boundingBox.y2 = shards[yc][xc].pt1Y;

	    float dist[4] = {0}, longestDist = 0;
	    dist[0] = sqrt (powf ((shards[yc][xc].centerX - shards[yc][xc].pt0X), 2) +
			    powf ((shards[yc][xc].centerY - shards[yc][xc].pt0Y), 2) +
			    powf (thickness,2));
	    dist[1] = sqrt (powf ((shards[yc][xc].centerX - shards[yc][xc].pt1X), 2) +
			    powf ((shards[yc][xc].centerY - shards[yc][xc].pt1Y), 2) +
			    powf (thickness,2));
	    dist[2] = sqrt (powf ((shards[yc][xc].centerX - shards[yc][xc].pt2X), 2) +
			    powf ((shards[yc][xc].centerY - shards[yc][xc].pt2Y), 2) +
			    powf (thickness,2));
	    dist[3] = sqrt (powf ((shards[yc][xc].centerX - shards[yc][xc].pt3X), 2) +
			    powf ((shards[yc][xc].centerY - shards[yc][xc].pt3Y), 2) +
			    powf (thickness,2));

	    for (int k = 0; k < 4; k++)
	    {
		if (dist[k] > longestDist)
		    longestDist = dist[k];
	    }

	    p->boundSphereRadius = longestDist;

	    // Reset remaining members
	    p->effectParameters = NULL;
	    p->moveStartTime = 0;
	    p->moveDuration = 0;
	    p->fadeStartTime = 0;
	    p->fadeDuration = 0;
	}
    }

    return true;
}

void
PolygonAnim::addGeometry (const GLTexture::MatrixList &matrix,
                          const CompRegion            &region,
                          const CompRegion            &clipRegion,
                          unsigned int                maxGridWidth,
                          unsigned int                maxGridHeight)
{
    // TODO
    // if polygon set is not valid or effect is not 3D (glide w/thickness=0)
    //if (!pset)
    //return;

    /* Some plugins (such as blur) pass a null argument to glAddGeometry
     * because they don't have such data and they need to calculate region
     * projections. In this case, we should not add polygon geometry
     * since the matrices will be null */

    if (!matrix.size ())
	return;

    GLWindow::Geometry &geometry = GLWindow::get (mWindow)->geometry ();
    geometry.vCount = 1; // Force glDrawGeometry to be called

    bool dontStoreClips = true;

    // If this clip doesn't match the corresponding stored clip,
    // clear the stored clips from this point (aw->nClipsPassed)
    // to the end and store the new ones instead.

    if (mNumClipsPassed < (int)mClips.size ()) // if we have clips stored earlier
    {
	Clip4Polygons *c = &mClips[mNumClipsPassed];
	// the stored clip at position aw->nClipsPassed

	// if either clip coordinates or texture matrix is different
	if (region.rects ()[0] != c->box ||
	    memcmp (&matrix[0], &c->texMatrix, sizeof (GLTexture::Matrix)))
	{
	    // get rid of the clips from here (aw->nClipsPassed) to the end
	    Clip4Polygons *curClip = c;
	    for (unsigned i = mNumClipsPassed; i < mClips.size (); i++, curClip++)
		foreach (PolygonClipInfo *pci,
		         curClip->intersectingPolygonInfos)
		    delete pci;
	    mClips.resize (mNumClipsPassed);
	    dontStoreClips = false;
	}
    }
    else
	dontStoreClips = false;

    if (dontStoreClips)
    {
	mNumClipsPassed += region.numRects ();
	return;
    }

    if (mClips.size () == 0)
    {
	mLastClipInGroup.clear ();
	mLastClipInGroup.reserve (2);
    }
    CompRect inRect (mAWindow->savedRectsValid () ?
		     mAWindow->savedInRect () :
		     mWindow->borderRect ());

    mClips.reserve (region.numRects ());

    // For each clip passed to this function
    foreach (const CompRect &rect, region.rects ())
    {
	// Add new clip

	Clip4Polygons newClip;

	newClip.box = rect;
	newClip.texMatrix = matrix[0];
	// nMatrix is not used for now
	// (i.e. only first texture matrix is considered)

	// avoid clipping along window edge
	// for the "window contents" clip
	if (rect.x1 () == inRect.x1 () &&
	    rect.y1 () == inRect.y1 () &&
	    rect.x2 () == inRect.x2 () &&
	    rect.y2 () == inRect.y2 ())
	{
	    newClip.boxf.x1 = rect.x1 () - 0.1f;
	    newClip.boxf.y1 = rect.y1 () - 0.1f;
	    newClip.boxf.x2 = rect.x2 () + 0.1f;
	    newClip.boxf.y2 = rect.y2 () + 0.1f;
	}
	else
	{
	    newClip.boxf.x1 = rect.x1 ();
	    newClip.boxf.y1 = rect.y1 ();
	    newClip.boxf.x2 = rect.x2 ();
	    newClip.boxf.y2 = rect.y2 ();
	}

	mClips.push_back (newClip);
	mNumClipsPassed++;
    }
    mClipsUpdated = true;
}

/// Allocates floats for texture coordinates:
///  2 {x, y} * 2 {front, back} * <# of polygon vertices>
PolygonClipInfo::PolygonClipInfo (const PolygonObject *p) :
    p (p)
{
    vertexTexCoords.resize (4 * p->nSides);
}

/// For each rectangular clip, this function finds polygons which
/// have a bounding box that intersects the clip. For intersecting
/// polygons, it computes the texture coordinates for the vertices
/// of that polygon (to draw the clip texture).
void
PolygonAnim::processIntersectingPolygons ()
{
    int numClips = mClips.size ();

    Clip4Polygons *c = &mClips[mFirstNondrawnClip];
    for (int j = mFirstNondrawnClip; j < numClips; j++, c++)
    {
	CompRect &cb = c->box;
	int nFrontVerticesTilThisPoly = 0; // only used when intersectsMostPolygons is true

	// if this clip is the "window contents" clip
	if (cb == mWindow->geometry())
	{
	    c->intersectsMostPolygons = true;

	    // allocate tex coords for all polygons
	    // 2 {x, y} * 2 {front, back} *
	    //     <total # of polygon front vertices>
	    c->polygonVertexTexCoords.resize (4 * mNumTotalFrontVertices);
	}
	else
	    c->intersectsMostPolygons = false;

	foreach (const PolygonObject *p, mPolygons)
	{
	    int nSides = p->nSides;
	    float px = p->centerPosStart.x ();
	    float py = p->centerPosStart.y ();
	    const Boxf &pb = p->boundingBox;
	    GLfloat *vTexCoords = NULL;

	    if (c->intersectsMostPolygons)
		vTexCoords =
		    &c->polygonVertexTexCoords[4 * nFrontVerticesTilThisPoly];
	    else
	    {
		if (pb.x2 + px <= cb.x1 () ||
		    pb.y2 + py <= cb.y1 () ||
		    pb.x1 + px >= cb.x2 () ||
		    pb.y1 + py >= cb.y2 ())   // no intersection
		    continue;

		PolygonClipInfo *pci = new PolygonClipInfo (p);
		c->intersectingPolygonInfos.push_back (pci);
		vTexCoords = &pci->vertexTexCoords[0];
	    }

	    for (int k = 0; k < nSides; k++)
	    {
		float x = p->vertices[3 * k]     + p->centerPosStart.x ();
		float y = p->vertices[3 * k + 1] + p->centerPosStart.y ();
		GLfloat tx;
		GLfloat ty;
		if (c->texMatrix.xy != 0.0f || c->texMatrix.yx != 0.0f)
		{	// "non-rect" coordinates
		    tx = COMP_TEX_COORD_XY (c->texMatrix, x, y);
		    ty = COMP_TEX_COORD_YX (c->texMatrix, x, y);
		}
		else
		{
		    tx = COMP_TEX_COORD_X (c->texMatrix, x);
		    ty = COMP_TEX_COORD_Y (c->texMatrix, y);
		}
		// for front vertices
		int ti = 2 * k;

		vTexCoords[ti] = tx;
		vTexCoords[ti + 1] = ty;

		// for back vertices
		ti = 2 * (2 * nSides - 1 - k);
		vTexCoords[ti] = tx;
		vTexCoords[ti + 1] = ty;
	    }
	    nFrontVerticesTilThisPoly += nSides;
	}
    }
}

// Correct perspective appearance by skewing
void
PolygonAnim::getPerspectiveCorrectionMat (const PolygonObject *p,
                                          GLfloat *mat,
                                          GLMatrix *matf,
                                          const CompOutput &output)
{
    Point center;
    CompRect outRect (mAWindow->savedRectsValid () ?
		      mAWindow->savedOutRect () :
		      mWindow->outputRect ());

    if (p) // for CorrectPerspectivePolygon
    {
	// use polygon's center
	center.setX (p->centerPos.x ());
	center.setY (p->centerPos.y ());
    }
    else // for CorrectPerspectiveWindow
    {
	// use window's center
	center.setX (outRect.x () + outRect.width () / 2);
	center.setY (outRect.y () + outRect.height () / 2);
    }

    GLfloat skewx = -(((center.x () - output.region ()->extents.x1) -
		       output.width () / 2) * 1.15);
    GLfloat skewy = -(((center.y () - output.region ()->extents.y1) -
		       output.height () / 2) * 1.15);

    if (mat)
    {
	// column-major order skew matrix
	GLfloat skewMat[16] =
	    {1,0,0,0,
	     0,1,0,0,
	     skewx,skewy,1,0,
	     0,0,0,1};
	memcpy (mat, skewMat, 16 * sizeof (GLfloat));
    }
    else if (matf)
    {
	// column-major order skew matrix
	float skewMat[16] =
	    {1,0,0,0,
	     0,1,0,0,
	     skewx,skewy,1,0,
	     0,0,0,1};
	*matf = GLMatrix (skewMat);
    }
}

void
PolygonAnim::prepareDrawingForAttrib (GLFragment::Attrib &attrib)
{
    if (GL::canDoSaturated && attrib.getSaturation () != COLOR)
    {
	GLfloat constant[4];

	if (GL::canDoSlightlySaturated && attrib.getSaturation () > 0)
	{
	    constant[3] = attrib.getOpacity () / 65535.0f;
	    constant[0] = constant[1] = constant[2] = constant[3] *
		attrib.getBrightness () / 65535.0f;

	    glTexEnvfv (GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constant);
	}
	else
	{
	    constant[3] = attrib.getOpacity () / 65535.0f;
	    constant[0] = constant[1] = constant[2] = constant[3] *
		attrib.getBrightness () / 65535.0f;

	    constant[0] = 0.5f + 0.5f * RED_SATURATION_WEIGHT   * constant[0];
	    constant[1] = 0.5f + 0.5f * GREEN_SATURATION_WEIGHT * constant[1];
	    constant[2] = 0.5f + 0.5f * BLUE_SATURATION_WEIGHT  * constant[2];

	    glTexEnvfv (GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, constant);
	}
    }
    else
    {
	attrib.setBrightness (0.76 * attrib.getBrightness ());

	GLushort color =
	    (attrib.getOpacity () * attrib.getBrightness ()) >> 16;

	mGScreen->setTexEnvMode (GL_MODULATE);
	glColor4us (color, color, color, attrib.getOpacity ());
    }
}

inline void
PolygonAnim::drawPolygonClipIntersection (const PolygonObject *p,
                                          const Clip4Polygons &c,
                                          const GLfloat *vertexTexCoords,
                                          int pass,
                                          float forwardProgress,
                                          GLdouble clipPlanes[4][4],
                                          const CompOutput &output,
                                          float newOpacity,
                                          bool decelerates,
                                          GLfloat skewMat[16])
{
    int nSides = p->nSides;
    float newOpacityPolygon = newOpacity;

    // if fade-out duration is specified per polygon
    if (mAllFadeDuration == -1.0f)
    {
	float fadePassedBy = forwardProgress - p->fadeStartTime;
	// if "fade out starting point" is passed
	if (fadePassedBy > 1e-5)	// if true, then allFadeDuration > 0
	{
	    float opacityFac;

	    if (decelerates)
		opacityFac =
		    1 - progressDecelerate (fadePassedBy /
					    p->fadeDuration);
	    else
		opacityFac = 1 - fadePassedBy / p->fadeDuration;
	    if (opacityFac < 0)
		opacityFac = 0;
	    if (opacityFac > 1)
		opacityFac = 1;
	    newOpacityPolygon = newOpacity * opacityFac;
	}
    }

    if (newOpacityPolygon < 1e-5)	// if polygon object is invisible
	return;

    if (pass == 0)
    {
	if (newOpacityPolygon < 0.9999)	// if not fully opaque
	    return;	// draw only opaque ones in pass 0
    }
    else if (newOpacityPolygon > 0.9999)	// if fully opaque
	return;	// draw only non-opaque ones in pass 1

    glPushMatrix ();

    if (mCorrectPerspective == CorrectPerspectivePolygon)
	getPerspectiveCorrectionMat (p, skewMat, NULL, output);

    if (mCorrectPerspective != CorrectPerspectiveNone)
	glMultMatrixf (skewMat);

    // Center
    glTranslatef (p->centerPos.x (),
		  p->centerPos.y (),
		  p->centerPos.z ());

    // Scale z first
    glScalef (1.0f, 1.0f, 1.0f / ::screen->width ());

    transformPolygon (p);

    // Move by "rotation axis offset"
    glTranslatef (p->rotAxisOffset.x (),
		  p->rotAxisOffset.y (),
		  p->rotAxisOffset.z ());

    // Rotate by desired angle
    glRotatef (p->rotAngle,
	       p->rotAxis.x (), p->rotAxis.y (), p->rotAxis.z ());

    // Move back to center
    glTranslatef (-p->rotAxisOffset.x (),
		  -p->rotAxisOffset.y (),
		  -p->rotAxisOffset.z ());

    // Scale back
    glScalef (1.0f, 1.0f, ::screen->width ());

    clipPlanes[0][3] = -(c.boxf.x1 - p->centerPosStart.x ());
    clipPlanes[1][3] = -(c.boxf.y1 - p->centerPosStart.y ());
    clipPlanes[2][3] = (c.boxf.x2 - p->centerPosStart.x ());
    clipPlanes[3][3] = (c.boxf.y2 - p->centerPosStart.y ());
    glClipPlane (GL_CLIP_PLANE0, clipPlanes[0]);
    glClipPlane (GL_CLIP_PLANE1, clipPlanes[1]);
    glClipPlane (GL_CLIP_PLANE2, clipPlanes[2]);
    glClipPlane (GL_CLIP_PLANE3, clipPlanes[3]);

    for (int k = 0; k < 4; k++)
	glEnable (GL_CLIP_PLANE0 + k);
    bool fadeBackAndSides =
	mBackAndSidesFadeDur > 0 &&
	forwardProgress <= mBackAndSidesFadeDur;

    float newOpacityPolygon2 = newOpacityPolygon;

    if (fadeBackAndSides)
    {
	// Fade-in opacity for back face and sides
	newOpacityPolygon2 *=
	    (forwardProgress / mBackAndSidesFadeDur);
    }

    GLFragment::Attrib attrib = mCurPaintAttrib;
    attrib.setOpacity (newOpacityPolygon2 * OPAQUE);

    prepareDrawingForAttrib (attrib);

    // Draw back face
    glVertexPointer (3, GL_FLOAT, 0, p->vertices + 3 * nSides);
    if (mThickness > 0)
	glNormalPointer (GL_FLOAT, 0, p->normals + 3 * nSides);
    else
	glNormal3f (0.0f, 0.0f, -1.0f);
    glTexCoordPointer (2, GL_FLOAT, 0, &vertexTexCoords[2 * nSides]);
    glDrawArrays (GL_POLYGON, 0, nSides);

    // Vertex coords
    glVertexPointer (3, GL_FLOAT, 0, p->vertices);
    if (mThickness > 0)
	glNormalPointer (GL_FLOAT, 0, p->normals);
    else
	glNormal3f (0.0f, 0.0f, 1.0f);
    glTexCoordPointer (2, GL_FLOAT, 0, vertexTexCoords);

    // Draw quads for sides
    for (int k = 0; k < nSides; k++)
    {
	// GL_QUADS uses a different vertex normal than the first
	// so I use GL_POLYGON to make sure the normals are right.
	glDrawElements (GL_POLYGON, 4,
		       GL_UNSIGNED_SHORT,
		       p->sideIndices + k * 4);
    }

    // if opacity was changed just above
    if (fadeBackAndSides)
    {
	// Go back to normal opacity for front face
	attrib = mCurPaintAttrib;
	attrib.setOpacity (newOpacityPolygon * OPAQUE);
	prepareDrawingForAttrib (attrib);
    }
    // Draw front face
    glDrawArrays (GL_POLYGON, 0, nSides);
    for (int k = 0; k < 4; k++)
	glDisable (GL_CLIP_PLANE0 + k);

    glPopMatrix ();
}

void
PolygonAnim::drawGeometry ()
{
    mNumDrawGeometryCalls++;

    // draw windows only on current viewport unless it's on all viewports
    CompPoint pnt = mCScreen->windowPaintOffset ();
    if ((pnt.x () != 0 || pnt.y () != 0) &&
	!mWindow->onAllViewports ())
    {
	return;
	// since this is not the viewport the window was drawn
	// just before animation started
    }

    // TODO
    // if polygon set is not valid or effect is not 3D (glide w/thickness=0)
    //if (!pset)
    //return;

    int numClips = mClips.size ();

    if (mFirstNondrawnClip < 0 ||
	mFirstNondrawnClip >= numClips)
    {
	return;
    }

    if (mClipsUpdated)
	processIntersectingPolygons ();

    int lastClip;				// last clip to draw

    if (mClipsUpdated)
    {
	lastClip = numClips - 1;
    }
    else
    {
	assert (!mLastClipInGroup.empty());
	lastClip = mLastClipInGroup[mNumDrawGeometryCalls - 1];
    }

    float forwardProgress = progressLinear ();

    // OpenGL stuff starts here

    GLboolean normalArrayWas = false;

    if (mThickness > 0)
    {
	glPushAttrib (GL_NORMALIZE);
	glEnable (GL_NORMALIZE);

	normalArrayWas = glIsEnabled (GL_NORMAL_ARRAY);
	glEnableClientState (GL_NORMAL_ARRAY);
    }

    if (mDoLighting)
    {
	glPushAttrib (GL_SHADE_MODEL);
	glShadeModel (GL_FLAT);

	glPushAttrib (GL_LIGHT0);
	glPushAttrib (GL_COLOR_MATERIAL);
	glPushAttrib (GL_LIGHTING);
	glEnable (GL_COLOR_MATERIAL);
	glEnable (GL_LIGHTING);

	GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 0.3f };
	GLfloat diffuseLight[] = { 0.9f, 0.9f, 0.9f, 0.9f };
	GLfloat light0Position[] = { -0.5f, 0.5f, 9.0f, 0.0f };

	glLightfv (GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv (GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv (GL_LIGHT0, GL_POSITION, light0Position);
    }

    glPushMatrix ();

    glPushAttrib (GL_STENCIL_BUFFER_BIT);
    glDisable (GL_STENCIL_TEST);

    if (mDoDepthTest)
    {
	// Depth test
	glPushAttrib (GL_DEPTH_FUNC);
	glPushAttrib (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);
	glEnable (GL_DEPTH_TEST);
    }

    // Clip planes
    GLdouble clipPlanes[4][4] = {{ 1, 0, 0, 0 },
				 { 0, 1, 0, 0 },
				 { -1, 0, 0, 0 },
				 { 0, -1, 0, 0 }};

    // Save old color values
    GLfloat oldColor[4];

    glGetFloatv (GL_CURRENT_COLOR, oldColor);

    // Determine where we are called from in paint.c's drawWindowTexture
    // to find out how we should change the opacity
    GLint prevActiveTexture = GL_TEXTURE0_ARB;
    bool saturationFull = true;

    if (GL::canDoSaturated && mCurPaintAttrib.getSaturation () != COLOR)
    {
	saturationFull = false;
	if (GL::canDoSlightlySaturated &&
	    mCurPaintAttrib.getSaturation () > 0)
	{
	    if (mCurPaintAttrib.getOpacity () < OPAQUE ||
		mCurPaintAttrib.getBrightness () != BRIGHT)
		prevActiveTexture = GL_TEXTURE3_ARB;
	    else
		prevActiveTexture = GL_TEXTURE2_ARB;
	}
	else
	    prevActiveTexture = GL_TEXTURE1_ARB;
    }

    float opacity = mCurPaintAttrib.getOpacity () / 65535.0;

    float newOpacity = opacity;
    float fadePassedBy;

    bool decelerates = deceleratingMotion ();

    glPushAttrib (GL_BLEND);
    glEnable (GL_BLEND);

    if (saturationFull)
	mGScreen->setTexEnvMode (GL_MODULATE);

    // if fade-out duration is not specified per polygon
    if (mAllFadeDuration > -1.0f)
    {
	fadePassedBy = forwardProgress - (1 - mAllFadeDuration);

	// if "fade out starting point" is passed
	if (fadePassedBy > 1e-5)	// if true, allFadeDuration should be > 0
	{
	    float opacityFac;

	    if (decelerates)
		opacityFac = 1 - progressDecelerate (fadePassedBy /
						     mAllFadeDuration);
	    else
		opacityFac = 1 - fadePassedBy / mAllFadeDuration;
	    if (opacityFac < 0)
		opacityFac = 0;
	    if (opacityFac > 1)
		opacityFac = 1;
	    newOpacity = opacity * opacityFac;
	}
    }

    const CompOutput *output =
	static_cast<ExtensionPluginAnimAddon*> (getExtensionPluginInfo ())->
	output ();

    GLfloat skewMat[16];
    if (mCorrectPerspective == CorrectPerspectiveWindow)
	getPerspectiveCorrectionMat (NULL, skewMat, NULL, *output);

    // pass: 0: draw opaque ones
    //       1: draw transparent ones
    for (int pass = 0; pass < 2; pass++)
    {
	Clip4Polygons *c = &mClips[mFirstNondrawnClip];
	for (int j = mFirstNondrawnClip; j <= lastClip; j++, c++)
	{
	    if (c->intersectsMostPolygons)
	    {
		const GLfloat *vTexCoords = &c->polygonVertexTexCoords[0];
		foreach (const PolygonObject *p, mPolygons)
		{
		    drawPolygonClipIntersection (p, *c,
		                                 vTexCoords,
						 pass, forwardProgress,
						 clipPlanes, *output,
						 newOpacity,
						 decelerates,
						 skewMat);
		    vTexCoords += 4 * p->nSides;
		}
	    }
	    else
	    {
		foreach (const PolygonClipInfo *pci,
		         c->intersectingPolygonInfos)
		{
		    drawPolygonClipIntersection (pci->p, *c,
		                                 &pci->vertexTexCoords[0],
						 pass, forwardProgress,
						 clipPlanes, *output,
						 newOpacity,
						 decelerates,
						 skewMat);
		}
	    }
	}
    }
    // Restore
    // -----------------------------------------

    // Restore old color values
    glColor4f (oldColor[0], oldColor[1], oldColor[2], oldColor[3]);

    glPopAttrib (); // GL_BLEND

    if (mDoDepthTest)
    {
	glPopAttrib (); // GL_DEPTH_TEST
	glPopAttrib (); // GL_DEPTH_FUNC
    }

    glPopAttrib (); // GL_STENCIL_BUFFER_BIT

    // Restore texture stuff
    if (saturationFull)
	mGScreen->setTexEnvMode (GL_REPLACE);

    glPopMatrix ();
    (GL::activeTexture) (prevActiveTexture);

    if (mDoLighting)
    {
	glPopAttrib (); // GL_LIGHTING
	glPopAttrib (); // GL_COLOR_MATERIAL
	glPopAttrib (); // GL_LIGHT0
	glPopAttrib (); // GL_SHADE_MODEL
    }

    if (mThickness > 0)
    {
	glPopAttrib (); // GL_NORMALIZE

	if (normalArrayWas)
	    glEnableClientState (GL_NORMAL_ARRAY);
	else
	    glDisableClientState (GL_NORMAL_ARRAY);
    }
    else
	glNormal3f (0.0f, 0.0f, -1.0f);

    if (mClipsUpdated)		// set the end mark for this group of clips
	mLastClipInGroup.push_back (lastClip);

    assert (!mLastClipInGroup.empty ());
    // Next time, start drawing from next group of clips
    mFirstNondrawnClip =
	mLastClipInGroup[mNumDrawGeometryCalls - 1] + 1;
}

void
PolygonAnim::prePaintWindow ()
{
    mNumDrawGeometryCalls = 0;

    mFirstNondrawnClip = 0;
}

void
PolygonAnim::postPaintWindow ()
{
    if (mClipsUpdated &&	// clips should be dropped only in the 1st step
	mNumDrawGeometryCalls == 0)	// if clips not drawn
    {
	// drop these unneeded clips (e.g. ones passed by blurfx)
	mClips.resize (mFirstNondrawnClip);
    }
}

// Computes polygon's new position and orientation
// with linear movement
void
PolygonAnim::stepPolygon (PolygonObject *p,
			  float forwardProgress)
{
    float moveProgress = forwardProgress - p->moveStartTime;

    if (p->moveDuration > 0)
	moveProgress /= p->moveDuration;
    if (moveProgress < 0)
	moveProgress = 0;
    else if (moveProgress > 1)
	moveProgress = 1;

    p->centerPos.setX (moveProgress * p->finalRelPos.x () +
                       p->centerPosStart.x ());
    p->centerPos.setY (moveProgress * p->finalRelPos.y () +
                       p->centerPosStart.y ());
    p->centerPos.setZ (1.0f / ::screen->width () *
	moveProgress * p->finalRelPos.z () + p->centerPosStart.z ());

    p->rotAngle = moveProgress * p->finalRotAng + p->rotAngleStart;
}

// Similar to stepPolygon,
// but slightly ac/decelerates movement
void
PolygonAnim::deceleratingAnimStepPolygon (PolygonObject *p,
                                          float forwardProgress)
{
    // TODO: Refactor

    float moveProgress = forwardProgress - p->moveStartTime;

    if (p->moveDuration > 0)
	moveProgress /= p->moveDuration;
    if (moveProgress < 0)
	moveProgress = 0;
    else if (moveProgress > 1)
	moveProgress = 1;

    moveProgress = progressDecelerate (moveProgress);

    p->centerPos.setX (moveProgress * p->finalRelPos.x () +
                       p->centerPosStart.x ());
    p->centerPos.setY (moveProgress * p->finalRelPos.y () +
                       p->centerPosStart.y ());
    p->centerPos.setZ (1.0f / ::screen->width () *
	moveProgress * p->finalRelPos.z () + p->centerPosStart.z ());

    p->rotAngle = moveProgress * p->finalRotAng + p->rotAngleStart;
}

void
PolygonAnim::step ()
{
    float forwardProgress = progressLinear ();

    foreach (PolygonObject *p, mPolygons)
	stepPolygon (p, forwardProgress);
}

void
PolygonAnim::updateBB (CompOutput &output)
{
    GLScreen *gScreen = GLScreen::get (::screen);
    GLMatrix wTransform;
    GLMatrix wTransform2;

    prepareTransform (output, wTransform, wTransform2);

    const float *screenProjection = gScreen->projectionMatrix ();
    GLdouble dModel[16];
    GLdouble dProjection[16];
    for (int i = 0; i < 16; i++)
    {
	dProjection[i] = screenProjection[i];
    }
    GLint viewport[4] =
	{output.region ()->extents.x1,
	 output.region ()->extents.y1,
	 output.width (),
	 output.height ()};
    GLdouble px, py, pz;

    GLMatrix *modelViewTransform = &wTransform;

    GLMatrix skewMat;
    if (mCorrectPerspective == CorrectPerspectiveWindow)
    {
	getPerspectiveCorrectionMat (NULL, NULL, &skewMat, output);
	wTransform2 = wTransform * skewMat;
    }
    if (mCorrectPerspective == CorrectPerspectiveWindow ||
	mCorrectPerspective == CorrectPerspectivePolygon)
	modelViewTransform = &wTransform2;

    foreach (const PolygonObject *p, mPolygons)
    {
	if (mCorrectPerspective == CorrectPerspectivePolygon)
	{
	    getPerspectiveCorrectionMat (p, NULL, &skewMat, output);
	    wTransform2 = wTransform * skewMat;
	}

	// if modelViewTransform == wTransform2, then
	// it changes for each polygon
	const float *modelViewMatrix = modelViewTransform->getMatrix ();
	for (int j = 0; j < 16; j++)
	    dModel[j] = modelViewMatrix[j];

	Point3d center = p->centerPos;
	float radius = p->boundSphereRadius + 2;

	// Take rotation axis offset into consideration and
	// properly enclose polygon in the bounding cube
	// whatever the rotation angle is:

	// Add rotation axis offset to center (rotated) polygon correctly
	// within bounding cube
	center.add (p->rotAxisOffset.x (),
	            p->rotAxisOffset.y (),
	            p->rotAxisOffset.z () / ::screen->width ());

	// Add rotation axis offset to radius to enlarge the bounding cube
	radius += MAX (MAX (fabs (p->rotAxisOffset.x ()),
			    fabs (p->rotAxisOffset.y ())),
		       fabs (p->rotAxisOffset.z ()));

	float zradius = radius / ::screen->width ();

static const unsigned short N_POINTS = 8;
	// Corners of bounding cube
	Point3d cubeCorners[N_POINTS];
	cubeCorners[0].set (center.x () - radius, center.y () - radius,
	                    center.z () + zradius);
	cubeCorners[1].set (center.x () - radius, center.y () + radius,
	                    center.z () + zradius);
	cubeCorners[2].set (center.x () + radius, center.y () - radius,
	                    center.z () + zradius);
	cubeCorners[3].set (center.x () + radius, center.y () + radius,
	                    center.z () + zradius);
	cubeCorners[4].set (center.x () - radius, center.y () - radius,
	                    center.z () - zradius);
	cubeCorners[5].set (center.x () - radius, center.y () + radius,
	                    center.z () - zradius);
	cubeCorners[6].set (center.x () + radius, center.y () - radius,
	                    center.z () - zradius);
	cubeCorners[7].set (center.x () + radius, center.y () + radius,
	                    center.z () - zradius);
	Point3d *pnt = cubeCorners;
	for (int j = 0; j < N_POINTS; j++, pnt++)
	{
	    if (!gluProject (pnt->x (), pnt->y (), pnt->z (),
			     dModel, dProjection, viewport,
			     &px, &py, &pz))
		return;

	    py = ::screen->height () - py;
	    mAWindow->expandBBWithPoint (px + 0.5, py + 0.5);
	}
#undef N_POINTS
    }
}

bool
PolygonAnim::prePreparePaint (int msSinceLastPaint)
{
    mNumClipsPassed = 0;
    mClipsUpdated = false;

    return false;
}

bool
PolygonAnim::moveUpdate (int dx, int dy)
{
    foreach (PolygonObject *p, mPolygons)
    {
	p->centerPosStart.setX (p->centerPosStart.x () + dx);
	p->centerPosStart.setY (p->centerPosStart.y () + dy);

	p->centerPos.setX (p->centerPos.x () + dx);
	p->centerPos.setY (p->centerPos.y () + dy);

	p->finalRelPos.setX (p->finalRelPos.x () + dx);
	p->finalRelPos.setY (p->finalRelPos.y () + dy);
    }

    return true;
}

void
ExtensionPluginAnimAddon::prePaintOutput (CompOutput *output)
{
    CompString pluginName ("animationaddon");

    mOutput = output;

    // Find out if an animation running now uses depth test
    bool depthUsed = false;
    foreach (CompWindow *w, ::screen->windows ())
    {
	AnimWindow *aw = AnimWindow::get (w);
	Animation *anim = aw->curAnimation ();

	if (anim && anim->remainingTime () > 0 &&
	    anim->getExtensionPluginInfo ()->name == pluginName)
	{
	    BaseAddonAnim *animBase = dynamic_cast<BaseAddonAnim *> (anim);
	    if (animBase->needsDepthTest ())
	    {
		depthUsed = true;
		break;
	    }
	}
    }
    if (depthUsed)
    {
	glClearDepth (1000.0f);
	glClear (GL_DEPTH_BUFFER_BIT);
    }
}
