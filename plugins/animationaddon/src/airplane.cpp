/*
 * Animation plugin for compiz/beryl
 *
 * airplane3d.c
 *
 * Copyright : (C) 2006 Erkin Bahceci
 * E-mail    : erkinbah@gmail.com
 *
 * Based on Wobbly and Minimize plugins by
 *           : David Reveman
 * E-mail    : davidr@novell.com>
 *
 * Airplane added by : Carlo Palma
 * E-mail            : carlopalma@salug.it
 * Based on code originally written by Mark J. Kilgard
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

#include "private.h"

#define BORDER_W(w) ((w)->width () + (w)->border ().left + (w)->border ().right)
#define BORDER_H(w) ((w)->height () + (w)->border ().top + (w)->border ().bottom)
#define BORDER_X(w) ((w)->x () - (w)->border ().left)
#define BORDER_Y(w) ((w)->y () - (w)->border ().top)

// Divide the window in 8 polygons (6 quadrilaters and 2 triangles (all of them draw as quadrilaters))
// Based on tessellateIntoRectangles and tessellateIntoHexagons. Improperly called tessellation.

const float AirplaneAnim::kDurationFactor = 1.82;

AirplaneAnim::AirplaneAnim (CompWindow *w,
		    WindowEvent curWindowEvent,
		    float duration,
		    const AnimEffect info,
		    const CompRect &icon) :
    Animation::Animation (w, curWindowEvent, kDurationFactor * duration, info,
			  icon),
    PolygonAnim::PolygonAnim (w, curWindowEvent, kDurationFactor * duration,
			      info, icon)
{
}

AirplaneAnim::~AirplaneAnim ()
{
    freePolygonObjects ();
}


bool
AirplaneAnim::tesselateIntoAirplane ()
{
    float winLimitsX;		// boundaries of polygon tessellation
    float winLimitsY;
    float winLimitsW;
    float winLimitsH;

    winLimitsX = BORDER_X (mWindow);
    winLimitsY = BORDER_Y (mWindow);
    winLimitsW = BORDER_W (mWindow);
    winLimitsH = BORDER_H (mWindow);

    unsigned int numpol = 8;
    if (mPolygons.size () != numpol)
    {
	freePolygonObjects ();

	for (unsigned int i = 0; i < numpol; i++)
	    mPolygons.push_back (new AirplanePolygonObject);
    }

    float thickness = 0;
    thickness /= screen->width ();
    mThickness = thickness;
    mNumTotalFrontVertices = 0;

    float W = (float)winLimitsW;
    float H2 = (float)winLimitsH / 2;
    float H3 = (float)winLimitsH / 3;
    float H6 = (float)winLimitsH / 6;
    float halfThick = mThickness / 2;

    /**
     *
     * These correspond to the polygons:
     * based on GLUT sample origami.c code by Mark J. Kilgard
     *                  
     *       |-               W              -| 
     *       |-    H2     -|
     *
     * - --  +----+--------+------------------+
     * |     |    |       /                   |
     *       |    | 6   /                     | 
     *       | 7  |   /              5        |
     *   H2  |    | +                         |
     *       |    +--------+------------------+
     *       |  /                 4           |
     * H __  |/____________.__________________|
     *       |\          center               |
     *       |  \                 3           |
     *       |    +--------+------------------+
     *       |    | +                         |
     *       | 0  |   \                       |
     *       |    |  1  \            2        |  
     * |     |    |       \                   |
     * -     +----+--------+------------------+
     *
     *
     */

    int i = 0;

    foreach (PolygonObject *pol, mPolygons)
    {
	AirplanePolygonObject *p = (AirplanePolygonObject *) pol;
    
	float topRightY, topLeftY, bottomLeftY, bottomRightY;
	float topLeftX, topRightX, bottomLeftX, bottomRightX;

	p->centerPos.setX (winLimitsX + H2);
	p->centerPosStart.setX (winLimitsX + H2);
	p->centerPos.setY (winLimitsY + H2);
	p->centerPosStart.setY (winLimitsY + H2);
	p->centerPos.setZ (-halfThick);
	p->centerPosStart.setZ (-halfThick);
	p->rotAngle = p->rotAngleStart = 0;

	p->nSides = 4;
	p->nVertices = 2 * 4;
	mNumTotalFrontVertices += 4;

	switch (i)
	{
	case 0:
	    topLeftX = -H2;
	    topLeftY = 0;
	    bottomLeftX = -H2;
	    bottomLeftY = H2;
	    bottomRightX = -H3;
	    bottomRightY = H2;
	    topRightX = -H3;
	    topRightY = H6;
	    break;
	case 1:
	    topLeftX = -H3;
	    topLeftY = H6;
	    bottomLeftX = -H3;
	    bottomLeftY = H2;
	    bottomRightX = 0;
	    bottomRightY = H2;
	    topRightX = 0;
	    topRightY = H2;
	    break;
	case 2:
	    topLeftX = -H3;
	    topLeftY = H6;
	    bottomLeftX = 0;
	    bottomLeftY = H2;
	    bottomRightX = W - H2;
	    bottomRightY = H2;
	    topRightX = W - H2;
	    topRightY = H6;
	    break;
	case 3:
	    topLeftX = -H2;
	    topLeftY = 0;
	    bottomLeftX = -H3;
	    bottomLeftY = H6;
	    bottomRightX = W - H2;
	    bottomRightY = H6;
	    topRightX = W - H2;
	    topRightY = 0;
	    break;
	case 4:
	    topLeftX = -H3;
	    topLeftY = -H6;
	    bottomLeftX = -H2;
	    bottomLeftY = 0;
	    bottomRightX = W - H2;
	    bottomRightY = 0;
	    topRightX = W - H2;
	    topRightY = -H6;
	    break;
	case 5:
	    topLeftX = 0;
	    topLeftY = -H2;
	    bottomLeftX = -H3;
	    bottomLeftY = -H6;
	    bottomRightX = W - H2;
	    bottomRightY = -H6;
	    topRightX = W - H2;
	    topRightY = -H2;
	    break;
	case 6:
	    topLeftX = -H3;
	    topLeftY = -H2;
	    bottomLeftX = -H3;
	    bottomLeftY = -H6;
	    bottomRightX = -H3;
	    bottomRightY = -H6;
	    topRightX = 0;
	    topRightY = -H2;
	    break;
	default:
	    topLeftX = -H2;
	    topLeftY = -H2;
	    bottomLeftX = -H2;
	    bottomLeftY = 0;
	    bottomRightX = -H3;
	    bottomRightY = -H6;
	    topRightX = -H3;
	    topRightY = -H2;
	    break;
	}

	// 4 front, 4 back vertices
	p->vertices = (GLfloat *) calloc (8 * 3, sizeof (GLfloat));
	if (!p->vertices)
	{
	    compLogMessage ("animation", CompLogLevelError,
			    "Not enough memory");
	    freePolygonObjects ();
	    return false;
	}

	GLfloat *pv = p->vertices;

	// Determine 4 front vertices in ccw direction
	pv[0] = topLeftX;
	pv[1] = topLeftY;
	pv[2] = halfThick;

	pv[3] = bottomLeftX;
	pv[4] = bottomLeftY;
	pv[5] = halfThick;

	pv[6] = bottomRightX;
	pv[7] = bottomRightY;
	pv[8] = halfThick;

	pv[9] = topRightX;
	pv[10] = topRightY;
	pv[11] = halfThick;

	// Determine 4 back vertices in cw direction
	pv[12] = topRightX;
	pv[13] = topRightY;
	pv[14] = -halfThick;

	pv[15] = bottomRightX;
	pv[16] = bottomRightY;
	pv[17] = -halfThick;

	pv[18] = bottomLeftX;
	pv[19] = bottomLeftY;
	pv[20] = -halfThick;

	pv[21] = topLeftX;
	pv[22] = topLeftY;
	pv[23] = -halfThick;

	// 16 indices for 4 sides (for quad strip)
	p->sideIndices = (GLushort *) calloc (4 * 4, sizeof (GLushort));
	if (!p->sideIndices)
	{
	    compLogMessage ("animation", CompLogLevelError,
			    "Not enough memory");
	    freePolygonObjects ();
	    return false;
	}

	GLushort *ind = p->sideIndices;
	int id = 0;

	ind[id++] = 0;
	ind[id++] = 7;
	ind[id++] = 6;
	ind[id++] = 1;

	ind[id++] = 1;
	ind[id++] = 6;
	ind[id++] = 5;
	ind[id++] = 2;

	ind[id++] = 2;
	ind[id++] = 5;
	ind[id++] = 4;
	ind[id++] = 3;

	ind[id++] = 3;
	ind[id++] = 4;
	ind[id++] = 7;
	ind[id++] = 0;

	if (i < 4)
	{
	    p->boundingBox.x1 = p->centerPos.x () + topLeftX;
	    p->boundingBox.y1 = p->centerPos.y () + topLeftY;
	    p->boundingBox.x2 = ceil (p->centerPos.x () + bottomRightX);
	    p->boundingBox.y2 = ceil (p->centerPos.y () + bottomRightY);
	}
	else
	{
	    p->boundingBox.x1 = p->centerPos.x () + bottomLeftX;
	    p->boundingBox.y1 = p->centerPos.y () + topLeftY;
	    p->boundingBox.x2 = ceil (p->centerPos.x () + bottomRightX);
	    p->boundingBox.y2 = ceil (p->centerPos.y () + bottomLeftY);
	}
	
	i++;
    }
    return true;
}

void
AirplaneAnim::init ()
{
    if (!tesselateIntoAirplane ())
	return;

    float airplanePathLength =
	optValF (AnimationaddonOptions::AirplanePathLength);

    float winLimitsW;		// boundaries of polygon tessellation
    float winLimitsH;

    winLimitsW = BORDER_W (mWindow);
    winLimitsH = BORDER_H (mWindow);

    float H4 = (float)winLimitsH / 4;
    float H6 = (float)winLimitsH / 6;

    int i = 0;
    foreach (PolygonObject *pol, mPolygons)
    {
	AirplanePolygonObject *p = (AirplanePolygonObject *) pol;
    
	p->moveStartTime = 0.00;
	p->moveDuration = 0.19;

	p->moveStartTime2 = 0.19;
	p->moveDuration2 = 0.19;

	p->moveStartTime3 = 0.38;
	p->moveDuration3 = 0.19;

	p->moveStartTime4 = 0.58;
	p->moveDuration4 = 0.09;

	p->moveDuration5 = 0.41;

	p->flyFinalRotation.set (90, 10, 0);

	p->flyTheta = 0;
	
	p->centerPosFly.set (0, 0, 0);

	p->flyScale = 0;
	p->flyFinalScale = 6 * (winLimitsW / (screen->width () / 2));

	switch (i)
	{
	case 0:
	    p->rotAxisOffset.set (-H4, H4, 0.0f); 	    
	    p->rotAxis.set (1.00, 1.00, 0.00);
	    p->finalRotAng = 179.5;

	    p->rotAxisOffsetA.set (0, 0, 0);	    
	    p->rotAxisA.set (1.00, 0.00, 0.00);
	    p->finalRotAngA = 84;
	    
	    p->rotAxisOffsetB.set (0, 0, 0);	    
	    p->rotAxisB.set (0.00, 0.00, 0.00);
	    p->finalRotAngB = 0;
	    break;

	case 1:

	    p->rotAxisOffset.set (-H4, H4, 0.0f);
	    p->rotAxis.set (1.00, 1.00, 0.00);
	    p->finalRotAng = 179.5;
	    
	    p->rotAxisOffsetA.set (0, 0, 0);
	    p->rotAxisA.set (1.00, 0.00, 0.00);
	    p->finalRotAngA = 84;
	    
	    p->rotAxisOffsetB.set (0, H6, 0);
	    p->rotAxisB.set (1.00, 0.00, 0.00);
	    p->finalRotAngB = -84;
	    break;

	case 2:
	    p->moveDuration = 0.00;
	    
	    p->rotAxisOffset.set (0, 0, 0);
	    p->rotAxis.set (0, 0, 0);
	    p->finalRotAng = 0;

	    p->rotAxisOffsetA.set (0, 0, 0);
	    p->rotAxisA.set (1.00, 0, 0);
	    p->finalRotAngA = 84;

	    p->rotAxisOffsetB.set (0, H6, 0);
	    p->rotAxisB.set (1.00, 0, 0);
	    
	    p->finalRotAngB = -84;

	    break;

	case 3:
	    p->moveDuration = 0.00;
	    
	    p->rotAxisOffset.set (0, 0, 0);
	    p->rotAxis.set (0, 0, 0);
	    p->finalRotAng = 0;
	    
	    p->rotAxisOffsetA.set (0, 0, 0);
	    p->rotAxisA.set (1.00, 0, 0);
	    p->finalRotAngA = 84;

	    p->moveDuration3 = 0.00;
	    
	    p->rotAxisOffsetB.set (0, 0, 0);
	    p->rotAxisB.set (0, 0, 0);
	    p->finalRotAngB = 0;

	    break;

	case 4:
	    p->moveDuration = 0.00;
	    
	    p->rotAxisOffset.set (0, 0, 0);
	    p->rotAxis.set (0, 0, 0);
	    p->finalRotAng = 0;
	    
	    p->rotAxisOffsetA.set (0, 0, 0);
	    p->rotAxisA.set (1.00, 0, 0);
	    p->finalRotAngA = -84;

	    p->moveDuration3 = 0.00;
	    
	    p->rotAxisOffsetB.set (0, 0, 0);
	    p->rotAxisB.set (0, 0, 0);
	    p->finalRotAngB = 0;
	    
	    break;

	case 5:
	    p->moveDuration = 0.00;
	    
	    p->rotAxisOffset.set (0, 0, 0);
	    p->rotAxis.set (0, 0, 0);
	    p->finalRotAng = 0;
	    
	    p->rotAxisOffsetA.set (0, 0, 0);
	    p->rotAxisA.set (1.00, 0, 0);
	    p->finalRotAngA = -84;
	    
	    p->rotAxisOffsetB.set (0, -H6, 0);
	    p->rotAxisB.set (1.00, 0, 0);
	    p->finalRotAngB = 84;

	    break;

	case 6:
	    p->rotAxisOffset.set (-H4, -H4, 0);
	    p->rotAxis.set (1.00, -1.00, 0.00);
	    p->finalRotAng = -179.5;
	    
	    p->rotAxisOffsetA.set (0, 0, 0);
	    p->rotAxisA.set (1.00, 0.00, 0.00);
	    p->finalRotAngA = -84;

	    p->rotAxisOffsetB.set (0, -H6, 0);
	    p->rotAxisB.set (1.00, 0, 0);
	    p->finalRotAngB = 84;

	    break;

	case 7:
	    p->rotAxisOffset.set (-H4, -H4, 0);
	    p->rotAxis.set (1.00, -1.00, 0.00);
	    p->finalRotAng = -179.5;

	    p->rotAxisOffsetA.set (0, 0, 0);
	    p->rotAxisA.set (1, 0, 0);
	    p->finalRotAngA = -84;
	    
	    p->rotAxisOffsetB.set (0, 0, 0);
	    p->rotAxisB.set (0.00, 0.00, 0.00);

	    p->finalRotAngB = 0;
	    break;
	}
	
	i++;
    }

    if (airplanePathLength >= 1)
	mAllFadeDuration = 0.30f / airplanePathLength;
    else
	mAllFadeDuration = 0.30f;

    mDoDepthTest = true;
    mDoLighting = true;
    mCorrectPerspective = CorrectPerspectivePolygon;
    mBackAndSidesFadeDur = 0;
/*
    mExtraPolygonTransformFunc =
	&AirplaneExtraPolygonTransformFunc;
*/
    // Duration extension
    mTotalTime *= 2 + airplanePathLength;
    mRemainingTime = mTotalTime;
}

void
AirplaneAnim::stepPolygon (PolygonObject *pol,
			   float forwardProgress)
{
    AirplanePolygonObject *p = (AirplanePolygonObject *) pol;
    
    /* A stupid hack */
    if (pol == mPolygons.front ())
    {
    	short x, y;
	// Make sure the airplane always flies towards mouse pointer
	if (mCurWindowEvent == WindowEventClose)
	    AnimScreen::get (screen)->getMousePointerXY (&x, &y);

	mIcon.setX (x); mIcon.setY (y);
    }
    
    float airplanePathLength =
	optValF (AnimationaddonOptions::AirplanePathLength);
    bool airplaneFly2TaskBar =
	optValB (AnimationaddonOptions::AirplaneFlyToTaskbar);

    /*  Phase1: folding: flaps, folding center, folding wings.
     *  Phase2: rotate and fly.
     */

    if (forwardProgress > p->moveStartTime &&
	forwardProgress < p->moveStartTime4)
	// Phase1: folding: flaps, center, wings.
    {
	float moveProgress1 = forwardProgress - p->moveStartTime;
	if (p->moveDuration > 0)
	    moveProgress1 /= p->moveDuration;
	else
	    moveProgress1 = 0;
	if (moveProgress1 < 0)
	    moveProgress1 = 0;
	else if (moveProgress1 > 1)
	    moveProgress1 = 1;

	float moveProgress2 = forwardProgress - p->moveStartTime2;
	if (p->moveDuration2 > 0)
	    moveProgress2 /= p->moveDuration2;
	else
	    moveProgress2 = 0;
	if (moveProgress2 < 0)
	    moveProgress2 = 0;
	else if (moveProgress2 > 1)
	    moveProgress2 = 1;

	float moveProgress3 = forwardProgress - p->moveStartTime3;
	if (p->moveDuration3 > 0)
	    moveProgress3 /= p->moveDuration3;
	else
	    moveProgress3 = 0;
	if (moveProgress3 < 0)
	    moveProgress3 = 0;
	else if (moveProgress3 > 1)
	    moveProgress3 = 1;

	p->centerPos = p->centerPosStart;

	p->rotAngle = moveProgress1 * p->finalRotAng;
	p->rotAngleA = moveProgress2 * p->finalRotAngA;
	p->rotAngleB = moveProgress3 * p->finalRotAngB;

	p->flyRotation.set (0, 0, 0);
	p->flyScale = 0;
    }
    else if (forwardProgress >= p->moveStartTime4)
	// Phase2: rotate and fly 
    {
	float moveProgress4 = forwardProgress - p->moveStartTime4;
	if (p->moveDuration4 > 0)
	    moveProgress4 /= p->moveDuration4;
	if (moveProgress4 < 0)
	    moveProgress4 = 0;
	else if (moveProgress4 > 1)
	    moveProgress4 = 1;

	float moveProgress5 = forwardProgress - (p->moveStartTime4 + .01);
	if (p->moveDuration5 > 0)
	    moveProgress5 /= p->moveDuration5;
	if (moveProgress5 < 0)
	    moveProgress5 = 0;
	else if (moveProgress5 > 1)
	    moveProgress5 = 1;


	p->rotAngle = p->finalRotAng;
	p->rotAngleA = p->finalRotAngA;
	p->rotAngleB = p->finalRotAngB;

	p->flyRotation.set (moveProgress4 * p->flyFinalRotation.x (),
			    moveProgress4 * p->flyFinalRotation.y (), 0);

	// flying path

	float icondiffx = 0;
	p->flyTheta = moveProgress5 * -M_PI_2 * airplanePathLength;
	p->centerPosFly.setX (screen->width () * .4 * sin (2 * p->flyTheta));

	if (((mCurWindowEvent == WindowEventMinimize ||
	      mCurWindowEvent == WindowEventUnminimize) &&
	     airplaneFly2TaskBar) ||
	    mCurWindowEvent == WindowEventOpen ||
	    mCurWindowEvent == WindowEventClose)
	{
	    // flying path ends at icon/pointer

	    int sign = 1;
	    if (mCurWindowEvent == WindowEventUnminimize ||
		mCurWindowEvent == WindowEventOpen)
		sign = -1;

	    icondiffx =
		(((mIcon.x () + mIcon.width () / 2)
		  - (p->centerPosStart.x () +
		     sign * screen->width () * .4 *
		     sin (2 * -M_PI_2 * airplanePathLength))) *
		 moveProgress5);
	    p->centerPosFly.setY (
		(((int) mIcon.x () + (int) mIcon.height () / 2) -
		 p->centerPosStart.y ()) *
		-sin (p->flyTheta / airplanePathLength));
	}
	else
	{
	    if (p->centerPosStart.y () < screen->height () * .33 ||
		p->centerPosStart.y () > screen->height () * .66)
		p->centerPosFly.setY (
		    screen->height () * .6 * sin (p->flyTheta / 3.4));
	    else
		p->centerPosFly.setY (
		    screen->height () * .4 * sin (p->flyTheta / 3.4));
	    if (p->centerPosStart.y () < screen->height () * .33)
		p->centerPosFly.setY (p->centerPosFly.y () * -1);
	}

	p->flyFinalRotation.setZ (
	    ((atan (2.0) + M_PI_2) * sin (p->flyTheta) - M_PI_2) * 180 / M_PI);
	p->flyFinalRotation.add (0, 0, 90);


	if (mCurWindowEvent == WindowEventMinimize ||
	    mCurWindowEvent == WindowEventClose)
	{
	    p->flyFinalRotation.setZ (p->flyFinalRotation.z () * -1);
	}
	else if (mCurWindowEvent == WindowEventUnminimize ||
		 mCurWindowEvent == WindowEventOpen)
	{
	    p->centerPosFly.setX (p->centerPosFly.x () * -1);
	}

	p->flyRotation.setZ (p->flyFinalRotation.z ());

	p->centerPos.setX (p->centerPosStart.x () + p->centerPosFly.x () + icondiffx);
	p->centerPos.setY (p->centerPosStart.y () + p->centerPosFly.y ());
	p->centerPos.setZ (p->centerPosStart.z () + p->centerPosFly.z ());

	p->flyScale = moveProgress5 * p->flyFinalScale;
    }
}

void
AirplaneAnim::transformPolygon (const PolygonObject *pol)
{
    AirplanePolygonObject *p = (AirplanePolygonObject *) pol;

    glRotatef (p->flyRotation.x (), 1, 0, 0);	//rotate on axis X
    glRotatef (-p->flyRotation.y (), 0, 1, 0);	// rotate on axis Y
    glRotatef (p->flyRotation.z (), 0, 0, 1);	// rotate on axis Z

    glScalef (1.0 / (1.0 + p->flyScale),
	      1.0 / (1.0 + p->flyScale), 1.0 / (1.0 + p->flyScale));

    // Move by "rotation axis offset A"
    glTranslatef (p->rotAxisOffsetA.x (), p->rotAxisOffsetA.y (),
		  p->rotAxisOffsetA.z ());

    // Rotate by desired angle A
    glRotatef (p->rotAngleA, p->rotAxisA.x (), p->rotAxisA.y (),
	       p->rotAxisA.z ());

    // Move back to center from  A
    glTranslatef (-p->rotAxisOffsetA.x (), -p->rotAxisOffsetA.y (),
		  -p->rotAxisOffsetA.z ());


    // Move by "rotation axis offset B"
    glTranslatef (p->rotAxisOffsetB.x (), p->rotAxisOffsetB.y (),
		  p->rotAxisOffsetB.z ());

    // Rotate by desired angle B
    glRotatef (p->rotAngleB, p->rotAxisB.x (), p->rotAxisB.y (),
	       p->rotAxisB.z ());

    // Move back to center from B
    glTranslatef (-p->rotAxisOffsetB.x (), -p->rotAxisOffsetB.y (),
		  -p->rotAxisOffsetB.z ());
}


/* TODO: Damage a region, not the whole screen */
void
AirplaneAnim::updateBB (CompOutput &)
{
    Box screenBox = {0, static_cast <short int> (screen->width ()), 0, static_cast <short int> (screen->height ()) };
    
    mAWindow->expandBBWithBox (screenBox);
}

void
AirplaneAnim::freePolygonObjects ()
{
    while (!mPolygons.empty ())
    {
	AirplanePolygonObject *p = (AirplanePolygonObject *) mPolygons.back ();
	
	if (p->nVertices > 0)
	{
	    if (p->vertices)
	    {
		free (p->vertices);
		p->vertices = NULL;
	    }
	    if (p->sideIndices)
	    {
		free (p->sideIndices);
		p->sideIndices = NULL;
	    }
	}
	    
	delete p;
	
	mPolygons.pop_back ();
    }
    
    mPolygons.clear ();
}
