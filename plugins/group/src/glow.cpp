/**
 *
 * Compiz group plugin
 *
 * glow.cpp
 *
 * Copyright : (C) 2006-2010 by Patrick Niklaus, Roi Cohen,
 * 				Danny Baumann, Sam Spilsbury
 * Authors: Patrick Niklaus <patrick.niklaus@googlemail.com>
 *          Roi Cohen       <roico.beryl@gmail.com>
 *          Danny Baumann   <maniac@opencompositing.org>
 * 	    Sam Spilsbury   <smspillaz@gmail.com>
 *
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
 **/

#include "group.h"
#include "group_glow.h"

const unsigned short GLOWQUAD_TOPLEFT	    = 0;
const unsigned short GLOWQUAD_TOPRIGHT	    = 1;
const unsigned short GLOWQUAD_BOTTOMLEFT    = 2;
const unsigned short GLOWQUAD_BOTTOMRIGHT   = 3;
const unsigned short GLOWQUAD_TOP	    = 4;
const unsigned short GLOWQUAD_BOTTOM	    = 5;
const unsigned short GLOWQUAD_LEFT	    = 6;
const unsigned short GLOWQUAD_RIGHT	    = 7;
const unsigned short NUM_GLOWQUADS	    = 8;

const GlowTextureProperties glowTextureProperties[2] = {
    /* GlowTextureRectangular */
    {glowTexRect, 32, 21},
    /* GlowTextureRing */
    {glowTexRing, 32, 16}
};

/*
 * GroupWindow::getOutputExtents
 *
 * Wrappable function, return the extents of how much we expect to be painting
 * on the window including the glow (since the glow goes outside the default
 * clip region, so we need to let core know that the clip region needs to be
 * /slightly/ larger
 *
 */
void
GroupWindow::getOutputExtents (CompWindowExtents &output)
{
    GROUP_SCREEN (screen);

    window->getOutputExtents (output);

    /* Only bother if this window would have glow */
    if (mGroup && mGroup->mWindows.size () > 1)
    {
	int glowSize = gs->optionGetGlowSize ();
	int glowType = gs->optionGetGlowType ();
	int glowTextureSize = gs->mGlowTextureProperties[glowType].textureSize;
	int glowOffset = gs->mGlowTextureProperties[glowType].glowOffset;

	glowSize = glowSize * (glowTextureSize - glowOffset) / glowTextureSize;

	/* glowSize is the size of the glow outside the window decoration
	 * (w->border), while w->output includes the size of w->border
	 * this is why we have to add w->input here */
	output.left   = MAX (output.left, glowSize + window->border ().left);
	output.right  = MAX (output.right, glowSize + window->border ().right);
	output.top    = MAX (output.top, glowSize + window->border ().top);
	output.bottom = MAX (output.bottom, glowSize + window->border ().bottom);
    }
}

/*
 * GroupWindow::paintGlow
 *
 * Takes our glow texture, stretches the appropriate positions in the glow texture,
 * adds those geometries (so plugins like wobby and deform this texture correctly)
 * and then draws the glow texture with this geometry (plugins like wobbly and friends
 * will automatically deform the texture based on our set geometry)
 */

void
GroupWindow::paintGlow (GLFragment::Attrib        &attrib,
			const CompRegion	  &paintRegion,
			unsigned int		  mask)
{
    CompRegion reg;
    int    i;
    
    GROUP_SCREEN (screen);

    /* There are 8 glow parts of the glow texture which we wish to paint
     * separately with different transformations
     */
    for (i = 0; i < NUM_GLOWQUADS; i++)
    {
	/* Using precalculated quads here */
	reg = CompRegion (mGlowQuads[i].mBox);

	if (reg.boundingRect ().x1 () < reg.boundingRect ().x2 () &&
	    reg.boundingRect ().y1 () < reg.boundingRect ().y2 ())
	{
	    GLTexture::MatrixList matl;
	    reg = CompRegion (reg.boundingRect ().x1 (),
			      reg.boundingRect ().y1 (),
			      reg.boundingRect ().width (),
			      reg.boundingRect ().height ());

	    matl.push_back (mGlowQuads[i].mMatrix);
	    gWindow->glAddGeometry (matl, reg, paintRegion);
	}
    }

    /* If the geometry add succeeded */
    if (gWindow->geometry ().vertices)
    {
	GLFragment::Attrib fAttrib (attrib);
	GLushort       average;
	GLushort       color[3] = {mGroup->mColor[0],
				   mGroup->mColor[1],
				   mGroup->mColor[2]};

	/* Apply brightness to color. */
	color[0] *= (float)attrib.getBrightness () / BRIGHT;
	color[1] *= (float)attrib.getBrightness () / BRIGHT;
	color[2] *= (float)attrib.getBrightness () / BRIGHT;

	/* Apply saturation to color. */
	average = (color[0] + color[1] + color[2]) / 3;
	color[0] = average + (color[0] - average) *
		   attrib.getSaturation () / COLOR;
	color[1] = average + (color[1] - average) *
		   attrib.getSaturation () / COLOR;
	color[2] = average + (color[2] - average) *
		   attrib.getSaturation () / COLOR;

	fAttrib.setOpacity (OPAQUE);
	fAttrib.setSaturation (COLOR);
	fAttrib.setBrightness  (BRIGHT);

	gs->gScreen->setTexEnvMode (GL_MODULATE);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4us (color[0], color[1], color[2], attrib.getOpacity ());

	/* we use PAINT_WINDOW_TRANSFORMED_MASK here to force
	   the usage of a good texture filter */
	foreach (GLTexture *tex, gs->mGlowTexture)
	{
	    gWindow->glDrawTexture (tex, fAttrib, mask | 
					PAINT_WINDOW_BLEND_MASK       |
					PAINT_WINDOW_TRANSLUCENT_MASK |
					PAINT_WINDOW_TRANSFORMED_MASK);
	}

	glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	gs->gScreen->setTexEnvMode (GL_REPLACE);
	glColor4usv (defaultColor);
    }
}

/*
 * GroupWindow::computeGlowQuads
 *
 * This function computures the matrix transformation required for each
 * part of the glow texture which we wish to stretch to some rectangular
 * dimentions
 *
 * There are eight quads different parts of the texture which we wish to
 * paint here, the 4 sides and four corners, eg:
 *
 *		     ------------------
 *		     | 1 |   4    | 6 |
 * -------------     ------------------
 * | 1 | 4 | 6 |     |   |        |   |
 * -------------     |   |	  |   |
 * | 2 | n | 7 | ->  | 2 |   n    | 7 |
 * -------------     |   |        |   |
 * | 3 | 5 | 8 |     |   |        |   |
 * -------------     ------------------
 *		     | 3 |   5    | 8 |
 *		     ------------------
 *
 * In this example here, 2, 4, 5 and 7 are stretched, and the matrices for
 * each quad rect adjusted accordingly for it's size compared to the original
 * texture size.
 *
 * When we are adjusting the matrices here, the initial size of each corner has
 * a size of of "1.0f", so according to 2x2 matrix rules,
 * the scale factor is the inverse of the size of the glow (which explains
 * while you will see here that matrix->xx is (1 / glowSize)
 * where glowSize is the size the user specifies they want their glow to extend.
 * (likewise, matrix->yy is adjusted similarly for corners and for top/bottom)
 *
 * matrix->x0 and matrix->y0 here are set to be the top left edge of the rect
 * adjusted by the matrix scale factor (matrix->xx and matrix->yy)
 *
 */
void
GroupWindow::computeGlowQuads (GLTexture::Matrix *matrix)
{
    CompRect	      *box;
    int		      x1, x2, y1, y2;
    GLTexture::Matrix *quadMatrix;
    int               glowSize, glowOffset;
    int		      glowType;
    CompWindow	      *w = window;

    GROUP_SCREEN (screen);

    /* Passing NULL to this function frees the glow quads
     * (so the window is not painted with glow) */

    if (gs->optionGetGlow () && matrix)
    {
	if (!mGlowQuads)
	    mGlowQuads = new GlowQuad[NUM_GLOWQUADS];
	if (!mGlowQuads)
	    return;
    }
    else
    {
	if (mGlowQuads)
	{
	    delete[] mGlowQuads;
	    mGlowQuads = NULL;
	}
	return;
    }

    glowSize = gs->optionGetGlowSize ();
    glowType = gs->optionGetGlowType ();
    glowOffset = (glowSize * gs->mGlowTextureProperties[glowType].glowOffset /
		  gs->mGlowTextureProperties[glowType].textureSize) + 1;

    /* Top left corner */
    box = &mGlowQuads[GLOWQUAD_TOPLEFT].mBox;
    mGlowQuads[GLOWQUAD_TOPLEFT].mMatrix = *matrix;
    quadMatrix = &mGlowQuads[GLOWQUAD_TOPLEFT].mMatrix;

    /* Set the desired rect dimentions
     * for the part of the glow we are painting */

    x1 = WIN_REAL_X (w) - glowSize + glowOffset;
    y1 = WIN_REAL_Y (w) - glowSize + glowOffset;

    /* 2x2 Matrix here, adjust both x and y scale factors
     * and the x and y position
     *
     * Scaling both parts of the texture in a positive direction
     * here (left to right top to bottom)
     *
     * The base position (x0 and y0) here requires us to move backwards
     * on the x and y dimentions by the calculated rect dimentions
     * multiplied by the scale factors
     */

    quadMatrix->xx = 1.0f / glowSize;
    quadMatrix->yy = 1.0f / (glowSize);
    quadMatrix->x0 = -(x1 * quadMatrix->xx);
    quadMatrix->y0 = -(y1 * quadMatrix->yy);

    x2 = MIN (WIN_REAL_X (w) + glowOffset,
	      WIN_REAL_X (w) + (WIN_REAL_WIDTH (w) / 2));
    y2 = MIN (WIN_REAL_Y (w) + glowOffset,
	      WIN_REAL_Y (w) + (WIN_REAL_HEIGHT (w) / 2));

    *box = CompRect (x1, y1, x2 - x1, y2 - y1);

    /* Top right corner */
    box = &mGlowQuads[GLOWQUAD_TOPRIGHT].mBox;
    mGlowQuads[GLOWQUAD_TOPRIGHT].mMatrix = *matrix;
    quadMatrix = &mGlowQuads[GLOWQUAD_TOPRIGHT].mMatrix;

    /* Set the desired rect dimentions
     * for the part of the glow we are painting */

    x1 = WIN_REAL_X (w) + WIN_REAL_WIDTH (w) - glowOffset;
    y1 = WIN_REAL_Y (w) - glowSize + glowOffset;
    x2 = WIN_REAL_X (w) + WIN_REAL_WIDTH (w) + glowSize - glowOffset;

    /* 2x2 Matrix here, adjust both x and y scale factors
     * and the x and y position
     *
     * Scaling the y part of the texture in a positive direction
     * and the x part in a negative direction here
     * (right to left top to bottom)
     *
     * The base position (x0 and y0) here requires us to move backwards
     * on the y dimention and forwards on x by the calculated rect dimentions
     * multiplied by the scale factors (since we are moving forward on x we
     * need the inverse of that which is 1 - x1 * xx
     */

    quadMatrix->xx = -1.0f / glowSize;
    quadMatrix->yy = 1.0f / glowSize;
    quadMatrix->x0 = 1.0 - (x1 * quadMatrix->xx);
    quadMatrix->y0 = -(y1 * quadMatrix->yy);

    x1 = MAX (WIN_REAL_X (w) + WIN_REAL_WIDTH (w) - glowOffset,
	      WIN_REAL_X (w) + (WIN_REAL_WIDTH (w) / 2));
    y2 = MIN (WIN_REAL_Y (w) + glowOffset,
	      WIN_REAL_Y (w) + (WIN_REAL_HEIGHT (w) / 2));

    *box = CompRect (x1, y1, x2 - x1, y2 - y1);

    /* Bottom left corner */
    box = &mGlowQuads[GLOWQUAD_BOTTOMLEFT].mBox;
    mGlowQuads[GLOWQUAD_BOTTOMLEFT].mMatrix = *matrix;
    quadMatrix = &mGlowQuads[GLOWQUAD_BOTTOMLEFT].mMatrix;

    x1 = WIN_REAL_X (w) - glowSize + glowOffset;
    y1 = WIN_REAL_Y (w) + WIN_REAL_HEIGHT (w) - glowOffset;
    x2 = WIN_REAL_X (w) + glowOffset;
    y2 = WIN_REAL_Y (w) + WIN_REAL_HEIGHT (w) + glowSize - glowOffset;

    /* 2x2 Matrix here, adjust both x and y scale factors
     * and the x and y position
     *
     * Scaling the x part of the texture in a positive direction
     * and the y part in a negative direction here
     * (left to right bottom to top)
     *
     * The base position (x0 and y0) here requires us to move backwards
     * on the x dimention and forwards on y by the calculated rect dimentions
     * multiplied by the scale factors (since we are moving forward on x we
     * need the inverse of that which is 1 - y1 * yy
     */

    quadMatrix->xx = 1.0f / glowSize;
    quadMatrix->yy = -1.0f / glowSize;
    quadMatrix->x0 = -(x1 * quadMatrix->xx);
    quadMatrix->y0 = 1.0f - (y1 * quadMatrix->yy);

    y1 = MAX (WIN_REAL_Y (w) + WIN_REAL_HEIGHT (w) - glowOffset,
	      WIN_REAL_Y (w) + (WIN_REAL_HEIGHT (w) / 2));
    x2 = MIN (WIN_REAL_X (w) + glowOffset,
	      WIN_REAL_X (w) + (WIN_REAL_WIDTH (w) / 2));

    *box = CompRect (x1, y1, x2 - x1, y2 - y1);

    /* Bottom right corner */
    box = &mGlowQuads[GLOWQUAD_BOTTOMRIGHT].mBox;
    mGlowQuads[GLOWQUAD_BOTTOMRIGHT].mMatrix = *matrix;
    quadMatrix = &mGlowQuads[GLOWQUAD_BOTTOMRIGHT].mMatrix;

    x1 = WIN_REAL_X (w) + WIN_REAL_WIDTH (w) - glowOffset;
    y1 = WIN_REAL_Y (w) + WIN_REAL_HEIGHT (w) - glowOffset;
    x2 = WIN_REAL_X (w) + WIN_REAL_WIDTH (w) + glowSize - glowOffset;
    y2 = WIN_REAL_Y (w) + WIN_REAL_HEIGHT (w) + glowSize - glowOffset;

    /* 2x2 Matrix here, adjust both x and y scale factors
     * and the x and y position
     *
     * Scaling the both parts of the texture in a negative direction
     * (right to left bottom to top)
     *
     * The base position (x0 and y0) here requires us to move forwards
     * on both dimentions by the calculated rect dimentions
     * multiplied by the scale factors
     */

    quadMatrix->xx = -1.0f / glowSize;
    quadMatrix->yy = -1.0f / glowSize;
    quadMatrix->x0 = 1.0 - (x1 * quadMatrix->xx);
    quadMatrix->y0 = 1.0 - (y1 * quadMatrix->yy);

    x1 = MAX (WIN_REAL_X (w) + WIN_REAL_WIDTH (w) - glowOffset,
	      WIN_REAL_X (w) + (WIN_REAL_WIDTH (w) / 2));
    y1 = MAX (WIN_REAL_Y (w) + WIN_REAL_HEIGHT (w) - glowOffset,
	      WIN_REAL_Y (w) + (WIN_REAL_HEIGHT (w) / 2));

    *box = CompRect (x1, y1, x2 - x1, y2 - y1);

    /* Top edge */
    box = &mGlowQuads[GLOWQUAD_TOP].mBox;
    mGlowQuads[GLOWQUAD_TOP].mMatrix = *matrix;
    quadMatrix = &mGlowQuads[GLOWQUAD_TOP].mMatrix;

    x1 = WIN_REAL_X (w) + glowOffset;
    y1 = WIN_REAL_Y (w) - glowSize + glowOffset;
    x2 = WIN_REAL_X (w) + WIN_REAL_WIDTH (w) - glowOffset;
    y2 = WIN_REAL_Y (w) + glowOffset;

    /* 2x2 Matrix here, adjust both x and y scale factors
     * and the x and y position
     *
     * No need to scale the x part of the texture here, but we
     * are scaling on the y part in a positive direciton
     *
     * The base position (y0) here requires us to move backwards
     * on the x dimention and forwards on y by the calculated rect dimentions
     * multiplied by the scale factors
     */

    quadMatrix->xx = 0.0f;
    quadMatrix->yy = 1.0f / glowSize;
    quadMatrix->x0 = 1.0;
    quadMatrix->y0 = -(y1 * quadMatrix->yy);

    *box = CompRect (x1, y1, x2 - x1, y2 - y1);

    /* Bottom edge */
    box = &mGlowQuads[GLOWQUAD_BOTTOM].mBox;
    mGlowQuads[GLOWQUAD_BOTTOM].mMatrix = *matrix;
    quadMatrix = &mGlowQuads[GLOWQUAD_BOTTOM].mMatrix;

    x1 = WIN_REAL_X (w) + glowOffset;
    y1 = WIN_REAL_Y (w) + WIN_REAL_HEIGHT (w) - glowOffset;
    x2 = WIN_REAL_X (w) + WIN_REAL_WIDTH (w) - glowOffset;
    y2 = WIN_REAL_Y (w) + WIN_REAL_HEIGHT (w) + glowSize - glowOffset;

    /* 2x2 Matrix here, adjust both x and y scale factors
     * and the x and y position
     *
     * No need to scale the x part of the texture here, but we
     * are scaling on the y part in a negative direciton
     *
     * The base position (y0) here requires us to move forwards
     * on y by the calculated rect dimentions
     * multiplied by the scale factors
     */

    quadMatrix->xx = 0.0f;
    quadMatrix->yy = -1.0f / glowSize;
    quadMatrix->x0 = 1.0;
    quadMatrix->y0 = 1.0 - (y1 * quadMatrix->yy);

    *box = CompRect (x1, y1, x2 - x1, y2 - y1);

    /* Left edge */
    box = &mGlowQuads[GLOWQUAD_LEFT].mBox;
    mGlowQuads[GLOWQUAD_LEFT].mMatrix = *matrix;
    quadMatrix = &mGlowQuads[GLOWQUAD_LEFT].mMatrix;

    x1 = WIN_REAL_X (w) - glowSize + glowOffset;
    y1 = WIN_REAL_Y (w) + glowOffset;
    x2 = WIN_REAL_X (w) + glowOffset;
    y2 = WIN_REAL_Y (w) + WIN_REAL_HEIGHT (w) - glowOffset;

    /* 2x2 Matrix here, adjust both x and y scale factors
     * and the x and y position
     *
     * No need to scale the y part of the texture here, but we
     * are scaling on the x part in a positive direciton
     *
     * The base position (x0) here requires us to move backwards
     * on x by the calculated rect dimentions
     * multiplied by the scale factors
     */

    quadMatrix->xx = 1.0f / glowSize;
    quadMatrix->yy = 0.0f;
    quadMatrix->x0 = -(x1 * quadMatrix->xx);
    quadMatrix->y0 = 1.0;

    *box = CompRect (x1, y1, x2 - x1, y2 - y1);

    /* Right edge */
    box = &mGlowQuads[GLOWQUAD_RIGHT].mBox;
    mGlowQuads[GLOWQUAD_RIGHT].mMatrix = *matrix;
    quadMatrix = &mGlowQuads[GLOWQUAD_RIGHT].mMatrix;

    x1 = WIN_REAL_X (w) + WIN_REAL_WIDTH (w) - glowOffset;
    y1 = WIN_REAL_Y (w) + glowOffset;
    x2 = WIN_REAL_X (w) + WIN_REAL_WIDTH (w) + glowSize - glowOffset;
    y2 = WIN_REAL_Y (w) + WIN_REAL_HEIGHT (w) - glowOffset;

    /* 2x2 Matrix here, adjust both x and y scale factors
     * and the x and y position
     *
     * No need to scale the y part of the texture here, but we
     * are scaling on the x part in a negative direciton
     *
     * The base position (x0) here requires us to move forwards
     * on x by the calculated rect dimentions
     * multiplied by the scale factors
     */

    quadMatrix->xx = -1.0f / glowSize;
    quadMatrix->yy = 0.0f;
    quadMatrix->x0 = 1.0 - (x1 * quadMatrix->xx);
    quadMatrix->y0 = 1.0;

    *box = CompRect (x1, y1, x2 - x1, y2 - y1);
}
