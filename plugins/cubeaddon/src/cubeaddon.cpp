/*
 * Compiz cube reflection and cylinder deformation plugin
 *
 * cubeaddon.cpp
 *
 * Copyright : (C) 2009 by Dennis Kasprzyk
 * E-mail    : onestone@opencompositing.org
 *
 * includes code from cubecaps.c
 *
 * Copyright : (C) 2007 Guillaume Seguin
 * E-mail    : guillaume@segu.in
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
 */

#include "cubeaddon.h"

COMPIZ_PLUGIN_20090315 (cubeaddon, CubeaddonPluginVTable);

const unsigned short CUBEADDON_GRID_SIZE = 100;
const unsigned short CAP_ELEMENTS    = 15;
const unsigned int   CAP_NVERTEX	    = (((CAP_ELEMENTS * (CAP_ELEMENTS + 1)) + 2) * 3);
const unsigned int   CAP_NIDX	    = (CAP_ELEMENTS * (CAP_ELEMENTS - 1) * 4);

const unsigned int   CAP_NIMGVERTEX  = (((CAP_ELEMENTS + 1) * (CAP_ELEMENTS + 1)) * 5);
const unsigned int   CAP_NIMGIDX	    = (CAP_ELEMENTS * CAP_ELEMENTS * 4);

const float RAD2I1024 = 162.9746617f;

/*
 * Initiate a CubeCap
 */
CubeaddonScreen::CubeCap::CubeCap ()
{
    mCurrent    = 0;
    mLoaded     = false;
}

/*
 * Attempt to load current cap image (if any)
 */
void
CubeaddonScreen::CubeCap::load (bool scale, bool aspect, bool clamp)
{
    CompSize tSize;
    float    xScale, yScale;

    CUBE_SCREEN (screen);

    mTexture.clear ();

    mLoaded = false;

    if (mFiles.empty ())
	return;

    mCurrent = mCurrent % mFiles.size ();


    CompString imgName = mFiles[mCurrent].s ();
    CompString pname = "cubeaddon";
    mTexture = GLTexture::readImageToTexture (imgName, pname, tSize);

    if (mTexture.empty ())
    {
	compLogMessage ("cubeaddon", CompLogLevelWarn,
			"Failed to load slide: %s",
			mFiles[mCurrent].s ().c_str ());
	return;
    }

    mLoaded = true;
    mTexMat.reset ();

    mTexMat[0] = mTexture[0]->matrix ().xx;
    mTexMat[1] = mTexture[0]->matrix ().yx;
    mTexMat[4] = mTexture[0]->matrix ().xy;
    mTexMat[5] = mTexture[0]->matrix ().yy;
    mTexMat[12] = mTexture[0]->matrix ().x0;
    mTexMat[13] = mTexture[0]->matrix ().y0;

    if (aspect)
    {
	if (scale)
	    xScale = yScale = MIN (tSize.width (), tSize.height ());
	else
	    xScale = yScale = MAX (tSize.width (), tSize.height ());
    }
    else
    {
	xScale = tSize.width ();
	yScale = tSize.height ();
    }

    mTexMat.translate (tSize.width () / 2.0, tSize.height () / 2.0, 0.0);
    mTexMat.scale (xScale / 2.0, yScale / 2.0, 1.0);

    if (scale)
	xScale = 1.0 / sqrtf (((cs->distance () * cs->distance ()) + 0.25));
    else
	xScale = 1.0 / sqrtf (((cs->distance () * cs->distance ()) + 0.25) * 0.5);

    mTexMat.scale (xScale, xScale, 1.0);

    mTexture[0]->enable (GLTexture::Good);

    if (clamp)
    {
	if (GL::textureBorderClamp)
	{
	    glTexParameteri (mTexture[0]->target (), GL_TEXTURE_WRAP_S,
			     GL_CLAMP_TO_BORDER);
	    glTexParameteri (mTexture[0]->target (), GL_TEXTURE_WRAP_T,
			     GL_CLAMP_TO_BORDER);
	}
	else
	{
	    glTexParameteri (mTexture[0]->target (), GL_TEXTURE_WRAP_S,
			     GL_CLAMP_TO_EDGE);
	    glTexParameteri (mTexture[0]->target (), GL_TEXTURE_WRAP_T,
			     GL_CLAMP_TO_EDGE);
	}
    }
    else
    {
	glTexParameteri (mTexture[0]->target (), GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (mTexture[0]->target (), GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    mTexture[0]->disable ();
}

/* Settings handling -------------------------------------------------------- */

/*
 * Switch cap, load it and damage screen if possible
 */
bool
CubeaddonScreen::changeCap (bool top, int change)
{
    CubeCap *cap = (top)? &mTopCap : &mBottomCap;
    if (cap->mFiles.size ())
    {
	int count = cap->mFiles.size ();
	cap->mCurrent = (cap->mCurrent + change + count) % count;
	if (top)
	{
	    cap->load (optionGetTopScale (), optionGetTopAspect (),
		       optionGetTopClamp ());
	}
	else
	{
	    cap->load (optionGetBottomScale (), optionGetBottomAspect (),
		       optionGetBottomClamp ());
	    cap->mTexMat.scale (1.0, -1.0, 1.0);
	}
	cScreen->damageScreen ();
    }

    return false;
}

bool
CubeaddonScreen::setOption (const CompString &name, CompOption::Value &value)
{

    unsigned int index;

    bool rv = CubeaddonOptions::setOption (name, value);

    if (!rv || !CompOption::findOption (getOptions (), name, &index))
        return false;

    switch (index) {
	case CubeaddonOptions::TopImages :
	    mTopCap.mFiles   = optionGetTopImages ();
            mTopCap.mCurrent = 0;
            changeCap (true, 0);
	    break;
	case CubeaddonOptions::BottomImages :
	    mBottomCap.mFiles   = optionGetBottomImages ();
            mBottomCap.mCurrent = 0;
            changeCap (false, 0);
	    break;
	case CubeaddonOptions::TopScale :
	case CubeaddonOptions::TopAspect :
	case CubeaddonOptions::TopClamp :
            changeCap (true, 0);
	    break;
	case CubeaddonOptions::BottomScale :
	case CubeaddonOptions::BottomAspect :
	case CubeaddonOptions::BottomClamp :
            changeCap (false, 0);
	    break;
	default:
	    break;
    }

    return rv;
}

void
CubeaddonScreen::drawBasicGround ()
{
    float i;

    glPushMatrix ();

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glLoadIdentity ();
    glTranslatef (0.0, 0.0, -DEFAULT_Z_CAMERA);

    i = optionGetIntensity () * 2;

    glBegin (GL_QUADS);
    glColor4f (0.0, 0.0, 0.0, MAX (0.0, 1.0 - i) );
    glVertex2f (0.5, 0.0);
    glVertex2f (-0.5, 0.0);
    glColor4f (0.0, 0.0, 0.0, MIN (1.0, 1.0 - (i - 1.0) ) );
    glVertex2f (-0.5, -0.5);
    glVertex2f (0.5, -0.5);
    glEnd ();

    if (optionGetGroundSize () > 0.0)
    {
	glBegin (GL_QUADS);
	glColor4usv (optionGetGroundColor1 () );
	glVertex2f (-0.5, -0.5);
	glVertex2f (0.5, -0.5);
	glColor4usv (optionGetGroundColor2 () );
	glVertex2f (0.5, -0.5 + optionGetGroundSize () );
	glVertex2f (-0.5, -0.5 + optionGetGroundSize () );
	glEnd ();
    }

    glColor4usv (defaultColor);

    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable (GL_BLEND);
    glPopMatrix ();
}

bool 
CubeaddonScreen::cubeCheckOrientation (const GLScreenPaintAttrib &sAttrib,
				       const GLMatrix            &transform,
				       CompOutput                *output,
				       std::vector<GLVector>     &points)
{
    bool status;

    status = cubeScreen->cubeCheckOrientation (sAttrib, transform,
					       output, points);

    if (mReflection)
	return !status;

    return status;
}

void 
CubeaddonScreen::cubeGetRotation (float &x, float &v, float &progress)
{

    cubeScreen->cubeGetRotation (x, v, progress);

    if (optionGetMode () == ModeAbove && v > 0.0 && mReflection)
    {
	mVRot = v;
	v = 0.0;
    }
    else
	mVRot = 0.0;
}

void 
CubeaddonScreen::cubeClearTargetOutput (float xRotate, float vRotate)
{

    if (mReflection)
	glCullFace (GL_BACK);

    cubeScreen->cubeClearTargetOutput (xRotate, mBackVRotate);

    if (mReflection)
	glCullFace (GL_FRONT);
}

bool 
CubeaddonScreen::cubeShouldPaintViewport (const GLScreenPaintAttrib &sAttrib,
					  const GLMatrix            &transform,
					  CompOutput                *output,
					  PaintOrder                order)
{
    bool rv = false;

    rv = cubeScreen->cubeShouldPaintViewport (sAttrib, transform,
					      output, order);

    if (rv || cubeScreen->unfolded ())
	return rv;

    if (mDeform > 0.0 && optionGetDeformation () == DeformationCylinder)
    {
	float z[3];
	bool  ftb1, ftb2, ftb3;

	z[0] = cubeScreen->invert () * cubeScreen->distance ();
	z[1] = z[0] + (0.25 / cubeScreen->distance ());
	z[2] = cubeScreen->invert () * 
	       sqrtf (0.25 + (cubeScreen->distance () * cubeScreen->distance ()));

	std::vector<GLVector> vPoints[3];
	
	vPoints[0].push_back (GLVector (-0.5,  0.0, z[0], 1.0));
	vPoints[0].push_back (GLVector ( 0.0,  0.5, z[1], 1.0));
	vPoints[0].push_back (GLVector ( 0.0,  0.0, z[1], 1.0));
	vPoints[1].push_back (GLVector ( 0.5,  0.0, z[0], 1.0));
	vPoints[1].push_back (GLVector ( 0.0, -0.5, z[1], 1.0));
	vPoints[1].push_back (GLVector ( 0.0,  0.0, z[1], 1.0));
	vPoints[2].push_back (GLVector (-0.5,  0.0, z[2], 1.0));
	vPoints[2].push_back (GLVector ( 0.0,  0.5, z[2], 1.0));
	vPoints[2].push_back (GLVector ( 0.0,  0.0, z[2], 1.0));

	ftb1 = cubeScreen->cubeCheckOrientation (sAttrib, transform,
					         output, vPoints[0]);
	ftb2 = cubeScreen->cubeCheckOrientation (sAttrib, transform,
					         output, vPoints[1]);
	ftb3 = cubeScreen->cubeCheckOrientation (sAttrib, transform,
					         output, vPoints[2]);

	rv = (order == FTB && (ftb1 || ftb2 || ftb3)) ||
	     (order == BTF && (!ftb1 || !ftb2 || !ftb3));
    }
    else if (mDeform > 0.0 && optionGetDeformation () == DeformationSphere)
    {
	float z[4];
	bool  ftb1, ftb2, ftb3, ftb4;

	z[0] = sqrtf (0.5 + (cubeScreen->distance () * cubeScreen->distance ()));
	z[1] = z[0] + (0.25 / cubeScreen->distance ());
	z[2] = sqrtf (0.25 + (cubeScreen->distance () * cubeScreen->distance ()));
	z[3] = z[2] + 0.5;
	
	std::vector<GLVector> vPoints[4];
	
	vPoints[0].push_back (GLVector ( 0.0,  0.0, z[3], 1.0));
	vPoints[0].push_back (GLVector (-0.5,  0.5, z[2], 1.0));
	vPoints[0].push_back (GLVector ( 0.0,  0.5, z[2], 1.0));
	vPoints[1].push_back (GLVector ( 0.0,  0.0, z[3], 1.0));
	vPoints[1].push_back (GLVector ( 0.5, -0.5, z[2], 1.0));
	vPoints[1].push_back (GLVector ( 0.0, -0.5, z[2], 1.0));
	vPoints[2].push_back (GLVector ( 0.0,  0.0, z[1], 1.0));
	vPoints[2].push_back (GLVector (-0.5, -0.5, z[0], 1.0));
	vPoints[2].push_back (GLVector (-0.5,  0.0, z[0], 1.0));
	vPoints[3].push_back (GLVector ( 0.0,  0.0, z[1], 1.0));
	vPoints[3].push_back (GLVector ( 0.5,  0.5, z[0], 1.0));
	vPoints[3].push_back (GLVector ( 0.5,  0.0, z[0], 1.0));

	ftb1 = cubeScreen->cubeCheckOrientation (sAttrib, transform,
						 output, vPoints[0]);
	ftb2 = cubeScreen->cubeCheckOrientation (sAttrib, transform,
						 output, vPoints[1]);
	ftb3 = cubeScreen->cubeCheckOrientation (sAttrib, transform,
						 output, vPoints[2]);
	ftb4 = cubeScreen->cubeCheckOrientation (sAttrib, transform,
						 output, vPoints[3]);

	rv = (order == FTB && (ftb1 || ftb2 || ftb3 || ftb4)) ||
	     (order == BTF && (!ftb1 || !ftb2 || !ftb3 || !ftb4));
    }

    return rv;
}

void
CubeaddonScreen::paintCap (const GLScreenPaintAttrib &sAttrib,
			   const GLMatrix            &transform,
			   CompOutput                *output,
			   int                       size,
			   bool                      top,
			   bool                      adjust)
{
    GLScreenPaintAttrib sa;
    GLMatrix            sTransform;
    int                 i, l, opacity;
    int                 cullNorm, cullInv;
    bool                wasCulled = glIsEnabled (GL_CULL_FACE);
    float               cInv = (top) ? 1.0: -1.0;
    CubeCap             *cap;
    bool                cAspect;
    unsigned short	*color;

    glGetIntegerv (GL_CULL_FACE_MODE, &cullNorm);
    cullInv   = (cullNorm == GL_BACK)? GL_FRONT : GL_BACK;
    if (top)
	color = cubeScreen->topColor ();
    else
	color = cubeScreen->bottomColor ();

    opacity = cubeScreen->desktopOpacity () * color[3] / 0xffff;

    glPushMatrix ();
    glEnable (GL_BLEND);

    if (top)
    {
	cap     = &mTopCap;
	cAspect = optionGetTopAspect ();
    }
    else
    {
	cap     = &mBottomCap;
	cAspect = optionGetBottomAspect ();
    }


    glDisableClientState (GL_TEXTURE_COORD_ARRAY);

    if (optionGetDeformation () == DeformationSphere &&
        optionGetDeformCaps ())
	glEnableClientState (GL_NORMAL_ARRAY);

    glVertexPointer (3, GL_FLOAT, 0, mCapFill);

    glEnable(GL_CULL_FACE);

    for (l = 0; l < ((cubeScreen->invert () == 1) ? 2 : 1); l++)
    {
	if (optionGetDeformation () == DeformationSphere &&
	    optionGetDeformCaps ())
	{
	    glNormalPointer (GL_FLOAT, 0, (l == 0) ? mCapFill : mCapFillNorm);
	}
	else
	    glNormal3f (0.0, (l == 0) ? 1.0 : -1.0, 0.0);

	glCullFace(((l == 1) ^ top) ? cullInv : cullNorm);

	for (i = 0; i < size; i++)
	{
	    sa = sAttrib;
	    sTransform = transform;
	    if (cubeScreen->invert () == 1)
	    {
		sa.yRotate += (360.0f / size) * cubeScreen->xRotations ();
		if (!adjust)
		    sa.yRotate -= (360.0f / size) * screen->vp ().x ();
	    }
	    else
	    {
		sa.yRotate += 180.0f;
		sa.yRotate -= (360.0f / size) * cubeScreen->xRotations ();
		if (!adjust)
		    sa.yRotate += (360.0f / size) * screen->vp ().x ();
	    }
	    sa.yRotate += (360.0f / size) * i;

	    gScreen->glApplyTransform (sa, output, &sTransform);

	    glLoadMatrixf (sTransform.getMatrix ());
	    glTranslatef (cubeScreen->outputXOffset (), -cubeScreen->outputYOffset (), 0.0f);
	    glScalef (cubeScreen->outputXScale (), cubeScreen->outputYScale (), 1.0f);

	    glScalef (1.0, cInv, 1.0);

	    glColor4us (color[0] * opacity / 0xffff,
		color[1] * opacity / 0xffff,
		color[2] * opacity / 0xffff,
		opacity);

	    glDrawArrays (GL_TRIANGLE_FAN, 0, CAP_ELEMENTS + 2);
	    if (optionGetDeformation () == DeformationSphere &&
	        optionGetDeformCaps ())
		glDrawElements (GL_QUADS, CAP_NIDX, GL_UNSIGNED_SHORT,
				mCapFillIdx);

	    if (cap->mLoaded)
	    {
		float    s_gen[4], t_gen[4];
		GLMatrix texMat = cap->mTexMat;

		if (cubeScreen->invert () != 1)
		    texMat.scale (-1.0, 1.0, 1.0);

		glColor4us (cubeScreen->desktopOpacity (), cubeScreen->desktopOpacity (),
			    cubeScreen->desktopOpacity (), cubeScreen->desktopOpacity ());
	        cap->mTexture[0]->enable (GLTexture::Good);

		if (cAspect)
		{
		    float scale, xScale = 1.0, yScale = 1.0;
		    scale = (float)output->width () / (float)output->height ();

		    if (output->width () > output->height ())
		    {
			xScale = 1.0;
			yScale = 1.0 / scale;
		    }
		    else
		    {
			xScale = scale;
			yScale = 1.0;
		    }

		    if (optionGetTopScale ())
		    {
			scale = xScale;
			xScale = 1.0 / yScale;
			yScale = 1.0 / scale;
		    }

		    texMat.scale (xScale, yScale, 1.0);
		}
		
		texMat.rotate (-(360.0f / size) * i, 0.0, 0.0, 1.0);

		s_gen[0] = texMat[0];
		s_gen[1] = texMat[8];
		s_gen[2] = texMat[4];
		s_gen[3] = texMat[12];
		t_gen[0] = texMat[1];
		t_gen[1] = texMat[9];
		t_gen[2] = texMat[5];
		t_gen[3] = texMat[13];

		glTexGenfv(GL_T, GL_OBJECT_PLANE, t_gen);
		glTexGenfv(GL_S, GL_OBJECT_PLANE, s_gen);

		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);

		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);

		glDrawArrays (GL_TRIANGLE_FAN, 0, CAP_ELEMENTS + 2);
		if (optionGetDeformation () == DeformationSphere &&
	            optionGetDeformCaps ())
		    glDrawElements (GL_QUADS, CAP_NIDX, GL_UNSIGNED_SHORT,
				    mCapFillIdx);

		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		cap->mTexture[0]->disable ();
	    }
	}
    }

    glEnableClientState (GL_TEXTURE_COORD_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);
    glDisable (GL_BLEND);
    glNormal3f (0.0, -1.0, 0.0);

    glCullFace (cullNorm);
    if (!wasCulled)
	glDisable (GL_CULL_FACE);

    glPopMatrix ();

    glColor4usv (defaultColor);
}

void 
CubeaddonScreen::cubePaintTop (const GLScreenPaintAttrib &sAttrib,
			       const GLMatrix            &transform,
			       CompOutput                *output,
			       int                       size)
{
    if ((!optionGetDrawBottom () && cubeScreen->invert () == -1) ||
        (!optionGetDrawTop () && cubeScreen->invert () == 1))
    {
	cubeScreen->cubePaintTop (sAttrib, transform, output, size);
    }

    if (!optionGetDrawTop ())
        return;

    paintCap (sAttrib, transform, output, size,
	      true, optionGetAdjustTop ());
}

void 
CubeaddonScreen::cubePaintBottom (const GLScreenPaintAttrib &sAttrib,
				  const GLMatrix            &transform,
				  CompOutput                *output,
				  int                       size)
{
    if ((!optionGetDrawBottom () && cubeScreen->invert () == 1) ||
        (!optionGetDrawTop () && cubeScreen->invert () == -1))
    {
	cubeScreen->cubePaintBottom (sAttrib, transform, output, size);
    }

    if (!optionGetDrawBottom ())
        return;

    paintCap (sAttrib, transform, output, size,
	      false, optionGetAdjustBottom ());
}

void 
CubeaddonWindow::glAddGeometry (const GLTexture::MatrixList &matrix,
				const CompRegion            &region,
				const CompRegion            &clip,
				unsigned int                maxGridWidth,
				unsigned int                maxGridHeight)
{
    if (caScreen->mDeform > 0.0)
    {
	GLWindow::Geometry &geometry = gWindow->geometry ();
	int                i, oldVCount = geometry.vCount;
	GLfloat            *v;
	int                offX = 0, offY = 0;
	int                sx1, sx2, sw, sy1, sy2, sh;
	float              radSquare, last[2][4];
	float              inv = (cubeScreen->invert () == 1) ? 1.0 : -1.0;

	float              ang, sx1g, sx2g, sy1g, sy2g;
	
	CubeScreen::MultioutputMode   cMOM = cubeScreen->multioutputMode ();
	int                           caD = caScreen->optionGetDeformation ();
	float                         cDist = cubeScreen->distance ();
	

	if (caD == CubeaddonScreen::DeformationCylinder || cubeScreen->unfolded ())
	{
	    radSquare = (cDist * cDist) + 0.25;
	}
	else
	{
	    maxGridHeight = MIN (CUBEADDON_GRID_SIZE, maxGridHeight);
	    radSquare = (cDist * cDist) + 0.5;
	}

	gWindow->glAddGeometry (matrix, region, clip, 
				MIN (CUBEADDON_GRID_SIZE, maxGridWidth),
				maxGridHeight);
	
	v  = geometry.vertices;
	v += geometry.vertexStride - 3;
	v += geometry.vertexStride * oldVCount;

	if (!window->onAllViewports ())
	{
	    CompPoint offset = caScreen->cScreen->windowPaintOffset ();
	    offset = window->getMovementForOffset (offset);
	    offX = offset.x ();
	    offY = offset.y ();
	}
	
	if (cMOM == CubeScreen::OneBigCube)
	{
	    sx1 = 0;
	    sx2 = screen->width ();
	    sw  = screen->width ();
	    sy1 = 0;
	    sy2 = screen->height ();
	    sh  = screen->height ();
	}
	else if (cMOM == CubeScreen::MultipleCubes)
	{
	    sx1 = caScreen->mLast->x1 ();
	    sx2 = caScreen->mLast->x2 ();
	    sw  = sx2 - sx1;
	    sy1 = caScreen->mLast->y1 ();
	    sy2 = caScreen->mLast->y2 ();
	    sh  = sy2 - sy1;
	}
	else
	{
	    if (cubeScreen->nOutput () != (int) screen->outputDevs ().size ())
	    {
		sx1 = 0;
		sx2 = screen->width ();
		sw  = screen->width ();
		sy1 = 0;
		sy2 = screen->height ();
		sh  = screen->height ();
	    }
	    else
	    {
		sx1 = screen->outputDevs ()[cubeScreen->sourceOutput ()].x1 ();
		sx2 = screen->outputDevs ()[cubeScreen->sourceOutput ()].x2 ();
		sw  = sx2 - sx1;
		sy1 = screen->outputDevs ()[cubeScreen->sourceOutput ()].y1 ();
		sy2 = screen->outputDevs ()[cubeScreen->sourceOutput ()].y2 ();
		sh  = sy2 - sy1;
	    }
	}
	
	sx1g = sx1 - CUBEADDON_GRID_SIZE;
	sx2g = sx2 + CUBEADDON_GRID_SIZE;
	sy1g = sy1 - CUBEADDON_GRID_SIZE;
	sy2g = sy2 + CUBEADDON_GRID_SIZE;

	if (caD == CubeaddonScreen::DeformationCylinder || cubeScreen->unfolded ())
	{
	    float lastX = std::numeric_limits <float>::min (), lastZ = 0.0;
	
	    for (i = oldVCount; i < geometry.vCount; i++)
	    {
		if (v[0] == lastX)
		{
		    v[2] = lastZ;
		}
		else if (v[0] + offX >= sx1g &&
			 v[0] + offY < sx2g)
		{
		    ang = (((v[0] + offX - sx1) / (float)sw) - 0.5);
		    ang *= ang;
		    if (ang < radSquare)
		    {
			v[2] = sqrtf (radSquare - ang) - cDist;
			v[2] *= caScreen->mDeform * inv;
		    }
		}

		lastX = v[0];
		lastZ = v[2];

		v += geometry.vertexStride;
	    }
	}
	else
	{

	    last[0][0] = -1000000000.0;
	    last[1][0] = -1000000000.0;

	    int cLast = 0;
	    for (i = oldVCount; i < geometry.vCount; i++)
	    {
		if (last[0][0] == v[0] && last[0][1] == v[1])
		{
		    v[0] = last[0][2];
		    v[2] = last[0][3];
		    v += geometry.vertexStride;
		    continue;
		}
		else if (last[1][0] == v[0] && last[1][1] == v[1])
		{
		    v[0] = last[1][2];
		    v[2] = last[1][3];
		    v += geometry.vertexStride;
		    continue;
		}
		
		float vpx = v[0] + offX;
		float vpy = v[1] + offY;
		
		if (vpx >= sx1g && vpx < sx2g &&
		    vpy >= sy1g && vpy < sy2g)
		{
		    last[cLast][0] = v[0];
		    last[cLast][1] = v[1];
		    float a1 = (((vpx - sx1) / (float)sw) - 0.5);
		    float a2 = (((vpy - sy1) / (float)sh) - 0.5);
		    a2 *= a2;

		    ang = atanf (a1 / cDist);
		    a2 = sqrtf (radSquare - a2);
		    int iang = (((int)(ang * RAD2I1024)) + 1024) & 0x3ff;

		    v[2] += ((caScreen->mCosT [iang] * a2) - cDist) * caScreen->mDeform * inv;
		    v[0] += ((caScreen->mSinT [iang] * a2) - a1) * sw * caScreen->mDeform;
		    last[cLast][2] = v[0];
		    last[cLast][3] = v[2];
		    cLast = (cLast + 1) & 1;
		}
		v += geometry.vertexStride;
	    }
	}
    }
    else
    {
	gWindow->glAddGeometry (matrix, region, clip, maxGridWidth, maxGridHeight);
    }
}

bool 
CubeaddonWindow::glDraw (const GLMatrix     &transform,
			 GLFragment::Attrib &attrib,
			 const CompRegion   &region,
			 unsigned int       mask)
{
    if (!(mask & PAINT_WINDOW_TRANSFORMED_MASK) && caScreen->mDeform)
    {
	CompPoint offset;
	int x1, x2;

	if (!window->onAllViewports ())
	{
	    offset = caScreen->cScreen->windowPaintOffset ();
	    offset = window->getMovementForOffset (offset);
	}
	
	x1 = window->x () - window->output ().left + offset.x ();
	x2 = window->x () + window->width () + window->output ().right + offset.x ();
	if (x1 < 0 && x2 < 0)
	    return false;
	if (x1 > screen->width () && x2 > screen->width ())
	    return false;
    }

    return gWindow->glDraw (transform, attrib, region, mask);
}

void 
CubeaddonWindow::glDrawTexture (GLTexture           *texture,
				GLFragment::Attrib& attrib,
				unsigned int        mask)
{
    if (caScreen->mDeform > 0.0 && caScreen->gScreen->lighting ())
    {
	int       i;
	int       sx1, sx2, sw, sy1, sy2, sh;
	int       offX = 0, offY = 0;
	float     x, y, ym;
	GLfloat   *v, *n;
	float     inv;
	
	GLWindow::Geometry           &geometry = gWindow->geometry ();
	CubeScreen::MultioutputMode  cMOM = cubeScreen->multioutputMode ();
	float                        cDist = cubeScreen->distance ();

	inv = (cubeScreen->invert () == 1) ? 1.0: -1.0;
	ym  = (caScreen->optionGetDeformation () == CubeaddonScreen::DeformationCylinder) ? 0.0 : 1.0;
	
	if ((int) caScreen->mWinNormSize < geometry.vCount * 3)
	{
	    delete [] caScreen->mWinNormals;
	    caScreen->mWinNormals = new GLfloat[geometry.vCount * 3];
	    caScreen->mWinNormSize = geometry.vCount * 3;
	}
	
	if (!window->onAllViewports ())
	{
	    CompPoint offset = caScreen->cScreen->windowPaintOffset ();
	    offset = window->getMovementForOffset (offset);
	    offX = offset.x ();
	    offY = offset.y ();
	}
	
	if (cMOM == CubeScreen::OneBigCube)
	{
	    sx1 = 0;
	    sx2 = screen->width ();
	    sw  = screen->width ();
	    sy1 = 0;
	    sy2 = screen->height ();
	    sh  = screen->height ();
	}
	else if (cMOM == CubeScreen::MultipleCubes)
	{
	    sx1 = caScreen->mLast->x1 ();
	    sx2 = caScreen->mLast->x2 ();
	    sw  = sx2 - sx1;
	    sy1 = caScreen->mLast->y1 ();
	    sy2 = caScreen->mLast->y2 ();
	    sh  = sy2 - sy1;
	}
	else
	{
	    if (cubeScreen->nOutput () != (int) screen->outputDevs ().size ())
	    {
		sx1 = 0;
		sx2 = screen->width ();
		sw  = screen->width ();
		sy1 = 0;
		sy2 = screen->height ();
		sh  = screen->height ();
	    }
	    else
	    {
		sx1 = screen->outputDevs ()[cubeScreen->sourceOutput ()].x1 ();
		sx2 = screen->outputDevs ()[cubeScreen->sourceOutput ()].x2 ();
		sw  = sx2 - sx1;
		sy1 = screen->outputDevs ()[cubeScreen->sourceOutput ()].y1 ();
		sy2 = screen->outputDevs ()[cubeScreen->sourceOutput ()].y2 ();
		sh  = sy2 - sy1;
	    }
	}
	
	v = geometry.vertices + (geometry.vertexStride - 3);
	n = caScreen->mWinNormals;

	if (cubeScreen->paintOrder () == FTB)
	{
	    for (i = 0; i < geometry.vCount; i++)
	    {
		x = (((v[0] + offX - sx1) / (float)sw) - 0.5);
		y = (((v[1] + offY - sy1) / (float)sh) - 0.5);

		*(n)++ = x / sw * caScreen->mDeform;
		*(n)++ = y / sh * caScreen->mDeform * ym;
		*(n)++ = v[2] + cDist;

		v += geometry.vertexStride;
	    }
	}
	else
	{
	    for (i = 0; i < geometry.vCount; i++)
	    {
		x = (((v[0] + offX - sx1) / (float)sw) - 0.5);
		y = (((v[1] + offY - sy1) / (float)sh) - 0.5);

		*(n)++ = -x / sw * caScreen->mDeform * inv;
		*(n)++ = -y / sh * caScreen->mDeform * ym * inv;
		*(n)++ = -(v[2] + cDist);
    
		v += geometry.vertexStride;
	    }
	}
	
	glEnable (GL_NORMALIZE);
	glNormalPointer (GL_FLOAT,0, caScreen->mWinNormals);
	
	glEnableClientState (GL_NORMAL_ARRAY);
	
	gWindow->glDrawTexture (texture, attrib, mask);

	glDisable (GL_NORMALIZE);
	glDisableClientState (GL_NORMAL_ARRAY);
	glNormal3f (0.0, 0.0, -1.0);
	return;
    }

    gWindow->glDrawTexture (texture, attrib, mask);
}

bool
CubeaddonScreen::cubeShouldPaintAllViewports ()
{
    bool status = cubeScreen->cubeShouldPaintAllViewports ();
    return (!optionGetDrawTop () ||
	    !optionGetDrawBottom () ||
	    (mDeform > 0.0) ||
	    status);
}

void 
CubeaddonScreen::glPaintTransformedOutput (const GLScreenPaintAttrib &sAttrib,
					   const GLMatrix            &transform,
					   const CompRegion          &region,
					   CompOutput                *output,
					   unsigned int              mask)
{
    static GLfloat light0Position[] = { -0.5f, 0.5f, -9.0f, 1.0f };
    GLMatrix       sTransform = transform;
    float          cDist = cubeScreen->distance ();
    float          cDist2 = cubeScreen->distance () * cubeScreen->distance ();

    if (optionGetDeformation () != DeformationNone && 
	screen->vpSize ().width () * cubeScreen->nOutput () > 2 && 
	screen->desktopWindowCount () &&
	(cubeScreen->rotationState () == CubeScreen::RotationManual ||
	(cubeScreen->rotationState () == CubeScreen::RotationChange &&
	!optionGetCylinderManualOnly ()) || mWasDeformed) &&
        (!cubeScreen->unfolded () || optionGetUnfoldDeformation ()))
    {
	float x, progress;
	
	cubeScreen->cubeGetRotation (x, x, progress);
	mDeform = progress;

	if (optionGetSphereAspect () > 0.0 && cubeScreen->invert () == 1 &&
	    optionGetDeformation () == DeformationSphere)
	{
	    float scale, val = optionGetSphereAspect () * mDeform;

	    if (output->width () > output->height ())
	    {
		scale = (float)output->height () / (float)output->width ();
		scale = (scale * val) + 1.0 - val;
		sTransform.scale (scale, 1.0, 1.0);
	    }
	    else
	    {
		scale = (float)output->width () / (float)output->height ();
		scale = (scale * val) + 1.0 - val;
		sTransform.scale (1.0, scale, 1.0);
	    }
	}
    }
    else
    {
	mDeform = 0.0;
    }

    cubeScreen->cubeShouldPaintAllViewportsSetEnabled (this, true);

    if (mCapDistance != cDist)
    {
	changeCap (true, 0);
	changeCap (false, 0);
    }

    if (mDeform != mCapDeform || mCapDistance != cDist ||
        mCapDeformType != optionGetDeformation ())
    {
	float       *quad;
	int         j;
	float       rS, r, x, y, z;
	if (optionGetDeformation () != DeformationSphere ||
	    !optionGetDeformCaps ())
	{
	    rS = cDist2 + 0.5;

	    mCapFill[0] = 0.0;
	    mCapFill[1] = 0.5;
	    mCapFill[2] = 0.0;
	    mCapFillNorm[0] = 0.0;
	    mCapFillNorm[1] = -1.0;
	    mCapFillNorm[2] = 0.0;

	    z = cDist;
	    r = 0.25 + cDist2;

	    for (j = 0; j <= CAP_ELEMENTS; j++)
	    {
		x = -0.5 + ((float)j / (float)CAP_ELEMENTS);
		z = ((sqrtf(r - (x * x)) - cDist) * mDeform) + cDist;
		y = 0.5;

		quad = &mCapFill[(1 + j) * 3];

		quad[0] = x;
		quad[1] = y;
		quad[2] = z;

		quad = &mCapFillNorm[(1 + j) * 3];

		quad[0] = -x;
		quad[1] = -y;
		quad[2] = -z;
	    }
	}
	else
	{
	    int i;
	    float w;
	    rS = cDist2 + 0.5;

	    mCapFill[0] = 0.0;
	    mCapFill[1] = ((sqrtf (rS) - 0.5) * mDeform) + 0.5;
	    mCapFill[2] = 0.0;
	    mCapFillNorm[0] = 0.0;
	    mCapFillNorm[1] = -1.0;
	    mCapFillNorm[2] = 0.0;

	    for (i = 0; i < CAP_ELEMENTS; i++)
	    {
		w = (float)(i + 1) / (float)CAP_ELEMENTS;

		r = (((w / 2.0) * (w / 2.0)) + (cDist2 * w * w));

		for (j = 0; j <= CAP_ELEMENTS; j++)
		{
		    x = - (w / 2.0) + ((float)j * w / (float)CAP_ELEMENTS);
		    z = ((sqrtf(r - (x * x)) - (cDist * w)) * mDeform) + (cDist * w);
		    y = ((sqrtf(rS - (x * x) - (r - (x * x))) - 0.5) * mDeform) + 0.5;

		    quad = &mCapFill[(1 + (i * (CAP_ELEMENTS + 1)) + j) * 3];

		    quad[0] = x;
		    quad[1] = y;
		    quad[2] = z;

		    quad = &mCapFillNorm[(1 + (i * (CAP_ELEMENTS + 1)) + j) * 3];

		    quad[0] = -x;
		    quad[1] = -y;
		    quad[2] = -z;
		}
	    }
	}

	mCapDeform     = mDeform;
	mCapDistance   = cDist;
	mCapDeformType = optionGetDeformation ();
    }

    if (cubeScreen->invert () == 1 && mFirst && optionGetReflection ())
    {
	mFirst = false;
	mReflection = true;

	if (screen->grabExist ("cube"))
	{
	    GLMatrix rTransform = sTransform;

	    rTransform.translate (0.0, -1.0, 0.0);
	    rTransform.scale (1.0, -1.0, 1.0);
	    glCullFace (GL_FRONT);

	    gScreen->glPaintTransformedOutput (sAttrib, rTransform,
					       region, output, mask);

	    glCullFace (GL_BACK);
	    drawBasicGround ();
	}
	else
	{
	    GLMatrix      rTransform = sTransform;
	    GLMatrix      pTransform;
	    float         angle = 360.0 / ((float) screen->vpSize ().width () * 
				  cubeScreen->nOutput ());
	    float         xRot, vRot, xRotate, xRotate2, vRotate, p;
	    float         rYTrans;
	    GLVector      point (-0.5, -0.5, cDist, 1.0);
	    GLVector      point2 (-0.5,  0.5, cDist, 1.0);
	    float         deform = 0.0f;

	    cubeScreen->cubeGetRotation (xRot, vRot, p);

	    mBackVRotate = 0.0;

	    xRotate  = xRot;
	    xRotate2 = xRot;
	    vRotate  = vRot;

	    if (vRotate < 0.0)
		xRotate += 180;

	    vRotate = fmod (fabs (vRotate), 180.0);
	    xRotate = fmod (fabs (xRotate), angle);
	    xRotate2 = fmod (fabs (xRotate2), angle);

	    if (vRotate >= 90.0)
		vRotate = 180.0 - vRotate;

	    if (xRotate >= angle / 2.0)
		xRotate = angle - xRotate;

	    if (xRotate2 >= angle / 2.0)
		xRotate2 = angle - xRotate2;

	    xRotate = (mDeform * angle * 0.5) +
		      ((1.0 - mDeform) * xRotate);
	    xRotate2 = (mDeform * angle * 0.5) +
		       ((1.0 - mDeform) * xRotate2);

	    pTransform.reset ();
	    pTransform.rotate (xRotate, 0.0f, 1.0f, 0.0f);
	    pTransform.rotate (vRotate, cosf (xRotate * (M_PI / 180.0f)),
			       0.0f, sinf (xRotate * (M_PI / 180.0f)));

	    point = pTransform * point;

	    pTransform.reset ();
	    pTransform.rotate (xRotate2, 0.0f, 1.0f, 0.0f);
	    pTransform.rotate (vRotate, cosf (xRotate2 * (M_PI / 180.0f)),
			       0.0f, sinf (xRotate2 * (M_PI / 180.0f)));

	    point2 = pTransform * point2;

	    switch (optionGetMode ()) {
		case ModeJumpyReflection:
		    mYTrans    = 0.0;
		    if (optionGetDeformation () == DeformationSphere &&
			optionGetDeformCaps () && optionGetDrawBottom ())
		    {
			rYTrans = sqrt (0.5 + cDist2) * -2.0;
		    }
		    else
		    {
			rYTrans = point[1] * 2.0;
		    }
		    break;
		case ModeDistance:
		    mYTrans = 0.0;
		    rYTrans = sqrt (0.5 + cDist2) * -2.0;
		    break;
		default:

		    if (optionGetDeformation () == DeformationSphere &&
			optionGetDeformCaps () && optionGetDrawBottom ())
		    {
			mYTrans =  mCapFill[1] - 0.5;
			rYTrans = -mCapFill[1] - 0.5;
		    }
		    else if (optionGetDeformation () == DeformationSphere &&
			     vRotate > atan (cDist * 2) / (M_PI / 180.0f))
		    {
			mYTrans = sqrt (0.5 + cDist2) - 0.5;
			rYTrans = -sqrt (0.5 + cDist2) - 0.5;
		    }
		    else
		    {
			mYTrans = -point[1] - 0.5;
			rYTrans =  point[1] - 0.5;
		    }
		    break;
	    }


	    if (!optionGetAutoZoom () ||
		((cubeScreen->rotationState () != CubeScreen::RotationManual) &&
		 optionGetZoomManualOnly ()))
	    {
		mZTrans = 0.0;
	    }
	    else
		mZTrans = -point2[2] + cDist;

	    if (optionGetMode () == ModeAbove)
		mZTrans = 0.0;

	    if (optionGetDeformation () == DeformationCylinder) 
		deform = (sqrt (0.25 + cDist2) - cDist) * -mDeform;
	    else if (optionGetDeformation () == DeformationSphere) 
		deform = (sqrt (0.5 + cDist2) - cDist) * -mDeform;

	    if (mDeform > 0.0)
	        mZTrans = deform;

	    if (optionGetMode () == ModeAbove && mVRot > 0.0)
	    {
		mBackVRotate = mVRot;
		if (optionGetDeformation () == DeformationSphere &&
		    optionGetDeformCaps () && optionGetDrawBottom ())
		{
		    mYTrans =  mCapFill[1] - 0.5;
		    rYTrans = -mCapFill[1] - 0.5;
		}
		else
		{
		    mYTrans = 0.0;
		    rYTrans = -1.0;
		}

		pTransform.reset ();
		
		gScreen->glApplyTransform (sAttrib, output, &pTransform);
		point = GLVector (0.0, 0.0, -cDist + deform, 1.0);
		point = pTransform * point;
		
		rTransform.translate (0.0, 0.0, point[2]);
		rTransform.rotate (mVRot, 1.0, 0.0, 0.0);
		rTransform.scale (1.0, -1.0, 1.0);
		rTransform.translate (0.0, -rYTrans, -point[2] + mZTrans);
	    }
	    else
	    {
		rTransform.translate (0.0, rYTrans, mZTrans);
		rTransform.scale (1.0, -1.0, 1.0);
	    }

	    glPushMatrix ();
	    glLoadIdentity ();
	    glScalef (1.0, -1.0, 1.0);
	    glLightfv (GL_LIGHT0, GL_POSITION, light0Position);
	    glPopMatrix ();
	    glCullFace (GL_FRONT);

	    gScreen->glPaintTransformedOutput (sAttrib, rTransform,
					       region, output, mask);

	    glCullFace (GL_BACK);
	    glPushMatrix ();
	    glLoadIdentity ();
	    glLightfv (GL_LIGHT0, GL_POSITION, light0Position);
	    glPopMatrix ();

	    if (optionGetMode () == ModeAbove && mVRot > 0.0)
	    {
		int   j;
		float i, c;
		float v = MIN (1.0, mVRot / 30.0);
		float col1[4], col2[4];

		glPushMatrix ();

		glEnable (GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glLoadIdentity ();
		glTranslatef (0.0, 0.0, -DEFAULT_Z_CAMERA);

		i = optionGetIntensity () * 2;
		c = optionGetIntensity ();

		glBegin (GL_QUADS);
		glColor4f (0.0, 0.0, 0.0,
			   ((1 - v) * MAX (0.0, 1.0 - i)) + (v * c));
		glVertex2f (0.5, v / 2.0);
		glVertex2f (-0.5, v / 2.0);
		glColor4f (0.0, 0.0, 0.0,
			   ((1 - v) * MIN (1.0, 1.0 - (i - 1.0))) + (v * c));
		glVertex2f (-0.5, -0.5);
		glVertex2f (0.5, -0.5);
		glEnd ();

		for (j = 0; j < 4; j++)
		{
		    col1[j] = (1.0 - v) * optionGetGroundColor1 () [j] +
			      (v * (optionGetGroundColor1 () [j] +
				    optionGetGroundColor2 () [j]) * 0.5);
		    col1[j] /= 0xffff;
		    col2[j] = (1.0 - v) * optionGetGroundColor2 () [j] +
			      (v * (optionGetGroundColor1 () [j] +
				    optionGetGroundColor2 () [j]) * 0.5);
		    col2[j] /= 0xffff;
		}

		if (optionGetGroundSize () > 0.0)
		{
		    glBegin (GL_QUADS);
		    glColor4fv (col1);
		    glVertex2f (-0.5, -0.5);
		    glVertex2f (0.5, -0.5);
		    glColor4fv (col2);
		    glVertex2f (0.5, -0.5 +
				((1 - v) * optionGetGroundSize ()) + v);
		    glVertex2f (-0.5, -0.5 +
				((1 - v) * optionGetGroundSize ()) + v);
		    glEnd ();
		}

		glColor4usv (defaultColor);

		glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glDisable (GL_BLEND);
		glPopMatrix ();
	    }
	    else
		drawBasicGround ();
	}
	
	cubeScreen->repaintCaps ();
	mReflection = false;
    }
    else
	cubeScreen->cubeShouldPaintAllViewportsSetEnabled (this, false);

    if (!optionGetReflection ())
    {
	mYTrans = 0.0;
	mZTrans = (sqrt (0.25 + cDist2) - cDist) * -mDeform;
    }

    sTransform.translate (0.0, mYTrans, mZTrans);

    gScreen->glPaintTransformedOutput (sAttrib, sTransform,
				       region, output, mask);
}

bool 
CubeaddonScreen::glPaintOutput (const GLScreenPaintAttrib &sAttrib,
				const GLMatrix            &transform,
				const CompRegion          &region,
				CompOutput                *output,
				unsigned int              mask)
{

    if (mLast != output)
	mFirst = true;

    mLast = output;

    return gScreen->glPaintOutput (sAttrib, transform, region, output, mask);
}

void 
CubeaddonScreen::donePaint ()
{
    mFirst  = true;
    mYTrans = 0.0;
    mZTrans = 0.0;

    mWasDeformed = (mDeform > 0.0);

    if (mDeform > 0.0 && mDeform < 1.0)
    {
	cScreen->damageScreen ();
	mDeform = 0.0;
    }

    cScreen->donePaint ();
}


CubeaddonScreen::CubeaddonScreen (CompScreen *s) :
    PluginClassHandler<CubeaddonScreen, CompScreen> (s),
    CubeaddonOptions (),
    cScreen (CompositeScreen::get (s)),
    gScreen (GLScreen::get (s)),
    cubeScreen (CubeScreen::get (s)),
    mReflection (false),
    mFirst (true),
    mLast (0),
    mYTrans (0.0),
    mZTrans (0.0),
    mDeform (0.0),
    mWinNormals (0),
    mWinNormSize (0),
    mCapDeform (-1.0),
    mCapDistance (cubeScreen->distance ())
{
    GLushort *idx;

    idx = mCapFillIdx;
    for (int i = 0; i < CAP_ELEMENTS - 1; i++)
    {
	for (int j = 0; j < CAP_ELEMENTS; j++)
	{
	    idx[0] = 1 + (i * (CAP_ELEMENTS + 1)) + j;
	    idx[1] = 1 + ((i + 1) * (CAP_ELEMENTS + 1)) + j;
	    idx[2] = 2 + ((i + 1) * (CAP_ELEMENTS + 1)) + j;
	    idx[3] = 2 + (i * (CAP_ELEMENTS + 1)) + j;
	    idx += 4;
	}
    }

    mTopCap.mFiles = optionGetTopImages ();
    mBottomCap.mFiles = optionGetBottomImages ();
    
    for (int i = 0; i < 1024; i++)
    {
	mSinT[i] = sinf(i / RAD2I1024);
	mCosT[i] = cosf(i / RAD2I1024);
    }

    changeCap (true, 0);
    changeCap (false, 0);

#define BIND_ACTION(opt, t, d) \
    optionSet##opt##Initiate (boost::bind (&CubeaddonScreen::changeCap, this, t, d))

    BIND_ACTION (TopNextKey, true, 1);
    BIND_ACTION (TopPrevKey, true, -1);
    BIND_ACTION (BottomNextKey, false, 1);
    BIND_ACTION (BottomNextKey, false, -1);

    BIND_ACTION (TopNextButton, true, 1);
    BIND_ACTION (TopPrevButton, true, -1);
    BIND_ACTION (BottomNextButton, false, 1);
    BIND_ACTION (BottomNextButton, false, -1);

#undef BIND_ACTION

    CompositeScreenInterface::setHandler (cScreen, true);
    GLScreenInterface::setHandler (gScreen, true);
    CubeScreenInterface::setHandler (cubeScreen, true);
}

CubeaddonScreen::~CubeaddonScreen ()
{
    if (mWinNormals)
	delete [] mWinNormals;
}

CubeaddonWindow::CubeaddonWindow (CompWindow *w) :
    PluginClassHandler<CubeaddonWindow, CompWindow> (w),
    window (w),
    gWindow (GLWindow::get (w)),
    caScreen (CubeaddonScreen::get (screen)),
    cubeScreen (CubeScreen::get (screen))
{
    GLWindowInterface::setHandler (gWindow, true);
}

bool
CubeaddonPluginVTable::init ()
{
    if (!CompPlugin::checkPluginABI ("core", CORE_ABIVERSION) ||
	!CompPlugin::checkPluginABI ("composite", COMPIZ_COMPOSITE_ABI) ||
	!CompPlugin::checkPluginABI ("opengl", COMPIZ_OPENGL_ABI) ||
	!CompPlugin::checkPluginABI ("cube", COMPIZ_CUBE_ABI))
	return false;

    return true;
}

